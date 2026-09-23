# T389 S1: Model 40 Board-Timing Reconciliation

`M5:T389:S1:MODEL40-BOARD-TIMING-RECONCILIATION:OK`

## Decision

T388 removes the unsafe path from an unallocated successful retirement into a
*selected physical* clock contract. It does not qualify the Model 40's
successful-retirement axis as a 16 MHz oscillator. The current Model-40
composition explicitly keeps `retirement_time_contract` at its zero-value
`CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC` and configures all six Core clock
domains with the neutral `{ 1, 1, 0 }` ratio. Therefore the established
Model-40 physical facts must not be converted into Core ratios, DCLK grants,
wait states or service durations in this task yet.

This is a post-T388 reconciliation, not a reopening of T388. Its negative
physical-clock conclusion remains the governing safety contract.

## Reconciled Board-Domain Ledger

| Domain | Confirmed source/current fact | Current owner and behavior | S1 disposition |
| --- | --- | --- | --- |
| CPU-related clock | D3PE identifies the 80386/16 and a phase-related 32 MHz oscillator. | `vm_session_create_model40_byob()` and `vm_session_create_model40_private()` select the 80386 profile but declare deterministic `1:1` Core domains. | The physical oscillator cannot be assigned to retirement ticks; transfer to a separate shared 80386 physical-retirement qualification. |
| System and auxiliary PITs | D3PE identifies two 8254s and a compatible 1.19318 MHz source. | `core_machine_arbitration_tick()` advances both PITs through one `pit_clock`; Model 40 exposes the second block at `48h` but no physical ratio. | Logical topology/order is retained; CPU-to-PIT conversion and phase/reset proof remain blocked by the shared qualification receiver. |
| DMA/DCLK and waits | D3PE identifies 4 MHz DCLK and one wait on every DMA cycle. | Core dual-8237A arbitration owns HOLD/ack/release and advances at `dma_clock`; Model 40 binds channel 2/cascade without DCLK or wait placement. | No inferred `1/4` clock ratio or local delay. The DCLK/wait receiver follows physical-retirement qualification, then needs a Model-40 transaction trace. |
| PIC/order | Model-40 dual-PIC topology and IRQ routes are functionally closed. | Arbitration callback order is DMA, both PITs, then PIC refresh; a readiness-produced IRQ becomes eligible at the next arbitration due tick. | Deterministic equal-tick order is retained. No propagation/settle physical duration is proved. |
| RTC/CMOS | MC146818, `70h/71h`, NMI mask and 32768 Hz selection are source-backed. | Readiness callback advances RTC after FDC/HDC service and before peripheral work. | Logical route/order is retained; physical calendar and IRQ phase require a qualified time axis. |
| 8042/input | The selected no-AUX 8042/101-key topology and reset/A20 functional routes are closed. | Peripheral callback advances KBC after readiness; Core retains zero calibrated typematic/response duration for the Model 40. | Do not invent command, typematic or reset-settle duration; later physical-device/firmware task receives device-service timing. |
| Port `61h`, refresh, NMI and reset | D3PE's composite refresh/timer, IOCHK/failsafe and reset facts are reconciled by T387 S5. | Core D4 platform owns latches, masks, NMI publication and reset clearing; system-PIT counter-1 is logically programmed at reset. | State/order is retained. Refresh phase, failsafe period and electrical/reset timing require the shared physical axis and then board-specific proof. |
| FDC, HDC and CECG availability | T386 closes logical Model-40 FDC/DMA2/IRQ6, Compaq HDC/IRQ14 and CECG routes. | They advance in Core's readiness/peripheral schedule. | Command-to-DRQ/IRQ service, physical-media, raster/monitor and ISA availability are explicitly for the following physical-device/firmware candidate, not S1. |
| D4 RAM/ROM and expansion bus | D3PE gives memory-cycle categories and a re-synchronised expansion bus averaging below 8 MHz. | VM D4 mappings and Core transactions are functional; no Model-40 READY/availability plan exists. | The bound/range is not an exact scalar. Board availability/waits require primary-supported or qualified-observation contract after physical-retirement qualification. |

## Owner And Consumer Sweep

The S1 sweep used:

```text
rg -n -i "model40|deskpro|dclk|bclk|timclk|port.?61|iochk|refresh|failsafe|retirement_time|elapsed_ticks|physical" src/core src/vm tests CMakeLists.txt cmake
rg -n "retirement_time|clock_domain|advance_time|elapsed_ticks|core_machine_publish_elapsed_ticks" src/core src/vm
rg -n "port_61|port61|refresh|failsafe|iochk|nmi|second_pit|dma|pic|rtc|keyboard" src/vm/composition/session/model40_composition.c src/core/machine
```

It confirms one machine publisher, `core_machine_publish_elapsed_ticks()`, and
one deterministic three-callback timeline. `core_machine_execute()` refuses an
unallocated source row before publication only when the physical contract is
selected; `core_machine_advance_time()` also rejects external injection in
that contract. No Model-40 composition selects it. The only Model-40 Core
clock plan is the six-domain neutral plan above. No duplicate profile-local
scheduler, DCLK bridge or wait-state owner was found.

## Earliest Receiver And Transfer

The first missing mechanism is **not** a DMA, PIT, PIC or DeskPro-local delay.
It is a shared 80386 physical-retirement qualification: a bounded source/form
matrix or qualified observation must establish when each selected successful
80386 retirement may represent the Model-40 CPU-related physical domain, with
publication, reset, failure and consumer proof. It must retain T388's rule
that every unqualified success is nonphysical. That work is outside this
board-level proposal and must be queued before any physical board-clock
implementation.

Only after that receiver may a later T389 subtask define the 16 MHz to
1.19318 MHz/4 MHz conversion and then prove Model-40 PIT, DMA wait and
availability behavior. Device command service, ROM-visible programming,
physical media and monitor/raster behavior remain downstream in the existing
physical-device/firmware candidate and TODO boundaries.

## Result

S1 completes its ledger exit condition and finds no truthful board-timing code
slice that can start under the current proposal. It makes no source, build,
asset, artifact, physical-clock or L3 change.