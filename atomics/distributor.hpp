#ifndef DISTRIBUTOR_HPP
#define DISTRIBUTOR_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <vector>
#include <limits>

#include "customer_data.hpp"

using namespace cadmium;

// ---- FIXED LANE COUNTS ----
static constexpr int CASH_LANES = 3;
static constexpr int SELF_LANES = 2;
static constexpr int TOTAL_LANES = CASH_LANES + SELF_LANES;

// ---- RULE: items <= SELF_ITEM_LIMIT go to self-checkout if possible ----
static constexpr int SELF_ITEM_LIMIT = 15;

// ---- Max queue per lane (adjust if your spec says otherwise) ----
static constexpr int MAX_QUEUE = 2;

struct DistributorState {
    enum class Phase { IDLE, SEND } phase;

    std::vector<int> queues;   // size TOTAL_LANES

    // what we will emit on output()
    bool emitHold = false;
    bool emitOk   = false;

    int  chosenLane = -1;      // for debugging/logging (optional)
    CustomerData pendingCust;
    bool hasPendingCust = false;

    DistributorState()
        : phase(Phase::IDLE),
          queues(TOTAL_LANES, 0) {}
};

inline std::ostream& operator<<(std::ostream& os, const DistributorState& s) {
    os << "phase=" << (s.phase == DistributorState::Phase::IDLE ? "IDLE" : "SEND")
       << " chosenLane=" << s.chosenLane
       << " hold=" << s.emitHold
       << " ok=" << s.emitOk
       << " q=[";
    for (size_t i=0;i<s.queues.size();i++) os << s.queues[i] << (i+1<s.queues.size()?",":"");
    os << "]";
    return os;
}

class Distributor : public Atomic<DistributorState> {
public:
    // Inputs
    Port<CustomerData> in_customer;
    Port<int>          in_laneFreed;  // laneId that completed service

    // Outputs
    // lane ports (CustomerData goes straight to chosen lane)
    Port<CustomerData> out_cash0;
    Port<CustomerData> out_cash1;
    Port<CustomerData> out_cash2;

    Port<CustomerData> out_self0;
    Port<CustomerData> out_self1;

    // control feedback to Generator
    Port<bool> out_holdOff;
    Port<bool> out_okGo;

    // optional debug output (not required)
    Port<int> out_whichLane;

    Distributor(const std::string& id)
        : Atomic<DistributorState>(id, DistributorState())
    {
        in_customer  = addInPort<CustomerData>("in_customer");
        in_laneFreed = addInPort<int>("in_laneFreed");

        out_cash0 = addOutPort<CustomerData>("out_cash0");
        out_cash1 = addOutPort<CustomerData>("out_cash1");
        out_cash2 = addOutPort<CustomerData>("out_cash2");

        out_self0 = addOutPort<CustomerData>("out_self0");
        out_self1 = addOutPort<CustomerData>("out_self1");

        out_holdOff = addOutPort<bool>("out_holdOff");
        out_okGo     = addOutPort<bool>("out_okGo");

        out_whichLane = addOutPort<int>("out_whichLane");
    }

    void internalTransition(DistributorState& s) const override {
        // After SEND, return to IDLE and clear pending signals
        s.phase = DistributorState::Phase::IDLE;
        s.emitHold = false;
        s.emitOk = false;
        s.chosenLane = -1;
        s.hasPendingCust = false;
        s.pendingCust = CustomerData();
    }

    void externalTransition(DistributorState& s, double /*e*/) const override {
        // Apply lane freed events (can be multiple)
        if (!in_laneFreed->empty()) {
            for (int laneId : in_laneFreed->getBag()) {
                if (0 <= laneId && laneId < TOTAL_LANES && s.queues[laneId] > 0) {
                    s.queues[laneId]--;
                }
            }
            // spec-style "OK" pulse
            s.emitOk = true;
            s.phase = DistributorState::Phase::SEND;
        }

        // Route arriving customers
        if (!in_customer->empty()) {
            for (const CustomerData& cust : in_customer->getBag()) {
                int lane = chooseLane(s, cust);
                if (lane >= 0) {
                    s.queues[lane]++;
                    s.pendingCust = cust;
                    s.hasPendingCust = true;
                    s.chosenLane = lane;
                } else {
                    // nowhere to send
                    s.emitHold = true;
                }
                s.phase = DistributorState::Phase::SEND;
            }
        }
    }

    void output(const DistributorState& s) const override {
        if (s.phase != DistributorState::Phase::SEND) return;

        // control signals
        if (s.emitHold) out_holdOff->addMessage(true);
        if (s.emitOk)   out_okGo->addMessage(true);

        // debug lane id
        if (s.hasPendingCust && s.chosenLane >= 0) {
            out_whichLane->addMessage(s.chosenLane);

            // emit customer to the chosen lane port
            if (s.chosenLane == 0) out_cash0->addMessage(s.pendingCust);
            else if (s.chosenLane == 1) out_cash1->addMessage(s.pendingCust);
            else if (s.chosenLane == 2) out_cash2->addMessage(s.pendingCust);
            else if (s.chosenLane == 3) out_self0->addMessage(s.pendingCust);
            else if (s.chosenLane == 4) out_self1->addMessage(s.pendingCust);
        }
    }

    double timeAdvance(const DistributorState& s) const override {
        if (s.phase == DistributorState::Phase::IDLE)
            return std::numeric_limits<double>::infinity();
        return 0.0;
    }

private:
    // Prefer self-checkout for <= SELF_ITEM_LIMIT, else staffed cash.
    // Within the chosen group: pick the smallest queue with available space.
    int chooseLane(const DistributorState& s, const CustomerData& cust) const {
        auto pickSmallest = [&](int start, int count) -> int {
            int best = -1;
            int bestLen = 1e9;
            for (int i = 0; i < count; i++) {
                int lane = start + i;
                if (s.queues[lane] < MAX_QUEUE && s.queues[lane] < bestLen) {
                    best = lane;
                    bestLen = s.queues[lane];
                }
            }
            return best;
        };

        // If online orders should bypass registers in your spec, handle it here.
        // For now: online orders still go through lanes unless your coupled model bypasses them.
        if (cust.numItems <= SELF_ITEM_LIMIT) {
            int self = pickSmallest(CASH_LANES, SELF_LANES);
            if (self >= 0) return self;
            return pickSmallest(0, CASH_LANES);
        } else {
            int cash = pickSmallest(0, CASH_LANES);
            if (cash >= 0) return cash;
            return pickSmallest(CASH_LANES, SELF_LANES);
        }
    }
};

#endif