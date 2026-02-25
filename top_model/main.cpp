#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include <memory>

#include "grocery_store.hpp"

int main() {
    auto model = std::make_shared<GroceryStore>("store");

    auto rootCoordinator = cadmium::RootCoordinator(model);

    // logs to CSV (you can change filename + delimiter)
    auto logger = std::make_shared<cadmium::CSVLogger>("simulation_results/log.csv", ";");
    rootCoordinator.setLogger(logger);

    rootCoordinator.start();
    rootCoordinator.simulate(std::numeric_limits<double>::infinity());
    rootCoordinator.stop();

    return 0;
}