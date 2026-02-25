# GroceryStoreDEVS – Test Results Summary

Date: 2026-02-25

This document summarizes the simple atomic tests, their outputs, and why each test is important. All tests were executed via `make run-tests` and completed with exit code 0.

---

## 1) Cash Atomic – test_cash
**Purpose:** Verify a cashier lane processes a single customer, outputs to payment, and emits a lane-free signal.

**Input:** [input_data/cash_one_customer.txt](../input_data/cash_one_customer.txt)

**Expected Behavior:**
- Cash lane receives the customer.
- Service time equals `items * timePerItem` (12 items × 1.0 = 12s).
- At service completion, outputs `out_toPayment` and `out_free`.

**Observed Output (summary):**
- At $t=0$, lane enters BUSY with $\sigma=12$.
- At $t=12$, `out_toPayment` and `out_free` are emitted.
- Lane returns to IDLE.

**Why it worked:** The timing matches the expected service time and the correct outputs fired at completion.

---

## 2) Distributor Atomic – test_distributor
**Purpose:** Confirm routing to correct lanes, queue management, and lane-free feedback handling.

**Inputs:**
- [input_data/distributor_customers.txt](../input_data/distributor_customers.txt)
- [input_data/distributor_lane_freed.txt](../input_data/distributor_lane_freed.txt)

**Expected Behavior:**
- Small-item customers prefer self-checkout when available.
- Large-item customers prefer staffed cash lanes.
- Lane freed messages decrement queue and emit `okGo`.

**Observed Output (summary):**
- Customer 1 (5 items) routed to self-checkout lane 3.
- Customer 2 (20 items) routed to cash lane 0.
- Lane-free messages at $t=0.5$ and $t=1.5$ emit `out_okGo` and reduce queue lengths.

**Why it worked:** Routing decisions and queue updates match the rule set, and `okGo` pulses appear on lane-free events.

---

## 3) PaymentProcessor Atomic – test_payment
**Purpose:** Validate queuing and sequential payment processing with variable service times.

**Input:** [input_data/payment_two_customers.txt](../input_data/payment_two_customers.txt)

**Expected Behavior:**
- First customer starts service immediately.
- Second customer queues while busy.
- Outputs occur in order of arrival after respective processing times.

**Observed Output (summary):**
- Customer 1 begins service at $t=0$ and completes first.
- Customer 2 queues and completes after customer 1.

**Why it worked:** The service order and queued processing behavior match expected single-server logic.

---

## 4) Traveler Atomic – test_traveler
**Purpose:** Ensure travel takes a fixed number of steps and emits completion at the right time.

**Input:** [input_data/one_customer.txt](../input_data/one_customer.txt)

**Expected Behavior:**
- Travel begins immediately after input.
- One step per second for 10 steps.
- `custArrived` emitted at $t=10$.

**Observed Output (summary):**
- Traveler transitions to TRAVELING at $t=0$.
- Emits `custArrived` at $t=10$.

**Why it worked:** Timing and step progression match the configured 10-step travel model.

---

## 5) Packer Atomic – test_packer
**Purpose:** Verify that only online orders are packed and that packing time is honored.

**Input:** [input_data/packer_orders.txt](../input_data/packer_orders.txt)

**Expected Behavior:**
- Online order is accepted and packed.
- Walk-in order is ignored.
- Output occurs at `searchTime` for online order (3s).

**Observed Output (summary):**
- Online order accepted at $t=0$, packed at $t=3.5$.
- Walk-in order at $t=0.5$ ignored (no output for it).

**Why it worked:** The model correctly filters non-online orders and uses search time for packing.

---

## 6) CurbsideDispatcher Atomic – test_curbside
**Purpose:** Confirm pickup timing and queueing of online orders.

**Input:** [input_data/curbside_orders.txt](../input_data/curbside_orders.txt)

**Expected Behavior:**
- First order completes after travel time (5s).
- Second order queues and completes after 2s more.

**Observed Output (summary):**
- Order 1 finished at $t=5$.
- Order 2 finished at $t=7$.

**Why it worked:** Service times match travel time and queued order follows immediately.

---

## 7) CustomerSink Atomic – test_customer_sink
**Purpose:** Ensure sink counts received customers.

**Input:** [input_data/sink_customers.txt](../input_data/sink_customers.txt)

**Expected Behavior:**
- Count increments for each input message.

**Observed Output (summary):**
- Count increments to 1 at $t=0$ and to 2 at $t=0.2$.

**Why it worked:** Sink state reflects the total number of inputs received.

---

## 8) Generator Atomic – test_generator
**Purpose:** Validate generator behavior with control signals (hold/resume) and custom parameters.

**Inputs:**
- [input_data/input_holdOff.txt](../input_data/input_holdOff.txt)
- [input_data/input_okGo.txt](../input_data/input_okGo.txt)

**Expected Behavior:**
- **Test 1 (control cycle):** generator starts RUNNING, produces output, pauses on `holdOff`, resumes on `okGo`, and `holdOff` takes precedence if both arrive.
- **Test 2 (custom params):** faster arrivals, all online orders, no card payments.

**Observed Output (summary):**
- Test 1 shows output at start, silence during the pause window, and output resuming afterward.
- Test 2 shows multiple outputs within 60s, all with `isOnlineOrder=true` and `paymentType=false`.

**Why it worked:** Control ports and constructor parameters are exercised directly, demonstrating both pause/resume logic and parameterized generation.

**Determinism note:** The generator test uses a fixed RNG seed so outputs are repeatable across runs.

---

## 9) Coupled Checkout – test_coupled_checkout
**Purpose:** Validate the walk‑in checkout chain as a coupled model (Distributor → Cash → Payment → Traveler).

**Input:** [input_data/checkout_customers.txt](../input_data/checkout_customers.txt)

**Expected Behavior:**
- Walk‑in customers route to lanes, complete scanning, pay, and arrive via Traveler.

**Observed Output (summary):**
- Lane outputs and `out_free` pulses are followed by Payment and Traveler events.

**Why it worked:** It exercises multiple atomics together and shows correct handoff between stages.

---

## 10) Coupled Online Flow – test_coupled_online
**Purpose:** Validate online orders bypass checkout and flow through Packer → Curbside.

**Input:** [input_data/online_customers.txt](../input_data/online_customers.txt)

**Expected Behavior:**
- Distributor emits `out_online`, packer processes, curbside finishes after travel time.

**Observed Output (summary):**
- Online orders skip lanes and are handled by packing and curbside stages.

**Why it worked:** Confirms the modern online‑order path as a coupled subsystem.

---

## 11) Full System (Deterministic) – test_full_system
**Purpose:** End‑to‑end integration test using file‑driven inputs instead of random generator.

**Input:** [input_data/full_system_customers.txt](../input_data/full_system_customers.txt)

**Expected Behavior:**
- Walk‑ins go through checkout and traveler.
- Online orders bypass checkout to packing/curbside.

**Observed Output (summary):**
- Both paths appear in the log with consistent ordering and timing.

**Why it worked:** Provides a reproducible full‑system test without stochastic arrivals.

---

## 9) One-Customer Integration – test_one_customer
**Purpose:** Validate the core walk-in flow from distributor → lane → payment → traveler.

**Input:** [input_data/one_customer.txt](../input_data/one_customer.txt)

**Expected Behavior:**
- Routed to a lane (self-checkout for 5 items).
- Payment completes, then traveler finishes in 10 steps.

**Observed Output (summary):**
- Routed to self-checkout lane 3.
- Payment completes, traveler emits arrival at ~$t=99$.

**Why it worked:** The full walk-in path is exercised and the final arrival occurs after expected processing and travel delays.

---

## 10) Pickup System Coupled – test_pickup_system
**Purpose:** Validate the hierarchical coupling of Packer and CurbsideDispatcher as a single pickup_system subsystem. Tests internal coordination between packing orders and curbside dispatch.

**Input:** [input_data/packer_orders.txt](../input_data/packer_orders.txt)

**Expected Behavior:**
- Online orders arrive at packer input port.
- Packer begins packing with $\sigma=3$ (3 seconds per order).
- Packed orders flow to curbside dispatcher.
- Curbside dispatcher immediately processes and outputs finished customers.
- Both models remain synchronized via internal coupling.

**Observed Output (summary):**
- $t=0$: Order 1 (6 items, online, card) arrives at packer; packer enters PACKING with $\sigma=3$.
- $t=0.5$: Order 2 (4 items, walk-in, cash) arrives at packer input.
- $t=3.5$: Packer completes and outputs Order 1 to curbside.
- $t=3.5$: Curbside receives order, immediately processes it (dispatch time is minimal), and emits FINISHED port.
- $t=3.5$: Sink receives completed customer; packer returns to IDLE.

**Key Observations:**
- Internal coupling between `packer->out_packed` and `curbside->orderIn` works correctly.
- External ports `in_order` and `finished` properly expose the subsystem interface.
- Order flow is smooth: no buffering delays between packer output and curbside input.

**Why it worked:** The pickup_system coupled model correctly encapsulates the packer and curbside dispatcher, preserving their interaction while exposing clean input/output ports for top-level composition.
