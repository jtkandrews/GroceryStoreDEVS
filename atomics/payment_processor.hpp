//
// Created by Joseph Andrews on 2026-02-24.
//

#ifndef SYSC4906G_PAYMENT_PROCESSOR_H
#define SYSC4906G_PAYMENT_PROCESSOR_H

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>
#include <queue>
#include <random>
#include "customer_data.hpp"

using namespace cadmium;

enum class PaymentPhase { PASSIVE, ACTIVE };

struct PaymentProcessorState {
    PaymentPhase             phase;
    double                   sigma;
    CustomerData             currentCustomer;
    std::queue<CustomerData> waitQueue;

    explicit PaymentProcessorState()
        : phase(PaymentPhase::PASSIVE),
          sigma(std::numeric_limits<double>::infinity()),
          currentCustomer(),
          waitQueue() {}
};

inline std::ostream& operator<<(std::ostream& os, const PaymentProcessorState& s) {
    os << "{phase:"     << (s.phase == PaymentPhase::PASSIVE ? "passive" : "active")
       << ",sigma:"     << s.sigma
       << ",queued:"    << s.waitQueue.size() << "}";
    return os;
}

class PaymentProcessor : public Atomic<PaymentProcessorState> {
public:
    // Ports
    Port<CustomerData> custForPayment;  // in:  customer arriving from Cash
    Port<CustomerData> custPaid;        // out: customer forwarded to Traveler

    explicit PaymentProcessor(const std::string& id)
        : Atomic<PaymentProcessorState>(id, PaymentProcessorState()),
          cardDist_(5.0,  15.0),   // tap/card: 5–15 seconds
          cashDist_(30.0, 120.0)   // cash:    30–120 seconds
    {
        custForPayment = addInPort<CustomerData> ("custForPayment");
        custPaid       = addOutPort<CustomerData>("custPaid");
    }

    // δext — accept new customer; queue if busy
    void externalTransition(PaymentProcessorState& state, double e) const override {
        if (!custForPayment->empty()) {
            for (const auto& cust : custForPayment->getBag()) {
                if (state.phase == PaymentPhase::PASSIVE) {
                    state.currentCustomer = cust;
                    state.phase           = PaymentPhase::ACTIVE;
                    state.sigma           = samplePayTime(cust.paymentType);
                } else {
                    state.waitQueue.push(cust);
                    state.sigma -= e;   // preserve remaining service time
                }
            }
        }
    }

    // λ — forward completed customer
    void output(const PaymentProcessorState& state) const override {
        if (state.phase == PaymentPhase::ACTIVE) {
            custPaid->addMessage(state.currentCustomer);
        }
    }

    // δint — serve next customer or go passive
    void internalTransition(PaymentProcessorState& state) const override {
        if (!state.waitQueue.empty()) {
            state.currentCustomer = state.waitQueue.front();
            state.waitQueue.pop();
            state.phase = PaymentPhase::ACTIVE;
            state.sigma = samplePayTime(state.currentCustomer.paymentType);
        } else {
            state.phase = PaymentPhase::PASSIVE;
            state.sigma = std::numeric_limits<double>::infinity();
        }
    }

    [[nodiscard]] double timeAdvance(const PaymentProcessorState& state) const override {
        return state.sigma;
    }

private:
    mutable std::mt19937                           rng_{std::random_device{}()};
    mutable std::uniform_real_distribution<double> cardDist_;
    mutable std::uniform_real_distribution<double> cashDist_;

    double samplePayTime(bool paymentType) const {
        return paymentType ? cardDist_(rng_) : cashDist_(rng_);
    }
};

#endif //SYSC4906G_PAYMENT_PROCESSOR_H