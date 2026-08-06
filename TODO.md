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
| CPU | Real-mode 8086-plus executor with selected 80186/286/386 decode; `FPU=none` consumes legal ESC encodings | Not trusted 80386, protected mode, paging, task switching, or present FPU. |
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
- [ ] **286/386 protected-mode program (`TODO(Low)`, T252--T254).** Admit
  descriptors, exceptions, control transfer, CRx/paging, CPL/IOPL, TSS I/O map,
  task switching, and remaining instructions as individually probed work. It
  is lower ROI than the real-mode PC/AT device route.
- [ ] **Present x87 (`TODO(Low)`, T255).** Define state, operations,
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

- [ ] **Frequency-accurate device clocks (`TODO(Medium)`).** Define exact
  rational conversion, phase, rounding, reset origin, and event ordering for
  CPU, PIT, CGA, DMA, and RTC clocks under each profile.
- [ ] **Instruction-timed execution (`TODO(Medium)`).** Give each admitted
  instruction deterministic profile-specific cost, including applicable
  prefix/branch/memory/I/O variants, before expanding the timing corpus.
- [ ] **Bus-timed PC/AT operation (`TODO(High)`).** Model memory/I/O wait
  states, DMA bus ownership, and device visibility at transaction boundaries.
- [ ] **Cycle-exact profiles (`TODO(High)`).** Only where a profile genuinely
  requires it, model clock phases, prefetch/bus behavior, and device
  microstates without silently changing the retained executor.

## Architecture, Portability, And Product Boundaries

- [ ] **Core/mantle product naming and external VDM boundary (`TODO(High)`).**
  Treat the following names as the forward product definitions and reconcile
  legacy `ntvdm64.exe` terminology, CMake target names, requirements, and
  release documents in one approved migration before any runnable VDM work:
  - **Core / `core.dll`:** the releasable generic machine, platform, and
    product-tool foundation rooted at `src/core`. It has no DOS-runtime,
    Microsoft-specific, or PC/AT product policy.
  - **NXVM / `nxvm.exe`:** the bootable whole-machine product rooted at
    `src/vm`. It owns its PC/AT profile, synthesized BIOS contents, boot/media
    policy, and product UI.
  - **Mantle / `mantle.dll`:** a releasable VDM host layer rooted at the future
    `src/mantle`. It may contain only mechanism demonstrably shared by NXVDM
    and an external wrapper, such as a VDM runtime envelope over core. It has
    no external-runtime-specific ABI, selector, manifest, DOS namespace, or
    protected guest asset.
  - **NXVDM / `nxvdm.exe`:** the independently implemented DOS VDM product
    rooted at `src/vdm` and built over mantle/core. It contains no Microsoft-
    specific guest ABI, BOP/SVC contract, or Microsoft guest asset. Its optional
    debug Console is a project-owned product experience.
  - **External `ntvdm.exe`:** a future program in a separate local research
    project. It builds over mantle/core and locally combines a user-supplied
    runtime. It is neither an ntvdm64 product nor a default build, runtime,
    dependency, release artifact, or promised workflow.

  The future physical artifact and dependency shape is:

  ```text
  src/core/       -> core.dll
  src/vm/         -> nxvm.exe       -> core.dll
  src/mantle/     -> mantle.dll     -> core.dll
  src/vdm/        -> nxvdm.exe      -> mantle.dll -> core.dll
  external wrapper -> ntvdm.exe     -> mantle.dll -> core.dll
  ```

  Keep all-product-neutral tooling in `core/product`; keep NXVM composition and
  UX in `src/vm`, NXVDM composition and UX in `src/vdm`, and external-runtime
  composition outside this repository. Mantle admission requires two real
  consumers and a contract without product, DOS-namespace, or runtime-specific
  policy. VM multi-session run/pause/reset/stop control remains VM-owned unless a future shared
  requirement is demonstrated. The existing generic debug-target contract is
  reused by NXVDM through a product composition binding; do not add a debug-
  event contract without an observed need. Retained NXVM Console and NXVDM
  debug UX remain product-owned. First make this a clean source-level boundary
  under the existing single-executable build; do not create `src/mantle`, DLL,
  or SDK packaging before an admitted shared mantle mechanism exists. The
  shared machine start contract is one validated
  `initial-state`/entry-plan: NXVM supplies the conventional x86 reset-state
  plus its mapped BIOS image, while NXVDM and an external wrapper may supply
  direct prepared state. Existing core reset returns only to a clean machine
  baseline; composition then applies the selected initial state. A reset vector
  is therefore a VM profile choice, not a separate core boot mode.

  The core-boundary delta must classify every item as unchanged, `VM -> core`,
  `core -> VM`, core extension, or new capability. Present findings identify
  no `core -> VM` migration. Candidate `VM -> core` mechanism extractions are
  CMOS/RTC, reusable FDC/HDC controller mechanics, and policy-free shared host
  providers; image-file backends, media selection, PC/AT defaults, firmware
  generation, and UX remain in VM. Mantle is not a presumed extraction target:
  no source moves there until its two-consumer, policy-free contract is proven.
  Core extensions/new work include checked
  guest-memory access, externally observable A20/HMA semantics,
  protected-mode/V86/exception/IRET correctness, generic ROM mapping, generic
  initial-state application, generic guest transitions, and narrow policy-free
  host capability contracts. Core
  platform admission begins with copied normalized host events, presentation
  sinks, wait/monotonic-host-clock support that never sets guest time, and an
  optional byte-stream contract when NXVM and NXVDM prove identical use.
  An opened random-access media/backend contract is conditional on shared
  controller use; mount paths, geometry, eject policy, and media selection stay
  in VM. Generic file/directory, drive-visibility, DOS-path, and DOS-namespace
  contracts are not admitted merely because a VDM or external wrapper uses
  host files. Core machine/product may later emit copied, classified diagnostic
  and trace facts; core platform only supplies an optional host-facing sink.
  Composition, product, or an external wrapper owns redaction, display, and
  export policy. This observability contract is not a prerequisite for
  initial-state or transition admission. Microsoft selector meanings, SVC
  details, guest communication-block layouts, BYOB validation, DOS namespace
  policy, and guest files remain external-wrapper concerns. A transition
  facility matches only consumer-registered undefined-instruction patterns;
  core defines no fixed opcode sequence, selector namespace, or guest-service
  semantics.

- [ ] **M5 T243: checked guest-memory contract (`TODO(High)`).** S1 maps the
  current memory/execution boundary and specifies the minimum checked
  read/write/copy/mapping-query contract. S2 implements it with focused fault
  tests and preserved NXVM regressions. It must not expose raw RAM pointers,
  DOS structures, or host-policy callbacks.
- [ ] **M5 T244: registered undefined-instruction transition (`TODO(High)`).**
  After T243 S2, add a consumer-registered, mode-restricted transition facility
  with checked state access and constrained outcomes. A match occurs at the
  architected invalid-instruction boundary before default fault delivery; core
  alone owns matching-byte IP consumption. The only dispositions are
  `unhandled` (normal invalid-instruction path), `handled-resume` (core applies
  a validated CPU-state patch, then consumes the registered pattern length),
  `stop`, and `fault`. The initial patch may alter only `EAX`, `EBX`, `ECX`,
  `EDX`, `ESI`, `EDI`, `EBP`, and arithmetic-status `CF`, `PF`, `AF`, `ZF`,
  `SF`, `OF`; it cannot alter `EIP`, `ESP`, other FLAGS, segments, descriptor
  caches, control/debug registers, FPU state, or guest memory. Handler-visible
  state is a copy. Core validates and commits the complete permitted patch and
  fixed IP advance atomically, or commits none of it. A later task must admit
  guest-buffer writes through its own bounded transactional contract before any
  transition service needs them. Any CPU mode transition remains exclusively in
  the core CPU's formally supported
  instruction/exception/IRET semantics. It defines no fixed instruction bytes,
  selector namespace, or guest-service ABI; NXVM registers none by default.
- [ ] **M5 T245: generic ROM mapping (`TODO(High)`).** Separate immutable
  read-only image mapping from VM firmware generation and boot policy. The VM
  profile remains the first consumer; image contents and firmware services stay
  outside core.
- [ ] **M5 T246: validated initial-state/entry-plan (`TODO(High)`).** Apply a
  validated immutable plan after clean core reset, supporting reset-state plus
  mapped image and direct prepared state without a second core boot mode.
- [ ] **M5 T247: narrow host-capability admission (`TODO(Medium)`).** Design
  copied event/presentation/wait contracts first. Admit each provider only with
  demonstrated policy-free reuse; generic file/directory and DOS namespace
  remain outside core/platform.

- [ ] **M5 architecture closure audit (`TODO(High)`).** Before M5 closes,
  rescan every core/vm/vdm module, composition root, target, test, and entry
  point for duplicate state, parallel paths, forwarding wrappers, global/TLS
  selectors, illegal dependencies, and misleading names.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Linux source is a
  portability asset, not a support claim. Add native POSIX compile and runtime
  probes after an approved POSIX environment is available; do not install WSL
  merely for this item.
- [ ] **Machine-profile admission (`TODO(Medium)`).** Design external machine
  profiles and optional user-supplied ROM manifests before PC110, Compaq,
  Award, or Phoenix behavior. Third-party ROMs are never bundled, downloaded,
  or committed.
- [ ] **VDM remains design-gated (`TODO(High)`).** Keep the scaffold
  non-runnable until its design milestones define the owned DOS module: loader,
  PSP/environment/DTA, DOS interrupt subset, vectors, fixture filesystem,
  input/display providers, cancellation, and corpus.
- [ ] **NXVDM CLI and host policy (`TODO(High)`).** After the VDM design gate
  and the shared-core product-naming migration, implement `nxvdm run`
  display/debug/no-program semantics, host-drive
  whitelist/hide policy with canonicalization and reparse checks, and the
  Windows 7--11 support matrix. Existing NXVM display selection is not a
  substitute for that product contract.

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
