# T386 S29: Model-40 Selected-Device Functional Closure Audit

`M5:T386:S29:FUNCTIONAL-CLOSURE-AUDIT:OK`

## Decision

T386's selected-device functional candidate is complete.  This audit accepts
only logical device and profile functionality; it transfers the named board,
firmware and physical-media questions to their existing receivers.  It does
**not** accept a runnable external firmware path, board timing, physical media
fidelity or Model-40 L3 accuracy.

The audit found one false-completion weakness in the retained evidence: the
S26 Model-40 HDC smoke was built, but was absent from the canonical current
smoke list.  This S adds that existing test to the list and updates the two
exact T345 inventory assertions that deliberately count current tests.  No HDC
or media behavior changed.  The canonical gate now has 281 registered targets;
the focused CTest replay executed `current.vm-model40-hdc-s26-smoke` and
reported both retained S26 markers.

## Selected Functional Matrix

| Selected area | Current owner and accepted proof | Functional disposition | Exact later boundary |
| --- | --- | --- | --- |
| 80386DX-16, no x87 | Shared CPU/profile owners; [T385 closure](t385-s1-task-closure-audit.md). | Complete selected CPU/profile function. | Model-40 board clock/availability in the DeskPro board candidate; no FPU route. |
| 1 MiB RAM, D4 compatibility RAM, 128 KiB external-ROM geometry | `vm/profile/model40`, Model-40 composition and [S14](t386-s14-model40-rom-layout.md), [S16](t386-s16-d4-memory-rom-map.md), [S25](t386-s25-rom-alias-mechanism.md). | Complete fixed carrier, mapping and reset-visible logical routes. | S25's CPU-to-PIT calibration is a board-timing receiver; vendor firmware execution remains external/BYOB. |
| D4 parity, IOCHK/NMI, SKEY/A20 and reset arbitration | Shared Core parity/NMI/reset owners and Model-40 binding; [S22](t386-s22-d4-parity-diagnostic.md), [S23](t386-s23-d4-skey-reset-arbitration.md). | Complete logical producer, latch, publication, clear and reset routes. | Electrical parity, NMI and reset service/phase behavior belongs to the DeskPro board candidate. |
| Dual PIC, dual DMA and two PITs | Shared Core controllers bound by Model-40 composition; [S3](t386-s3-second-pit-owner.md), [S7](t386-s7-private-model40-composition.md), [S8](t386-s8-model40-integration.md), [S17](t386-s17-model40-dual-dma.md). | Complete selected logical topology, cascade/reset and consumer routes. | Arbitration, service durations and phase placement belong to the DeskPro board candidate. |
| RTC/CMOS, 8042/101-key keyboard, IRQ1-only and no AUX | Shared Core routes plus Model-40 profile selection; [S7](t386-s7-private-model40-composition.md), [S8](t386-s8-model40-integration.md), [S20](t386-s20-model40-byob-profile.md). | Complete selected logical topology and exclusions. | Board event ordering/timing, firmware programming and native input fidelity belong to the DeskPro board candidate or later supported-device work. |
| 1.2 MB FDC/FDD, DMA2/IRQ6 | Shared Core FDC/FDD plus reusable VM drive profile and Model-40 binding; [S18](t386-s18-model40-floppy-device-profile.md), [S24](t386-s24-model40-12mb-fdc.md). | Complete raw-IMG command/state/reset/DMA/IRQ/error logical path. | Flux, CRC, index, rotation, physical CHRN and electrical/mechanical timing remain physical-media/board work. |
| Fixed 40 MB Compaq HDC, IRQ14 | Core HDC and VM checked startup-media owner; `tests/products/vm_model40_hdc_s26_smoke.c`; [S5](t386-s5-deskpro-storage-controller.md), [S26](t386-s26-model40-fixed-hdd.md). | Complete selected 925/5/17 RAW-IMG CHS/PIO/reset/error/IRQ14 startup route; no hot replacement after publication. | Long/format/ECC/recovery, physical-sector semantics and controller/drive service timing remain [TODO](../../states/TODO.md#hardware-and-compatibility-debt) and the DeskPro board candidate. |
| Compaq Enhanced Color Graphics and color monitor declaration | Shared VADP plus a Model-40 CECG personality; [S6](t386-s6-compaq-ega-personality.md), [S9--S13](t386-s9-cecg-control-static-configuration.md), [S27](t386-s27-cecg-residual-audit.md), [S28](t386-s28-cecg-odd-even-page.md). | Complete source-determined digital personality, selected static/status controls, CPU memory gate, I/O base, Odd/Even page state, reset and copied-frame consumer route. | Special Features, monitor/sync/live-video behavior, option-ROM programming, physical signalling, raster service and ISA waits remain the explicit CECG TODO and DeskPro board receiver. |
| External Rev-E ROM / fixed backbone | VM-local validated BYOB manifest and copied carrier; [S20](t386-s20-model40-byob-profile.md), [S25](t386-s25-rom-alias-mechanism.md). | Complete configuration/mapping admission boundary only. | No vendor ROM is committed or defaulted; real firmware execution and its programming effects require an independently admitted BYOB/firmware lifecycle. |

The matrix leaves no selected device merely syntactically composed: every row
has a current logical owner plus an accepted focused proof, or a named, already
recorded non-functional boundary.  Generic PC/AT, ATA/IBM-MFM substitution,
IBM EGA and generic generated firmware remain forbidden substitutes.

## Verification And Similar-Issue Sweep

- Inspected the retained S21 matrix, S22--S28 evidence, Model-40 composition,
  profile, controller and smoke registrations; also reviewed Queue, TODO,
  T386 proposal and task history.
- Static registration sweep: `rg -n "vm-model40-hdc-s26-smoke|PROJECT_CURRENT_SMOKE_TARGETS|PROJECT_CURRENT_MEDIA_SMOKE_TARGETS" CMakeLists.txt` found the HDC executable but no canonical current-gate entry.  The entry is now in the one canonical list, so normal T344 partition validation and CTest registration apply.
- Inventory sweep: the added pure test changes only the exact T345 counts:
  strict owner-test targets 164 to 165 and deferred ownership rows/tests 227/167 to 228/168.  The T345 verifier remains the sole inventory owner; no parallel list or test-only product API was added.
- Focused execution: `ctest -V -R "^current\\.vm-model40-hdc-s26-smoke$"` passed and printed `M5:T386:S26:MODEL40-HDC-STARTUP:OK` and `M5:T386:S26:MODEL40-HDC-FIXED-MEDIA:OK`.
- Full replay: configured `mingw-gcc-x64`, then ran `verify-documentation-governance`, `verify-current-specialized-gates` and `run-current-smokes`.  The resulting gates passed with 281 current targets, 266 non-media / 15 media, and the T345 verifier reported 228 rows (168 owner tests, 7 embedded, 1 type, 2 safely separable and 50 mixed/inherited).

## Closure And Transfer

The next Queue candidate is the DeskPro 386 board bus and device timing
closure.  It receives availability, waits, arbitration, service duration,
clock calibration, phase placement, electrical NMI/reset behavior and the
selected device command-to-IRQ/DRQ ordering.  The physical-media and CECG
items retain their existing TODO rows; no TODO row is silently closed here.

T386 itself is therefore ready for its task-level closure audit, but this S
makes no Model-40 L3 claim.

## Coordinator Acceptance

After actual-change review of P1 `a9210fde`, the coordinator accepts S29 and
closes T386.  Review confirmed that the CMake change registers the existing HDC
smoke in the sole canonical current-gate list, and that the three exact T345
inventory assertions are the only dependent count owners.  Documentation
governance, specialized gates, the 281-target current gate and the focused HDC
CTest replay pass.  The task-level decision remains functional-only: T386
transfers to the next Queue candidate and makes no Model-40 L3 claim.
