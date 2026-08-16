# T386 S21: Model-40 Current Functional Matrix And Profile Transfer

`M5:T386:S21:MODEL40-CURRENT-FUNCTIONAL-MATRIX:OK`

`M5:T386:S21:PROFILE-VARIANT-TRANSFER:OK`

## Baseline And Method

This documentation/contract S reconciles the accepted Model-40 selection and
CPU records with T386 S16--S20. It supersedes only S15's current-state matrix
and its obsolete next-step narrative; historical evidence remains unchanged.
No firmware, guest-media, local path, vendor hash, third-party source, runtime
source or artifact is added.

"Complete" below means the selected functional register/state/reset and
consumer contract has an owner and selected-composition proof. It never means
service duration, bus phase, physical medium, firmware execution or L3.

## Current Functional Matrix

| Selected Model-40 requirement | Current disposition | Required receiver / truthful boundary |
| --- | --- | --- |
| 80386DX-16, protected/paged/VM86 state, no x87 | Complete. | T385; board clock/phase only transfers to DeskPro timing. |
| 1 MiB standard RAM; D4 compatibility/replacement map | Capacity and source-bounded mapping complete; parity and diagnostics incomplete. | Remaining T386: parity producers, diagnostic/status latches and IOCHK behavior. Board memory-cycle timing transfers later. |
| Dual 8259A selected routes | Complete selected logical routes. | IRQ availability/cascade timing transfers later. |
| Dual 8237A-5, cascade and word channels | Complete reusable selected topology. | Unbound expansion endpoints are not selected devices; DCLK/wait/arbitration/pins transfer to timing. |
| Two 8254 blocks | Complete selected register/reset topology. | Failsafe cadence, speed control and board service timing transfer later. |
| MC146818/CMOS/NMI mask | Complete selected register/reset route. | Persistence/oscillator timing transfers later. |
| 101-key keyboard, 8042/8742, IRQ1, no AUX | Complete selected controller route. | D4-SKEY A20 and power/shutdown/8042 reset arbitration remain T386 functional; electrical timing transfers later. |
| IOCHK/failsafe NMI and D4 platform control | Partial. | Remaining T386: parity/diagnostic IOCHK publication, D4-SKEY transform and reset arbitration. |
| 765A FDC, 1.2 MB drive, DMA2/IRQ6 | Drive geometry/profile and controller reset/Sense composition complete; selected logical media behavior partial. | Remaining T386: raw-IMG command/state/error/recovery, DMA2/IRQ6 and BIOS-visible consumer proof. Flux/CRC/index/rotation/physical CHRN remain TODO physical-media debt. |
| Compaq Multipurpose Fixed Disk Controller and one 40 MB disk | Normal controller CHS/PIO/reset/error/IRQ14 path partial over RAW-IMG; public fitted-disk startup contract absent. | Remaining T386: selected normal route consumer proof and constrained startup-only external fixed-media contract. Long/format/ECC/recovery/physical sectors remain `TODO(Medium)`. No ATA or IBM MFM substitution. |
| Compaq Enhanced Color Graphics and Color Monitor | Shared VADP personality/palette, selected static controls and bounded routing are partial. | Remaining T386: source-backed miscellaneous-output/monitor/status/firmware-programming effects and reset/isolation proof. IBM EGA and analog inference are excluded. Raster/ISA waits transfer later. |
| Rev-E ROM policy | Fixed public two-chip BYOB manifest/validation/copy/map is complete as a carrier. | Firmware execution, ROM-to-D4 copy and BIOS service behavior are not implied; later probes/audit decide them without committing assets. |
| Public Model-40 session profile | Fixed 80386/no-FPU/1 MiB/1.2 MB/ROM backbone is declared; the selected 40 MB fixed-disk media route is not yet public, and no generic variant surface exists. | Remaining T386 publishes only the fitted disk under a constrained startup-only contract. The later current-product profile-capability task owns allowed variants across Model-339, Model-40 and XT. |
| D4 compatible expansion bus | Functional topology only after listed T386 rows close. | Board/device timing owns resynchronization, waits, arbitration and electrical behavior. |

## Transfer And Proposal Reconciliation

S20's catalog is intentionally a fixed-backbone, frozen-entry mechanism; it is
not evidence of generic variant selection. The later current-product
profile-capability candidate must therefore start by defining per-backbone
fixed/default/allowed/prohibited facts and startup-only versus removable-media
rules. It must reject arbitrary composition. This S changes neither candidate
order nor the Model-40 historical selection.

The T386 functional candidate now receives only the four remaining functional
clusters in the matrix: D4/platform, FDC/1.2 MB logical media, the fitted 40 MB
controller/startup-media path, and CECG behavior. It receives no physical-media,
board-timing or generic-variant implementation work.

## Similar-Issue Sweep

The sweep examined the T384 capability audit, T385 closure, T386 S15--S20
evidence, the DeskPro shared context and functional proposal, the future
current-product profile proposal, Queue, Roadmap and the relevant TODO rows.

- The stale S15 "next S16" narrative is superseded here; it remains historical
  evidence and is not rewritten.
- S18 and S20 correctly state that generic variants are absent; the future
  profile proposal's former assertion that S20 already supplied them is
  corrected.
- The functional proposal's former aggregate wording did not name the current
  four remaining functional clusters or the fixed-disk startup contract; this
  record and its revision correct that omission.
- The existing physical-media and CECG TODO rows remain truthful; no new
  unclassified production issue was found.

## Verification

The S21 matrix was checked against the named accepted evidence and current
Queue/proposal boundaries. The documentation governance gate, link/index
checks and final coordinator actual-change review are recorded with S21
closure. No runtime verification is claimed because this S changes no runtime
surface.
