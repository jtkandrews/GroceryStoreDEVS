#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "grocery_store_test.hpp"
#include "customer_data.hpp"

using namespace cadmium;

// Full-system deterministic test using grocery_store_test (no generator).
struct top_test_full_system : public Coupled {
    Port<CustomerData> out_walkin_done;
    Port<CustomerData> out_online_done;

    top_test_full_system(const std::string& id) : Coupled(id) {
        out_walkin_done = addOutPort<CustomerData>("out_walkin_done");
        out_online_done = addOutPort<CustomerData>("out_online_done");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/full_system_customers.txt"
        );

        auto store = addComponent<grocery_store_test>("store_test");

        addCoupling(in_reader->out, store->in_customer);
        addCoupling(store->out_walkin_done, out_walkin_done);
        addCoupling(store->out_online_done, out_online_done);
    }
};

int main() {
    std::cout << "=== Full System Deterministic Test ===\n";
    auto sys = std::make_shared<top_test_full_system>("test_full_system");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(500.0);
    rc.stop();
}
