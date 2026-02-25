# GroceryStoreDEVS – Main Simulation Run Summary (300s)

Date: 2026-02-25

This report explains the `make run` output from the 300s full simulation and highlights why the behavior indicates a correct end‑to‑end run with **online orders bypassing checkout**.

---

## 1) What this output represents
Each log line is a state change or output event from an atomic model. The core flow is:

**Generator → Distributor → Cash/Self → Payment → Traveler → Walk‑in Sink**

**Generator → Distributor → Packer → Curbside → Online Sink**

---

## 2) Evidence in the output

### A) Online orders skip checkout
At $t=0$, customer id 0 is **online=1** and the Distributor emits `out_online` (not a lane output):
- `out_online` fired at $t=0$ for id 0
- Packer immediately enters PACKING with `sigma=115.729`

This confirms online orders bypass the checkout lanes and payment.

### B) Walk‑in customers go through lanes and payment
At $t=177.078$, customer id 2 is **online=0** and is routed to cash lane 0:
- Cash lane enters BUSY with `sigma=27` (27 items)
- At $t=204.078`, cash lane outputs `out_toPayment` and `out_free`
- Payment processes the customer, then Traveler starts and completes after 10 seconds
- `sink_walkin` count increments to 1 at $t=221.963`

This shows the full walk‑in path is working (lane → payment → traveler → sink).

### C) Payment type affects timing
You can see both **card** and **cash** payments in the log (e.g., id 2 card, id 5 cash). The payment processor’s `sigma` varies based on payment type, which matches the model’s random timing distributions.

### D) Queue and lane‑free feedback
When a lane completes service, `out_free` triggers Distributor’s `out_okGo`, freeing capacity and allowing new arrivals to be accepted. This feedback loop appears repeatedly and keeps the queues stable.

### E) Online orders reach curbside later
At $t=191.51`, the first packed online order is sent to curbside and curbside enters BUSY with `sigma=352.986`. Because travel times are large, `sink_online` remains 0 during the 300s run. This is expected for this short window.

---

## 3) Why the run is correct
- **Generator** outputs appear at stochastic times (e.g., 0, 75.7806, 177.078), matching exponential inter‑arrival behavior.
- **Distributor** routes online orders directly to Packer and offline orders to lanes based on item counts.
- **Cash/Self** service times match `items × timePerItem` (e.g., 27 items → 27 seconds).
- **Payment** processes customers in order and uses variable service times tied to card/cash.
- **Traveler** completes walk‑ins after 10 seconds, and **sink_walkin** counts increase accordingly.
- **Packer/Curbside** handle online orders independently of checkout.

---

## 4) Summary
This 300s run shows both workflows functioning:
- Online orders bypass checkout and go directly to packing/curbside.
- Walk‑ins use lanes, payment, and traveler, then increment the walk‑in sink.

The observed events in the log match the intended DEVS model behavior.
