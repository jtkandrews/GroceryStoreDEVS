#ifndef CUSTOMER_DATA_HPP
#define CUSTOMER_DATA_HPP

#include <ostream>
#include <istream>
#include <string>

struct CustomerData {
    int    customerId    = -1;
    int    numItems      = 0;
    bool   isOnlineOrder = false;   // true = curbside pickup order, false = walk-in
    bool   paymentType   = true;    // true = card/tap, false = cash
    double travelTime    = 0.0;     // used by Traveler + CurbsideDispatcher
    double searchTime    = 0.0;     // used by Packer

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
       << ",pay:" << (c.paymentType ? "card" : "cash")
       << ",travel:" << c.travelTime
       << ",search:" << c.searchTime
       << "}";
    return os;
}

// Only needed if you ever read CustomerData from a text stream/logger.
// Format: id items online(0/1) pay(0/1) travel search
inline std::istream& operator>>(std::istream& is, CustomerData& c) {
    int online = 0;
    std::string payToken;
    if (!(is >> c.customerId >> c.numItems >> online >> payToken >> c.travelTime >> c.searchTime)) {
        return is;
    }

    c.isOnlineOrder = (online != 0);

    if (payToken == "cash" || payToken == "0") {
        c.paymentType = false;
    } else if (payToken == "card" || payToken == "tap" || payToken == "1") {
        c.paymentType = true;
    } else {
        // Fallback for any other numeric-like token
        try {
            c.paymentType = (std::stoi(payToken) != 0);
        } catch (...) {
            c.paymentType = true;
        }
    }

    return is;
}

#endif // CUSTOMER_DATA_HPP
