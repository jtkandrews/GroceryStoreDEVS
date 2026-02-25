#ifndef CASH_HPP
#define CASH_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include "customer_data.hpp"

using namespace cadmium;

struct CashState {
    enum class Phase { IDLE, BUSY } phase;
    int laneId;
    double timePerItem;
    double sigma;
    CustomerData current;

    CashState(int lane = 0, double tpi = 1.0)
        : phase(Phase::IDLE),
          laneId(lane),
          timePerItem(tpi),
          sigma(std::numeric_limits<double>::infinity()),
          current() {}
};

inline std::ostream& operator<<(std::ostream& os, const CashState& s) {
    os << "{phase:" << (s.phase == CashState::Phase::IDLE ? "idle" : "busy")
       << ",lane:" << s.laneId
       << ",sigma:" << s.sigma
       << "}";
    return os;
}

class Cash : public Atomic<CashState> {
public:
    Port<CustomerData> in_customer;
    Port<CustomerData> out_toPayment; 
    Port<int>          out_free;      

    Cash(const std::string& id, int lane, double timePerItem = 1.0)
        : Atomic<CashState>(id, CashState(lane, timePerItem))
    {
        in_customer   = addInPort<CustomerData>("in_customer");
        out_toPayment = addOutPort<CustomerData>("out_toPayment");
        out_free      = addOutPort<int>("out_free");
    }

    void externalTransition(CashState& s, double /*e*/) const override {
        if (s.phase == CashState::Phase::IDLE && !in_customer->empty()) {
            s.current = in_customer->getBag().back();
            s.phase = CashState::Phase::BUSY;

            s.sigma = (s.current.numItems > 0)
                ? (static_cast<double>(s.current.numItems) * s.timePerItem)
                : s.timePerItem;
        }
    }

    void output(const CashState& s) const override {
        if (s.phase == CashState::Phase::BUSY) {
            out_toPayment->addMessage(s.current);
            out_free->addMessage(s.laneId);
        }
    }

    void internalTransition(CashState& s) const override {
        s.phase = CashState::Phase::IDLE;
        s.sigma = std::numeric_limits<double>::infinity();
        s.current = CustomerData();
    }

    [[nodiscard]] double timeAdvance(const CashState& s) const override {
        return (s.phase == CashState::Phase::IDLE)
            ? std::numeric_limits<double>::infinity()
            : s.sigma;
    }
};

#endif 
