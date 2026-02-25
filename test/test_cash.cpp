#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "cash.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_cash : public Coupled {
    Port<CustomerData> out_to_payment_test;
    Port<int>          out_free_test;

    top_test_cash(const std::string& id) : Coupled(id) {
        out_to_payment_test = addOutPort<CustomerData>("out_to_payment_test");
        out_free_test       = addOutPort<int>("out_free_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cash_reader", "input_data/cash_one_customer.txt"
        );

        auto lane0 = addComponent<Cash>("cash0", 0, 1.0);

        addCoupling(in_reader->out, lane0->in_customer);
        addCoupling(lane0->out_toPayment, out_to_payment_test);
        addCoupling(lane0->out_free, out_free_test);
    }
};

int main() {
    std::cout << "=== Cash Test: One Customer ===\n";
    auto sys = std::make_shared<top_test_cash>("test_cash");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(100.0);
    rc.stop();
}
