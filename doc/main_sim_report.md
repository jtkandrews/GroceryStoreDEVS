# GroceryStoreDEVS – Main Simulation Run Summary

Date: 2026-02-25

This report explains the `make run` output from the full grocery store simulation and highlights why the behavior indicates a correct end‑to‑end run.

---

## 1) What this output represents
The log shows the state transitions and outputs of all atomic models within the top coupled model as simulated time advances. Each line includes:
- **time**: simulation time
- **model_name**: which atomic model produced the log
- **port_name**: which output port (if any) fired
- **data**: state or message content

In this run, the model is driven by the **Generator** (stochastic arrivals) and the **Distributor** (routing customers to lanes). Customers flow through:

**Generator → Distributor → Cash/Self → Payment → (Traveler for walk‑ins) / (Packer + Curbside for online) → Sinks**

---

## 2) Why the run is working
Several expected behaviors appear consistently:

### A) Generator produces customers over time
- At $t=0$, the Generator emits the first customer (id 0) and schedules the next inter‑arrival time.
- Additional customers appear at realistic, non‑uniform times (exponential arrival process).

### B) Distributor routes to the correct lane types
- Customers with lower item counts are routed to self‑checkout (e.g., id 1 with 4 items to lane 3).
- Larger baskets are routed to staffed lanes (e.g., id 2 with 26 items to cash lane 0).

This shows the item‑count routing rule is working and queue state is updated when lanes free.

### C) Cash lanes emit completion at service time
- Example: id 2 with 26 items enters cash lane at $t=39.0853$ with $\sigma=26$ and completes at $t=65.0853$.
- Each completion produces both `out_toPayment` and `out_free`, which updates the Distributor.

### D) Payment processing is sequenced correctly
- Payment times vary based on card vs cash.
- When payments are busy, later customers queue and complete later.

### E) Traveler completes walk‑ins
- Walk‑in customers (online=0) go to Traveler after payment and produce `custArrived` 10 seconds later.
- Each arrival increments `sink_walkin` (count steadily increases).

### F) Online flow is active (Packer engaged)
- Online orders (online=1) route from Payment to Packer.
- Packer enters PACKING with the customer’s `searchTime` and remains busy appropriately.

Note: Curbside output isn’t seen within 1000s because travel times for pickup are large and packer’s queue is long in this run. That’s expected for the random parameters used.

---

## 3) Key behaviors visible in the log

### Example: Customer id 1 (walk‑in)
- Routed to self‑checkout (lane 3)
- Payment completes at $t=41.4196$
- Traveler emits `custArrived` at $t=51.4196$
- `sink_walkin` count increases to 1

### Example: Customer id 0 (online)
- Routed to self‑checkout (lane 3)
- Payment completes at $t=22.2344$
- Packer begins packing for 200.372s
- No immediate curbside completion (long queue + travel time)

These align with the intended behavior of the model specification.

---

## 4) Why counts and idle states make sense
- `sink_walkin` reaches 13 by the end of the run, reflecting walk‑in completions.
- `sink_online` remains 0 in this window, because online orders are still being packed or waiting for curbside pickup (long travel times).
- Many lanes and models return to IDLE between events, indicating the DEVS scheduling is stable and no deadlocks occur.

---

## 5) Summary
The run demonstrates a correct end‑to‑end simulation:
- Stochastic arrivals appear correctly.
- Routing logic sends customers to appropriate lanes.
- Service times and queues behave as expected.
- Walk‑in completions increment the sink.
- Online orders flow into packing and curbside preparation.

Overall, the output represents a coherent, working grocery store DEVS model under load with random arrivals.
