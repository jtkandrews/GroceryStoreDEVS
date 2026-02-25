#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <limits>
#include <memory>

#include "grocery_store.hpp"

int main() {
    auto model = std::make_shared<GroceryStore>("GroceryStore");

    cadmium::RootCoordinator root(model);

    auto logger = std::make_shared<cadmium::STDOUTLogger>();
    root.setLogger(logger);

    root.start();
    root.simulate(100.0);   // simulate 100 seconds (adjust)
    root.stop();

    return 0;
}