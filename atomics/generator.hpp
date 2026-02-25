//
// Created by Joseph Andrews on 2026-02-24.
//

#ifndef SYSC4906G_GENERATOR_H
#define SYSC4906G_GENERATOR_H

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>
#include <random>
#include <cmath>
#include "customer_data.hpp"

using namespace cadmium;

enum class GeneratorPhase { RUNNING, PAUSED };

struct GeneratorState {
    GeneratorPhase phase;
    double         sigma;
    int            nextCustomerId;

    explicit GeneratorState()
        : phase(GeneratorPhase::RUNNING),
          sigma(0.0),         // fire immediately at t=0
          nextCustomerId(0) {}
};

inline std::ostream& operator<<(std::ostream& os, const GeneratorState& s) {
    os << "{phase:" << (s.phase == GeneratorPhase::RUNNING ? "running" : "paused")
       << ",sigma:" << s.sigma
       << ",nextId:" << s.nextCustomerId << "}";
    return os;
}

class Generator : public Atomic<GeneratorState> {
public:
    // Ports
    Port<bool>         okGo;
    Port<bool>         holdOff;
    Port<CustomerData> customerOut;

    // Constructor — all distribution parameters are configurable
    Generator(const std::string& id,
              double arrivalMean  = 60.0,   // mean inter-arrival time (seconds)
              double travelMean   = 300.0,  // mean travel time to exit/curbside (seconds)
              double travelStdDev = 60.0,
              double searchMean   = 120.0,  // mean pack/search time (seconds)
              double onlineProb   = 0.30,   // probability of isOnlineOrder
              double cardProb     = 0.70)   // probability of tap/card payment
        : Atomic<GeneratorState>(id, GeneratorState()),
          arrivalDist_(1.0 / arrivalMean),
          travelDist_ (travelMean, travelStdDev),
          searchDist_ (1.0 / searchMean),
          itemDist_   (1, 40),
          onlineDist_ (onlineProb),
          cardDist_   (cardProb)
    {
        okGo        = addInPort<bool>        ("okGo");
        holdOff     = addInPort<bool>        ("holdOff");
        customerOut = addOutPort<CustomerData>("customerOut");
    }

    // δext — respond to holdOff / okGo signals
    void externalTransition(GeneratorState& state, double e) const override {
        if (!holdOff->empty() && state.phase == GeneratorPhase::RUNNING) {
            state.phase = GeneratorPhase::PAUSED;
            state.sigma = std::numeric_limits<double>::infinity();
        } else if (!okGo->empty() && state.phase == GeneratorPhase::PAUSED) {
            state.phase = GeneratorPhase::RUNNING;
            state.sigma = sampleArrival();
        } else {
            state.sigma -= e;   // irrelevant event; preserve remaining countdown
        }
    }

    // λ — sample and emit a new CustomerData
    void output(const GeneratorState& state) const override {
        if (state.phase == GeneratorPhase::RUNNING) {
            int    id     = state.nextCustomerId;
            int    items  = itemDist_(rng_);
            bool   online = onlineDist_(rng_);
            bool   card   = cardDist_(rng_);
            double travel = std::max(0.0, travelDist_(rng_));
            double search = std::fabs(searchDist_(rng_));
            customerOut->addMessage(CustomerData(id, items, online, card, travel, search));
        }
    }

    // δint — increment ID and schedule next arrival
    void internalTransition(GeneratorState& state) const override {
        if (state.phase == GeneratorPhase::RUNNING) {
            ++state.nextCustomerId;
            state.sigma = sampleArrival();
        }
    }

    [[nodiscard]] double timeAdvance(const GeneratorState& state) const override {
        return state.sigma;
    }

private:
    mutable std::mt19937                            rng_{std::random_device{}()};
    mutable std::exponential_distribution<double>   arrivalDist_;
    mutable std::normal_distribution<double>        travelDist_;
    mutable std::exponential_distribution<double>   searchDist_;
    mutable std::uniform_int_distribution<int>      itemDist_;
    mutable std::bernoulli_distribution             onlineDist_;
    mutable std::bernoulli_distribution             cardDist_;

    double sampleArrival() const {
        return std::fabs(arrivalDist_(rng_));
    }
};

#endif //SYSC4906G_GENERATOR_H