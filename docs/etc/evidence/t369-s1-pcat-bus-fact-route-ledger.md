# T369 S1: IBM PC/AT Bus Fact And Production-Route Ledger

## Decision

T369 consumes the locked IBM PC/AT 5170 Model 339 Type 3 profile, not the
generic default PC/AT. The selected machine remains 8 MHz 80286, 512 KB planar
RAM, Rev.3 abstract firmware slot, CGA, no fixed disk and one compatible TEAC
1.44 MB field-upgrade drive. Its no-FPU profile and T366 planar-parity NMI
lifecycle are inputs, not timing values.

The source sweep proves that NXVM currently has one synchronous CPU/DMA
transaction owner and deterministic device-event ordering, but no board-local
availability state. IBM's 5170 reference supplies board topology and the
signals to investigate; Intel's 80286 hardware reference supplies the CPU's
READY and HOLD/HLDA response boundary. Neither source yields a Model-339-wide
memory/I/O wait table. S1 therefore admits no wait scalar, physical-clock
conversion, resumable instruction state, or device delay.

## Source And Reference Disposition

| Source | What it determines | What it cannot determine here | S1 disposition |
| --- | --- | --- | --- |
| [IBM PC/AT Technical Reference, September 1985](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf) and [March 1986 revision](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf) | 5170 board/ISA, dual-DMA, interrupt, diskette-controller and system-control topology; named READY/HOLD/HLDA-related board signals. | A single per-access CPU memory/port wait-state table or TEAC service duration for the selected field upgrade. | Primary topology/contract authority. Recover the applicable board timing/schematic evidence before a scalar or reservation policy is admitted. |
| [Intel 80286 Hardware Reference](https://www.dosdays.co.uk/media/intel/210760-002_80286_Hardware_Reference_Manual_1987.pdf) | CPU READY sampling and HOLD-to-HLDA functional/minimum-latency boundary. | Which Model 339 requests HOLD, selected wait insertion, ISA/card readiness, or resulting elapsed-time conversion. | Primary CPU interface authority only; any range remains a range until a Model-339 contract selects its inputs. |
| [86Box IBM AT notes](https://github.com/86Box/docs/blob/master/hardware/machinespecific.rst) | IBM AT on-board RAM limit is 512 KB, agreeing with the selected planar population. | A selected 5170 wait-state or DMA arbitration value; its settings expose generic 286/386 memory wait options. | Compatibility cross-check only, not a timing authority. |
| [MAME AT implementation](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/pc/at.cpp) | Its `ibm5170` route has distinct 6 MHz/1664 KB defaults, while `ibm5170a` changes the clock to 8 MHz; it binds logical PIC acknowledgement. | The locked Model 339's 512 KB configuration or an authoritative wait/DMA value. | Incomparable timing model; retain only as a later qualitative logical-INTA cross-check. |
| [PCjs 80286 announcement](https://www.pcjs.org/blog/2014/08/28/) and [PCjs repository](https://github.com/mgarlanger/pcjs) | A 5170 chipset/80286 configuration and an explicit project statement that exactness is not claimed. | Model-339 wait/hold values or an authority-level timing result. | Incomparable timing model; not a scalar source. |

No third-party code, ROM, disk, diagnostics program, generated trace, local
path or model implementation is imported. The source-policy boundary remains
unchanged.

## Production Route Ledger

| Selected family | Current production owner and validation-to-publication path | Reset/trace state | S1 timing status and receiver |
| --- | --- | --- | --- |
| CPU fetch/data memory | `cpu_instructions.c` physical read/write helpers call `core_machine_transaction_begin`, checked physical memory, then commit or cancel. `machine.c` publishes source instruction ticks only after successful retirement. | `core_machine_cold_reset` resets the transaction before rebuilding the deterministic timeline; machine trace adapts begin/commit/cancel. | No selected availability or wait state. S2 must decide a single CPU/DMA availability contract only after qualifying board evidence; it may not double-charge the closed T368 source row. |
| CPU ordinary/string port I/O | CPU port helpers validate a frozen port route, begin one CPU port transaction, invoke the port provider, then commit/cancel before architectural publication. | Same transaction/reset/trace owner; stopped/paused external APIs intentionally remain immediate. | No selected port READY or ISA wait fact. S2 owns only executing CPU I/O; debugger/configuration APIs are excluded with their existing lifecycle proof. |
| Dual 8237A memory traffic and hold intent | `dma.c` preflights, starts a DMA memory transaction, performs transfer/provider work, then commits/cancels; `core_machine_arbitration_tick` advances the dual-DMA owner before PIT/PIC. | DMA and transaction state reset through cold reset; transaction trace exposes logical begin/commit/cancel. | Existing deterministic grant is not HOLD/HLDA. S2 must first recover the selected HOLD/HLDA contract; S3 may then reconcile request/grant ordering without claiming pin waveform timing. |
| PIC acknowledgement | Device sources refresh the PIC after DMA/PIT; CPU delivery performs logical acknowledgement. | PIC reset is current device lifecycle; trace records refresh rather than physical INTA. | MAME's callback only corroborates the logical boundary. Physical INTA remains a later source-backed transfer; no S1 implementation. |
| FDC DRQ/IRQ6/DMA2 | `core_machine_readiness_tick` advances and refreshes FDC after arbitration; FDC owns service state and its frozen IRQ6/DMA2 binding. | FDC reset/cancellation stays controller-owned; advance/refresh trace is recorded. | The selected TEAC field upgrade fixes no controller or DRQ-to-grant timing. Transfer device service to the ordered T370 corpus after a valid S2 bus contract. |
| NMI/parity and other devices | Model-339 planar parity is the sole selected NMI producer; RTC/CMOS/PIC remain non-producers. KBC, CGA, PIT and RTC keep their existing converted domains. | Existing owners reset and publish through the timeline/trace contract. | No availability input is inferred from a device source or IRQ. Retain device-specific timing for T370. |
| Profile topology and storage exclusion | `pc_at_profile.c` descriptor selects 80286, 512 KB, no HDC, planar parity, CGA and the Rev.3 abstract slot; its Model-339 firmware service list omits HDC. | Descriptor validation rejects a Model-339 HDC configuration. | Current ATA/HDC ports are excluded and cannot become IBM MFM/ST-506 terminology or timing. The separately recorded MFM TODO remains the only fixed-disk receiver. |

## Sweep And Bounded Receiver Plan

The S1 repository sweep used:

```text
rg -n "core_machine_(advance_scheduler|arbitration_tick|readiness_tick|peripheral_tick|transaction_trace|cold_reset)|core_machine_transaction_(begin|commit|cancel)|core_machine_dma_advance_transaction|core_machine_pic_refresh|fdc.*(advance|refresh)|model_339|MODEL_339|pc_at" src/core/machine/machine.c src/core/machine/cpu_instructions.c src/core/machine/dma.c src/vm/profile/default_profile tests/machine tests/core
rg -n "Model 339|Type 3|wait state|WAIT|HOLD|HLDA|READY|bus availability|FD-235" docs/etc docs/history docs/proposals src/vm/profile src/core/machine tests
```

Every production hit has one disposition above. Test-only and stopped/paused
external operations are excluded because they are intentionally outside guest
execution and already prove their immediate lifecycle boundary.

1. **S2 -- board evidence and availability contract.** Recover and reconcile the
   selected board timing/schematic evidence with the Intel signal contract.
   It may introduce one shared availability state only if this produces a
   source-labelled, resettable, traceable rule without a guessed scalar.
2. **S3 -- CPU/DMA and logical-INTA competition.** Runs only after S2. It
   reconciles selected DMA request/grant/HOLD visibility, FDC DMA2/IRQ6 and
   logical PIC acknowledgement with the shared transaction state.
3. **T370 -- device service-timing corpus.** Owns FDC/DRQ readiness, controller
   service and other selected device timing after T369's bus contract. It does
   not inherit an unproved same-tick rule.

## Exit Result

The first implementable unit is the bounded S2 source-and-contract
reconciliation, not an artificial wait counter. Until it qualifies a
Model-339-specific board fact, the existing immediate CPU/DMA commits and
deterministic event order remain truthful but are not bus timing or 5170 L3.
