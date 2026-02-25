# GroceryStoreDEVS
Assignment 1 (SYSC4906G) — Grocery store DEVS model in Cadmium.

## Overview
This project models a modern grocery store with walk‑in and online (curbside) orders. Online orders bypass checkout and go directly to packing and curbside pickup.

## Requirements
- C++17 compiler (g++)
- Cadmium headers installed at `$HOME/cadmium_v2` or set `CADMIUM_INCLUDE` when running make

## Build
Run the main simulator build:

```
make
```

## Run main simulation

```
make run
```

## Run tests

```
make run-tests
```

## Inputs
Input files for tests live in input_data/.

## Logs
Saved logs are under simulation_results/:
- main_run.log
- tests_run.log

## Notes
The simulation run time is set in top_model/main.cpp. You can shorten or extend it by changing the value passed to `root.simulate(...)`.
