#ifndef CUSTOMER_DATA_HPP
#define CUSTOMER_DATA_HPP

#include <ostream>
#include <istream>

struct CustomerData {
    int    customerId   = -1;
    int    numItems     = 0;
    bool   isOnlineOrder= false;
    bool   paymentType  = true;   // true = card/tap, false = cash
    double travelTime   = 0.0;    // used by Traveler + CurbsideDispatcher
    double searchTime   = 0.0;    // used by Packer (and/or store time)

    CustomerData() = default;

    CustomerData(int id, int items, bool online, bool payType, double travel, double search)
        : customerId(id),
          numItems(items),
          isOnlineOrder(online),
          paymentType(payType),
          travelTime(travel),
          searchTime(search) {}
};

inline std::ostream& operator<<(std::ostream& os, const CustomerData& c) {
    os << "{id:" << c.customerId
       << ",items:" << c.numItems
       << ",online:" << (c.isOnlineOrder ? "1" : "0")
       << ",payType:" << (c.paymentType ? "card" : "cash")
       << ",travel:" << c.travelTime
       << ",search:" << c.searchTime
       << "}";
    return os;
}

// Optional: only needed if you ever use IEStream<CustomerData> from a text file.
// Format: id items online(0/1) payType(0/1) travel search
inline std::istream& operator>>(std::istream& is, CustomerData& c) {
    int online = 0, pay = 1;
    is >> c.customerId >> c.numItems >> online >> pay >> c.travelTime >> c.searchTime;
    c.isOnlineOrder = (online != 0);
    c.paymentType = (pay != 0);
    return is;
}

#endif // CUSTOMER_DATA_HPP