# T43 S8: Six-Corpus Quality Audit

## Scope And Evidence Limits

Owner admitted audit first on 2026-09-25, then approved A-F for repair in S8.
Baseline: `ed818b047`. The initial audit was read-only; the implementation
disposition below supersedes its pending-repair decisions.
Architecture and coding governance skills guided ownership-first review:
do not merge different contracts or mechanically rewrite integers as booleans.

Tracked inventory from `git ls-files`:

- src/lib: 109 files.
- src/common: 23 files.
- src/x86: 14 files.
- test/lib: 51 files.
- test/common: 20 files.
- test/x86: 10 files.

Total: 227 files. All six roots were searched for boolean-like declarations,
literal assignments, stale/unused markers, symbol references and repeated
registration/cleanup paths. Findings below were manually checked against the
relevant definitions and uses. This is not a claim that every line of all 227
files received complete semantic verification or that no other defect exists.
Search hits alone are not findings; absence of an NXVM caller does not prove a
shared API is dead in other consumers.

## Confirmed Findings And Proposed Disposition

### A. Internal predicates use integer vocabulary

- Lib: `kvm-window/render.h` frame-size/render predicates and valid pointer;
  `motion.h` valid field and move/scale predicates; `geometry.c` fit/cursor
  predicates; `win32/mouse.c` bounds/captured predicates; `win32/component.c`
  surface_valid, left_button, right_button and mouse_delivery_posted.
- Common: `session/control_state.h` stores bool state but exposes i32
  console_control/graphics/exists parameters and predicate results;
  `session/control_state.c` assigns 0/1 to those flags. UI event delivery also
  expresses acceptance as i32. Session run result must be classified separately
  from acceptance predicates, not swept into a boolean rename.
- X86: `debug/command.c` exit_requested and pending_line_available;
  `xasm32/aasm32.c` label flags include the newly introduced S7 flag_resolved,
  still u8 with literal 1 assignments. This is also a local repair omission,
  not solely inherited code. Other assembler flags require operator-by-operator
  classification before changing their original implementation.

Proposal: use lib_bool and LIB_TRUE/LIB_FALSE for actual predicates, updating
declarations, definitions, function-pointer signatures and receiving callers
together. Keep original assembler tables, command formatting and control flow.
Current lib_bool is an alias of lib_i32; this does not make u8 conversions
layout-neutral. Header/implementation signature disagreement was NOT confirmed;
the issue is inconsistent semantic vocabulary across fields and APIs.

### B. Public copied-value boolean fields need a distinct compatibility review

- `lib/kvm-window/frame_interface.h`: valid and graphics use lib_u32.
- `lib/kvm-base/frame_interface.h` and `lib/console/console_interface.h`:
  cursor_visible and cursor_phase use lib_u8.
- `lib/kvm-base/event_interface.h`: key.pressed and mouse.relative use lib_u8,
  whereas normalized keyboard records already use lib_bool.

Proposal: agree on boolean vocabulary for these contracts, then rebuild all
receivers and tests. u8-to-lib_bool changes structure layout; do not present it
as an ABI-neutral cleanup. Do not change glyph-bank indices, dimensions, scan
codes, bitmasks or serialized bytes merely because examples use 0 and 1.

### C. Boolean literals and test fixtures are not fully migrated

- `lib/base/sync.c:121`: bool base_sync_task_cancelled returns literal 0.
- `lib/kvm-console/console.c`: relative mouse and cursor visibility literals.
- `common/session/control_state.c` and `control.c`: boolean assignments 0/1.
- `test/lib/kvm_frame_damage_mouse_smoke.c`: i32 valid flags mirror render API.
- `test/x86/xasm32/xasm32_smoke.c:86`: i32 has_sib is a predicate (also added
  into the expected byte length, requiring an explicit numeric conversion).
- `test/lib/linux_wait_fakes.h`: fake alive/locked/failure switches coexist
  with real counters and native errno/status results; classify individually.
  Common tests exercise the same integer predicate APIs in control-state and
  presentation-plan matrices; migration must preserve each matrix case.

Proposal: migrate tests with their production contracts, preserving matrix
coverage. Native return conventions, counters and deliberate invalid inputs
remain numeric; no blanket replacement of every 0/1 in assertions or fixtures.

### D. Dead names and obsolete comments are real but limited

- aasm32.c:239 and dasm32.c:28 define unused trace macros referencing context
  fields that no longer exist.
- dasm32.c:52-53 retains a commented-out _GetStackSize using old vcpu state.
- debug/command.c:72 contains a redundant self typedef of x86_debug_watch_kind.

Proposal: delete these residues. Do NOT remove live XASM32_TRACE parser-flow
markers: their names do not make them unused logging. The command.c include
of lib/types/file.h is needed for variadic formatting; it is NOT dead merely
because L/W now uses storage. No broad unused-public-API deletion is justified
by this audit. Full compiler-based include minimization has not been run.

### E. Small flattening is preferable to new abstractions

- `lib/base/process.c:20`: after *separator is zeroed, the special root case
  writes the same byte again. Delete the redundant branch.
- `lib/storage/file.c`: reader_close and writer_close each wrap their close,
  release and return in an unnecessary inner block. Flatten locally, keeping
  close failure propagation; do not add a polymorphic cleanup framework.
- `lib/console-broker/console.c`: owner-local lock/unlock wrappers only forward
  to backend operations. Direct calls are a candidate, not a correctness fix;
  keep them if removing them makes repeated ownership intent less readable.

### F. Optional test-build duplication, not a prerequisite framework project

The three test CMakeLists repeat executable registration, UNDEBUG flags, test
labels, timeout and target-list plumbing. A small common registration helper
could reduce this, but would add a shared build dependency. Each test root must
retain independent configure/build/test entry. No merged suite/fixture owner
or additional framework is proposed; discuss whether savings justify it.

## Explicit Non-findings / Keep Separate

- Atomic/interlocked storage and native ABI types must satisfy their primitives.
- Index, extent, byte-count, bitmask, enum, status and multi-result functions are
  not booleans; formatting functions returning lengths/-1 stay numeric.
- Machine input FIFO and Session control FIFO have different capacities,
  failure handling and generation/lifecycle responsibilities; do not merge them
  just because both enqueue/dequeue.
- Platform implementations are intentionally different owners of native work.
- Types aliases and boundary wrappers are part of the approved vocabulary;
  being thin does not make them redundant.
- Previous S7 asynchronous event-failure follow-up remains in TODO; this audit
  does not claim to have implemented or closed it.

## Verification And Next Decision

19 selected static CTest checks passed in 30.87 seconds: all six manifests,
Types layout/test boundary, component DAG and negative fixtures, KVM naming,
control ownership, Common/X86 corpus and negative verifiers.
Command used the existing MyNES x64 tree with a name filter for these static
checks, not runtime suites. No compilation, EXE refresh, integration execution
or runtime acceptance is claimed for this documentation-only phase.
MyNES documentation governance and scoped whitespace verification also pass.

Discuss A/C/D/E first, B explicitly as a public-layout change, and F as optional.
This initial report itself did not pre-approve repair. S8 and T43 remained open.
Preserve unrelated NXVM.ini plus concurrently appearing MyNES composition and
native integration-test modifications; they are not audit outputs.
Owner subsequently authorized reviewing, verifying and submitting these extra
changes with later repairs. They must still receive separate target-scoped
review and commits; this does not authorize Shared repairs before discussion.

## Approved Implementation Disposition

Owner subsequently approved all six findings within S8. The repair batch is
finite A-F above, not a claim that every possible defect in the packages is gone.

- A/C: predicates, copied boolean fields, callbacks and matching fixtures use
  lib_bool and explicit truth constants. Common run is a success predicate,
  not an exit code. Debug helpers retain the original true-means-error convention.
  Test failure-loop indices remain numeric and assign a separate boolean switch.
- B: cursor fields change from u8 to i32-backed lib_bool. Text-frame sizes grow
  by eight bytes; tests assert the new layouts and boolean field types. Both
  receiving products are rebuilt; old object files are not ABI-compatible.
  Frame copy length/tail checks remain, as do all encoded text-cell byte fields.
- D: removed obsolete assembler trace aliases, dead vcpu comment and self
  typedef. Removed unused file.h includes from shared tests after direct-use
  review and strict compilation. Live parser-flow trace macros and the Debug
  variadic-formatting include remain.
- E: flattened storage close paths, removed the duplicate process-path write,
  replaced broker lock-only forwarding helpers with direct backend calls, and
  consolidated duplicate mouse-button clearing without changing release order.
- F: one test/lib/register.cmake owns repeated CMake registration only.
  Independent test/lib, test/common and test/x86 builds still select 49, 18 and
  10 tests respectively; no product target, merged suite or new fixture owner.

Native ABI results/atomics, serialized control payload bytes, glyph-bank and
size selectors, lengths, counters, errno values and tri-state results remain
numeric. Original assembler opcode tables, command dispatch and parser control
flow are unchanged. Existing queues and native resource owners remain unique.
No new public API was introduced; public predicate spelling/layout changed as
explicitly approved. Importers must take matching source and tests and rebuild.

The initial runtime pass caught two stale frame-size assertions in lib_console
and kvm_frame_copy. Both were corrected from the reviewed field layouts, not
disabled. A strict-build cursor-case signedness mismatch was corrected by
typing the expectation as boolean. Final proof is recorded separately below.

## Final Verification And Receiving Review

- Shared/MyNES complete configured suite: x64 130/130 (181.35 seconds),
  x86 130/130 (162.43 seconds). Includes all 77 Shared tests/gates and 53
  product tests, including native Console/Window lifecycle/title checks.
- NXVM complete repository-only unit suite: x64 335/335 (31.83 seconds),
  x86 335/335 (70.81 seconds).
- Six manifest revisions are shared-m6-t43-s8-p1; hashes and Types/DAG/corpus
  gates pass in both configured suites. All tests remain registered.
- Independent test-root configure/build/test: Lib 49/49 (67.84 seconds),
  Common 18/18 (23.31 seconds), X86 10/10 (5.64 seconds).
- Both product documentation gates and whitespace checks pass.
- No external-ROM NXVM boot integration, Linux runtime or physical audio
  quality claim is made. This is S delivery, not T closure or owner hand-test.

Commands: complete MyNES build and ctest for both release trees; NXVM
run-unit-tests in both t41-s8-nxvm trees. Independent entries use cmake -S
test/<corpus> followed by build/ctest. Product targets are mynes-0-0-0043 and
vm-0-5-0535, optimized stripped x64/x86. All four NXVM profiles are rebuilt in
each receiving tree and the cache selection restored to default. A stale
pre-asset-merge Makefiles cache was abandoned in favor of these current Ninja
trees; it is not a source or missing-asset defect.

Owner MyNES edits preserve execution and fixture logic while updating native
Window titles to explicit Running/Paused. Formatting-only changes are retained,
not attributed to this Shared cleanup. The default NXVM.ini owner's numeric
console_control=0 initially failed its true/false-only parser. The owner then
required only 0/1 for every INI boolean, rejecting textual aliases. The existing
NXVM parser and all four supplied NXVM INIs now follow that contract. MyNES has
no boolean INI key; its rom/display configuration remains unchanged.

The repository-only INI regression accepts 0 and 1, rejects ten other spellings
and checks cleared output on rejection. Complete NXVM unit reruns pass x64
335/335 (22.70 seconds) and x86 335/335 (24.00 seconds). Product source/test
correction is +17/-3, net +14, across ini.c and nxvm_ini_smoke.c; the increase
is the table-driven rejection regression, not a compatibility layer. Shared
source/manifests and verified MyNES binaries remain unchanged by this correction.

Shared changed source/test code, including the 26-line registration helper:
+706/-776, net -70, across 90 files. Counted C/H/CMake changes under the six
roots with git diff --numstat, excluding manifests and README. The owner's
three MyNES C files add 233/remove 164 (net +69), primarily retained formatting.
No generic queue/worker framework was introduced to obtain this reduction.

MyNES receiving artifacts (PE machine / bytes / SHA-256):

- x64 / 8664 / 246798 /
  29015CF9A005581AE9DF60E10ABA5DEC89861425BE766BA9B85A8DB6212344FA
- x86 / 014C / 240142 /
  82786F47D3D87C4F3CF4929FE644548B1470FFE5703366759F6F88EDE85AE224

Both retain 0043 identity; objdump confirms no debug sections. NXVM artifact
identity and hashes belong to its receiving evidence, not a second task packet.

Shared implementation is S8 P1 bfcbd31b2, committed and pushed. The six manifest
roots are pinned by that commit; no sibling repository was modified.

Numeric-only INI correction is delivered in NXVM S8 P4 5373a1a05, including all
four INIs and eight receiving binaries. Coordinator actual-change review and
both documentation gates pass. The current shared revision and verified MyNES
artifacts need no additional rebuild for this product-only parser change.
S8 delivery awaits owner hand-testing; neither S8 nor T43 is declared closed.
