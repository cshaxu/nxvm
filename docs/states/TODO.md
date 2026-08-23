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
- [ ] **IBM 5170 MFM/ST-506 fixed-disk route (`TODO(Medium)`).** The selected
  Model 339 Type 3 L3 baseline deliberately has no fixed disk. Its historical
  30 MB storage uses IBM's Fixed Disk/Diskette Adapter and an MFM/ST-506 path,
  not the current ATA/HDC controller. Admit it only as a separately selected
  controller/drive profile with the adapter's port/IRQ14/reset/error and
  command/service contract, MFM data and timing ownership, one project-owned
  probe corpus, and a decision whether its adapter-parity state produces
  I/O-channel-check NMI. Do not alias ATA registers, ATA media or ATA timing to
  IBM MFM, import firmware/media, or use a generic hard-disk backend as proof.
- [ ] **Compaq 40 MB physical fixed-disk media semantics (`TODO(Medium)`).** T386 S5 establishes only the primary-backed normal CHS PIO controller path over the existing pure logical RAW-IMG medium. Read/Write Long, format-track parameter/data semantics, ECC/error-recovery classes, physical sector identity, drive-side controller service and their IRQ/DRQ timing require a separately selected physical-media representation and board receiver. Admit this only after the selected DeskPro storage/profile path is published and a primary source plus bounded corpus defines the required observables; preserve the current normal-route proof, do not reinterpret ATA/MFM media, and do not claim raw IMG represents ECC or physical sectors.
- [ ] **Broaden digital CGA beyond the frozen supported surface (`TODO(Medium)`).**
  The baseline-machine capability ledger, 5170 selected-device closure, and
  current-product device closure own every CGA capability that remains publicly
  supported. Retain only later geometry or light-pen breadth here. Every later
  admission retains one VADP owner, B8000h mapping, copied-frame output, and
  port/memory/frame plus DOS-fixture evidence; it may not use this debt entry
  to leave an exposed CGA mode incomplete.
- [ ] **Compaq CECG physical, firmware and board behavior (`TODO(Medium)`).** T386 S6/S9/S10/S11/S12/S13 establish the selected Compaq Enhanced Color Graphics digital personality, 64-colour capture mapping, static configuration reads, source-backed `3C6h` state/reset, Feature-Control-to-Environment state, the selected no-light-pen latch/status, `3C2h` bit-1 CPU-video-memory decode gating, bit-0 `3Bxh`/`3Dxh` CRTC/status/Feature/light-pen routing, and bits 2--3 SW1/Input-Status-0 observability for the selected no-Special-Features/no-vertical-IRQ declaration. T386 S28 closes `3C2h` bit-5 Odd/Even page selection through the shared VADP state, memory and copied-snapshot receiver. Before DeskPro board/device timing or final Model-40 acceptance, admit the remaining source-backed physical/firmware effects: Special-Features selection, monitor blanking/selection, sync polarity, multiplexed live-video status, option-ROM/BIOS mode programming, physical monitor signalling, raster/service duration, ISA availability/waits and board memory availability through the selected DeskPro composition/firmware and board receiver. Do not infer these from generic/IBM EGA, expose an incomplete profile, or call the captured palette an analog-monitor model.
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

- [ ] **VM firmware materialization and dead-code closure (`TODO(Medium)`).**
  Manual audit Td S127 reconfirmed the retained T345 evidence: composition
  `profile_firmware.c` materializes five generated-firmware C string literals
  above the C11 guaranteed translation limit, while `session.c` retains the
  unused `vm_session_read_u16` helper. Neither condition has a current TODO
  transfer. Admit one bounded firmware-owner task to replace oversized source
  literals with an explicit chunk/capacity materialization contract that proves
  byte-for-byte ROM equivalence, and remove or give a real caller to the dead
  helper. Do not silence warnings, mechanically reflow generated instruction
  text without equivalence proof, or use this cleanup to change firmware
  behavior.

- [ ] **Product session-command authority unification (`TODO(High)`).** Manual
  audit Td S127 found `core_product_session_command_execute()` implements a
  full `SESSION LIST/OPEN/SELECT/CLOSE` grammar but has no production caller;
  `vm/product/console.c` independently implements the same command family with
  different profile-open and final-session-close behavior. This is a duplicate
  product command abstraction with no single owner or contract. Admit one
  Core-product/VM-product task to select one command authority, define the
  product-specific profile-selection seam and exact error/output semantics, and
  retire the unused parallel path with focused regressions. Do not add aliases,
  leave two parsers, or move VM profile policy into generic Core session state.

- [ ] **Legacy VM media type/public-name retirement (`TODO(Medium)`).** Manual
  audit Td S125 found cross-module headers `vm/machine/fdd.h` and
  `vm/machine/hdd.h` export mutable `t_fdd`, `t_hdd`, `t_*_data`, and
  `t_*_connect` layouts. The unowned legacy names violate searchable
  ownership-path naming, while their public fields make callers depend on
  storage internals. After the cross-module contract task is admitted, retire
  these exported layouts in favor of owner-prefixed opaque media handles plus
  copied geometry/status observations. Inventory every production and test
  field access; do not do a cosmetic typedef rename or leave alias types.

- [ ] **VDM skeleton forwarding-layer disposition (`TODO(Medium)`).** Manual
  audit Td S125 found `src/vdm/composition/session.c` provides a second public
  API that, except for allocation and null checks, only forwards every
  operation to `vdm_machine_dos_minimal_*`. It performs no composition,
  translation, validation, or lifecycle policy and is therefore a pure
  forwarding abstraction prohibited by the coding rule. Before M6/M8 work,
  admit a bounded decision: remove the wrapper and consume the machine boundary
  directly, or give the composition root a real mantle/dos session-assembly
  responsibility with explicit lifetime and failure semantics. Preserve the
  current non-runnable VDM skeleton boundary; do not promote it to a product or
  add compatibility aliases.

- [ ] **Core machine coordinator decomposition (`TODO(High)`).** Manual audit
  Td S125 found `src/core/machine/machine.c` conflates distinct owners:
  source-timing models, external-cycle accounting, plan validation, scheduler
  advancement, firmware access, display/DMA/FDC/HDC/RTC/D4/PC-AT board
  configuration, lifecycle, and fault publication. These already have real
  interface and subsystem boundaries, so the single approximately 7,000-line
  coordinator is not a cohesive owner-local implementation. Admit an
  implementation task to define the coordinator's narrow assembly/lifecycle
  responsibility, move each independent mechanism behind its existing or one
  new owner-local boundary, preserve one Core transaction path and all current
  plan validation/rollback semantics, and update focused regressions. Do not
  mechanically split by line count, duplicate machine state, or create a new
  generic framework.

- [ ] **Cross-module VM contract boundary normalization (`TODO(High)`).** Manual
  architecture/code audit Td S125 found `vm/composition/session`, `vm/main`,
  and tests directly consuming non-contract headers such as
  `vm/platform/platform.h`, `vm/machine/fdd.h`, `vm/machine/hdd.h`,
  `vm/machine/debug.h`, `vm/product/console.h`, and
  `vm/profile/default_profile/pc_at_profile.h`, default-firmware `bios.h`, and
  composition `control.h`/`fault.h`. These headers expose concrete mutable
  layouts, owner-local lifecycle operations, and--in `bios.h`--large generated
  firmware implementation macros across declared module boundaries, contrary
  to the required minimal `*_interface.h` contract boundary. Td S128 further
  confirms that `console.h` exports parser/provider/session-manager/catalog
  state and `session_catalog.h` exports all mutable parsed records; the same
  pass found composition's `provider.h` includes that outer product contract
  and implements its adapter, reversing the intended dependency direction.
  Admit one
  implementation task to inventory every cross-owner consumer, define
  opaque/copied/bounded interfaces with explicit lifetime and failure semantics,
  move owner-private layouts and firmware construction behind them, relocate
  product adaptation to the composition root, update tests to prove the public
  boundary, and remove the direct paths atomically. Do not rename headers
  without retiring the exposed layouts/direct production consumers, retain the
  reverse product include through a forwarding header, or add test-only state
  accessors.

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

## Architecture And Portability Debt

- [ ] **CMake VM-composition native-link separation (`TODO(High)`).** Manual
  audit Td S127 found `CMakeLists.txt` unconditionally links the
  cross-platform `vm-composition` static library publicly to `user32` and
  `gdi32`, despite separate Linux and Win32 adapters. No alias or platform
  conditional surrounds those names. This makes every Linux composition/test
  consumer inherit Windows link requirements and contradicts the declared
  platform-adapter boundary. Admit one build-ownership task to move native
  libraries to the owning Win32 adapter/selected runnable target behind a
  platform condition, keep Linux link requirements in the Linux adapter, and
  verify both target graphs. Do not fork composition sources by host, add fake
  Unix `user32` targets, or make native GUI libraries a Core dependency.

- [ ] **CMake duplicate production-source ownership repair (`TODO(Medium)`).**
  Manual audit Td S127 found `src/vm/composition/session/machine_info.c` is in
  `VM_COMPOSITION_SOURCES` and therefore `vm-composition`, but
  `add_current_vm_artifact()` also compiles it directly while linking that same
  library; the unused `VM_RUNTIME_SOURCES` aggregate is a second stale build
  declaration. Static-archive extraction can mask duplicate symbols, yet the
  current product has two compilation/ownership routes for one composition
  source and dead target vocabulary. Admit one build-graph task to retain a
  single owning target, make the artifact consume its public composition
  capability, retire stale aggregates, and add a narrow source-to-target
  uniqueness check. Do not paper over the duplicate with linker ordering,
  object-library aliases, or a second forwarding wrapper.

- [ ] **Core platform stateful-interface encapsulation (`TODO(High)`).** Manual
  audit Td S126 found public Core platform headers expose owner-local mutable
  layouts across module boundaries: `core_platform_backing_resource` publishes
  its context/provider/closed state, `core_platform_input_source` publishes its
  lock/sink/context, and `core_platform_presentation_mailbox` publishes its
  lock/activity/frame. VM composition embeds and passes these layouts to host
  adapters and tests. Admit one Core-platform task to make the stateful
  instances opaque or owner-allocated, retain only bounded create/close,
  submit/publish/capture and copied-frame operations with explicit lifetime
  semantics, and migrate VM consumers atomically. Do not move host policy into
  Core, expose test-only fields, or replace the three mechanisms with a generic
  callback framework.

- [ ] **Host backing-resource production-path convergence (`TODO(High)`).**
  Manual audit Td S126 found `core_platform_backing_resource` is consumed only
  by its implementation and tests, while production FDD/HDD, media-save,
  Model-40 BYOB firmware, session catalog, and debugger paths independently
  call the global C file facade. This leaves a duplicate, unconsumed host-I/O
  abstraction and scattered open/read/write/flush/close failure semantics.
  Admit one bounded storage-boundary task to select one owned production path,
  map the current file-backed media and BYOB lifecycle/error behavior through
  it, and retire or narrow the unused parallel contract with focused native
  tests. Preserve synchronous guest-media semantics, atomic media-save
  behavior, and source-policy containment; do not introduce a generic
  filesystem API, asynchronous host I/O into guest callbacks, or change guest
  media formats.

- [ ] **Core machine collaborator-state interface sealing (`TODO(High)`).**
  Manual audit Td S126 found `core_machine_media_registry` and
  `core_machine_display_provider_slot` in public Core machine interfaces
  expose mutable provider/context bindings and frozen state, then are embedded
  directly in `vm_session`. These are Core-owned runtime collaborator states,
  not copied configuration or observations, so their public layouts violate
  the minimal cross-module contract boundary. Admit one Core-machine task to
  make the registry and display binding owner-private or opaque, provide only
  bounded setup/freeze/finalize and media/display operations with explicit
  lifetime/error semantics, and migrate VM composition/tests as one cutover.
  Preserve Core-owned routing and copied display/media observations; do not
  turn this into a second machine object, public owner token, or generic device
  registry.

- [ ] **Core machine-plan provider-endpoint closure (`TODO(High)`).** Manual
  audit Td S126 found the public `core_machine_plan` is not solely copied
  configuration: `core_machine_plan_memory_device` exports callback tables and
  an owner pointer; the topology also carries a D4 parity-mask pointer, display
  provider-slot pointer, and media-registry pointers. VM composition/profile
  code fills those endpoints directly before `core_machine_create_from_plan()`.
  This exposes raw device/registry state and lifetime across the Core/VM plan
  boundary, contrary to the cross-module contract rule despite Core copying the
  plan structure. Admit one Core-plan task to retain declarative copied board
  values while replacing endpoint storage with bounded, opaque registrations
  and explicit create/reset/destroy failure semantics; migrate default,
  Model-339, and Model-40 composition together. Do not remove atomic plan
  validation, reintroduce post-create mutable topology, or create a generic
  plugin/device framework.

- [ ] **Core product debugger context boundary repair (`TODO(High)`).** Manual
  audit Td S126 found `core/product/debug/debug.h` exports the mutable
  `core_product_debug_context` layout--command buffers, parse/address state,
  debug target, input provider, and wait dependency--and VM composition embeds
  it directly in `vm_session`. The roughly 2,600-line command interpreter owns
  that state, so the public layout is neither a minimal interface nor a copied
  observation. Admit one Core-product task to introduce an opaque debugger
  session/capability with bounded create, command, observation, and destroy
  operations; make target and wait lifetimes explicit; and migrate VM and
  focused tests. Preserve the retained debugger UX and Core/VM direction; do
  not add a second command interpreter, test-only field access, or a generic
  console framework.

- [ ] **Session-manager raw-object escape removal (`TODO(High)`).** Manual
  audit Td S125 found the public Core product session contract exposes
  `core_product_session_manager_borrow_selected(..., C_VOID **out_session)`,
  while its provider `open` callback returns the same untyped session object.
  `vm/composition/session/console_machine_adapter.c` casts that escape back to
  `vm_session *`. This is a raw cross-module session pointer with no typed
  lifetime, close, or failure contract, forbidden by the architecture rule.
  Admit one Core-product/VM-composition task to replace it with an opaque
  selected-session capability or bounded manager-dispatched operations, make
  the borrow/close lifetime explicit, and migrate console and tests. Do not
  expose a typed `vm_session *`, add test-only casts, or make the manager own
  VM machine internals.

- [ ] **Public raw-borrow verifier scope repair (`TODO(Medium)`).** Manual
  audit Td S125 found `tools/VerifyPublicRawBorrowClosure.ps1` reports success
  while the public session-manager contract exports
  `C_VOID **out_session` and VM casts that object to `vm_session *`. The script
  only recognizes a narrow historical set of Core borrow names, profile words,
  and private Core layouts, so its success is not evidence of the stated
  generic closure. After the raw-session escape is removed, admit one bounded
  verifier task to encode that concrete forbidden public object-escape shape
  and a positive/negative self-test. Keep the rule narrow and source-shape
  specific; do not build a general natural-language architecture classifier.

- [ ] **VM profile implementation-boundary repair (`TODO(High)`).** Manual
  audit Td S125 found `vm/profile/default_profile/pc_at_profile.h` and
  `vm/profile/model40/model40.h` publish owner-local composition state across
  module boundaries. The latter exposes a writable 128 KiB D4 backing array,
  parity state, and `core_machine *`; the former publishes the full PC/AT
  topology/configuration representation. Consumers and tests consequently
  couple to profile storage and Core object representation rather than declared
  profile operations or copied observations. Td S128 additionally found the
  test-only `vm_session_create_model40_private()` public construction path
  duplicates the production BYOB constructor's Model-40 Core configuration
  before both enter one lifecycle. Admit one profile-owner task to
  separate immutable profile selection data from private, stateful composition
  devices; retain one production construction/materialization route with
  copied synthetic-ROM fixture input; define bounded initialization,
  observation, lifetime, and failure contracts; and migrate every consumer and
  test atomically. Preserve the existing profile-specific hardware semantics;
  do not hide the problem behind typedef renames, a parallel test constructor,
  or a generic profile framework.

- [ ] **Cross-owner test-boundary repair (`TODO(High)`).** Manual
  audit Td S125/S126/S127 found product, platform, and machine integration
  tests directly depend on private `vm_session`, `core_machine`, media,
  firmware, control, and platform-handle fields. S127 confirmed that this is
  systemic rather than confined to a few fixtures: at least 33 `vm_*` machine
  tests dereference `session->core_machine` internals, including CPU, memory,
  port, topology, KBC, FDC, and HDC state; the shared
  `tests/support/core_machine_cpu_fixture.h` also exposes prepared Core
  storage to both Core and VM consumers. Examples include
  `tests/products/nxvm_default_profile_smoke.c`,
  `tests/products/vm_session_media_lifecycle_s3_smoke.c`,
  `tests/products/vm_model40_hdc_s26_smoke.c`, and
  `tests/platform/vm_multi_window_session_smoke.c`; many
  `tests/machine/vm_*` integration tests include session control/fault or BIOS
  internals, and `vm_model40_integration_s8_smoke.c` reaches embedded executor
  port/device state directly. Admit one test-boundary task to classify
  same-owner Core fixtures separately from cross-owner product/platform/machine
  tests, move any shared fixture to the owning test layer, replace the latter
  with declared operations and copied observations, and preserve each test's
  behavioral assertion. Do not make test-only getters, retain a cross-owner
  white-box fixture, or expand production public layouts to keep fixtures
  compiling.

- [ ] **VM platform adapter contract encapsulation (`TODO(High)`).** Manual
  audit Td S125/S126 found `vm/platform/platform.h` exports mutable
  `vm_platform_run_context` and `vm_platform_run_handle` layouts, while sibling
  `host_surface.h`, request transport, execution, and virtual-time headers
  similarly publish native handles, leases, locks, callback contexts, backend,
  execution transport, and display-transition state across the
  composition/platform boundary. This lets composition couple to host-adapter
  representation rather than bounded operations with explicit
  lifetime/failure semantics. Admit one platform-owner task to make stateful
  instances opaque or owner-private, expose only needed display/input/run and
  copied-observation operations, and preserve Linux/Win32 exclusive surface
  lease cleanup. Do not move host policy into Core or replace the two host
  adapters with a generic host framework.

- [ ] **Dormant VM request-bridge smoke interface drift (`TODO(Medium)`).**
  A T345 whole-tree audit reproduced that non-current
  `vm-request-bridge-smoke` still names removed `VM_PLATFORM_REQUEST_KEY_PRESS`
  and `key_press` request fields, so an `all` build stops before completion.
  Admit only with the request-transport owner: reconcile the smoke with the
  current request event contract, prove copied payload semantics, and decide
  whether it becomes current-gate or remains an explicitly non-current target.
  Do not restore production compatibility aliases or quietly exclude it from
  an all-target build.
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
- [ ] **Parallel Console profile-smoke isolation (`TODO(High)`).** The current-gate Console lifecycle and 64-bit-memory smokes write fixed YAML filenames into the shared source working directory, then invoke catalog discovery on `.`. A parallel `ctest` can make either test discover both profiles and select a different menu ordinal; the affected memory smoke failed once in the T386 S5 parallel replay and passed alone and in the 255/255 serial control. Admit a bounded test/CMake isolation repair that gives each smoke an owned profile directory or CTest working directory while retaining startup catalog discovery, stdin restoration and the existing product route. Prove repeated parallel current-gate runs; do not change Console selection semantics or hide the test from the gate.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Linux source is a
  portability asset, not a support claim. Add native POSIX compile and runtime
  probes after an approved POSIX environment is available; do not install WSL
  merely for this item.
- [ ] **AI DevBox exploration (`TODO(Low)`).** Assess controlled machine
  capabilities for legacy-program development only after an owner-approved
  product goal, trust boundary, and reproducible-fixture plan exist. It must
  not silently turn host computer-use access into a generic guest capability.
