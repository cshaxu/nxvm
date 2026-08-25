# T467 S2 Prior-Controller L2 Capability Reconciliation

## Decision rule

This owner-approved S2 extension corrects terminology in the retained
PIC/DMA/PIT/RTC/KBC ledgers without changing their completed chip contracts.
An existing construction-only path that can copy and consume a
source-qualified board value is a **Board-L3 capability now**.  A generic or
unproven value flowing through it is **Current data L2**, not an L2 interface.
An item stays L2 only when there is no complete source-qualified input and
sole Core consumer, or when the source deliberately does not define a digital
Core fact.  No L4 electrical fact is relabelled.

## Reconciled retained boundaries

| Controller and retained item | Current path | Correct disposition now | Required treatment |
| --- | --- | --- | --- |
| PIC elapsed request-to-CPU visibility (PIC-B1) | PIC owns IRR/ISR/INTA; scheduler refreshes it and CPU consumes the existing logical acknowledgement. | **L2, correctly retained.** Intel specifies causal order but no elapsed Core term; no numeric field should be invented. | Keep the direct causal L3 path. A future source-defined board visibility contract would need its own typed construction plan. |
| PIC selected topology (PIC-B2) | Immutable AT wiring is composed into the paired PIC owner. | Manual L3 already; no timing input is missing. | Do not add a profile callback or duplicate selector. |
| DMA clock and service phase (DMA-B1/B2) | Profile clock plan -> validated copied Core plan -> `dma_clock` -> scheduler -> `dma.c`. | **Board-L3 capability.** Model 339 already selects qualified clock/service data; generic 1/1 data remains Current data L2. | Retain the one plan and its rule validation; an exact ratio never upgrades an unqualified premise. |
| DMA refresh/DRQ and transaction integration (DMA-B3/B4) | Existing fixed PIT1 request binding and Core transaction owner. | Selected fixed route is L3; arbitrary board arbitration/DRQ timing is **L2** where a full board producer is absent. | No generic field or DMA scheduler: a later board topology must own its source-qualified dynamic endpoint. |
| PIT input clock (PIT-B1) | Profile clock plan -> validated copied Core plan -> `pit_clock` -> scheduler -> `pit.c`. | **Board-L3 capability.** Model 339's qualified ratio is L3; generic 1/4 is Current data L2. | Retain the existing construction plan and reset replay. |
| PIT reset phase (PIT-B3) | Existing copied `clock_ratio.reset_phase` is restored by the Core clock domain. | **Board-L3 capability.** The input exists; current selected phase has no source qualification and is Current data L2. | Accept a source-qualified phase through the same plan when one exists; do not manufacture a power-up phase from an unspecified manual fact. |
| PIT routes/electrical facts (PIT-B2/B4) | Core board composition owns OUT0/OUT1/OUT2 routes. | Selected logical routes are L3; pin/electrical tables remain L4, not an interface deficit. | Keep routes composition-private. |
| RTC divider/update timing | Profile -> copied `core_machine_rtc_timing_plan` -> sole RTC owner. | **Board-L3 capability.** Model 339's `L3_SOURCE` plan is L3; default PC/AT's `L2_RATIO` is Current data L2. | Existing plan is the sole receiver; retain provenance and do not introduce host time. |
| RTC VRT/persistence/board CPU visibility | RTC owns 64-byte state; board owns power/persistence policy and PIC/CPU observation. | **L2, correctly retained.** No selected board policy/consumer contract exists; it is not a missing RTC setter. | Admit only with an owned board policy and a complete persistence/visibility lifecycle. |
| KBC cadence values (reply, serial delivery, typematic) | Profile contract -> copied `core_machine_config` fields and `clock_plan.kbc` -> `machine.c` construction calls -> sole `kbc.c` owner. | **Board-L3 capability.** The existing construction path can consume source-qualified values; current abstract tick counts are Current data L2. | Preserve construction-only use. A future refinement may add provenance validation beside the existing values, but must not add a live setter or second KBC clock. |
| KBC serial-error state, AC diagnostic stream and physical keyboard protocol | KBC owns byte-level FIFO/commands; no selected 8042 ROM/RAM/PSW or serial-frame source model exists. | **L2, correctly retained.** There is no complete external input plus Core consumer for these omitted states. | A future profile must supply the entire source-qualified device/input contract, not isolated delays. |
| KBC IRQ1/A20/reset consumer phase | KBC has the sole logical publisher and output route. | Logical relation is L3; exact board consumption cadence is **Board-L3-capable only to the extent the existing KBC construction timing values cover it**, otherwise L2. | Do not use a host-side delay or duplicate signal mirror; expand the one construction plan only when a source defines the missing phase. |

## Result

The previous five controllers do not have one undifferentiated “L2 gap.”
DMA, PIT, RTC and KBC already have a copied, sole-owner path capable of
consuming Board-L3 input; PIC deliberately does not for elapsed latency because
its authority supplies no digital elapsed term.  The remaining L2 entries are
therefore exact missing board/device contracts, not reasons to add setters,
schedulers, callbacks or host-time paths.  This is the same classification
applied to CGA-T1/T2 in S2.
