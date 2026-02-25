#ifndef GROCERY_STORE_HPP
#define GROCERY_STORE_HPP

#include <cadmium/modeling/devs/coupled.hpp>

#include "generator.hpp"
#include "distributor.hpp"
#include "cash.hpp"
#include "payment_processor.hpp"
#include "traveler.hpp"
#include "pickup_system.hpp"
#include "customer_sink.hpp"

using namespace cadmium;

// Top-level coupled model for the grocery store.
struct grocery_store : public Coupled {
    grocery_store(const std::string& id) : Coupled(id) {
        // Components
        auto gen   = addComponent<Generator>("generator");

        auto dist  = addComponent<Distributor>("distributor");

        // 3 staffed cash lanes (laneId 0..2)
        auto cash0 = addComponent<Cash>("cash0", 0, 1.0);
        auto cash1 = addComponent<Cash>("cash1", 1, 1.0);
        auto cash2 = addComponent<Cash>("cash2", 2, 1.0);

        // 2 self-checkout lanes (laneId 3..4)
        auto self0 = addComponent<Cash>("self0", 3, 0.8);
        auto self1 = addComponent<Cash>("self1", 4, 0.8);

        auto pay   = addComponent<PaymentProcessor>("payment");
        auto walk  = addComponent<traveler>("traveler");

        auto pickup = addComponent<pickup_system>("pickup");

        auto sink_walkin = addComponent<CustomerSink>("sink_walkin");
        auto sink_online = addComponent<CustomerSink>("sink_online");

        // Couplings
        // Generator <-> Distributor
        addCoupling(gen->customerOut, dist->in_customer);
        addCoupling(dist->out_holdOff, gen->holdOff);
        addCoupling(dist->out_okGo,    gen->okGo);

        // Distributor -> lanes
        addCoupling(dist->out_cash0, cash0->in_customer);
        addCoupling(dist->out_cash1, cash1->in_customer);
        addCoupling(dist->out_cash2, cash2->in_customer);
        addCoupling(dist->out_self0, self0->in_customer);
        addCoupling(dist->out_self1, self1->in_customer);

        // lanes -> PaymentProcessor
        addCoupling(cash0->out_toPayment, pay->custIn);
        addCoupling(cash1->out_toPayment, pay->custIn);
        addCoupling(cash2->out_toPayment, pay->custIn);
        addCoupling(self0->out_toPayment, pay->custIn);
        addCoupling(self1->out_toPayment, pay->custIn);

        // lane free signals -> Distributor
        addCoupling(cash0->out_free, dist->in_laneFreed);
        addCoupling(cash1->out_free, dist->in_laneFreed);
        addCoupling(cash2->out_free, dist->in_laneFreed);
        addCoupling(self0->out_free, dist->in_laneFreed);
        addCoupling(self1->out_free, dist->in_laneFreed);

        addCoupling(pay->custOut, walk->custIn);
        addCoupling(walk->custArrived, sink_walkin->in);

        // Online orders: bypass checkout and go directly to pickup system
        addCoupling(dist->out_online, pickup->in_order);
        addCoupling(pickup->finished, sink_online->in);
    }
};

#endif // GROCERY_STORE_HPP
