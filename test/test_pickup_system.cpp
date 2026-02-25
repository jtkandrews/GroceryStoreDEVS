#include <iostream>

#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/modeling/devs/atomic.hpp>
#include <cadmium/modeling/devs/coupled.hpp>

#include <cadmium/lib/iestream.hpp>

#include "customer_data.hpp"
#include "pickup_system.hpp"
#include "customer_sink.hpp"

using namespace cadmium;

int main() {
    std::cout << "=== Pickup System Test: Packer + Curbside ===" << std::endl;

    // Create top-level coupled model
    auto TOP = std::make_shared<cadmium::Coupled>("TOP");

    // Add input reader for online orders
    auto in_reader = TOP->addComponent<cadmium::lib::IEStream<CustomerData>>(
        "input_reader", "input_data/packer_orders.txt"
    );

    // Add the pickup system (packer + curbside)
    auto pickup = TOP->addComponent<pickup_system>("pickup");

    // Add sink to collect finished orders
    auto sink = TOP->addComponent<CustomerSink>("sink");

    // Wire input -> pickup -> sink
    TOP->addCoupling(in_reader->out, pickup->in_order);
    TOP->addCoupling(pickup->finished, sink->in);

    // Run simulation
    auto root = cadmium::RootCoordinator(TOP);
    root.setLogger<cadmium::STDOUTLogger>();
    root.simulate(50.0);

    std::cout << "Simulation complete." << std::endl;
    return 0;
}
