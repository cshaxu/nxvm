# T555 S1 Selected-Controller And Board Baseline

## Scope And Method

This is the admission ledger for current NXVM controller and board
qualification. It reads the present source/test layout, selected profile
inputs, historical manual ledgers and historical repair evidence. It does not
reuse historical paths as live ownership, import manuals, or change source,
tests, firmware, media or assets.

The current implementation graph is:

```text
App profile input
  -> core/machine plan and composition
  -> core/board-base plus core/board-at or core/board-xt wiring
  -> core/chips mechanism
  -> copied Emulator-machine result / product presentation
```

`core/chips` owns mutable chip mechanisms. `core/board-base` owns shared PC
port, signal, deadline and consumer bindings; `core/board-at` and
`core/board-xt` own their distinct board wiring. `core/machine` constructs the
one Core/board pair. Each fixed App supplies immutable profile input and its
firmware binding; no App owns a second controller or scheduler.

## Current Route Inventory

| Boundary | Current mutable owner and board/App receiver | Direct current proof | Source/evidence tier | S1 disposition |
| --- | --- | --- | --- | --- |
| PIC 8259A | `core/chips/pic8259`; `core/board-base/pic_bus` binds IRQ sources and CPU delivery. | PIC command, OCW3, lifecycle and board-composition smokes. | Intel 8259A plus IBM board wiring; T507 S3 is the latest controller re-audit. | Selected cascade, EOI, poll and source lifecycle are Manual L3. Electrical INTA/CAS delay remains excluded/L2. S2 must recheck selected 5160/5170/default/Model40 cascade and delivery observations against current code. |
| PIT 8253/8254 | `core/chips/pit825x`; `core/board-base/pit_bus` binds IRQ0, refresh and speaker consumers; D4 auxiliary PIT remains a separate chip. | Waveform, 8253, read-back, IRQ0, refresh, speaker and D4 tests. | Intel 8253/8254 and IBM XT/AT board routes; T507 S5. | Counter, GATE and logical output routes are Manual L3. Board clock conversion is only profile-qualified where explicitly declared; other physical cadence stays L2. S2 verifies no profile publishes a synthetic deadline. |
| DMA 8237A | `core/chips/dma8237`; `core/board-base/dma_bus` and scheduler attach request, transaction and page routes. | Channel, binding-token, service/arbitration, FDC/Xebec and plan tests. | Intel 8237A and IBM XT/AT routes; T507 S4, T539 S6 and T506 closure. | Register/mode/priority/logical service are Manual L3; only qualified board clock/phase can publish guest time. S2 must check current refresh/DMA0, FDC/DMA2, Xebec/DMA3 and Model40 conversion ownership. |
| RTC/CMOS | `core/chips/rtc146818`; `core/board-base` owns 0070h/0071h and NMI-mask separation. | RTC/CMOS, port, authority and profile-clock tests. | Motorola MC146818A and IBM AT wiring; T507 S6. | Register/IRQ route is qualified where source-backed. Divider/SQW/UIP/power/reset and generic PC/AT time conversion contain bounded L2 work. S3 must decide whether retained profile scope supplies the missing selected facts; it must not create a second clock or CMOS store. |
| KBC and AT keyboard | `core/chips/kbc8042` and `keyboard`; `core/board-at` binds reset/A20/parity. | Controller, serial cadence, host ingress, AUX gating, PC/AT topology and reset tests. | Intel UPI-42 plus selected IBM board facts; T507 S7 and T552 S1--S4. | One FIFO/OBF/IRQ/A20/reset route is current. IBM 5170 correctly excludes later AUX; generic default may explicitly select it. Electrical frames, error latches and unsourced controller cadence remain L2. S3 audits exact selected boundaries. |
| XT PPI keyboard | `core/chips/ppi8255` and `xtkeyboard`; `core/board-xt` owns PPI/IRQ1/NMI/speaker board binding. | XT keyboard contract, PPI keyboard, BAT/serial-deadline and XT-profile tests. | IBM 5160 board/keyboard evidence; T507 S7. | Distinct XT route is current and must remain distinct from KBC. Keyboard serial/electrical timing outside selected conversion remains L2. S3 verifies reset and NMI separation. |
| FDC/media | `core/chips/fdc8272`; `core/board-base/fdc` owns DMA2/IRQ6 binding; Core media registry owns logical backing. | FDC command/reset/DMA-boundary, media-provider and firmware-route tests. | Intel 8272A and selected board facts; T507 S8. | Selected logical command/DRQ/IRQ routes are source-backed. Raw FM/MFM, rotation, CRC and mechanics remain L2 unless a profile selects those facts. S4 audits per-App ports, media geometry and deadline conversion. |
| HDC personalities | `core/chips/hdc`; `core/board-base/hdc` binds ports/IRQ/DMA where selected; Core media registry owns backing. | HDC, ATA, Compaq, Xebec, port and profile tests. | ATA-3, IBM WD1003/Xebec, Compaq evidence; T507 S10 and T510. | ATA PIO, WD1003/ST-506, Compaq WD and XT Xebec remain distinct selected personalities. Service duration/mechanics stay explicit L1/L2 absent a selected source conversion. S4 verifies no personality aliases another controller's request/IRQ/DMA route. |
| Video | `core/chips/video`; `core/board-base/vadp` owns port/aperture binding and copied capture; Apps choose generic/CECG profile facts. | CGA/EGA/CECG controller, aperture, frame and App-firmware tests. | IBM Options and Adapters plus selected Compaq facts; T507 S9. | Selected register/VRAM/copy route is source-backed. Raster/monitor/ISA timing and unselected adapter capacity remain L2. S5 verifies current aperture, clock and presentation boundaries without a second renderer/framebuffer. |
| Board reset, firmware, memory and observation | `core/board-base` with `core/board-at`/`core/board-xt`; `core/machine` performs one construction/lifecycle route. | Board plan, reset, firmware alias, electrical, deadline and App-composition/integration tests. | Selected product/profile evidence and previous board ledgers. | One construction and reset path exists. S6 must reconcile all four fixed Apps' firmware, reset, deadline and integration observations after controller S2--S5; no BIOS-specific controller path is allowed. |

## Fixed-App Inputs

| App | Fixed construction fact relevant to this task | Current qualification risk |
| --- | --- | --- |
| My5160 | XT PPI keyboard, 8253, XT refresh/Xebec path and external XT firmware. | Most controller-clock rules remain L2; S2/S3 must not infer AT behavior. |
| My5170 | 80286 AT, no AUX, IBM 5170 ports/CMOS/FDC/HDC routes and selected rational clocks. | Existing L3 board inputs must be re-proven against current composition rather than inherited from historical descriptors. |
| MyDeskPro386 | Model40 D4/CECG/auxiliary-PIT/Compaq personality and macro-proportional axis. | Several board ratios are expressly Other-L2; S2--S6 must preserve that classification and isolate model-specific wiring. |
| NXVM | Default AT, generic/extension-friendly profile and built-in firmware. | Controller timing rules intentionally remain L2; acceptance is correct routing and explicit non-physical claims, not a false 5170/DeskPro equivalence. |

## Confirmed Gaps And Receivers

1. **Proposal/source-layout drift.** The candidate used retired
   `x86/chips`, `ibmpc/board-*` and old artifact paths. This is repaired in
   the proposal during S1; no runtime behavior changed.
2. **No current, one-file qualification matrix.** Historical ledgers prove
   many individual facts but use pre-migration paths and old test identities.
   This ledger is the current receiver; S2--S6 must attach direct current
   source/test evidence rather than treating old closure language as proof.
3. **Profile timing is intentionally mixed.** My5170 has selected rational
   clock/service inputs; My5160, Model40 and default retain some L2 values.
   A later S may repair a wrong current conversion, but cannot promote a
   number to L3 without a selected board/controller source and one consumer.
4. **RTC source boundary remains substantive.** Divider/SQW/UIP/power/reset
   distinctions and IBM century-byte placement need an explicit decision from
   the retained profile facts before mechanism work. This belongs only to S3.
5. **FDC/HDC/video physical-media and monitor scope remains deliberately
   bounded.** Logical media/PIO/copy behavior is not proof of raw magnetic,
   cable, mechanical, raster or monitor behavior. S4/S5 must retain these as
   L2/L1/L4 unless a selected source admits them.
6. **Cross-profile closure is not implied by chip tests.** Each chip owner
   has focused tests, but profile wiring, reset aliases and firmware boot
   consumers require S6's four-App integration matrix.

## Non-Gaps

- There is no current evidence of a duplicate PIC, PIT, DMA, RTC, KBC, FDC,
  HDC or VADP state owner.
- Shared Lib/Emulator/x86 components do not own these controller mechanisms;
  this task has no Shared implementation receiver unless a later audit proves
  an actual boundary defect.
- Missing electrical waveforms, physical media, unselected adapters and
  unsourced wall-clock conversion are not implementation defects by default.

## S1 Verification

- Read current Core/App source and test-tree ownership, current CMake graph
  and the source-policy boundary.
- Reconciled T450 source/audit universe with newer T507/T510/T511/T552 owner
  repairs; historical claims were not accepted without a current owner route.
- No production, test, asset, firmware, INI, Shared corpus or external
  material changed. `git diff --check` is the applicable S1 verification.
