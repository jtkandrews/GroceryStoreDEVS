#ifndef PICKUP_SYSTEM_HPP
#define PICKUP_SYSTEM_HPP

#include <cadmium/modeling/devs/coupled.hpp>

#include "packer.hpp"
#include "curbside_dispatcher.hpp"

using namespace cadmium;

// Coupled model for online order processing: packing and curbside pickup
struct pickup_system : public Coupled {
    // External ports (
    Port<CustomerData> in_order;
    Port<CustomerData> finished;

    pickup_system(const std::string& id) : Coupled(id) {
        in_order = addInPort<CustomerData>("in_order");
        finished = addOutPort<CustomerData>("finished");

        auto pack = addComponent<Packer>("packer", 1.0);
        auto curb = addComponent<CurbsideDispatcher>("curbside");

        // External input -> Packer
        addCoupling(in_order, pack->in_order);

        // Packer output -> Curbside input
        addCoupling(pack->out_packed, curb->orderIn);

        // Curbside output -> External output
        addCoupling(curb->finished, finished);
    }
};

#endif // PICKUP_SYSTEM_HPP
