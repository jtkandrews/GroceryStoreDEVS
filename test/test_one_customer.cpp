#include <iostream>

#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/modeling/devs/atomic.hpp>
#include <cadmium/modeling/devs/coupled.hpp>

#include <cadmium/lib/iestream.hpp>

#include "../top_model/grocery_store_test.hpp"

using namespace cadmium;

int main() {
    // Coupled test harness (basil style)
    auto TOP = std::make_shared<cadmium::Coupled>("TOP");

    auto in_reader = TOP->addComponent<cadmium::lib::IEStream<CustomerData>>(
        "input_reader", "input_data/one_customer.txt"
    );
    auto store     = TOP->addComponent<grocery_store_test>("store_test");

    // Wire input -> store
    TOP->addCoupling(in_reader->out, store->in_customer);

    // Run
    cadmium::RootCoordinator root(TOP);
    root.setLogger<cadmium::STDOUTLogger>();
    root.start();
    root.simulate(500.0); // enough time for the one customer to finish
    root.stop();

    std::cout << "Done.\n";
    return 0;
}