#ifndef PACKER_HPP
#define PACKER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>

using namespace cadmium;

struct PackerState{
    enum class Phase{IDLE,PACKING} phase;
    double packTime;
};

#include <iostream>

inline std::ostream& operator<<(std::ostream& os, const PackerState& s) {
    os << "busy:" << s.busy
       << " queue:" << s.queueSize
       << " lastCustomer:" << s.lastCustomerId;
    return os;
}

class Packer : public Atomic<PackerState>{
public:

    Port<int> in_order;
    Port<int> out_ready;

    Packer(const std::string& id,double t=3.0)
    : Atomic<PackerState>(id,{PackerState::Phase::IDLE,t}){

        in_order=addInPort<int>("in_order");
        out_ready=addOutPort<int>("out_ready");
    }

    void internalTransition(PackerState& s) const override{
        s.phase=PackerState::Phase::IDLE;
    }

    void externalTransition(PackerState& s,double) const override{
        if(s.phase==PackerState::Phase::IDLE && !in_order->empty())
            s.phase=PackerState::Phase::PACKING;
    }

    void output(const PackerState& s) const override{
        if(s.phase==PackerState::Phase::PACKING)
            out_ready->addMessage(1);
    }

    double timeAdvance(const PackerState& s) const override{
        if(s.phase==PackerState::Phase::IDLE)
            return std::numeric_limits<double>::infinity();
        return s.packTime;
    }
};

#endif