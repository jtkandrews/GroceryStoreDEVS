#include <iostream>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>

#include "distributor.hpp"
#include "cash.hpp"
#include "payment_processor.hpp"
#include "traveler.hpp"
#include "customer_data.hpp"

using namespace cadmium;

// Coupled checkout chain: Distributor -> Cash -> Payment -> Traveler
struct top_test_coupled_checkout : public Coupled {
    Port<CustomerData> out_arrived_test;

    top_test_coupled_checkout(const std::string& id) : Coupled(id) {
        out_arrived_test = addOutPort<CustomerData>("out_arrived_test");

        auto in_reader = addComponent<cadmium::lib::IEStream<CustomerData>>(
            "cust_reader", "input_data/checkout_customers.txt"
        );

        auto dist  = addComponent<Distributor>("distributor");
        auto cash0 = addComponent<Cash>("cash0", 0, 1.0);
        auto cash1 = addComponent<Cash>("cash1", 1, 1.0);
        auto cash2 = addComponent<Cash>("cash2", 2, 1.0);
        auto self0 = addComponent<Cash>("self0", 3, 0.8);
        auto self1 = addComponent<Cash>("self1", 4, 0.8);
        auto pay   = addComponent<PaymentProcessor>("payment");
        auto walk  = addComponent<traveler>("traveler");

        addCoupling(in_reader->out, dist->in_customer);

        addCoupling(dist->out_cash0, cash0->in_customer);
        addCoupling(dist->out_cash1, cash1->in_customer);
        addCoupling(dist->out_cash2, cash2->in_customer);
        addCoupling(dist->out_self0, self0->in_customer);
        addCoupling(dist->out_self1, self1->in_customer);

        addCoupling(cash0->out_toPayment, pay->custIn);
        addCoupling(cash1->out_toPayment, pay->custIn);
        addCoupling(cash2->out_toPayment, pay->custIn);
        addCoupling(self0->out_toPayment, pay->custIn);
        addCoupling(self1->out_toPayment, pay->custIn);

        addCoupling(cash0->out_free, dist->in_laneFreed);
        addCoupling(cash1->out_free, dist->in_laneFreed);
        addCoupling(cash2->out_free, dist->in_laneFreed);
        addCoupling(self0->out_free, dist->in_laneFreed);
        addCoupling(self1->out_free, dist->in_laneFreed);

        addCoupling(pay->custOut, walk->custIn);
        addCoupling(walk->custArrived, out_arrived_test);
    }
};

int main() {
    std::cout << "=== Coupled Checkout Test ===\n";
    auto sys = std::make_shared<top_test_coupled_checkout>("test_coupled_checkout");
    auto rc  = cadmium::RootCoordinator(sys);

    rc.setLogger<cadmium::STDOUTLogger>();
    rc.start();
    rc.simulate(200.0);
    rc.stop();
}
