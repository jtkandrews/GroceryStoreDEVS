#ifndef DISTRIBUTOR_HPP
#define DISTRIBUTOR_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <vector>
#include <limits>
#include <algorithm>
#include "customer_data.hpp"

using namespace cadmium;

// ---- FIXED LANE COUNTS ----
static constexpr int CASH_LANES  = 3;
static constexpr int SELF_LANES  = 2;
static constexpr int TOTAL_LANES = CASH_LANES + SELF_LANES;

// ---- RULE: items <= SELF_ITEM_LIMIT go to self-checkout if possible ----
static constexpr int SELF_ITEM_LIMIT = 15;

// ---- Max queue per lane (simple cap) ----
static constexpr int MAX_QUEUE = 2;

struct DistributorState {
    enum class Phase { IDLE, SEND } phase;

    // Current queue lengths (one per lane)
    std::vector<int> queues; // size TOTAL_LANES

    // One-tick pulses back to Generator
    bool emitHold = false;
    bool emitOk   = false;

    // Outbox: all lane assignments we need to emit in output()
    struct Route {
        int lane = -1;
        CustomerData cust;
    };
    std::vector<Route> outbox;

    // Online orders bypass lanes
    std::vector<CustomerData> onlineOutbox;

    DistributorState()
        : phase(Phase::IDLE),
          queues(TOTAL_LANES, 0),
          outbox(),
          onlineOutbox() {}
};

inline std::ostream& operator<<(std::ostream& os, const DistributorState& s) {
    os << "{phase:" << (s.phase == DistributorState::Phase::IDLE ? "idle" : "send")
       << ",hold:" << (s.emitHold ? "1" : "0")
       << ",ok:"   << (s.emitOk ? "1" : "0")
       << ",q:[";
    for (size_t i = 0; i < s.queues.size(); ++i) {
        os << s.queues[i] << (i + 1 < s.queues.size() ? "," : "");
    }
    os << "],outbox:" << s.outbox.size()
       << ",online:" << s.onlineOutbox.size() << "}";
    return os;
}

class Distributor : public Atomic<DistributorState> {
public:
    // Inputs
    Port<CustomerData> in_customer;
    Port<int>          in_laneFreed; // laneId that completed service

    // Outputs to lanes
    Port<CustomerData> out_cash0;
    Port<CustomerData> out_cash1;
    Port<CustomerData> out_cash2;
    Port<CustomerData> out_self0;
    Port<CustomerData> out_self1;
    Port<CustomerData> out_online;

    // Feedback to Generator
    Port<bool> out_holdOff;
    Port<bool> out_okGo;

    // Optional debug
    Port<int> out_whichLane;

    explicit Distributor(const std::string& id)
        : Atomic<DistributorState>(id, DistributorState())
    {
        in_customer  = addInPort<CustomerData>("in_customer");
        in_laneFreed = addInPort<int>("in_laneFreed");

        out_cash0 = addOutPort<CustomerData>("out_cash0");
        out_cash1 = addOutPort<CustomerData>("out_cash1");
        out_cash2 = addOutPort<CustomerData>("out_cash2");
        out_self0 = addOutPort<CustomerData>("out_self0");
        out_self1 = addOutPort<CustomerData>("out_self1");
        out_online = addOutPort<CustomerData>("out_online");

        out_holdOff = addOutPort<bool>("out_holdOff");
        out_okGo     = addOutPort<bool>("out_okGo");

        out_whichLane = addOutPort<int>("out_whichLane");
    }

    void internalTransition(DistributorState& s) const override {
        // Clear one-tick outputs
        s.phase = DistributorState::Phase::IDLE;
        s.emitHold = false;
        s.emitOk   = false;
        s.outbox.clear();
        s.onlineOutbox.clear();
    }

    void externalTransition(DistributorState& s, double /*e*/) const override {
        // 1) Apply lane freed events
        if (!in_laneFreed->empty()) {
            for (int laneId : in_laneFreed->getBag()) {
                if (0 <= laneId && laneId < TOTAL_LANES && s.queues[laneId] > 0) {
                    s.queues[laneId]--;
                }
            }
            // "OK" pulse tells Generator it can resume (if it was paused)
            s.emitOk = true;
            s.phase  = DistributorState::Phase::SEND;
        }

        // 2) Route customers
        if (!in_customer->empty()) {
            for (const CustomerData& cust : in_customer->getBag()) {
                if (cust.isOnlineOrder) {
                    s.onlineOutbox.push_back(cust);
                    s.phase = DistributorState::Phase::SEND;
                    continue;
                }
                const int lane = chooseLane(s, cust);
                if (lane >= 0) {
                    s.queues[lane]++;
                    s.outbox.push_back({lane, cust});
                } else {
                    // No lane had space: ask Generator to hold
                    s.emitHold = true;
                }
                s.phase = DistributorState::Phase::SEND;
            }
        }
    }

    void output(const DistributorState& s) const override {
        if (s.phase != DistributorState::Phase::SEND) return;

        if (s.emitHold) out_holdOff->addMessage(true);
        if (s.emitOk)   out_okGo->addMessage(true);

        for (const auto& r : s.outbox) {
            out_whichLane->addMessage(r.lane);

            if (r.lane == 0)      out_cash0->addMessage(r.cust);
            else if (r.lane == 1) out_cash1->addMessage(r.cust);
            else if (r.lane == 2) out_cash2->addMessage(r.cust);
            else if (r.lane == 3) out_self0->addMessage(r.cust);
            else if (r.lane == 4) out_self1->addMessage(r.cust);
        }

        for (const auto& cust : s.onlineOutbox) {
            out_online->addMessage(cust);
        }
    }

    [[nodiscard]] double timeAdvance(const DistributorState& s) const override {
        return (s.phase == DistributorState::Phase::IDLE)
            ? std::numeric_limits<double>::infinity()
            : 0.0;
    }

private:
    // Prefer self-checkout for <= SELF_ITEM_LIMIT, else staffed cash.
    // Within the chosen group: pick the smallest queue with available space.
    static int chooseLane(const DistributorState& s, const CustomerData& cust) {
        auto pickSmallest = [&](int start, int count) -> int {
            int best = -1;
            int bestLen = 1e9;
            for (int i = 0; i < count; ++i) {
                const int lane = start + i;
                if (s.queues[lane] < MAX_QUEUE && s.queues[lane] < bestLen) {
                    best = lane;
                    bestLen = s.queues[lane];
                }
            }
            return best;
        };

        if (cust.numItems <= SELF_ITEM_LIMIT) {
            const int self = pickSmallest(CASH_LANES, SELF_LANES);
            if (self >= 0) return self;
            return pickSmallest(0, CASH_LANES);
        } else {
            const int cash = pickSmallest(0, CASH_LANES);
            if (cash >= 0) return cash;
            return pickSmallest(CASH_LANES, SELF_LANES);
        }
    }
};

#endif // DISTRIBUTOR_HPP
