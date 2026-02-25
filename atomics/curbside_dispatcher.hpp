#ifndef CURBSIDE_DISPATCHER_HPP
#define CURBSIDE_DISPATCHER_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <queue>
#include <algorithm>
#include "customer_data.hpp"

using namespace cadmium;

struct CurbsideDispatcherState {
    enum class Phase { IDLE, BUSY } phase;
    double sigma;

    CustomerData current;
    std::queue<CustomerData> q;

    CurbsideDispatcherState()
        : phase(Phase::IDLE),
          sigma(std::numeric_limits<double>::infinity()),
          current(),
          q() {}
};

inline std::ostream& operator<<(std::ostream& os, const CurbsideDispatcherState& s) {
    os << "{phase:" << (s.phase == CurbsideDispatcherState::Phase::IDLE ? "idle" : "busy")
       << ",sigma:" << s.sigma
       << ",queued:" << s.q.size()
       << "}";
    return os;
}

class CurbsideDispatcher : public Atomic<CurbsideDispatcherState> {
public:
    Port<CustomerData> orderIn;    // packed order from Packer
    Port<CustomerData> finished;   // customer collected their order

    explicit CurbsideDispatcher(const std::string& id)
        : Atomic<CurbsideDispatcherState>(id, CurbsideDispatcherState())
    {
        orderIn   = addInPort<CustomerData>("orderIn");
        finished  = addOutPort<CustomerData>("finished");
    }

    void externalTransition(CurbsideDispatcherState& s, double e) const override {
        if (s.phase == CurbsideDispatcherState::Phase::BUSY) {
            s.sigma = std::max(0.0, s.sigma - e);
        }

        if (orderIn->empty()) return;

        for (const auto& order : orderIn->getBag()) {
            if (s.phase == CurbsideDispatcherState::Phase::IDLE) {
                s.current = order;
                s.phase = CurbsideDispatcherState::Phase::BUSY;
                s.sigma = std::max(0.0, order.travelTime); // "time until pickup"
            } else {
                s.q.push(order);
            }
        }
    }

    void output(const CurbsideDispatcherState& s) const override {
        if (s.phase == CurbsideDispatcherState::Phase::BUSY) {
            finished->addMessage(s.current);
        }
    }

    void internalTransition(CurbsideDispatcherState& s) const override {
        if (!s.q.empty()) {
            s.current = s.q.front();
            s.q.pop();
            s.phase = CurbsideDispatcherState::Phase::BUSY;
            s.sigma = std::max(0.0, s.current.travelTime);
        } else {
            s.phase = CurbsideDispatcherState::Phase::IDLE;
            s.sigma = std::numeric_limits<double>::infinity();
            s.current = CustomerData();
        }
    }

    [[nodiscard]] double timeAdvance(const CurbsideDispatcherState& s) const override {
        return s.sigma;
    }
};

#endif // CURBSIDE_DISPATCHER_HPP
