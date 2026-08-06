# Long-Term Review Ledger

This ledger records open work, durable capability boundaries, and long-horizon
goals. It does not activate a task, allocate a numeric task identifier, or
override the current work recorded in
[status.md](docs/planning/status.md). Detailed completed-task evidence belongs
in its task record and Git history; the M5 implementation order belongs in
[M5 NXVM PC/AT Hardware Convergence](docs/planning/m5-pcat-hardware-convergence.md).

## Operating Rules

- `status.md` owns the one active subtask. The M5 convergence plan owns the
  ordered task queue. This file tracks cross-cutting open risks and deferred
  admissions only.
- A device admission follows the
  [hardware-device verification template](docs/compatibility/hardware-device-verification.md):
  S1 contract and port probe, S2 implementation at the real owner, and S3
  DOS/system-image regression.
- `core` never depends on `vm` or `vdm`; composition is the only cross-module
  assembly point; platform never mutates guest state. One mutable state has one
  owner and each product has one real run path.
- Bochs/PCjs may provide bounded differential observations, never copied code,
  a runtime dependency, or acceptance evidence in place of owned probes.
- Completed implementation detail is intentionally not duplicated here. Use
  task records, the current capability table, and the roadmap when assessing a
  claim.

## Current Capability Baseline

| Area | Current bounded capability | Open boundary |
| --- | --- | --- |
| NXVM product | One session/composition path, retained Console/debugger, FDD/HDD boot regressions, GCC artifact and CTest gates | Preserve this path while every device evolves; do not quietly start VDM behavior. |
| CPU | Real-mode 8086-plus executor plus a bounded 80286 GDT/CPL0 16-bit protected-mode corpus; `FPU=none` consumes legal ESC encodings | Not trusted 80386, general protected mode, paging, task switching, or present FPU. |
| Interrupts and time | PIC source lifecycle; deterministic core elapsed ticks; PIT/IRQ0 -> ROM -> BDA -> `INT 1Ah` evidence | Greater timing fidelity only when an explicit corpus requires it. |
| Keyboard | KBC, IRQ1/IRQ12, ROM `INT 09h`/`INT 16h`, set-1 break/E0/E1, typeahead, selection/query, translation observation, LED, command-state typematic, ACK/RESEND, bounded PS/2 AUX packets, and one DOS guest-driver corpus | Default core auto-repeat stays disabled until a profile-clock calibration defines human typematic time; set-2/3 conversion, wheel/advanced AUX protocol, broad guest mouse API compatibility, and native POSIX runtime validation remain deferred. |
| Display | CGA text plus bounded digital `320x200x4` and ROM-selectable `EGA-320x200x16-direct`; copied text/indexed frames; `console`/`window`/`auto` selection | Remaining digital CGA modes/CRTC behavior, composite video, broader EGA/VGA, VBE. |
| Storage | Bounded ATA PIO and FDD boot paths through declared ROM/device owners; core-owned 8237 DMA controller baseline with frozen FDC DMA2 binding | Full FDC state machine, broad DMA behavior, extended IDE, and error/timing compatibility. |
| VDM | Isolated non-runnable scaffold over the shared core | Owned DOS design, CLI, host-drive policy, and product implementation remain deferred. |

## Current M5 ROI Queue

The active task is always the one in [status.md](docs/planning/status.md).
These are the next owned admissions, not permission to work in parallel.

- [ ] **Advanced 8042 AUX protocol (`TODO(Medium)`).** T229 admits a bounded
  IRQ12 three-byte relative packet path only. Admit wheel IDs, sample-rate,
  scaling, remote/read-data/status, resend/error timing, and host capture only
  as separately probed controller work; do not turn platform input into a DOS
  API or guest-memory shortcut.
- [x] **FDC/FDD controller state machine (`TODO(High)`, T231).** Core-owned
  DMA2 now drives the bounded VM FDC/FDD command/result state machine; image
  files are validated backends, not controller shortcuts. Deferred separately:
  multi-drive mechanics, rotation latency, deleted/scan/read-track commands,
  and arbitrary FDC configuration.
- [x] **CMOS/RTC admission (`TODO(Medium)`, T232).** The admitted MC146818
  subset has owned registers/NVRAM, periodic/update/exact-alarm IRQ8, and
  deterministic elapsed-tick time. Deferred: persistent battery files,
  square-wave output, don't-care alarms, and broader RTC services.
- [x] **ATA PIO feature matrix (`TODO(Medium)`, T233).** Primary-master LBA28
  PIO and count-zero=256 are admitted; slave and secondary remain explicitly
  absent, with SRST/status/error/IRQ14 behavior probed. Deferred: IDE DMA,
  ATAPI, LBA48, cache, and host-clock timing.
- [ ] **Complete digital CGA (`TODO(Medium)`).** Keep it separate from EGA/VGA.
  Admit `640x200x2` and remaining 40/80-column text combinations, then the
  6845-visible start/page address, cursor, scanline geometry, display-enable,
  vertical-retrace, and any light-pen behavior. Every admission retains one
  VADP owner, B8000h mapping, copied-frame output, and port/memory/frame plus
  DOS-fixture evidence.
- [ ] **CGA composite-video fidelity (`TODO(Low)`).** Treat NTSC artifact
  color, phase, and colorburst as an optional renderer/profile capability only
  after digital CGA is complete. Do not fold it into VADP digital state or use
  it to claim EGA/VGA support.
- [x] **Bounded EGA direct-mode admission (`TODO(Medium)`, T235--T239).** T235 completes
  the profile-bound A0000h aperture/sequencer subset and T236 the graphics/
  attribute registers plus map-select classification. T238 completes one
  direct-port `EGA-320x200x16-direct` path: VADP-owned planar VRAM/latches,
  frozen core memory routing, and a copied 16-entry fixed RGBI frame, with no
  DAC. T239 admits only matching
  ROM `INT 10h` mode `0Dh` selection and `03h` exit through real VADP port
  state. Deferred work remains separate; do not make a single unbounded
  "VGA support" task.

## CPU, Time, And Debugging Boundaries

- [ ] **Broaden real-mode 8086 corpus (`TODO(High)`).** T240 established a
  reset-vector baseline for segment override, `REP`/direction strings,
  `INT`/`IRET`, port I/O, and fault retention.  Extend only through a failing
  ROM/DOS/device path to uncovered 8086 families such as arithmetic/FLAGS,
  conditional control transfer, stack edge cases, and string compare/scan.
  MS-DOS `MEM` remains a regression sample, not an 80386-completeness claim.
- [ ] **286/386 protected-mode program (`TODO(Low)`, T258--T261, T263).**
  T257 admits only GDT/CPL0 16-bit entry, selector loads, same-CPL far
  transfer, and diagnostic validation faults. T258 is CPL0 4 KiB paging plus
  narrowed CR0/CR2/CR3 forms and core diagnostic `#PF`; it deliberately does
  not create a false CPL3/TSS I/O test path. T259 first admits protected
  privilege and IDT delivery, T260 then admits TSS I/O-map behavior through a
  real CPL3 corpus, T261 admits task switching, and T263 keeps remaining
  instruction families corpus-specific. This remains lower ROI than the
  real-mode PC/AT device route.
- [ ] **Present x87 (`TODO(Low)`, T262).** Define state, operations,
  exceptions, and `FWAIT` before enabling any 8087/287/387 profile.
- [ ] **CPU-fault outcome audit (`TODO(Medium)`).** T214 established a
  session-owned fault result. Revisit only with a reproducible case showing a
  fault/detail is not available to the retained Console/debugger boundary;
  preserve first-fault evidence and never add a second run path.
- [ ] **Debugger assembler `checkop` review (`TODO(Low)`).** Keep runtime
  disassemble/reassemble comparison out of per-instruction refresh. Add a
  focused regression only when an owned BIOS/debugger defect requires it.
- [ ] **Bounded differential debugging (`TODO(Low)`).** A historical
  Bochx/Bochs bridge may be an optional developer tool with provenance,
  checkpoint schema, masks, instruction/time/no-progress/trace budgets, and
  cleanup. It is never a default build or acceptance substitute.

## Timing-Fidelity Ladder

The current core is deterministic and host-clock-independent: completed
instructions advance core elapsed ticks and devices consume frozen accumulated
clock ratios. These levels are optional compatibility admissions, not the
default definition of NXVM completion.

- [x] **Level 1 rational device clocks (T256).** Frozen profile ratios, phase,
  rounding, reset origin, and dispatch order now deterministically relate core
  DMA/PIT/VADP/KBC and VM-provider clocks to coarse instruction ticks. This
  does not claim real CPU-cycle attribution.
- [ ] **Instruction-timed execution (`TODO(Medium)`).** Give each admitted
  instruction deterministic profile-specific cost, including applicable
  prefix/branch/memory/I/O variants, before expanding the timing corpus.
- [ ] **Bus-timed PC/AT operation (`TODO(High)`).** Model memory/I/O wait
  states, DMA bus ownership, and device visibility at transaction boundaries.
- [ ] **Cycle-exact profiles (`TODO(High)`).** Only where a profile genuinely
  requires it, model clock phases, prefetch/bus behavior, and device
  microstates without silently changing the retained executor.

## Architecture, Portability, And Product Boundaries

- [ ] **M5 core/VM convergence (`TODO(High)`).** Close the actual boundary:
  core owns only neutral guest mechanics and policy-free capabilities; VM owns
  PC/AT CMOS/RTC, storage controllers, firmware, boot/media, and retained UX.
  Core retains deterministic guest ticks and generic PIT/PIC/DMA mechanics; a
  trusted external
  research report may prove a neutral requirement, but is never a core ABI,
  runtime consumer, or dependency. Complete the single-owner audit and the
  Windows 3.x startup prerequisite corpus before M5 closure.
- [ ] **M6 mantle session envelope (`TODO(High)`).** Build only the neutral
  lifecycle, execution pump, runtime bindings, transition gateway, and factual
  diagnostics over core. No DOS ABI, external ABI, host path policy, or UI.
- [ ] **M7 machine-profile system (`TODO(High)`).** Implement the T255
  admission contract for reproducible PC/AT, Compaq DeskPro 386, and IBM PC
  110 declarations with legal firmware/media boundaries; do not move profile
  quirks into core.
- [ ] **M8 owned DOS and NXVDM (`TODO(High)`).** Build `dos` and `vdm` over
  mantle: DOS owns loader/services/program state; VDM owns CLI, containment,
  presentation, debug UX, cancellation, and exit policy.

- [ ] **M5 architecture closure audit (`TODO(High)`).** Before M5 closes,
  rescan every core/vm/vdm module, composition root, target, test, and entry
  point for duplicate state, parallel paths, forwarding wrappers, global/TLS
  selectors, illegal dependencies, and misleading names.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Linux source is a
  portability asset, not a support claim. Add native POSIX compile and runtime
  probes after an approved POSIX environment is available; do not install WSL
  merely for this item.
- [x] **Machine-profile admission (T255).** Immutable profile declarations and
  future BYOB ROM-manifest boundaries are defined before PC110, Compaq, Award,
  or Phoenix behavior. Third-party ROMs remain never bundled, downloaded, or
  committed; implementation is deferred to individually admitted M7 tasks.
- [ ] **M8 VDM/DOS product admission (`TODO(High)`).** Keep mantle, DOS, and
  VDM non-runnable until their M6/M8 contracts exist. M8 must implement the
  approved `nxvdm run` display/debug/no-program semantics, host-drive
  containment, Windows 7--11 matrix, and exit/cancellation policy; NXVM
  display selection is not a substitute.

## Standing Closure Requirements

These are permanent execution requirements, not individually completable
tickets. The detailed rules live in
[execution-policy.md](docs/planning/execution-policy.md), the architecture
documents, and the M5 closure checklist.

- Keep build presets, current artifact target, CTest registrations, task
  revisions, and evidence truthful to the current source graph.
- Each behavior-changing implementation task produces one verified ignored
  artifact in `build/output/`; remove stale owned build trees, logs, traces,
  and generated binaries after use.
- Preserve Console, debugger, boot, ownership, and no-global/no-second-machine
  invariants unless an owner-approved task changes them with regressions.
- Control trace and differential-debug growth with byte, time, no-progress,
  process-cleanup, and deletion budgets.

## Long-Horizon Goals

### Profile Support

- IBM Palm Top PC 110, using a separately admitted profile and user-provided
  ROM/media workflow where needed.
- Compaq DeskPro 386 profile, informed by PCjs behavior research.
- Other clearly manifested machine profiles, never a blanket import of every
  Bochs machine definition.

### OS Compatibility

- Near term: robust real-mode DOS software and the device corpus it requires.
- Medium term: 386-capable DOS, `HIMEM.SYS`, and Windows 3.x Standard Mode and
  386 Enhanced Mode, each gated by the required CPU, display, DMA, storage,
  interrupt, and timing evidence.
- Long term: Windows 9x, only after the prior compatibility layers are proven.

### AI DevBox

- Expose controlled machine capabilities so AI agents can program and test
  legacy DOS, Win16, and Win32 projects inside a VM session.
- Support both explicit lower-level control APIs and computer-use interaction,
  with session ownership, host trust boundaries, and reproducible fixtures.
