# Long-Term Review Ledger

This ledger records only open debt and deferred admissions. Each entry carries
a priority and a bounded admission condition. It does not activate work,
allocate a task identifier, or override [CURRENT.md](CURRENT.md). Current goals,
candidate work, and detailed capability evidence belong in
[design/GOAL.md](../design/GOAL.md), [QUEUE.md](QUEUE.md), and
[etc/evidence/current-capability-baseline.md](../etc/evidence/current-capability-baseline.md).

T346 closes the selected deterministic PC/AT L3 event-ordering foundation and
transfers the unselected controller-service, peripheral, and compatibility
boundaries below. These entries remain unplanned until a later approved task
adopts them.


## Hardware And Compatibility Debt

- [ ] **External-ROM Model-40 and IBM 5170 boot-chain repair (`TODO(High)`).**
  T515 verifies one YAML-relative external asset path and complete unit
  coverage, but its full integration run records three real-ROM timeouts:
  DeskPro Model 40 with its declared Compaq EGA/CMOS/1.2 MB media, and IBM 5170
  with declared 1.2 MB and 360 KB media. Re-admit only as a source-led board,
  CPU, timer, controller and firmware checkpoint audit. Preserve T515's sole
  YAML asset request, Core device ownership and external ROM/CMOS mapping; do
  not bypass POST, synthesize F1/input, relax the terminal predicate, alter
  `build/output` YAML, or replace a real ROM with generated firmware.

- [ ] **XT/AT PIC topology and profile composition (`TODO(High)`).** T456
  implements the 8259A chip contract, not every machine wiring personality.
  A later task may add one immutable machine-composition declaration selecting
  a single XT PIC or an AT master/slave pair, including ports, source-to-line
  wiring and the selected cascade relation. Core owns PIC state, port behavior,
  interrupt arbitration and all guest-visible routing; VM/profile may only
  resolve and pass the frozen topology before Core machine creation. Admit only
  with IBM PC/XT and PC/AT board sources, a finite topology ledger, reset and
  IRQ/vector proof for each selected profile, and no second PIC dispatch path.
- [ ] **Later-AT ELCR and PCI interrupt routing (`TODO(High)`).** ELCR
  `4D0h/4D1h`, PIRQ/PCI INTx routing and any southbridge-specific masks are
  later chipset functions, not 8259A registers. Admit each concrete chipset or
  routing unit from its original manual as a separate Core-owned device and
  register/state contract; an extension profile may select the device, its
  immutable port map, wired inputs and supported configuration only. Do not add
  ELCR fields to `pic.c`, let VM mutate guest interrupt state, or infer one
  chipset's behavior from another's emulator implementation.
- [ ] **Local-APIC/IOAPIC transition (`TODO(Medium)`).** APIC and IOAPIC are
  separate interrupt-controller architectures, not a PIC extension. Admit them
  only after selected CPU/profile requirements, Intel APIC/IOAPIC primary
  manuals, register/delivery/reset ledgers and an explicit 8259A coexistence or
  handoff contract exist. Core owns their guest-visible state and delivery;
  profile chooses an immutable board presence/topology. Do not overload PIC
  ICW/OCW state, reuse PIC delivery helpers as a hidden second owner, or claim
  APIC support from PCI/ELCR work.

- [ ] **Advanced 8042 AUX protocol (`TODO(Medium)`).** T267 adds bounded
  sample-rate, resolution, and `E9h` status through the existing IRQ12 path.
  Defer wheel IDs/sample-rate handshake, scaling effect, remote/read-data,
  resend/error timing, and host capture to separately probed controller work;
  do not turn platform input into a DOS API or guest-memory shortcut.
- [ ] **Large/sparse and WASM raw HDD backing (`TODO(High)`).** The current VM
  provider accepts non-512-aligned images only within LBA28 and resident
  allocation limits. Keep that resident backend for small/offline images. A
  later M5 admission may add an opaque paged file/sparse backend for native
  files, OPFS, IndexedDB, or an already-populated WASM cache, with atomic
  persistence, generation/error mapping, cache ownership, and ATA's visible
  LBA28 subset; guest access beyond that subset also requires separately
  admitted LBA48. The current media-provider callback is synchronous: an HTTP
  Range cache miss must not block or mutate guest state from a host callback;
  it requires a separately admitted pending-I/O/controller-ready/IRQ timing
  model. Do not add an unconsumed generic filesystem API.
- [ ] **IBM 5170 MFM/ST-506 physical-media residual (`TODO(Medium)`).** T479
  corrects the earlier false claim that the selected 5170-339 has no fixed
  disk: IBM's product reference identifies its one 30 MB fixed disk and
  adapter. T479 owns the logical IBM adapter/controller and frozen profile
  route. Retain only raw MFM encoding, ECC recovery, drive mechanics, adapter
  parity/NMI evidence and source-qualified service-time phases here. Do not
  alias ATA media or timing to IBM MFM, import firmware/media, or use a generic
  hard-disk backend as proof.
- [ ] **Compaq 40 MB physical fixed-disk media semantics (`TODO(Medium)`).** T386 S5 establishes only the primary-backed normal CHS PIO controller path over the existing pure logical RAW-IMG medium. Read/Write Long, format-track parameter/data semantics, ECC/error-recovery classes, physical sector identity, drive-side controller service and their IRQ/DRQ timing require a separately selected physical-media representation and board receiver. Admit this only after the selected DeskPro storage/profile path is published and a primary source plus bounded corpus defines the required observables; preserve the current normal-route proof, do not reinterpret ATA/MFM media, and do not claim raw IMG represents ECC or physical sectors.
- [ ] **Broaden digital CGA beyond the frozen supported surface (`TODO(Medium)`).**
  The baseline-machine capability ledger, 5170 selected-device closure, and
  current-product device closure own every CGA capability that remains publicly
  supported. Retain only later geometry or light-pen breadth here. Every later
  admission retains one VADP owner, B8000h mapping, copied-frame output, and
  port/memory/frame plus DOS-fixture evidence; it may not use this debt entry
  to leave an exposed CGA mode incomplete.
- [ ] **Compaq CECG physical, firmware and board behavior (`TODO(Medium)`).** T386 S6/S9/S10/S11/S12/S13 establish the selected Compaq Enhanced Color Graphics digital personality, 64-colour capture mapping, static configuration reads, source-backed `3C6h` state/reset, Feature-Control-to-Environment state, the selected no-light-pen latch/status, `3C2h` bit-1 CPU-video-memory decode gating, bit-0 `3Bxh`/`3Dxh` CRTC/status/Feature/light-pen routing, and bits 2--3 SW1/Input-Status-0 observability for the selected no-Special-Features/no-vertical-IRQ declaration. T386 S28 closes `3C2h` bit-5 Odd/Even page selection through the shared VADP state, memory and copied-snapshot receiver. Before DeskPro board/device timing or final Model-40 acceptance, admit the remaining source-backed physical/firmware effects: Special-Features selection, monitor blanking/selection, sync polarity, multiplexed live-video status, option-ROM/BIOS mode programming, physical monitor signalling, raster/service duration, ISA availability/waits and board memory availability through the selected DeskPro composition/firmware and board receiver. Do not infer these from generic/IBM EGA, expose an incomplete profile, or call the captured palette an analog-monitor model.
- [ ] **DeskPro 386/16 ROM diskette diagnostic relation (`TODO(High)`).** T501 S2
  proves the selected one-drive composition, lawful unready unit-1 `ST3` path,
  continuous Core progression, Core-owned input and copied POST display.  The
  Compaq Maintenance and Service Guide defines `601` and F1 `RESUME`, but the
  available original material does not define the ROM's unit-1 test order,
  expected FDC/drive-change observation, or the board electrical/timing
  relation that distinguishes its `601` path.  Re-admit this one indivisible
  board/FDD/ROM relation only after original schematic, service timing, or a
  source-backed ROM diagnostic description supplies that missing relation.
  Preserve one FDD/media owner and the existing Core FDC/VM composition; do
  not add a second drive, fake ready/change state, alter CMOS, patch firmware,
  bypass F1, infer a delay, or reopen scheduler/keyboard/display owners.  See
  [T501 List 1](../etc/evidence/t501-s2-model40-diagnostic-list-1.md) and
  [List 2](../etc/evidence/t501-s2-model40-diagnostic-list-2.md).
- [ ] **Broaden digital EGA and admit VGA compatibility (TODO(Medium)).** T352
  closes only direct planar `320x200x16` and mode-10 `640x350x16` through one
  VADP owner. The completed T386 shared-VADP receiver consumes only the CECG-required Odd/Even page mechanism; admit all remaining adapter breadth only with a complete IBM EGA/VGA
  register/mode/CRTC matrix, explicit aperture and palette semantics, reset and
  copied-frame proof, and focused plus guest-path evidence. Do not infer VGA,
  VBE, generic BIOS-mode, or arbitrary CRTC support from T352's selected paths.
- [ ] **CGA composite-video fidelity (`TODO(Low)`).** Treat NTSC artifact
  color, phase, and colorburst as an optional renderer/profile capability only
  after digital CGA is complete. Do not fold it into VADP digital state or use
  it to claim EGA/VGA support.
- [ ] **PC/AT host speaker rendering and PPI breadth (`TODO(Medium)`).** T421
  provides one Core-owned port-`61h`/PIT-channel-2 logical speaker line shared
  by Model-40 D4 and 5170 planar selection. It does not provide host audio,
  waveform/amplitude or physical-frequency behavior, 8255 PPI breadth, or a
  DOS/Windows audio corpus. Re-admit only with a named consumer, deterministic
  event ownership, reset proof and a copied host-audio boundary; host audio
  time must never become guest time.
- [ ] **Serial, parallel, and game-port interfaces (TODO(Low)).** These
  controllers have no selected core owners. Admit one interface at a time only
  from a named corpus and hardware contract, preserving core-owned port,
  IRQ/DMA, reset, and deterministic-event boundaries; do not add generic host
  device passthrough or infer Windows relevance from reference-emulator scope.

## CPU, Time, And Debugging Debt


- [ ] **Architectural reset, shutdown, and triple-fault policy (`TODO(Medium)`).**
  The machine currently exposes a KBC-originated reset request and lifecycle
  stop, but it has no Intel-defined CPU reset/shutdown/triple-fault contract.
  Admit only as a dedicated shared CPU/machine lifecycle task after defining
  the reset vector, state publication, fault escalation, and product-visible
  recovery boundary; do not infer that policy from a device request or fold it
  into the 80386DX instruction matrix.

- [ ] **Broaden real-mode 8086 corpus (`TODO(High)`).** T240 established a
  reset-vector baseline for segment override, `REP`/direction strings,
  `INT`/`IRET`, port I/O, and fault retention.  Extend only through a failing
  ROM/DOS/device path to uncovered 8086 families such as arithmetic/FLAGS,
  conditional control transfer, stack edge cases, and string compare/scan.
  MS-DOS `MEM` remains a regression sample, not an 80386-completeness claim.
- [ ] **286/386 protected-mode program (`TODO(Low)`, T258--T261, T263).**
  T257 admits only GDT/CPL0 16-bit entry, selector loads, same-CPL far
  transfer, and diagnostic validation faults. T258 closes CPL0 4 KiB paging,
  narrowed CR0/CR2/CR3 forms, and core diagnostic `#PF`; it deliberately does
  not create a false CPL3/TSS I/O test path. T259 closes the first 16-bit
  CPL3-to-CPL0 software-gate, TSS-stack-source, outer-`IRET`, and `#GP`
  delivery subset; it does not prove CPL3 paging permissions or generic IDT
  delivery. T260 closes TSS I/O-map behavior through a real CPL3 corpus,
  T261 admits task switching. T263 closes the corpus-proven 80286 `ARPL
  r/m16,r16` profile-gate mismatch; every remaining instruction family remains
  corpus-specific. This remains lower ROI than the real-mode PC/AT device
  route.
- [ ] **System-extension consumer admission (`TODO(Low)`, T312).** T312
  withdrew 32-bit TSS/task-gate/nested-return, LDT breadth, V86, guest DR/TR,
  and remaining system forms because handlers or focused probes alone were not
  real VM/M5 consumers. Re-admit only from an owner-supplied bounded checkpoint
  or trace naming one form and its required state/fault boundary; preserve the
  per-family thresholds in
  [T312 evidence](../etc/evidence/t312-system-extension-admission.md). Do not use
  this ledger entry to start M6 or claim system-family completion.
- [ ] **Internal descriptor vocabulary cleanup (`TODO(Low)`).** Rename the
  internal segment-descriptor field vocabulary only through a future
  source-structure admission with an explicit caller inventory. Preserve data
  layout, debugger-visible behavior, and CPU execution semantics; this is not
  an instruction-family or compatibility claim.
- [ ] **Default PC/AT generated-ROM materialization (`TODO(Medium)`).** The
  default BIOS still constructs ROM code and BDA state during reset through the
  one profile firmware path. Split immutable ROM-image construction from BDA
  reset writes, then bind the image through the existing core immutable-ROM
  mapping contract. Do not call the current generated BIOS an immutable ROM,
  add a second boot route, or change the retained boot UX.
- [ ] **Core debug control-register mutation contract (`TODO(Medium)`).**
  `vm/composition/session/debug_target.c` reaches the operation-based
  `core_machine_debug_write_register()` / `core_machine_debug_patch_registers()`
  boundary for debugger-originated `CR0`--`CR4` writes. This is not a guest
  execution path, but it can construct paging/control state that guest `MOV
  CRx` may not. Admit a core-owned debug mutation contract only after
  preserving retained debugger UX and defining lifecycle, validation,
  raw-override policy, and focused regression; do not let VM composition
  acquire paging semantics.
- [ ] **Broaden present x87 (`TODO(Low)`).** T262 closes only the exact-8087
  finite-`m32real`/basic-arithmetic baseline with owned state, exceptions, and
  `FWAIT`. T317 S1 reconciles retained ESC/WAIT `#NM` vector-7 delivery tests
  but does not implement coprocessor execution. Re-admit 80287/80387, broad
  formats, complete IEEE behavior, environment save/restore, and protected-mode
  FPU delivery only with a corpus-driven instruction matrix; that admission must
  rerun the retained ESC/WAIT no-FPU, vector-7, optional-8087, and provider
  boundary tests alongside its new execution tests.
- [ ] **VME/PVI virtual-8086 extensions (`TODO(Medium)`).** The 80386DX
  program covers ordinary VM86 execution and delivery foundations, not the
  later VME/PVI extensions. Admit only with the Intel control-register and
  virtual-interrupt contract, VM86-to-CPL0 frame and TSS-stack evidence,
  privilege/fault matrix, and regressions that retain ordinary VM86 behavior.
  Do not infer VME/PVI support from a VM86 instruction or exception path.
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
- [ ] **Holistic execution-path architecture audit (`TODO(Medium)`).** Audit
  cross-cutting CPU and machine-state work for incremental "patch-on-patch"
  implementation that closes a local symptom without first establishing the
  complete architectural state model, validation order, commit boundary,
  caller set, and failure semantics. Admit only as a bounded family or
  subsystem review with an Intel-authoritative form/state matrix, source and
  ownership map, explicit preflight/commit/rollback contract, comparable
  focused regressions, and a transfer record for every excluded behavior.
  Repairs must address the shared model or deliberately retain a proven local
  boundary; do not turn this debt into a repository-wide rewrite, speculative
  abstraction, or an excuse to weaken existing evidence.  The concrete
  admission method and observed failure shape are retained in
  [the supporting debt report](../etc/evidence/holistic-execution-path-audit-debt.md).
## Timing-Fidelity Debt

The current core is deterministic and host-clock-independent: completed
instructions advance core elapsed ticks and T346 binds selected device work to
one reset-safe due-event timeline with stated equal-tick ordering and copied
transaction checkpoints. This is the project's L3 event-and-bus contract, not
cycle-exact hardware timing. These entries remain deferred compatibility
admissions, not the default definition of NXVM completion.

- [ ] **8086/80186 dynamic arithmetic source formula (`TODO(Medium)`).** T360
  S3 confirms that the selected 8086 `MUL`/`IMUL` and 80186
  `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate-`IMUL` rows are primary
  table ranges without an operand-to-clock rule. Their L3 debt remains an
  Intel-primary exact formula or separately owner-approved hardware-observation
  contract with profile-local capture and publication/rollback design. The
  queued CPU timing program must first apply the [Td S121 evidence/model
  ladder](../etc/evidence/td-s121-cpu-timing-evidence-first-governance.md) to
  establish a labelled L2 model (or an explicit L1 exception) for every form;
  no successful row may retain an anonymous `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`
  or one-clock hardware claim. The 8086 rows remain a required input to the
  queued 8088 CPU-profile closure; the 80186-only rows remain unplanned because
  M5 selects no 80186 machine.

- [ ] **DeskPro 386 vendor-BYOB firmware corpus (`TODO(Medium)`).** T424 proves the lawful manifest-to-copied-immutable-ROM-to-reset-vector execution and reset-replay route with project-owned synthetic chips. It does not validate an owner-supplied vendor firmware lifecycle, POST programming effects, device-service timing, cancellation behavior or a boot checkpoint. Admit only with a separately approved external BYOB corpus under the T390 S2 containment contract and existing Core/VM lifecycle owners. Do not commit firmware, record a machine-local path, make vendor ROM a default dependency or infer behavior from the ROM mapping. |
- [ ] **DeskPro 386 physical-observable device timing (`TODO(Medium)`).** T397 closes the selected logical FDC/DMA2/IRQ6, HDC/IRQ14, CECG, KBC/NMI and D4 routes, but no route has the required primary-backed physical unit/phase, guest-visible checkpoint and qualified retirement conversion. Re-admit each receiver only with its selected Model-40 source/measurement contract, reset/cancellation replay, trace containment and existing Core owner. T408 adds original D4 CPU-memory evidence (initial/row-miss two waits; row hit zero waits), T409 classifies prefetch, execution fetch, page walk and data transactions, and T419 S5 adds the single Core external-cycle/prefetch-overlap boundary: only an explicitly declared in-flight sequential prefetch can receive the 2 KiB hit result. T419 S6 proves the current synchronous 15-byte Core prefetch refill has no in-flight predecessor and therefore cannot publish the existing overlap declaration. The remaining receiver is an original- or reference-backed asynchronous external-prefetch lifecycle plus D4 physical phase/row-bank evidence; synchronous completed adjacency must remain a miss. Preserve raw-IMG, copied presentation and deterministic Core timeline behavior. Do not derive any remaining scalar from generic AT, PCjs, host time or a new Model-40 scheduler. |
- [ ] **Unsupported physical timing (`TODO(Medium)`).** The M5 Queue owns the
  three baseline machines and every device capability currently supported by
  the product. Retain only behavior outside that surface here: analog/pin-level
  behavior, unsupported peripherals, optional protocols not exposed by the
  product, and a historical clone without an approved primary contract. Admit
  one boundary only after the M5 L3 audit records why it is not supported; do
  not create a second bus or device-timing owner.
- [ ] **Profile physical-timebase closure (`TODO(Medium)`).** T470 retains
  default PC/AT, Model-339 and Model-40 at `UNQUALIFIED` because none proves a
  complete CPU retirement, transaction/wait, controller-deadline and
  oscillator/divider chain on the one Core axis. Admit a profile only after
  primary-backed evidence closes every named chain and its immutable rate;
  then add Standard's ahead-only host wait as a VM consumer of copied Core
  progress. Do not use nominal MHz, PIT ratios, host elapsed time, a VM tick
  accumulator or a second scheduler to bridge the gap.

## Architecture And Portability Debt
- [ ] **Console speed-help accuracy (`TODO(Low)`).** `SPEED` currently says
  Standard retains a fixed L2 HLT host-load backoff and Turbo is merely
  reserved, although the admitted T474 policy is copied-observation Standard
  pacing when a macro axis is available and Turbo no-wait. Admit this only as
  a bounded Console-text/test correction that preserves the existing two-mode
  parser and session speed authority; do not add a pacing path, tick source or
  third speed mode.
- [ ] **Residual direct-production strict compilation (`TODO(Medium)`).**
  T345 closes the whole configured direct-command ledger but retains 52
  production-source entries that cannot truthfully receive target-local
  `-Werror` as part of a mixed target: the core machine executor, xasm/debug
  and its failure smokes, retained Console and its failure smoke, mixed Win32
  platform, VM machine, and VM composition/firmware targets. Their exact
  source list, warning counts, behavior risk, and one admission trigger per
  domain are retained in [the T345 production ledger](../etc/evidence/t345-s3-production-ownership.md).
  Admit one named ownership domain only when its source is substantively
  changed or can be safely separated; record the direct diagnostic baseline,
  caller/behavior sweep, target-local options, and regression result. Do not
  add a global flag, treat a linked strict library as direct coverage, or use a
  blanket inherited exemption.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Linux source is a
  portability asset, not a support claim. Add native POSIX compile and runtime
  probes after an approved POSIX environment is available; do not install WSL
  merely for this item.
- [ ] **AI DevBox exploration (`TODO(Low)`).** Assess controlled machine
  capabilities for legacy-program development only after an owner-approved
  product goal, trust boundary, and reproducible-fixture plan exist. It must
  not silently turn host computer-use access into a generic guest capability.
