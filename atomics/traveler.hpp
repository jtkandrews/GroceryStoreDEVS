#ifndef TRAVELER_HPP
#define TRAVELER_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <string>
#include "customer_data.hpp"

using namespace cadmium;

struct travelerState {
    enum Phase { IDLE, TRAVELING } phase = IDLE;

    double sigma = std::numeric_limits<double>::infinity();

    int remainingSteps = 0;
    CustomerData current;
    bool hasCustomer = false;
};

inline std::ostream& operator<<(std::ostream& os, const travelerState& s){
    os << "{phase:" << (s.phase==travelerState::IDLE?"idle":"travel")
       << ",steps:" << s.remainingSteps
       << ",sigma:" << s.sigma
       << "}";
    return os;
}

class traveler : public Atomic<travelerState> {
public:

    Port<CustomerData> custIn;
    Port<CustomerData> custArrived;

    int steps; 

    traveler(const std::string& id, int steps_ = 10)
        : Atomic<travelerState>(id, travelerState()),
          steps(steps_)
    {
        custIn = addInPort<CustomerData>("custIn");
        custArrived = addOutPort<CustomerData>("custArrived");
    }

    // INTERNAL
    void internalTransition(travelerState& s) const override {

        if(s.phase == travelerState::TRAVELING){
            s.remainingSteps--;

            if(s.remainingSteps <= 0){
                s.phase = travelerState::IDLE;
                s.sigma = std::numeric_limits<double>::infinity();
                s.hasCustomer = false;
            } else {
                s.sigma = 1.0; // 1 time unit per step
            }
        }
    }

    // EXTERNAL
    void externalTransition(travelerState& s, double e) const override {

        // advance time
        if(s.sigma != std::numeric_limits<double>::infinity())
            s.sigma -= e;

        if(custIn->empty()) return;

        const auto& bag = custIn->getBag();
        const CustomerData c = bag.back();

        // ignore online orders
        if(c.isOnlineOrder) return;

        if(s.phase == travelerState::IDLE){
            s.current = c;
            s.hasCustomer = true;
            s.remainingSteps = steps;
            s.phase = travelerState::TRAVELING;
            s.sigma = 1.0;
        }
    }

    // OUTPUT
    void output(const travelerState& s) const override {

        if(s.phase == travelerState::TRAVELING &&
           s.hasCustomer &&
           s.remainingSteps == 1)
        {
            custArrived->addMessage(s.current);
        }
    }

    // TIME ADVANCE
    [[nodiscard]] double timeAdvance(const travelerState& s) const override {
        return s.sigma;
    }
};

#endif