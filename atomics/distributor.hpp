#ifndef DISTRIBUTOR_HPP
#define DISTRIBUTOR_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <vector>
#include <limits>

using namespace cadmium;

struct DistributorState {
    enum class Phase { IDLE, SEND } phase;
    enum class Decision { NONE, ROUTE, HOLD, OK } decision;

    int selectedLane;
    std::vector<int> queues;
    int numRegisters;
    int maxQueue;

    DistributorState(int n=2, int m=2)
        : phase(Phase::IDLE),
          decision(Decision::NONE),
          selectedLane(-1),
          queues(n,0),
          numRegisters(n),
          maxQueue(m) {}
};

class Distributor : public Atomic<DistributorState> {
public:

    Port<int> in_customer;      // customer id
    Port<int> in_cashFreed;     // lane id freed

    Port<int>  out_lane;
    Port<bool> out_hold;
    Port<bool> out_ok;

    Distributor(const std::string& id,int n=2,int m=2)
    : Atomic<DistributorState>(id,DistributorState(n,m))
    {
        in_customer = addInPort<int>("in_customer");
        in_cashFreed = addInPort<int>("in_cashFreed");

        out_lane = addOutPort<int>("out_lane");
        out_hold = addOutPort<bool>("out_hold");
        out_ok   = addOutPort<bool>("out_ok");
    }

    void internalTransition(DistributorState& s) const override {
        s.phase = DistributorState::Phase::IDLE;
        s.decision = DistributorState::Decision::NONE;
    }

    void externalTransition(DistributorState& s,double) const override {

        if(!in_cashFreed->empty()){
            for(int id: in_cashFreed->getBag()){
                if(id>=0 && id<s.numRegisters && s.queues[id]>0)
                    s.queues[id]--;
            }
            s.decision = DistributorState::Decision::OK;
            s.phase = DistributorState::Phase::SEND;
        }

        if(!in_customer->empty()){
            for(int : in_customer->getBag()){
                int best=-1;
                int bestLen=999999;

                for(int i=0;i<s.numRegisters;i++){
                    if(s.queues[i]<s.maxQueue && s.queues[i]<bestLen){
                        best=i;
                        bestLen=s.queues[i];
                    }
                }

                if(best>=0){
                    s.selectedLane=best;
                    s.queues[best]++;
                    s.decision=DistributorState::Decision::ROUTE;
                }else{
                    s.decision=DistributorState::Decision::HOLD;
                }
                s.phase=DistributorState::Phase::SEND;
            }
        }
    }

    void output(const DistributorState& s) const override {
        if(s.phase!=DistributorState::Phase::SEND) return;

        if(s.decision==DistributorState::Decision::ROUTE)
            out_lane->addMessage(s.selectedLane);
        else if(s.decision==DistributorState::Decision::HOLD)
            out_hold->addMessage(true);
        else if(s.decision==DistributorState::Decision::OK)
            out_ok->addMessage(true);
    }

    double timeAdvance(const DistributorState& s) const override {
        if(s.phase==DistributorState::Phase::IDLE)
            return std::numeric_limits<double>::infinity();
        return 0.0;
    }
};

#endif