#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "payment_processor.hpp"
#include "customer_data.hpp"

using namespace cadmium;

struct top_test_payment : public Coupled {
    Port<CustomerData> out_done_test;

    top_test_payment(const std::string& id) : Coupled(id) {
        out_done_test = addOutPort<CustomerData>("out_done_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "pay_reader", "input_data/payment_two_customers.txt"
        );

        auto pay = addComponent<PaymentProcessor>("payment");

        addCoupling(in_reader->out, pay->custIn);
        addCoupling(pay->custOut, out_done_test);
    }
};

int main() {
    std::cout << "=== Payment Test: Two Customers Queue ===\n";
    auto sys = std::make_shared<top_test_payment>("test_payment");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(200.0); // payment times can be bigger
    rc.stop();
}