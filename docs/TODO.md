# Long-Term Review Ledger

This ledger records only open debt and deferred admissions. Each entry carries
a priority and a bounded admission condition. It does not activate work,
allocate a task identifier, or override [STATUS.md](STATUS.md). Current goals,
candidate work, and detailed capability evidence belong in
[design/GOAL.md](design/GOAL.md), [QUEUE.md](QUEUE.md), and
[etc/evidence/current-capability-baseline.md](etc/evidence/current-capability-baseline.md).


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
- [ ] **Complete digital CGA (`TODO(Medium)`).** Keep it separate from EGA/VGA.
  T254 admitted `640x200x2`; T266 closes the bounded 6845 text subset:
  display-start/page, cursor address/shape/hidden state, and deterministic
  display-enable/vertical-retrace. Deferred: additional 40/80-column geometry
  breadth and light pen. Every admission retains one VADP owner, B8000h
  mapping, copied-frame output, and port/memory/frame plus DOS-fixture evidence.
- [ ] **CGA composite-video fidelity (`TODO(Low)`).** Treat NTSC artifact
  color, phase, and colorburst as an optional renderer/profile capability only
  after digital CGA is complete. Do not fold it into VADP digital state or use
  it to claim EGA/VGA support.

## CPU, Time, And Debugging Debt

- [ ] **Legacy LOCK-prefix legality matrix (`TODO(Medium)`).** The
  8086/80186/80286 profile path in `PREFIX_LOCK` advances past `F0` without
  validating the following opcode; T316 S29 reproducibly observes `F0 98`
  execute CBW on 8086 rather than follow the 80386-path `#UD` rule. This is
  outside the bounded 80386 S29 slice because a correction changes shared
  prefix behavior for every legacy opcode. Admit only with an Intel
  profile-by-profile legality decision, an opcode/ModRM whitelist matrix,
  focused valid-memory and invalid/register LOCK coverage for every affected
  legacy route, and a retained 80386 regression. Do not special-case
  `98`/`99` or import 80386 validation into legacy profiles without that
  review.

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
  [T312 evidence](etc/evidence/t312-system-extension-admission.md). Do not use
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
- [ ] **Paused-boundary debug borrow retirement (`TODO(Medium)`).**
  `core_machine_debug_*_borrow()` remains a transitional adapter used only
  after a returned command boundary; it is not a second owner or executor.
  Replace it only with copied or operation-specific core debug access that
  preserves the retained Console/debugger semantics and multi-session
  isolation.
- [ ] **Core debug control-register mutation contract (`TODO(Medium)`).**
  `vm/composition/session/debug_target.c` still directly writes `CR0`--`CR4`
  through the debugger borrow. This is not a guest execution path, but it can
  construct paging/control state that guest `MOV CRx` may not. Admit a
  core-owned debug mutation contract only after preserving retained debugger
  UX and defining lifecycle, validation, raw-override policy, and focused
  regression; do not let VM composition acquire paging semantics.
- [ ] **Broaden present x87 (`TODO(Low)`).** T262 closes only the exact-8087
  finite-`m32real`/basic-arithmetic baseline with owned state, exceptions, and
  `FWAIT`. T317 S1 reconciles retained ESC/WAIT `#NM` vector-7 delivery tests
  but does not implement coprocessor execution. Re-admit 80287/80387, broad
  formats, complete IEEE behavior, environment save/restore, and protected-mode
  FPU delivery only with a corpus-driven instruction matrix; that admission must
  rerun the retained ESC/WAIT no-FPU, vector-7, optional-8087, and provider
  boundary tests alongside its new execution tests.
- [ ] **CPU-fault outcome audit (`TODO(Medium)`).** T214 established a
  session-owned fault result. Revisit only with a reproducible case showing a
  fault/detail is not available to the retained Console/debugger boundary;
  preserve first-fault evidence and never add a second run path.
- [ ] **Debugger assembler `checkop` review (`TODO(Low)`).** Keep runtime
  disassemble/reassemble comparison out of per-instruction refresh. Add a
  focused regression only when an owned BIOS/debugger defect requires it.
- [ ] **xasm API capacity and failure semantics (`TODO(Medium)`).** Admit only
  with explicit input/output capacities, return and failure-atomicity
  contracts, caller inventory/migration, and bounded assembler/disassembler
  regressions. A local string-operation substitution cannot claim closure.
- [ ] **Bounded differential debugging (`TODO(Low)`).** A historical
  Bochx/Bochs bridge may be an optional developer tool with provenance,
  checkpoint schema, masks, instruction/time/no-progress/trace budgets, and
  cleanup. It is never a default build or acceptance substitute.

## Timing-Fidelity Debt

The current core is deterministic and host-clock-independent: completed
instructions advance core elapsed ticks and devices consume frozen accumulated
clock ratios. These levels are deferred compatibility admissions, not the
default definition of NXVM completion.

- [ ] **Instruction-timed execution (`TODO(Medium)`).** Give each admitted
  instruction deterministic profile-specific cost, including applicable
  prefix/branch/memory/I/O variants, before expanding the timing corpus.
- [ ] **Bus-timed PC/AT operation (`TODO(High)`).** T269 admits deterministic
  one-unit DMA grant pacing and its fixed visibility boundary. Model remaining
  memory/I/O wait states, CPU bus ownership, and device-specific timing only
  through later corpus-driven admissions.
- [ ] **Cycle-exact profiles (`TODO(High)`).** Only where a profile genuinely
  requires it, model clock phases, prefetch/bus behavior, and device
  microstates without silently changing the retained executor.

## Architecture And Portability Debt

- [ ] **Inherited NXVM strict compilation (`TODO(Medium)`).** Admit only when
  an inherited unit is substantively changed or safely separated from a mixed
  target. Record the diagnostic baseline, exact modification scope, selected
  constraints, and regression result.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Linux source is a
  portability asset, not a support claim. Add native POSIX compile and runtime
  probes after an approved POSIX environment is available; do not install WSL
  merely for this item.
- [ ] **AI DevBox exploration (`TODO(Low)`).** Assess controlled machine
  capabilities for legacy-program development only after an owner-approved
  product goal, trust boundary, and reproducible-fixture plan exist. It must
  not silently turn host computer-use access into a generic guest capability.
