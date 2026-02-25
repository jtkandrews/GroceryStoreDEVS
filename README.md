# GroceryStoreDEVS
Assignment 1 (SYSC4906G) — Grocery store DEVS model in Cadmium.

## Overview
This project models a modern grocery store with walk‑in and online (curbside) orders. Online orders bypass checkout and go directly to packing and curbside pickup.

## Requirements
- C++17 compiler (g++)
- Ensure the `CADMIUM_INCLUDE` in make file has a path that works properly for your cadmium installation, currently setup to work when both cadmium and project sit at the same level

## Build
Run the main simulator build:

```
make
```

## Run main simulation

```
make run
```

## Build Tests
Run the tests build:

```
make tests
```

## Run tests

```
make run-tests
```

## Tests

### Atomic Tests
Nine atomic unit tests validate individual model components:
- **test_cash**: Cash register lane processing
- **test_payment**: Payment processor for card transactions
- **test_traveler**: Traveler (walk-out) model
- **test_distributor**: Customer distribution logic
- **test_packer**: Online order packing
- **test_curbside**: Curbside pickup dispatcher
- **test_customer_sink**: Customer sink/exit points
- **test_generator**: Customer generator with stochastic arrivals
- **test_one_customer**: Single customer through full system

### Coupled Subsystem Tests
Two coupled tests validate interaction between model subsystems:
- **test_coupled_checkout**: Walk-in customer flow through distributor → cash lanes → payment processor → traveler
  - Input: `input_data/checkout_customers.txt`
  - Tests lane selection logic and payment processing chain
- **test_coupled_online**: Online customer flow through distributor → packer → curbside dispatcher
  - Input: `input_data/online_customers.txt`
  - Validates online bypass of checkout lanes

### Full System Test
- **test_full_system**: Complete integration with deterministic customer mix (walk-in and online)
  - Input: `input_data/full_system_customers.txt`
  - Validates end-to-end routing and correct separation of customer paths

Run all tests with `make run-tests`. Each test auto-discovers input files from `input_data/` and logs to stdout.

## Inputs
Input files for tests live in input_data/:
- **one_customer.txt**: Single walk-in customer
- **cash_one_customer.txt**: Single cash payment customer
- **payment_two_customers.txt**: Two card payment customers
- **checkout_customers.txt**: Three walk-in customers for coupled checkout test
- **online_customers.txt**: Two online customers for coupled online test
- **full_system_customers.txt**: Mixed walk-in and online customers for full integration test
- Additional atomic test inputs for generator, distributor, packer, curbside, and sink models

## Logs
Saved logs are under simulation_results/:
- main_run.log (main simulation output)
- tests_run.log (all atomic and coupled test output)

## Notes
The simulation run time is set in top_model/main.cpp. You can shorten or extend it by changing the value passed to `root.simulate(...)`.
