#include <iostream>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>

#include "grocery_store.hpp"

int main() {
    auto model = std::make_shared<grocery_store>("grocery_store_simulation");

    cadmium::RootCoordinator root(model);
    root.setLogger<cadmium::STDOUTLogger>();

    root.start();
    root.simulate(300.0); // seconds of simulated time
    root.stop();

    std::cout << "Grocery store simulation completed." << std::endl;
    return 0;
}
