# T369 S4: PC/AT Bus-Stage Closure Audit

## Scope decision

This audit closes T369's bounded **bus-stage** work, not IBM PC/AT 5170
Model-339 L3.  It rechecks the source-labelled Model-339 input, the one core
transaction owner, the S3 logical DMA handoff, and the selected FDC/PIC
visibility paths.  No audit finding requires a code repair or a second
scheduler.  Facts for which the IBM/Intel inputs select no board-local value
remain explicit transfers rather than fabricated timing.

## Production-route disposition

| T369 boundary | Actual owner and audited disposition |
| --- | --- |
| Model-339 baseline and NMI input | T366 remains the accepted source: Model 339/Type 3, 8 MHz 80286, 512 KB planar RAM, CGA, no fixed disk, compatible field-upgrade TEAC 1.44 MB drive, and planar-parity NMI. These are topology/lifecycle inputs, not a wait table or device-duration result. |
| CPU memory and port availability | CPU helpers retain the sole `transaction_begin`/commit/cancel path and T368 retains successful-retirement time publication. Neither IBM's board material nor Intel's 80286 interface material chooses a Model-339 memory/ROM/CGA/ISA/port READY interval. No synthetic wait state is admitted. |
| DMA exclusion | `dma.c` owns request selection and one transfer transaction; `machine.c` owns the single arbitration callback. S3 adds the 80286-only logical request/acknowledge/same-owner transaction/release lifetime, including trace and reset release. It has no elapsed-time or pin-waveform assertion. |
| PIC visibility and acknowledgement | `pic.c` owns source selection, logical interrupt vector acknowledgement and refresh; the arbitration callback refreshes PIC after DMA/PIT. The logical acknowledgement remains distinct from physical INTA. No source supports an INTA waveform, spacing or board wait value, so no trace/event is misrepresented as one. |
| FDC DMA2/IRQ6 visibility | `fdc.c` owns DRQ assertion/deassertion and IRQ source state; its request reaches the existing DMA owner, while advance/refresh run at the separate readiness boundary after same-tick arbitration. The next arbitration tick can observe that request. This is deterministic visibility ordering only; controller command, DRQ-to-grant, TEAC mechanics and service duration transfer to T370. |
| Reset, finalize and trace | `core_machine_cold_reset` resets DMA, FDC, PIC, transaction and timeline owners; transaction reset cancels then releases a hold while retaining its copied trace callback. FDC finalization deasserts DMA and IRQ. The transaction trace is copied through the machine trace adapter; no mutable internal state crosses that boundary. |
| Stopped/paused operations | External memory/port/debug configuration operations remain outside executing guest rounds and retain their immediate lifecycle contract. They neither request HOLD nor constitute an unreviewed CPU/DMA path. |

## Similar-route sweep

The audit used the following production and focused-test sweep:

```text
rg -n "core_machine_transaction_(begin|commit|cancel|hold_)|core_machine_dma_(has_pending_request|advance_transaction|request_)|CORE_MACHINE_TRACE_TRANSACTION|core_machine_(cold_reset|reset)" src tests CMakeLists.txt
rg -n "core_machine_(pic_refresh|pic_get_interrupt|fdc.*(advance|refresh|request|irq)|readiness_tick|arbitration_tick)|CORE_MACHINE_TRACE_(PIC|FDC|CPU.*INT|INTA)" src/core/machine tests/core tests/machine
```

The first query confirms one production transaction owner and no alternate
hold owner. The second confirms that PIC logical acknowledgement stays in the
PIC owner, and that FDC request/advance/refresh stays in the FDC plus existing
readiness/arbitration owners. The test hits exercise those public owner
boundaries; they do not create test-only state. No unclassified production
route was found.

## Verification and artifact

On 2026-08-14, rebuilding the `current-gcc` preset required no additional
compile work and retained the current artifact target marker:

```text
M5:T197:S1:CURRENT-ARTIFACT-TARGET:vm-0-5-0369:OK
M5:T354:S2:TRANSACTION:OK
M5:T354:S3:COMPETITION:OK
M5:T369:S3:PCAT-HOLD:OK
M5:T354:S4:TRANSACTION-LIFECYCLE:OK
```

The local developer artifact remains `build/output/nxvm_0_5_0369.exe`, built
from implementation source commit `12ac65ac`, with SHA-256
`460B470378405F9BB958E75B95F379FC8726BCAAB40AC83267CAD8F10DC2C121` and
compiled identity `Neko's x86 Virtual Machine [0.5.0369]`. It is not a release
artifact and contains no protected firmware or guest media.

## Exact transfers and closure result

| Open boundary | Ordered receiver |
| --- | --- |
| Model-339 READY/wait values for CPU memory, ROM, CGA, ISA and ports; maximum HOLD latency; physical HOLD/HLDA/DREQ/DACK/AEN/INTA behavior | Closed T371 [Selected-profile model-L3 phase refinement](../../history/M5-T371-selected-profile-phase-refinement-proposal.md). |
| FDC/controller/TEAC command, DRQ-to-grant and mechanical service durations; selected device readiness timing | Closed T370 [PC/AT device service-timing corpus](../../history/M5-T370-pcat-device-service-timing-corpus-proposal.md); physical durations now transfer to phase refinement. |
| Selected-profile phase integration and IBM PC/AT final model audit | The following phase-refinement and [5170 model-L3 closure audit](../../proposals/m5-l3-machine-closure-audit.md) candidates. |
| IBM fixed disk MFM/ST-506 controller/drive | Existing `TODO(Medium)` MFM/ST-506 admission; ATA/HDC is neither the Model-339 device nor its timing evidence. |
| CPU source-range/external-bus facts outside accepted successful-retirement rows | The ordered CPU/bus closure candidates and their explicit source transfers. |

T369 is therefore complete as a source-backed ownership and logical-availability
stage. It proves no physical wait count, bus-cycle duration, pin waveform or
device service interval. **The 5170 Model-339 L3 baseline remains open.**
