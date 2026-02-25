#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <random>
#include <optional>
#include <cmath>
#include "customer_data.hpp"

using namespace cadmium;

struct GeneratorState {
    enum class Phase { RUNNING, PAUSED } phase;
    double sigma;
    int nextCustomerId;

    GeneratorState()
        : phase(Phase::RUNNING),
          sigma(0.0),  //fire immediately 
          nextCustomerId(0) {}
};

inline std::ostream& operator<<(std::ostream& os, const GeneratorState& s) {
    os << "{phase:" << (s.phase == GeneratorState::Phase::RUNNING ? "running" : "paused")
       << ",sigma:" << s.sigma
       << ",nextId:" << s.nextCustomerId
       << "}";
    return os;
}

class Generator : public Atomic<GeneratorState> {
public:
    // Inputs from Distributor
    Port<bool> okGo;
    Port<bool> holdOff;

    // Output to Distributor
    Port<CustomerData> customerOut;

    Generator(const std::string& id,
              double arrivalMean  = 60.0,   // mean inter-arrival time (seconds)
              double travelMean   = 300.0,  // mean travel time (seconds)
              double travelStdDev = 60.0,
              double searchMean   = 120.0,  // mean pack/search time (seconds)
              double onlineProb   = 0.30,   // probability of isOnlineOrder
              double cardProb     = 0.70,   // probability of tap/card payment
              std::optional<unsigned int> seed = std::nullopt)
        : Atomic<GeneratorState>(id, GeneratorState()),
          arrivalDist_(1.0 / std::max(1e-9, arrivalMean)),
          travelDist_ (travelMean, travelStdDev),
          searchDist_ (1.0 / std::max(1e-9, searchMean)),
          itemDist_   (1, 40),
          onlineDist_ (onlineProb),
          cardDist_   (cardProb)
    {
        if (seed.has_value()) {
            rng_.seed(*seed);
        }
        okGo        = addInPort<bool>("okGo");
        holdOff     = addInPort<bool>("holdOff");
        customerOut = addOutPort<CustomerData>("customerOut");
    }

    void externalTransition(GeneratorState& s, double e) const override {
        // Advance local clock if we were counting down
        if (s.phase == GeneratorState::Phase::RUNNING && s.sigma != std::numeric_limits<double>::infinity()) {
            s.sigma = std::max(0.0, s.sigma - e);
        }

        const bool gotHold = !holdOff->empty();
        const bool gotOk   = !okGo->empty();

        // Pause takes precedence over OK (if both arrive)
        if (gotHold && s.phase == GeneratorState::Phase::RUNNING) {
            s.phase = GeneratorState::Phase::PAUSED;
            s.sigma = std::numeric_limits<double>::infinity();
            return;
        }

        if (gotOk && s.phase == GeneratorState::Phase::PAUSED) {
            s.phase = GeneratorState::Phase::RUNNING;
            s.sigma = sampleArrival();
        }
    }

    void output(const GeneratorState& s) const override {
        if (s.phase != GeneratorState::Phase::RUNNING) return;

        const int    id     = s.nextCustomerId;
        const int    items  = itemDist_(rng_);
        const bool   online = onlineDist_(rng_);
        const bool   card   = cardDist_(rng_);
        const double travel = std::max(0.0, travelDist_(rng_));
        const double search = std::fabs(searchDist_(rng_));

        customerOut->addMessage(CustomerData(id, items, online, card, travel, search));
    }

    void internalTransition(GeneratorState& s) const override {
        if (s.phase == GeneratorState::Phase::RUNNING) {
            ++s.nextCustomerId;
            s.sigma = sampleArrival();
        }
    }

    [[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
        return s.sigma;
    }

private:
    mutable std::mt19937                          rng_{std::random_device{}()};
    mutable std::exponential_distribution<double> arrivalDist_;
    mutable std::normal_distribution<double>      travelDist_;
    mutable std::exponential_distribution<double> searchDist_;
    mutable std::uniform_int_distribution<int>    itemDist_;
    mutable std::bernoulli_distribution           onlineDist_;
    mutable std::bernoulli_distribution           cardDist_;

    double sampleArrival() const {
        return std::fabs(arrivalDist_(rng_));
    }
};

#endif // GENERATOR_HPP
