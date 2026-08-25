# T469 S1: Controller Deadline Eligibility And Manual Re-audit

`M5:T469:S1:CONTROLLER-DEADLINE-ELIGIBILITY:OK`

## Decision

This is a corrective, row-complete re-audit of the frozen 145-row controller
universe. It reuses the original manual Checklist 1 documents and their
Checklist 2 implementation-gap audits; it does not replace a manual with an
emulator. The cross-source observations already retained by the individual
checklists remain corroboration only. The resulting classification supersedes
Td S148 where its wording was ambiguous, while preserving its exact row
identifiers and source facts.

The hierarchy is `L0 < L1 < L2 < L3 < L4`:

- A documented number, period, frequency, range or formula is **Manual L3**.
  It stays so even if today's Core conversion to its shared tick is L2.
- A qualified emulator/model observation is **Other L3**; it cannot override
  Intel, IBM, Motorola or ATA normative material.
- A copied board/profile input that reaches the one Core owner is **Input L3**.
- Only an unsourced numerical conversion or estimate is L2. Causal order with
  no numerical model is L1; absent Core logical order is L0; source-backed
  electrical, mechanical or monitor facts are L4 and out of scope.

## Source And Row Completeness

| Domain | Normative checklist and cross-check set | Rows re-audited |
| --- | --- | --- |
| PIC 8259A | `t450-s3-pic-8259a-function-timing-checklist.md`; Intel 8259A and IBM AT | PIC-R1--R5, PIC-F1--F8, PIC-T1--T5 (18) |
| DMA 8237A | `t450-s5-dma-8237a-function-timing-checklist.md`; Intel/IBM, 86Box, MAME, PCjs, Bochs, QEMU | DMA-R1--R4, DMA-F1--F7, DMA-T1--T5 (16) |
| PIT 8254 | `t450-s7-pit-8254-function-timing-checklist.md`; Intel 8254 and IBM AT | PIT-R1--R4, PIT-F1--F8, PIT-T1--T5 (17) |
| RTC/CMOS | `t450-s9-rtc-mc146818a-function-timing-checklist.md`; MC146818A and IBM AT | RTC-R1--R4, RTC-F1--F7, RTC-T1--T5 (16) |
| KBC/NMI | `t450-s11-kbc-8042-nmi-function-timing-checklist.md`; 8042/IBM AT | KBC-R1--R4, KBC-F1--F7, KBC-T1--T5 (16) |
| 8272A FDC | `t450-s13-fdc-8272a-logical-media-function-timing-checklist.md`; Intel 8272A and retained emulator comparisons | FDC-R1--R5, FDC-F1--F5, FDC-T1--T5 (15) |
| VADP EGA | `t450-s15-vadp-function-timing-checklist.md`; IBM EGA and retained model checks | VADP-R1--R5, VADP-F1--F5, VADP-T1--T5 (15) |
| VADP CGA | `t467-s1-cga-function-timing-checklist.md`; IBM/Motorola and retained 86Box model | CGA-R1--R5, CGA-F1--F7, CGA-T1--T5 (17) |
| ATA/HDC | `t468-s1-ata3-function-timing-checklist.md` and `t450-s17-hdc-ata-function-timing-checklist.md`; ATA-3 and retained emulator checks | ATA-R1--R5, ATA-F1--F5, ATA-T1--T5 (15) |

The complete row-by-row disposition, including every Manual-L3 source fact,
Input-L3 receiver, L2 conversion, L1 relation, L0 omission and L4 exclusion,
is the identifier-complete table in
[Td S148](td-s148-eight-controller-l-level-reclassification-audit.md). This
re-audit verified every group above against its original Checklist 1 entry and
did not find a manual number or formula that Td S148 had demoted below Manual
L3. Its corrections are limited to retaining L4 as a higher physical exclusion,
rather than incorrectly calling it L0.

## Deadline Qualification Sweep

A source fact or an Input-L3 receiver is not automatically an eligible runtime
deadline. A deadline must be the earliest **guest-observable** change, owned
by Core, queryable without exposing a controller/timeline pointer, and valid
across reset and cancellation. The present scheduler instead re-enqueues its
arbitration, readiness and peripheral maintenance callbacks at `due_tick + 1`.
Those callbacks are not a deadline API and must not be exported to VM.

| Owner | Existing source/input and state | S1 deadline disposition |
| --- | --- | --- |
| PIC | Manual causal IRQ/INTA ordering; no numeric visibility term and timing rule is fallback-only. | L1 order only; not eligible. |
| DMA | Manual IBM 3 MHz/five-clock formula and Model-339 rational input are L3, but servicing is per-tick maintenance. | Source/Input L3 facts; no observable-deadline query, not eligible. |
| PIT | Manual counter/mode/output rules and copied clock declaration. | Candidate owner, but per-tick maintenance has no next output/IRQ observation; not eligible. |
| RTC | Manual periodic/update rules and copied RTC timing plan. | Candidate owner, but no next guest-visible update/IRQ observation; not eligible. |
| KBC | Copied response/serial/typematic inputs and remaining-tick fields. | Input L3 receiver; incomplete selected UPI/device schedule, not eligible. |
| 8272A FDC | Existing seek and transfer due fields; Intel unit facts are Manual L3. | `CORE_MACHINE_FDC_SEEK_TRACK_TICKS` is an L2 conversion and no source-qualified service plan exists; not eligible. |
| VADP CGA/EGA | Copied display/clock plan and raster state. | Input L3 receiver; no source-qualified next guest-visible raster/status deadline for every selected mode, not eligible. |
| ATA/HDC | One HDC/media/IRQ owner and causal PIO phases. | L1 service order, no service-time plan or deadline state; not eligible. |

Therefore no currently selected profile can truthfully export a composed L3
host-pacing deadline. S2 receives one bounded requirement only: add an opaque
composed observation **after** each listed owner supplies an eligible
guest-observable deadline and the selected profile supplies a verified physical
timebase. It must preserve the above ineligible dispositions rather than derive
time from host elapsed time or scheduler callback cadence.

## RTC Construction Cleanup

`vm_session_storage_initialize()` had the same `rtc_clock` condition twice,
with the second branch repeating the source-plan assignment. S1 retains one
condition and one else branch. The default PC/AT composition smoke now proves
`CORE_MACHINE_RTC_TIMING_L2_RATIO`; the Model-339 clock-contract smoke proves
`CORE_MACHINE_RTC_TIMING_L3_SOURCE`. The cleanup deletes a parallel decision
path; it adds no RTC feature, timing number or public interface.

## S2 Transfer

S2 may define an opaque Core-composed deadline observation only. It may not use
the current one-tick maintenance callbacks, pass a controller pointer to VM, or
let VM inject/skip guest ticks. Until the individual owner rows above become
eligible, Standard retains its declared fallback and Turbo may only omit host
pacing around the same Core-owned progression.
