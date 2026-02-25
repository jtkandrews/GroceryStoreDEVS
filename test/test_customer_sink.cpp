#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "customer_sink.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_customer_sink : public Coupled {
    top_test_customer_sink(const std::string& id) : Coupled(id) {
        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/sink_customers.txt"
        );

        auto sink = addComponent<CustomerSink>("sink");

        addCoupling(in_reader->out, sink->in);
    }
};

int main() {
    std::cout << "=== CustomerSink Test: Count ===\n";
    auto sys = std::make_shared<top_test_customer_sink>("test_customer_sink");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(20.0);
    rc.stop();
}
