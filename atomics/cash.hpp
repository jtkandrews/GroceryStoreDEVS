#ifndef CASH_HPP
#define CASH_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>

using namespace cadmium;

struct CashState{
    enum class Phase{IDLE,BUSY} phase;
    int laneId;
    double serviceTime;
};

class Cash : public Atomic<CashState>{
public:

    Port<int> in_customer;
    Port<int> out_done;
    Port<int> out_free;

    Cash(const std::string& id,int lane,double t=2.0)
    : Atomic<CashState>(id,{CashState::Phase::IDLE,lane,t}){

        in_customer=addInPort<int>("in_customer");
        out_done=addOutPort<int>("out_done");
        out_free=addOutPort<int>("out_free");
    }

    void internalTransition(CashState& s) const override{
        s.phase=CashState::Phase::IDLE;
    }

    void externalTransition(CashState& s,double) const override{
        if(s.phase==CashState::Phase::IDLE && !in_customer->empty()){
            s.phase=CashState::Phase::BUSY;
        }
    }

    void output(const CashState& s) const override{
        if(s.phase==CashState::Phase::BUSY){
            out_done->addMessage(1);
            out_free->addMessage(s.laneId);
        }
    }

    double timeAdvance(const CashState& s) const override{
        if(s.phase==CashState::Phase::IDLE)
            return std::numeric_limits<double>::infinity();
        return s.serviceTime;
    }
};

#endif