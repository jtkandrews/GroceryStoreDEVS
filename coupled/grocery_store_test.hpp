#ifndef GROCERY_STORE_TEST_HPP
#define GROCERY_STORE_TEST_HPP

#include <cadmium/modeling/devs/coupled.hpp>
using namespace cadmium;

#include "distributor.hpp"
#include "cash.hpp"
#include "payment_processor.hpp"
#include "traveler.hpp"
#include "pickup_system.hpp"

// A test-friendly top model:
// - NO generator
// - takes CustomerData from an input file (IEStream)
// - exposes "finished walk-in" and "finished online" as out ports
struct grocery_store_test : public Coupled {

    // external ports (so tests can hook file input + sinks)
    Port<CustomerData> in_customer;
    Port<CustomerData> out_walkin_done;
    Port<CustomerData> out_online_done;

    grocery_store_test(const std::string& id) : Coupled(id) {

        in_customer      = addInPort<CustomerData>("in_customer");
        out_walkin_done  = addOutPort<CustomerData>("out_walkin_done");
        out_online_done  = addOutPort<CustomerData>("out_online_done");

        // Components (same as your real model, minus generator + sinks)
        auto dist  = addComponent<Distributor>("distributor");

        auto cash0 = addComponent<Cash>("cash0", 0, 1.0);
        auto cash1 = addComponent<Cash>("cash1", 1, 1.0);
        auto cash2 = addComponent<Cash>("cash2", 2, 1.0);

        auto self0 = addComponent<Cash>("self0", 3, 0.8);
        auto self1 = addComponent<Cash>("self1", 4, 0.8);

        auto pay   = addComponent<PaymentProcessor>("payment");
        auto walk  = addComponent<traveler>("traveler");

        auto pickup = addComponent<pickup_system>("pickup");

        // EIC: file -> distributor
        addCoupling(in_customer, dist->in_customer);

        // Distributor -> lanes
        addCoupling(dist->out_cash0, cash0->in_customer);
        addCoupling(dist->out_cash1, cash1->in_customer);
        addCoupling(dist->out_cash2, cash2->in_customer);
        addCoupling(dist->out_self0, self0->in_customer);
        addCoupling(dist->out_self1, self1->in_customer);

        // lanes -> payment
        addCoupling(cash0->out_toPayment, pay->custIn);
        addCoupling(cash1->out_toPayment, pay->custIn);
        addCoupling(cash2->out_toPayment, pay->custIn);
        addCoupling(self0->out_toPayment, pay->custIn);
        addCoupling(self1->out_toPayment, pay->custIn);

        // lane free -> distributor
        addCoupling(cash0->out_free, dist->in_laneFreed);
        addCoupling(cash1->out_free, dist->in_laneFreed);
        addCoupling(cash2->out_free, dist->in_laneFreed);
        addCoupling(self0->out_free, dist->in_laneFreed);
        addCoupling(self1->out_free, dist->in_laneFreed);

        // payment -> traveler (walk-in path)
        addCoupling(pay->custOut, walk->custIn);

        // traveler -> top output (walk-ins done)
        addCoupling(walk->custArrived, out_walkin_done);

        // online orders bypass checkout and go to pickup system
        addCoupling(dist->out_online, pickup->in_order);

        // pickup system -> top output (online done)
        addCoupling(pickup->finished, out_online_done);
    }
};

#endif