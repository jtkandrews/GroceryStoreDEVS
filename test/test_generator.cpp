#include <iostream>
#include <string>
#include <cadmium/modeling/devs/coupled.hpp>
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/stdout.hpp>
#include <cadmium/lib/iestream.hpp>
#include "generator.hpp"

using namespace cadmium;

// ─── Test 1: Default RUNNING + hold/resume cycle ──────────────────────────────
// Wraps Generator with IEStream readers for both control ports.
struct topTestGenerator : public Coupled {
    Port<CustomerData> outCustomerTest;

    topTestGenerator(const std::string& id)
        : Coupled(id),
          outCustomerTest(addOutPort<CustomerData>("outCustomerTest"))
    {
        // Input readers driven by text files (format: "<time> <value>\n")
        auto holdOffReader = addComponent<cadmium::lib::IEStream<bool>>(
            "holdOffReader", "input_data/input_holdOff.txt");
        auto okGoReader    = addComponent<cadmium::lib::IEStream<bool>>(
            "okGoReader",    "input_data/input_okGo.txt");

        // Generator under test (default params: arrivalMean=60s)
        auto gen = addComponent<Generator>("gen",
                           60.0, 300.0, 60.0, 120.0, 0.30, 0.70,
                           42u);

        // EIC – wire readers to Generator control ports
        addCoupling(holdOffReader->out, gen->holdOff);
        addCoupling(okGoReader->out,    gen->okGo);

        // EOC – expose Generator output to test harness
        addCoupling(gen->customerOut, outCustomerTest);
    }
};


// Test 2: Custom generator parameters
// Verify Generator respects constructor params:
//   arrivalMean=10s (faster arrivals), onlineProb=1.0 (all online orders),
//   cardProb=0.0 (no tap-card payments).
// Expect: many CustomerData events in 60s, all with isOnlineOrder=true, card=false.
struct topTestCustomParams : public Coupled {
    Port<CustomerData> outCustomerTest;

    topTestCustomParams(const std::string& id)
        : Coupled(id),
          outCustomerTest(addOutPort<CustomerData>("outCustomerTest"))
    {
        // No external control signals needed – generator runs freely
        auto gen = addComponent<Generator>(
            "genCustom",
            /*arrivalMean=*/  10.0,
            /*travelMean=*/   300.0,
            /*travelStdDev=*/ 60.0,
            /*searchMean=*/   120.0,
            /*onlineProb=*/   1.0,   // all orders online
            /*cardProb=*/     0.0,   // no tap-card payments
            /*seed=*/         42u
        );

        addCoupling(gen->customerOut, outCustomerTest);
    }
};

// ─── main ─────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "=== Test 1: RUNNING start, holdOff/okGo cycle, precedence ===" << std::endl;
    {
        auto testSystem = std::make_shared<topTestGenerator>("testGeneratorSystem");
        auto rc = cadmium::RootCoordinator(testSystem);
        rc.setLogger<cadmium::STDOUTLogger>();
        rc.start();
        rc.simulate(600.0);
        rc.stop();
    }
    std::cout << "Test 1 complete. Verify in log:\n"
              << "  - CustomerData output at t=0 (id=0)\n"
              << "  - ~3-4 outputs between t=0 and t=180\n"
              << "  - No output between t=180 and t=360 (PAUSED)\n"
              << "  - Output resumes after t=360\n"
              << "  - No output after t=480 (holdOff wins over okGo)\n" << std::endl;


    std::cout << "=== Test 2: Custom params – fast arrivals, all online, no card ===" << std::endl;
    {
        auto testSystem = std::make_shared<topTestCustomParams>("testCustomParamSystem");
        auto rc = cadmium::RootCoordinator(testSystem);
        rc.setLogger<cadmium::STDOUTLogger>();
        rc.start();
        rc.simulate(60.0);
        rc.stop();
    }
    std::cout << "Test 3 complete. Verify in log:\n"
              << "  - ~6 CustomerData events in 60s (arrivalMean=10s)\n"
              << "  - All CustomerData have isOnlineOrder=true, card=false\n"
              << "  - items in [1,40], travel >= 0, search >= 0\n" << std::endl;

    return 0;
}