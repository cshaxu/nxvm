# T388 S1: Four-Profile Physical-Timebase Inventory

`M5:T388:S1:PHYSICAL-TIMEBASE-INVENTORY:OK`

## Sole Publication And Consumer Graph

`core_machine_run()` invokes `core_machine_instruction_cost()` only after a
successful executor refresh, then `core_machine_publish_elapsed_ticks()` adds
that result to the sole `elapsed_ticks` axis. `core_machine_advance_time()` is
the only non-retirement publisher and is restricted to an already-selected
external source. Both routes call one scheduler: its timeline invokes
arbitration, readiness and peripheral callbacks; those advance DMA/PIT,
RTC/FDC/HDC and KBC/VADP respectively. The provider clock is advanced directly
from the same elapsed input.

Thus a successful-retirement unit is not merely CPU accounting: it is an input
to every present board/device clock domain. No profile may call a rational
clock plan a physical frequency until that unit is eligible for every reachable
successful form.

## Classifier Anchors

The inventory retains the shared dispatch anchor `core_machine_instruction_cost` and the four terminal profile classifiers: `core_machine_8086_source_instruction_cost`, `core_machine_80186_source_instruction_cost`, `core_machine_80286_source_instruction_cost`, and `core_machine_80386_source_instruction_cost`. Each is therefore independently traceable from profile-specific instruction classification to the sole elapsed-tick publisher; no classifier is evidence that its returned value has a physical-clock unit until the next receiver establishes that unit.
## Four-Profile Publisher Matrix

| Profile | Source-backed successful rows | Explicit nonphysical successful routes | Disposition |
| --- | --- | --- | --- |
| 8086 | T359/T360 selected table/formula rows through primary, control/stack, string/I/O and legacy owners. | Legacy non-segment prefix and final lookup fallback return `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. | Not physical-clock eligible. T388 later work owns complete form or prohibition matrix. |
| 80186 | T359/T360 selected Table 1-16 rows through the shared legacy owner and shared classifiers. | The shared legacy source/fallback route covers rows not allocated by its selected contract, including range-only arithmetic. | Not physical-clock eligible. Primary range formula or qualified observation is required. |
| 80286 | T366/T368 source-ledger rows, including accepted Appendix-B context and selected system forms. | Legal residual prefixes plus successful `WAIT`/ESC with no FPU publish the one-tick transfer; T368 records them explicitly. | Not physical-clock eligible. Existing Model-339 ratio remains a deterministic profile contract, not proof that all active retirement ticks are 8 MHz cycles. |
| 80386 | T359/T363 selected primary, secondary, privileged, control and dynamic-multiply rows. | Prefix/default and unavailable lexical/context paths in the 80386 classifier return the one-tick transfer. | Not physical-clock eligible. Model-40 must retain neutral plan until T388 closes the shared boundary. |

Faulted, rejected, invalid-profile and other delivered synchronous paths return
before successful publication; they are not a physical-timebase input. Host
virtual time is also distinct: VM invokes it only while waiting for interrupt.
Model-339's 8 MHz binding is host pacing during HLT and cannot repair active
CPU retirement units.

## Static Recurrence Boundary

`verify-t388-physical-timebase-inventory` binds this inventory to the sole
publisher, the four profile classifiers, the explicit sentinel, the six clock
consumers, external-time API and waiting-only VM path. It cannot prove a cycle
value; it prevents those owners from silently disappearing while this task
remains open.

## Next Receiver

S1 allocates no value. T388 S2 must classify every reachable successful
sentinel route into an exact Intel row, a profile-local primary formula, or a
qualified secondary observation/prohibition record. It must preserve a single
Core publication boundary, provide focused regressions and then establish
whether any selected profile can lawfully feed a physical board clock.