#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "packer.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_packer : public Coupled {
    Port<CustomerData> out_packed_test;

    top_test_packer(const std::string& id) : Coupled(id) {
        out_packed_test = addOutPort<CustomerData>("out_packed_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "order_reader", "input_data/packer_orders.txt"
        );

        auto pack = addComponent<Packer>("packer", 1.0);

        addCoupling(in_reader->out, pack->in_order);
        addCoupling(pack->out_packed, out_packed_test);
    }
};

int main() {
    std::cout << "=== Packer Test: Online Orders Only ===\n";
    auto sys = std::make_shared<top_test_packer>("test_packer");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(50.0);
    rc.stop();
}
