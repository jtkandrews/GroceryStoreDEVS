#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "traveler.hpp"     // your traveler atomic
#include "customer_data.hpp" // wherever CustomerData is

using namespace cadmium;

struct top_test_traveler : public Coupled {
    Port<CustomerData> out_arrived_test;

    top_test_traveler(const std::string& id) : Coupled(id) {
        out_arrived_test = addOutPort<CustomerData>("out_arrived_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/one_customer.txt"
        );

        auto t = addComponent<traveler>("traveler");

        // match your traveler port names:
        addCoupling(in_reader->out, t->custIn);        // traveler input port
        addCoupling(t->custArrived, out_arrived_test); // traveler output port
    }
};

int main() {
    std::cout << "=== Traveler Test: One Customer ===\n";
    auto sys = std::make_shared<top_test_traveler>("test_traveler");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(50.0); // enough time to see travel finish
    rc.stop();
}