#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>

#include "generator.hpp"

using namespace cadmium;

struct top_test_generator : public Coupled {
    Port<CustomerData> out_customer_test;

    top_test_generator(const std::string& id) : Coupled(id) {
        out_customer_test = addOutPort<CustomerData>("out_customer_test");

        auto gen = addComponent<Generator>("generator");
        addCoupling(gen->customerOut, out_customer_test);
    }
};

int main() {
    std::cout << "=== Generator Test: Basic Output ===\n";
    auto sys = std::make_shared<top_test_generator>("test_generator");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(200.0);
    rc.stop();
}
