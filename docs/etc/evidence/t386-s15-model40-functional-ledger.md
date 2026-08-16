# T386 S15: Model 40 Functional-Ledger Reconciliation

`M5:T386:S15:MODEL40-FUNCTIONAL-LEDGER:OK`

`M5:T386:S15:NEXT-RECEIVER:OK`

## Method And Boundary

This is the current-source reconciliation required by the accepted DeskPro
selected-device functional proposal.  It starts from the frozen T384 Model 40
BOM, then checks every selected device against accepted T385 and T386 evidence,
current composition and focused test ownership.  `Complete` below means the
selected functional state/register/reset/consumer contract has an owner and
selected-composition evidence.  It does not mean a service duration, bus
phase, physical medium or L3 result.

Compaq primary material remains transient research under the source policy.
T384's technical-reference evidence is the selection authority.  A later
manufacturer-authored PC Tech Journal description was used only to recheck the
published D4 ROM-replacement wording; it does not override the selected 1986
Compaq board material.  No firmware, media, local path, hash, source text or
third-party code is retained here.

## Current Functional Matrix

| Selected Model-40 requirement | Current functional disposition | Owner and proof | Receiver / stop boundary |
| --- | --- | --- | --- |
| 80386DX-16, protected/paged/VM86 state, no x87 | **Complete.** Shared 80386 owner is selected without a DeskPro CPU fork; no-FPU profile is explicit. | T385 S1; private session config; CPU current-gate corpus. | Board clock/bus phases only: DeskPro timing candidate. |
| 1 MiB standard RAM | **Complete for capacity only.** Private composition creates exactly 1 MiB and CMOS base-memory values. | `vm_session_create_model40_private`, S8 integration. | D4 replacement/overlay behavior remains separate below. |
| D4 ROM and RAM-replacement map | **Incomplete and source-sensitive.** S14 proves the owner-supplied two 16 KiB chip carrier, interleaving, 64 KiB F0000h mirror and reset alias. T384 still records a 128 KiB FE0000h--FFFFFFh ROM/replacement contract. These cannot be treated as one mapping without a source-labelled reconciliation. | VM Model-40 firmware provider; S14 synthetic proof. | **Next S16:** revalidate D4 decode, E0000h replacement and reset alias, then implement only the source-supported mapping with synthetic tests. Stop before real-ROM loading/shadowing. |
| Dual 8259A / selected IRQ routes | **Complete for selected logical routes.** Existing dual-PIC owner serves the Model-40 IRQ1/6/8/14 consumers and reset lifecycle. | S7/S8 selected composition and shared PIC controls. | IRQ availability, cascade propagation timing and ISA phase placement: DeskPro timing candidate. |
| Dual 8237A-5 / DMA2 | **Partial.** Selected FDC DMA2 request, DACK and IRQ6 route are bound and tested; no Model-40 proof yet covers the D4 word-DMA/cascade channels or DCLK wait semantics. | `core_machine_configure_dma`, FDC topology/S8 integration. | Functional channel/cascade inventory follows S16 mapping; DCLK wait and arbitration are timing. |
| Two 8254 blocks | **Complete for selected register/reset function.** Primary and auxiliary PITs, `40h`--`43h` and `48h`--`4Bh`, are distinct; D4 receives the selected failsafe output. | T386 S3/S4 focused controls and S8 integration. | Failsafe cadence, speed control and board timing remain later. |
| MC146818 / CMOS / NMI mask | **Complete for selected register/reset and NMI-mask route.** | Model-40 RTC defaults; T386 S4 D4 NMI proof; S8 integration. | Battery persistence and oscillator/service timing remain later. |
| 101-key keyboard, 8042/8742, IRQ1, no AUX | **Complete for the selected controller route.** Keyboard-only composition rejects AUX/IRQ12 and uses the shared keyboard state/IRQ owner. | T351 controller/keyboard evidence; T386 S7/S8 selected controls. | Keyboard electrical/cable timing and any source-specific 101-key scan distinction need an admitted corpus. |
| IOCHK/failsafe NMI, KBC A20/reset | **Partial.** D4 owns `61h` latches, NMI publication and reset recovery, but D4-SKEY PAL transformation, power/shutdown arbitration and physical reset sequence are not modelled. | T386 S4 and Model-40 composition. | Source-backed board logic is a later functional receiver; pulse/settling is timing. |
| 765A FDC, 1.2 MB drive, DMA2/IRQ6 | **Partial.** Selected ports and controller reset/Sense state are composed; a selected 1.2 MB drive/media/BIOS-visible contract is not yet established. | T386 S5/S8; generic FDC controls. | Selected drive/media functional contract after S16; rotation, flux/CRC and service timing remain explicit debt. |
| Compaq Multipurpose Fixed Disk Controller / 40 MB disk | **Partial.** Compaq WD normal CHS PIO, reset, errors, IRQ14 and shared `3F7h` state are implemented; Long/format/ECC/recovery and physical sectors are intentionally excluded by pure IMG. | T386 S5; Compaq HDC controls. | Physical-media representation is an external constraint in TODO; controller service/board timing stays later. |
| Compaq Enhanced Color Graphics / Color Monitor | **Partial.** Shared VADP has the selected personality, palette, static identity and bounded `3C2h`/`3C6h`/status/CRTC effects. | T386 S6/S9--S13 and Model-40 controls. | Remaining `3C2h` effects, monitor/firmware programming and live-video/special-interface behavior require source-backed functional S; raster/ISA waits are timing. |
| Rev-E BIOS and firmware-visible policy | **Externally constrained.** The private in-memory carrier is not a public loader or proof of firmware execution. | T384 S2, T386 S14. | Owner-approved BYOB provider/manifest is a later, separate admission; no vendor asset enters Git. |
| D4 compatible expansion bus | **Timing-only after the above functional rows.** | T384 BOM. | DeskPro board/device timing closure. |

## Reconciliation Decision

The original S1 ledger correctly ordered the first functional owners, but it
predated S5--S14 and therefore cannot state current completeness.  The matrix
above is the sole current progress receiver for T386.  It neither rewrites
historical evidence nor turns S14's synthetic reset fetch into a BIOS, storage
or L3 result.

The first unambiguous next code receiver is **S16 D4 memory and ROM-map
reconciliation**.  It is earlier than expanded DMA/FDC/display work because
all firmware-visible device consumers depend on a truthful selected memory and
ROM map.  S16 must begin with the exact primary decode evidence and may use
project-owned synthetic bytes only.  If the source cannot reconcile the
128 KiB board window with the selected Rev-E pair, it must record the distinct
hardware configurations and stop rather than guess a merged map.

## Similar-Issue Sweep

The audit swept the T384 BOM; T385; T386 S1--S14 evidence; Model-40
composition/provider/session construction; selected PIC/DMA/PIT/RTC/KBC/FDC/
HDC/VADP/D4 APIs and focused tests; plus Queue/TODO transfers.  The only
current contradiction found is the D4 ROM/replacement contract above.  All
other incomplete rows already have an earliest owner or a source/physical
boundary.  No code, public ABI, artifact, firmware or guest media changes are
made by S15.

## Coordinator Acceptance

P1 `400e681a` is accepted after independent actual-change review.  The review
confirms the matrix covers the full frozen BOM rather than only implemented
features, distinguishes functional gaps from board timing and external assets,
and assigns the D4 ROM/replacement conflict to one explicit next receiver.
The delivery changes only active status and indexed evidence; documentation
governance passes, and no source, ROM, media, artifact or public interface is
changed.  S15 is accepted as the functional-completeness baseline, not a
functional or L3 closure.  T386 remains open.
