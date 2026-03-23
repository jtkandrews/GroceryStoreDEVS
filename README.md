# Grocery Store - DEVS Simulation

**Author:** Grant  
**Course:** SYSC4906G

## Overview
This project models a modern grocery store as a DEVS simulation with both walk-in and online (curbside) customer flows.

Walk-in customers follow checkout behavior (lane selection, payment, and exit travel), while online customers bypass checkout and are processed by a dedicated pickup subsystem.

### Model Hierarchy
* **Top Model:** Grocery Store
* **Coupled Models:**
  * `grocery_store`
  * `pickup_system`
* **Atomic Models:**
  * `Generator`
  * `Distributor`
  * `Cash`
  * `PaymentProcessor`
  * `traveler`
  * `Packer`
  * `CurbsideDispatcher`
  * `CustomerSink`

## File Organization
* **`atomics/`**: Atomic DEVS models (`.hpp`)
  * `generator.hpp`, `distributor.hpp`, `cash.hpp`, `payment_processor.hpp`, `traveler.hpp`, `packer.hpp`, `curbside_dispatcher.hpp`, `customer_sink.hpp`
* **`coupled/`**: Coupled DEVS models (`.hpp`)
  * `pickup_system.hpp`
  * `grocery_store.hpp`
  * `grocery_store_test.hpp`
* **`top_model/`**: Simulation entry point
  * `main.cpp`
* **`test/`**: Test benches for atomic/coupled/full-system behavior
* **`input_data/`**: Input files used by deterministic tests
* **`CMakeLists.txt`**: CMake build targets and include paths
* **`build.sh`**: Clean configure/build helper script
* **`bin/`**: Compiled executables

## Prerequisites
To compile and run this project:
* C++17 compiler (`g++`)
* **Cadmium** DEVS library
* `cmake` (3.14+)
* `make`
* `bash` (for `build.sh`)

> Cadmium is expected as a sibling folder to this repository:
> `../cadmium_v2/include`

## Compilation
This project supports both CMake and Makefile workflows.

### Recommended (clean build script)
```bash
./build.sh
```

What this script does:
* Removes any previous `build/` directory and old `.csv` files
* Reconfigures with `cmake ..`
* Compiles all targets with `make`
* Places executables in `bin/`

### Manual CMake build
```bash
mkdir -p build
cd build
cmake ..
make
cd ..
```

### Makefile build
```bash
make
```

## Testing
After building, run executables from `bin/` directly (or use the Makefile run targets).

### Main simulation
* `./bin/grocery_sim`
* or `make run_grocery_sim`

### Atomic tests
* `./bin/test_cash`
* `./bin/test_payment`
* `./bin/test_traveler`
* `./bin/test_distributor`
* `./bin/test_packer`
* `./bin/test_curbside`
* `./bin/test_customer_sink`
* `./bin/test_generator`

### Coupled / integration tests
* `./bin/test_pickup_system`
* `./bin/test_one_customer`
* `./bin/test_full_system`

## Inputs and Logs
* Deterministic test inputs are in `input_data/`
* Optional simulation/test logs can be written to `simulation_results/`

Example:
```bash
./bin/grocery_sim > simulation_results/main_run.log
```
