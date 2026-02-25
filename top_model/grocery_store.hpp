#ifndef GROCERY_STORE_HPP
#define GROCERY_STORE_HPP

#include <cadmium/modeling/devs/coupled.hpp>
#include <memory>
#include <string>

// atomics
#include "../atomics/customer_data.hpp"
#include "../atomics/distributor.hpp"
#include "../atomics/cash.hpp"
#include "../atomics/packer.hpp"

// these are YOUR existing files (adjust include names if needed)
#include "../atomics/generator.h"
#include "../atomics/payment_processor.h"
#include "../atomics/traveler.h"
#include "../atomics/curbside_dispatcher.h"

using namespace cadmium;

// Keep these consistent with distributor.hpp lane counts
static constexpr int CASH_LANES = 3;
static constexpr int SELF_LANES = 2;

// A simple “top” coupled model that wires everything together.
class GroceryStore : public Coupled {
public:
    GroceryStore(const std::string& id) : Coupled(id) {

        // --- Components ---
        auto gen   = addComponent<Generator>("gen");
        auto dist  = addComponent<Distributor>("dist");

        // 5 lanes: 0..2 = staffed cash, 3..4 = self checkout
        auto cash0 = addComponent<Cash>("cash0", 0, 1.0); // laneId, timePerItem
        auto cash1 = addComponent<Cash>("cash1", 1, 1.0);
        auto cash2 = addComponent<Cash>("cash2", 2, 1.0);

        auto self0 = addComponent<Cash>("self0", 3, 0.7); // self checkout faster/slower as you like
        auto self1 = addComponent<Cash>("self1", 4, 0.7);

        auto pay   = addComponent<PaymentProcessor>("pay");
        auto trav  = addComponent<Traveler>("trav");

        auto pack  = addComponent<Packer>("pack", 1.0); // packTimePerItem (fallback)
        auto disp  = addComponent<CurbsideDispatcher>("disp");

        // --- Couplings ---

        // Generator -> Distributor
        addCoupling(gen->customerOut, dist->in_customer);

        // Distributor feedback -> Generator
        addCoupling(dist->out_holdOff, gen->holdOff);
        addCoupling(dist->out_okGo,    gen->okGo);

        // Distributor -> each lane (CustomerData goes straight to chosen lane port)
        addCoupling(dist->out_cash0, cash0->in_customer);
        addCoupling(dist->out_cash1, cash1->in_customer);
        addCoupling(dist->out_cash2, cash2->in_customer);
        addCoupling(dist->out_self0, self0->in_customer);
        addCoupling(dist->out_self1, self1->in_customer);

        // Each lane -> Distributor (lane freed)
        addCoupling(cash0->out_free, dist->in_laneFreed);
        addCoupling(cash1->out_free, dist->in_laneFreed);
        addCoupling(cash2->out_free, dist->in_laneFreed);
        addCoupling(self0->out_free, dist->in_laneFreed);
        addCoupling(self1->out_free, dist->in_laneFreed);

        // All lanes -> PaymentProcessor
        addCoupling(cash0->out_toPayment, pay->custForPayment);
        addCoupling(cash1->out_toPayment, pay->custForPayment);
        addCoupling(cash2->out_toPayment, pay->custForPayment);
        addCoupling(self0->out_toPayment, pay->custForPayment);
        addCoupling(self1->out_toPayment, pay->custForPayment);

        // PaymentProcessor -> Traveler
        addCoupling(pay->custPaid, trav->custIn);

        // Traveler -> Packer (Packer will ignore non-online orders per your earlier design)
        addCoupling(trav->custArrived, pack->in_order);

        // Packer -> Dispatcher
        addCoupling(pack->out_packed, disp->orderIn);

        // Dispatcher -> (top output)
        // If your Coupled base supports top-level output ports, you can expose it.
        // If not, you can just rely on logs.
        //
        // Example (optional): create an output port on this coupled model and connect it.
        // out_done = addOutPort<CustomerData>("out_done");
        // addCoupling(disp->customerFinished, out_done);
    }
};

#endif // GROCERY_STORE_HPP