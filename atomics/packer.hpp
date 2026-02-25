#ifndef PACKER_HPP
#define PACKER_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include "customer_data.hpp"

using namespace cadmium;

struct PackerState {
    enum class Phase { IDLE, PACKING } phase;
    double defaultPackTimePerItem;
    double sigma;
    CustomerData current;

    explicit PackerState(double ptpi = 1.0)
        : phase(Phase::IDLE),
          defaultPackTimePerItem(ptpi),
          sigma(std::numeric_limits<double>::infinity()),
          current() {}
};

inline std::ostream& operator<<(std::ostream& os, const PackerState& s) {
    os << "{phase:" << (s.phase == PackerState::Phase::IDLE ? "idle" : "packing")
       << ",sigma:" << s.sigma
       << "}";
    return os;
}

class Packer : public Atomic<PackerState> {
public:
    Port<CustomerData> in_order;   // from PaymentProcessor (online orders only)
    Port<CustomerData> out_packed; // to CurbsideDispatcher

    Packer(const std::string& id, double packTimePerItem = 1.0)
        : Atomic<PackerState>(id, PackerState(packTimePerItem))
    {
        in_order   = addInPort<CustomerData>("in_order");
        out_packed = addOutPort<CustomerData>("out_packed");
    }

    void externalTransition(PackerState& s, double /*e*/) const override {
        if (s.phase == PackerState::Phase::IDLE && !in_order->empty()) {
            const CustomerData cust = in_order->getBag().back();

            // Only pack online orders; ignore walk-ins if they arrive here accidentally.
            if (!cust.isOnlineOrder) return;

            s.current = cust;
            s.phase = PackerState::Phase::PACKING;

            if (s.current.searchTime > 0.0) {
                s.sigma = s.current.searchTime;
            } else {
                s.sigma = (s.current.numItems > 0)
                    ? (static_cast<double>(s.current.numItems) * s.defaultPackTimePerItem)
                    : s.defaultPackTimePerItem;
            }
        }
    }

    void output(const PackerState& s) const override {
        if (s.phase == PackerState::Phase::PACKING) {
            out_packed->addMessage(s.current);
        }
    }

    void internalTransition(PackerState& s) const override {
        s.phase = PackerState::Phase::IDLE;
        s.sigma = std::numeric_limits<double>::infinity();
        s.current = CustomerData();
    }

    [[nodiscard]] double timeAdvance(const PackerState& s) const override {
        return (s.phase == PackerState::Phase::IDLE)
            ? std::numeric_limits<double>::infinity()
            : s.sigma;
    }
};

#endif // PACKER_HPP
