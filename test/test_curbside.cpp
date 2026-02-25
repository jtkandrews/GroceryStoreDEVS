#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "curbside_dispatcher.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_curbside : public Coupled {
    Port<CustomerData> out_finished_test;

    top_test_curbside(const std::string& id) : Coupled(id) {
        out_finished_test = addOutPort<CustomerData>("out_finished_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "order_reader", "input_data/curbside_orders.txt"
        );

        auto curb = addComponent<CurbsideDispatcher>("curbside");

        addCoupling(in_reader->out, curb->orderIn);
        addCoupling(curb->finished, out_finished_test);
    }
};

int main() {
    std::cout << "=== Curbside Test: Pickup Timing ===\n";
    auto sys = std::make_shared<top_test_curbside>("test_curbside");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(50.0);
    rc.stop();
}
