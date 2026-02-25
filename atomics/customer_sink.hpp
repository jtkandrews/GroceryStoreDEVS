#ifndef CUSTOMER_SINK_HPP
#define CUSTOMER_SINK_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>
#include "customer_data.hpp"

using namespace cadmium;

struct CustomerSinkState {
    int count = 0;
};

inline std::ostream& operator<<(std::ostream& os, const CustomerSinkState& s) {
    os << "{count:" << s.count << "}";
    return os;
}

// Simple sink: consumes CustomerData messages and counts them (no outputs).
class CustomerSink : public Atomic<CustomerSinkState> {
public:
    Port<CustomerData> in;

    explicit CustomerSink(const std::string& id)
        : Atomic<CustomerSinkState>(id, CustomerSinkState())
    {
        in = addInPort<CustomerData>("in");
    }

    void externalTransition(CustomerSinkState& s, double /*e*/) const override {
        if (!in->empty()) {
            s.count += static_cast<int>(in->getBag().size());
        }
    }

    void internalTransition(CustomerSinkState& /*s*/) const override {}

    void output(const CustomerSinkState& /*s*/) const override {}

    [[nodiscard]] double timeAdvance(const CustomerSinkState& /*s*/) const override {
        return std::numeric_limits<double>::infinity();
    }
};

#endif // CUSTOMER_SINK_HPP
