#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "distributor.hpp"
#include "packer.hpp"
#include "curbside_dispatcher.hpp"
#include "customer_data.hpp"

using namespace cadmium;

// Coupled online flow: Distributor -> Packer -> Curbside
struct top_test_coupled_online : public Coupled {
    Port<CustomerData> out_finished_test;

    top_test_coupled_online(const std::string& id) : Coupled(id) {
        out_finished_test = addOutPort<CustomerData>("out_finished_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/online_customers.txt"
        );

        auto dist = addComponent<Distributor>("distributor");
        auto pack = addComponent<Packer>("packer", 1.0);
        auto curb = addComponent<CurbsideDispatcher>("curbside");

        addCoupling(in_reader->out, dist->in_customer);
        addCoupling(dist->out_online, pack->in_order);
        addCoupling(pack->out_packed, curb->orderIn);
        addCoupling(curb->finished, out_finished_test);
    }
};

int main() {
    std::cout << "=== Coupled Online Test ===\n";
    auto sys = std::make_shared<top_test_coupled_online>("test_coupled_online");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(300.0);
    rc.stop();
}
