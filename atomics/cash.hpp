#ifndef CASH_HPP
#define CASH_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <limits>

#include "customer_data.hpp"

using namespace cadmium;

struct CashState{
    enum class Phase{IDLE,BUSY} phase;
    int laneId;
    double timePerItem;
    double sigma;
    CustomerData current;

    CashState(int lane=0, double tpi=1.0)
        : phase(Phase::IDLE), laneId(lane), timePerItem(tpi),
          sigma(std::numeric_limits<double>::infinity()), current() {}
};

class Cash : public Atomic<CashState>{
public:
    Port<CustomerData> in_customer;
    Port<CustomerData> out_toPayment;  // -> PaymentProcessor
    Port<int> out_free;               // -> Distributor (laneId freed)

    Cash(const std::string& id,int lane,double timePerItem=1.0)
        : Atomic<CashState>(id, CashState(lane, timePerItem))
    {
        in_customer  = addInPort<CustomerData>("in_customer");
        out_toPayment= addOutPort<CustomerData>("out_toPayment");
        out_free     = addOutPort<int>("out_free");
    }

    void internalTransition(CashState& s) const override{
        s.phase = CashState::Phase::IDLE;
        s.sigma = std::numeric_limits<double>::infinity();
        s.current = CustomerData();
    }

    void externalTransition(CashState& s,double /*e*/) const override{
        if(s.phase==CashState::Phase::IDLE && !in_customer->empty()){
            s.current = in_customer->getBag().back();
            s.phase = CashState::Phase::BUSY;

            // service time based on items
            s.sigma = (s.current.numItems > 0)
                ? (static_cast<double>(s.current.numItems) * s.timePerItem)
                : s.timePerItem;
        }
    }

    void output(const CashState& s) const override{
        if(s.phase==CashState::Phase::BUSY){
            out_toPayment->addMessage(s.current);
            out_free->addMessage(s.laneId);
        }
    }

    double timeAdvance(const CashState& s) const override{
        if(s.phase==CashState::Phase::IDLE)
            return std::numeric_limits<double>::infinity();
        return s.sigma;
    }
};

#endif