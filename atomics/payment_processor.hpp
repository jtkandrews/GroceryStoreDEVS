#ifndef PAYMENT_PROCESSOR_HPP
#define PAYMENT_PROCESSOR_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <queue>
#include <random>
#include <algorithm>
#include "customer_data.hpp"

using namespace cadmium;

struct PaymentProcessorState {
    enum class Phase { IDLE, BUSY } phase;
    double sigma;

    CustomerData current;
    std::queue<CustomerData> q;

    PaymentProcessorState()
        : phase(Phase::IDLE),
          sigma(std::numeric_limits<double>::infinity()),
          current(),
          q() {}
};

inline std::ostream& operator<<(std::ostream& os, const PaymentProcessorState& s) {
    os << "{phase:" << (s.phase == PaymentProcessorState::Phase::IDLE ? "idle" : "busy")
       << ",sigma:" << s.sigma
       << ",queued:" << s.q.size()
       << "}";
    return os;
}

class PaymentProcessor : public Atomic<PaymentProcessorState> {
public:
    Port<CustomerData> custIn;   // from registers
    Port<CustomerData> custOut;  // to Traveler + Packer

    explicit PaymentProcessor(const std::string& id)
        : Atomic<PaymentProcessorState>(id, PaymentProcessorState()),
          cardDist_(5.0,  15.0),    // tap/card: 5–15 seconds
          cashDist_(30.0, 120.0)    // cash:    30–120 seconds
    {
        custIn  = addInPort<CustomerData>("custIn");
        custOut = addOutPort<CustomerData>("custOut");
    }

    void externalTransition(PaymentProcessorState& s, double e) const override {
        // Advance remaining service time if we're busy
        if (s.phase == PaymentProcessorState::Phase::BUSY) {
            s.sigma = std::max(0.0, s.sigma - e);
        }

        if (custIn->empty()) return;

        for (const auto& cust : custIn->getBag()) {
            if (s.phase == PaymentProcessorState::Phase::IDLE) {
                s.current = cust;
                s.phase   = PaymentProcessorState::Phase::BUSY;
                s.sigma   = samplePayTime(cust.paymentType);
            } else {
                s.q.push(cust);
            }
        }
    }

    void output(const PaymentProcessorState& s) const override {
        if (s.phase == PaymentProcessorState::Phase::BUSY) {
            custOut->addMessage(s.current);
        }
    }

    void internalTransition(PaymentProcessorState& s) const override {
        if (!s.q.empty()) {
            s.current = s.q.front();
            s.q.pop();
            s.phase = PaymentProcessorState::Phase::BUSY;
            s.sigma = samplePayTime(s.current.paymentType);
        } else {
            s.phase = PaymentProcessorState::Phase::IDLE;
            s.sigma = std::numeric_limits<double>::infinity();
            s.current = CustomerData();
        }
    }

    [[nodiscard]] double timeAdvance(const PaymentProcessorState& s) const override {
        return s.sigma;
    }

private:
    mutable std::mt19937 rng_{std::random_device{}()};
    mutable std::uniform_real_distribution<double> cardDist_;
    mutable std::uniform_real_distribution<double> cashDist_;

    double samplePayTime(bool paymentType) const {
        return paymentType ? cardDist_(rng_) : cashDist_(rng_);
    }
};

#endif // PAYMENT_PROCESSOR_HPP
