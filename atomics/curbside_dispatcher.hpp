//
// Created by Joseph Andrews on 2026-02-24.
//

#ifndef SYSC4906G_CURBSIDE_DISPATCHER_H
#define SYSC4906G_CURBSIDE_DISPATCHER_H

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <queue>
#include "customer_data.hpp"

using namespace cadmium;

enum class DispatcherPhase { IDLE, BUSY };

struct CurbsideDispatcherState {
    DispatcherPhase          phase;
    double                   sigma;
    CustomerData             currentOrder;
    std::queue<CustomerData> orderQueue;

    explicit CurbsideDispatcherState()
        : phase(DispatcherPhase::IDLE),
          sigma(std::numeric_limits<double>::infinity()),
          currentOrder(),
          orderQueue() {}
};

inline std::ostream& operator<<(std::ostream& os, const CurbsideDispatcherState& s) {
    os << "{phase:"  << (s.phase == DispatcherPhase::IDLE ? "idle" : "busy")
       << ",sigma:"  << s.sigma
       << ",queued:" << s.orderQueue.size() << "}";
    return os;
}

class CurbsideDispatcher : public Atomic<CurbsideDispatcherState> {
public:
    // Ports
    Port<CustomerData> orderIn;          // in:  packed order from Packer
    Port<CustomerData> customerFinished; // out: customer collected their order

    explicit CurbsideDispatcher(const std::string& id)
        : Atomic<CurbsideDispatcherState>(id, CurbsideDispatcherState())
    {
        orderIn          = addInPort<CustomerData> ("orderIn");
        customerFinished = addOutPort<CustomerData>("customerFinished");
    }

    // δext — begin dispatching or queue the order
    void externalTransition(CurbsideDispatcherState& state, double e) const override {
        if (!orderIn->empty()) {
            for (const auto& order : orderIn->getBag()) {
                if (state.phase == DispatcherPhase::IDLE) {
                    state.currentOrder = order;
                    state.phase        = DispatcherPhase::BUSY;
                    // travelTime = how long until the online customer arrives to collect
                    state.sigma        = order.travelTime;
                } else {
                    state.orderQueue.push(order);
                    state.sigma -= e;   // preserve remaining wait for current order
                }
            }
        }
    }

    // λ — signal that this customer has collected their order
    void output(const CurbsideDispatcherState& state) const override {
        if (state.phase == DispatcherPhase::BUSY) {
            customerFinished->addMessage(state.currentOrder);
        }
    }

    // δint — serve next queued order or go idle
    void internalTransition(CurbsideDispatcherState& state) const override {
        if (!state.orderQueue.empty()) {
            state.currentOrder = state.orderQueue.front();
            state.orderQueue.pop();
            state.phase = DispatcherPhase::BUSY;
            state.sigma = state.currentOrder.travelTime;
        } else {
            state.phase = DispatcherPhase::IDLE;
            state.sigma = std::numeric_limits<double>::infinity();
        }
    }

    [[nodiscard]] double timeAdvance(const CurbsideDispatcherState& state) const override {
        return state.sigma;
    }
};


#endif //SYSC4906G_CURBSIDE_DISPATCHER_H