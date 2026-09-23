# T398 S1 DeskPro Model-L3 Requirement And Evidence Matrix

## Frozen Target And Decision Rule

The target is the original 1986 DeskPro 386 Model 40 / 80386DX-16 selected by
the private and BYOB Model-40 compositions. L3 here is an evidence-backed
selected-model readiness claim, not generic 80386-PC compatibility or physical
cycle-exact emulation. A row is ready only when its selected behavior has a
current owner, direct proof and no unresolved requirement that the claim would
cover. A retained receiver is truthful but makes that row **not ready**.

## Independent Matrix

| Selected capability | Current owner and direct proof | Current L3 disposition | Receiver / distinction |
| --- | --- | --- | --- |
| Configuration and CPU/state | VM Model-40 private/BYOB composition; T385/T394 selected 80386 evidence | Not ready | T394 qualified C0 safety does not choose a physical Model-40 descriptor or retirement-to-clock conversion. This differs from Model-339's separately frozen deterministic L3 contract. |
| Memory, D4 compatibility and ROM carrier | VM Model-40 profile plus Core checked immutable mappings; T386 S14/S16/S25 | Ready as functional deterministic mapping | External firmware execution remains BYOB-only and no vendor ROM is a product input. This does not claim firmware service timing. |
| CPU/DMA/PIT/PIC/RTC board topology | Core clock/timeline/transaction owners, Model-40 topology and T396 six-domain ledger | Not ready | Logical order/reset is proven; CPU-to-board unit, phase, DCLK/BWAIT and physical propagation are retained by the DeskPro physical-observable timing TODO. |
| D4 parity, IOCHK, port 61h, SKEY/A20 and reset | Core D4/platform/reset owners, T386 S22/S23 and T397 focused regressions | Not ready | Functional producer/latch/clear/reset path is proven; physical parity/failsafe/NMI propagation remains the physical-observable timing TODO. |
| 8042/keyboard and IRQ1-only/no-AUX | Core KBC and VM topology, T386 S7/S8/S20 | Not ready | Logical selected topology is proven; native-input and controller response/physical visibility timing remain the physical-observable timing TODO or later supported-device admission. |
| FDC/FDD, DMA2, IRQ6 | Core FDC/DMA/PIC plus VM drive profile, T386 S18/S24 and T397 CTests | Not ready | Logical RAW-IMG command/DRQ/IRQ/reset path is proven; physical command-to-DRQ/IRQ timing and media observables remain the physical-observable timing TODO. |
| 40 MB Compaq HDC, IRQ14 | Core HDC and VM fixed-media owner, T386 S5/S26 | Not ready | Normal RAW-IMG CHS/PIO/reset/IRQ is proven; ECC, format, physical sectors and drive service timing remain the Compaq physical-media TODO. |
| CECG and monitor declaration | Core VADP and Model-40 CECG personality, T386 S6/S9-S13/S28 | Not ready | Digital selected register/memory/copied-frame behavior is proven; raster, monitor/sync, ISA wait and firmware-visible effects remain the CECG TODO. |
| Reset, cancellation and deterministic order | Core cold reset/timeline, VM session lifecycle, T396/T397 route reset ledgers and current smokes | Ready as deterministic lifecycle behavior | No electrical reset/set-up/hold claim is made; those are already non-ready physical rows. |
| Product isolation and host boundary | Core/VM ownership, copied presentation and no host clock coupling | Ready as deterministic product boundary | Not a claim about external firmware, analog display or generic PC/AT behavior. |
| Regression and repository integrity | T397 full current gate (68 checks), 13 focused Model-40/device tests, documentation governance | Ready for the unchanged runnable graph | Passing tests do not discharge the physical rows above. |

## Cross-Model Boundary

Model-339's documented deterministic L3 readiness is a separate IBM 5170
configuration with its own source and consumer contracts. PC/XT 5160-268 is a
different 8088 external-bus/prefetch profile. Neither proves DeskPro physical
availability, firmware timing or any row in this matrix.

## S1 Result

The finite universe has no unclassified selected component. The matrix is
sufficient to admit the final decision subtask: current Model-40 is **not
ready** for the requested L3 claim because five physical-visible capability
rows retain explicit receivers. No code repair follows from this evidence-only
result.