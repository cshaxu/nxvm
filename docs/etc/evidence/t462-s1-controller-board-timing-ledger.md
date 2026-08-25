# T462 S1: Controller Board-Timing Ledger

## Frozen Universe And Method

This ledger consumes only the deferred board/timing rows of closed T456 PIC,
T460 DMA and T461 PIT.  The unit of coverage is one controller capability or
board route that could require a construction-time value.  Each row names the
source level, current owner and behavior, possible value unit, validation,
regression, and one disposition.  No profile-provided value can promote an L2
premise into a source-backed L3 board claim.

Existing T375 evidence proves that the IBM Model-339 profile already supplies
the source-backed 8254 conversion `1193182 / 8000000` as
`596591 / 4000000` through `clock_plan.pit`; reset reproduces the phase.  The
same evidence explicitly does not make elapsed Core ticks wall time or prove
availability while halted.  IBM AT material retained in T450 supplies the
selected 8254 1.190 MHz and channel wiring, and the 8237A 3 MHz/five-clock and
PIT1-refresh facts.  Intel 8259A supplies causal interrupt order but no Core
elapsed delivery quantum.  Those primary-source facts determine the entries
below; external emulators are not used as timing authority.

## Current One-Way Path

The current path is already copied and owner-local:

`VM descriptor -> core_machine_config.clock_plan/transaction_contract ->
core_machine_plan copy -> machine clock/transaction owners -> PIC/DMA/PIT`.

`machine_scheduler.c` advances `dma_clock`, uses the transaction owner, then
advances `pit_clock` and refreshes PIC.  PIT OUT and DMA request bindings are
Core-owned callbacks created by machine/board composition.  PIC IRR/ISR and
acknowledgement remain PIC-owned.  Dynamic GATE, IRQ, DREQ, EOP and BUSRDY
inputs are not configuration and remain their existing operations.

## Ledger

| ID | Source and capability | Current owner/behavior | Construction value and validator | Regression / disposition |
| --- | --- | --- | --- | --- |
| PIC-B1 | PIC-T2/T5: Intel defines request, priority, INT and two-step INTA causal order, but no elapsed Core latency. | `pic.c` owns state; scheduler refreshes PIC after DMA/PIT; CPU consumes the existing logical acknowledgement transaction. | No numeric field is eligible. A future discrete visibility contract needs a source rule plus a defined Core tick meaning. | Existing PIC phase/IRQ/CPU smokes. Retain explicit L2 elapsed visibility; S4 verifies it remains declared, not guessed. |
| PIC-B2 | PIC-T4: IBM AT defines selected paired-controller ports and IRQ2 cascade wiring. | `machine.c` creates the one paired PIC; PIC ICW3/SNGL consumes programmed cascade state. | No new field: current implementation supports only this selected topology, so an alternate route selector would be a placeholder. | T456 cascade/command/lifecycle smokes. Accepted selected topology; later unselected topology has T141/TODO receiver. |
| PIC-B3 | PIC-T3: electrical INTA, setup/hold and propagation tables. | No Core electrical owner. | No field; L4 is prohibited. | Retained exclusion. |
| DMA-B1 | DMA-T3: IBM AT supplies a 3 MHz DMA input clock. | `clock_plan.dma` is validated at creation, copied to `dma_clock`, and advanced only by the scheduler. | Reuse `clock_plan.dma` ratio and reset phase; S2 adds its plan-level rule qualification, never a duplicate clock. A source-backed selection requires a valid nonzero rational ratio and a named profile source. | Clock-domain and DMA phase tests. Eligible for source-backed L3 controller input; current generic 1/1 remains L2. |
| DMA-B2 | DMA-F2/F7 and DMA-T3: normal S1--S4/compressed state selection is Manual-L3; IBM lists five DMA clocks per AT data-transfer cycle. | `dma.c` advances exactly one logical service phase per delivered DMA tick; `machine_scheduler.c` owns grant/hold publication. | Do not reuse `dma_cycle_wait_quanta`: it is BUSRDY wait policy, not the five-clock term. S2 records only a qualified service-phase rule against the existing DMA clock; a numeric board conversion is eligible only when the profile's Core timebase is qualified. | DMA mode/phase smokes and transaction regressions. L2 board conversion retained until S3 proves a source-backed profile value. |
| DMA-B3 | DMA-T3/T4: PIT1 refresh request reaches DMA1; channel 1 is reserved. | `core_machine_configure_dma` binds one private DMA1 request and one PIT1 output callback. | No new route field: `core_machine_dma_wiring` already admits the supported FDC route and composition owns the selected fixed refresh route. | `core-machine-dma-rtc-authority-smoke` and T460 binding proof. Accepted selected route; alternate boards require a later topology capability. |
| DMA-B4 | DMA-T5: transaction/arbitration ownership is project integration, not a chip clock fact. | `machine_scheduler.c` and transaction owner are sole grant/hold publishers. | No field and no DMA scheduler. | Transaction/arbitration smokes. Explicit L2 integration boundary where source does not define the Core publication time. |
| PIT-B1 | PIT-T2/P17: IBM board input clock; T375 proves Model-339's 1.193182 MHz source ratio. | `clock_plan.pit` is validated/copies into `pit_clock`; `pit.c` owns count and phase. | Reuse `clock_plan.pit` ratio and reset phase; S2 adds plan-level rule qualification and must preserve reset replay. | `vm-model-339-clock-contract-smoke` and `core-machine-pit-divider-smoke`. Eligible source-backed L3 controller input for a qualified profile; generic 1/4 remains L2. |
| PIT-B2 | PIT-T1/P15: IBM AT routes OUT0 to IRQ0, OUT1 to refresh, and GATE2/OUT2 through port 61h speaker logic. | `machine.c` owns OUT0 to PIC; `machine_board.c` owns supported refresh/speaker bindings; `pit.c` owns pins. | No generic route callback/value: the existing callbacks are composition-private and each supported topology has one owner. | PIT IRQ0, DMA authority and speaker/board smokes. Selected routes remain accepted; new topologies require their own capability, not a profile callback. |
| PIT-B3 | P16: 8254 power-up phase is unspecified. | Clock-domain reset restores configured phase; PIT reset releases output. | `reset_phase` already has a stable value unit but cannot receive L3 qualification without a board source. | PIT divider/reset and IRQ lifecycle smokes. Retain L2 provenance. |
| PIT-B4 | P18: oscillator tolerance, pin setup/hold, propagation and audio/electrical topology. | No Core electrical owner. | No field; L4 is prohibited. | Retained exclusion. |

## S2 Decision

The only required new construction surface is a small controller-rule
qualification attached to the existing copied `core_machine_plan`; it must
classify the already-present PIT clock, DMA clock/service and PIC logical
visibility capabilities as either a named source-backed rule or explicit L2
fallback.  It must not duplicate `clock_plan`, `transaction_contract`,
`core_machine_dma_wiring`, or existing output/request callbacks.

The plan validator will accept an L3 controller declaration only when its
corresponding neutral rule kind and existing value prerequisites are valid.
VM/profile material remains responsible for selecting only a rule backed by
its evidence.  Core retains only copied neutral rule/value data and has no
profile name or provenance link.  PIC has no source-qualified elapsed rule in
this batch, so its current L2 declaration is preserved; a typed placeholder is
rejected.

S2 changes the plan/configuration owner and its focused plan regression only.
S3 consumes the admitted PIT/DMA qualifications at the existing owners and
proves they add no tick publisher or route.  S4 audits PIC's retained L2
boundary, all declarations and profile-to-Core direction.

`M5:T462:S1:CONTROLLER-BOARD-TIMING-LEDGER:OK`
