#ifndef TRAVELER_HPP
#define TRAVELER_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include <vector>
#include <algorithm>
#include "customer_data.hpp"

using namespace cadmium;

struct TravelerState {
    enum class Phase { PASSIVE, ACTIVE } phase;
    double sigma;

    // (remainingTime, CustomerData) — sorted ascending by remainingTime
    std::vector<std::pair<double, CustomerData>> travelers;

    TravelerState()
        : phase(Phase::PASSIVE),
          sigma(std::numeric_limits<double>::infinity()),
          travelers() {}
};

inline std::ostream& operator<<(std::ostream& os, const TravelerState& s) {
    os << "{phase:" << (s.phase == TravelerState::Phase::PASSIVE ? "passive" : "active")
       << ",sigma:" << s.sigma
       << ",enRoute:" << s.travelers.size()
       << "}";
    return os;
}

class Traveler : public Atomic<TravelerState> {
public:
    Port<CustomerData> custIn;      // paid walk-in customers
    Port<CustomerData> custArrived; // finished walk-in customers

    explicit Traveler(const std::string& id)
        : Atomic<TravelerState>(id, TravelerState())
    {
        custIn      = addInPort<CustomerData>("custIn");
        custArrived = addOutPort<CustomerData>("custArrived");
    }

    void externalTransition(TravelerState& s, double e) const override {
        // Advance all active travelers
        for (auto& [rem, cust] : s.travelers) rem -= e;

        // Enqueue new customers (walk-ins only)
        if (!custIn->empty()) {
            for (const auto& cust : custIn->getBag()) {
                if (cust.isOnlineOrder) continue; // online orders are handled by curbside pickup flow
                s.travelers.push_back({cust.travelTime, cust});
            }
        }

        refreshSigma(s);
    }

    void output(const TravelerState& s) const override {
        for (const auto& [rem, cust] : s.travelers) {
            if (rem <= 1e-9) custArrived->addMessage(cust);
        }
    }

    void internalTransition(TravelerState& s) const override {
        s.travelers.erase(
            std::remove_if(s.travelers.begin(), s.travelers.end(),
                           [](const auto& p) { return p.first <= 1e-9; }),
            s.travelers.end()
        );
        refreshSigma(s);
    }

    [[nodiscard]] double timeAdvance(const TravelerState& s) const override {
        return s.sigma;
    }

private:
    static void refreshSigma(TravelerState& s) {
        if (s.travelers.empty()) {
            s.phase = TravelerState::Phase::PASSIVE;
            s.sigma = std::numeric_limits<double>::infinity();
        } else {
            std::sort(s.travelers.begin(), s.travelers.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            s.phase = TravelerState::Phase::ACTIVE;
            s.sigma = std::max(0.0, s.travelers.front().first);
        }
    }
};

#endif // TRAVELER_HPP
