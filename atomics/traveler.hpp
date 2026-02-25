//
// Created by Joseph Andrews on 2026-02-24.
//

#ifndef SYSC4906G_TRAVELER_H
#define SYSC4906G_TRAVELER_H

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>
#include <vector>
#include <algorithm>
#include "customer_data.hpp"

using namespace cadmium;

enum class TravelerPhase { PASSIVE, ACTIVE };

struct TravelerState {
    TravelerPhase phase;
    double        sigma;
    // (remainingTime, CustomerData) — sorted ascending by remainingTime
    std::vector<std::pair<double, CustomerData>> travelers;

    explicit TravelerState()
        : phase(TravelerPhase::PASSIVE),
          sigma(std::numeric_limits<double>::infinity()),
          travelers() {}
};

inline std::ostream& operator<<(std::ostream& os, const TravelerState& s) {
    os << "{phase:"    << (s.phase == TravelerPhase::PASSIVE ? "passive" : "active")
       << ",sigma:"    << s.sigma
       << ",enRoute:"  << s.travelers.size() << "}";
    return os;
}

class Traveler : public Atomic<TravelerState> {
public:
    // Ports
    Port<CustomerData> custIn;       // in:  paid customer from PaymentProcessor
    Port<CustomerData> custArrived;  // out: customer who has reached destination

    explicit Traveler(const std::string& id)
        : Atomic<TravelerState>(id, TravelerState())
    {
        custIn      = addInPort<CustomerData> ("custIn");
        custArrived = addOutPort<CustomerData>("custArrived");
    }

    // δext — advance all clocks, then enqueue new customers
    void externalTransition(TravelerState& state, double e) const override {
        // Reduce remaining travel time for every active traveler
        for (auto& [rem, cust] : state.travelers) {
            rem -= e;
        }
        // Enqueue newly arriving customers using their CustomerData.travelTime
        if (!custIn->empty()) {
            for (const auto& cust : custIn->getBag()) {
                state.travelers.push_back({cust.travelTime, cust});
            }
        }
        refreshSigma(state);
    }

    // λ — emit all customers who have just arrived (remainingTime ≈ 0)
    void output(const TravelerState& state) const override {
        for (const auto& [rem, cust] : state.travelers) {
            if (rem <= 1e-9) {
                custArrived->addMessage(cust);
            }
        }
    }

    // δint — remove arrived customers; reschedule for the next earliest
    void internalTransition(TravelerState& state) const override {
        state.travelers.erase(
            std::remove_if(state.travelers.begin(), state.travelers.end(),
                           [](const auto& p) { return p.first <= 1e-9; }),
            state.travelers.end()
        );
        refreshSigma(state);
    }

    [[nodiscard]] double timeAdvance(const TravelerState& state) const override {
        return state.sigma;
    }

private:
    // Sort by remaining time; set sigma to the soonest arrival
    static void refreshSigma(TravelerState& state) {
        if (state.travelers.empty()) {
            state.phase = TravelerPhase::PASSIVE;
            state.sigma = std::numeric_limits<double>::infinity();
        } else {
            std::sort(state.travelers.begin(), state.travelers.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            state.phase = TravelerPhase::ACTIVE;
            state.sigma = std::max(0.0, state.travelers.front().first);
        }
    }
};

#endif //SYSC4906G_TRAVELER_H