#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "distributor.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_distributor : public Coupled {
    Port<CustomerData> out_cash0_test;
    Port<CustomerData> out_cash1_test;
    Port<CustomerData> out_cash2_test;
    Port<CustomerData> out_self0_test;
    Port<CustomerData> out_self1_test;
    Port<CustomerData> out_online_test;
    Port<int>          out_lane_test;
    Port<bool>         out_hold_test;
    Port<bool>         out_ok_test;

    top_test_distributor(const std::string& id) : Coupled(id) {
        out_cash0_test = addOutPort<CustomerData>("out_cash0_test");
        out_cash1_test = addOutPort<CustomerData>("out_cash1_test");
        out_cash2_test = addOutPort<CustomerData>("out_cash2_test");
        out_self0_test = addOutPort<CustomerData>("out_self0_test");
        out_self1_test = addOutPort<CustomerData>("out_self1_test");
        out_online_test = addOutPort<CustomerData>("out_online_test");
        out_lane_test  = addOutPort<int>("out_lane_test");
        out_hold_test  = addOutPort<bool>("out_hold_test");
        out_ok_test    = addOutPort<bool>("out_ok_test");

        auto cust_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/distributor_customers.txt"
        );
        auto lane_reader = addComponent<cadmium::lib::IEStream<int>>(
            "lane_reader", "input_data/distributor_lane_freed.txt"
        );

        auto dist = addComponent<Distributor>("distributor");

        addCoupling(cust_reader->out, dist->in_customer);
        addCoupling(lane_reader->out, dist->in_laneFreed);

        addCoupling(dist->out_cash0, out_cash0_test);
        addCoupling(dist->out_cash1, out_cash1_test);
        addCoupling(dist->out_cash2, out_cash2_test);
        addCoupling(dist->out_self0, out_self0_test);
        addCoupling(dist->out_self1, out_self1_test);
        addCoupling(dist->out_online, out_online_test);
        addCoupling(dist->out_whichLane, out_lane_test);
        addCoupling(dist->out_holdOff, out_hold_test);
        addCoupling(dist->out_okGo, out_ok_test);
    }
};

int main() {
    std::cout << "=== Distributor Test: Routing + Lane Freed ===\n";
    auto sys = std::make_shared<top_test_distributor>("test_distributor");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(20.0);
    rc.stop();
}
