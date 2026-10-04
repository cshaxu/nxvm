# T540 S94 Actual Source And Coverage Review

Baseline: accepted S92 d76d2b15e. S93 is superseded without acceptance.
This is a completed S94 review record, not component implementation acceptance.

## Review Universe

The [path inventory](t540-s94-path-review.tsv) combines
`git diff --name-only d76d2b15e` and
`git ls-files --others --exclude-standard`, restricted to source, tests,
CMake and tools plus root CMakeLists.txt. It records 609 paths: 448 existing
and 161 deleted (38 CMake, 60 source, 509 test and two tools). The owner's
verification-performance request adds the strict-matrix verifier as one new
review row: the current universe is 610 paths, 449 existing/161 deleted and
39 CMake paths; no original row is removed. The subsequently inspected App
debug adapter adds one comment-only repair row: 611 paths, 450 existing/161
deleted, with 61 source paths. The active packet records that bounded revision
before the repair; its classification behavior and public API remain unchanged.
The subsequent PIC fixture error-status repair adds one existing test helper:
612 paths, 451 existing/161 deleted and 510 test paths. No original path is
removed from the review universe.
The PIC IRQ lifecycle test setup repair adds one previously unchanged test:
613 paths, 452 existing/161 deleted and 511 test paths. Its bounded repair
was admitted in the active packet before implementation.
The same setup-status review adds the other three existing PIC test receivers:
616 paths, 455 existing/161 deleted and 514 test paths. Their repair scope was
recorded before implementation; no original inventory row is removed.
The unchecked IRQ-binding sweep implicates one previously unchanged PIT/IRQ0
test; add its source row before repair. The current universe is 617 paths,
456 existing/161 deleted and 515 test paths. No prior row is removed.
The gate-consumer sweep adds the previously unchanged FPU verifier. The current
universe is 618 paths: 457 existing/161 deleted, with 40 CMake, 61 source,
515 test and two tool paths. No original review obligation is removed.
The ignored-presentation-result sweep adds the two previously unchanged
DOS prompt/MEM-fault receivers: 620 paths, 459 existing/161 deleted,
40 CMake, 61 source, 517 test and two tool paths. Their admitted repair
preserves the original guest programs, budgets and success assertions.
Current SHA-256 binds review to actual bytes; a changed hash requires
re-review. Source-read is not whole-diff or behavioral qualification.
Documents and artifact declarations receive separate review, not exclusion.

Logical relocation pairing and original assertion/coverage mapping remain
required. Tracked-only diff statistics omit untracked receiving files and
are not the final code-size result. Count added receivers and deduplicate
moves, stating the method. Pending rows cannot support a clean-code claim.

## Product CMake Whole-Source Reconciliation

Read the complete product CMake source and all 1,381 original baseline-diff
lines in bounded portions. Check production source groups, migrated test
definitions, owner-local fixtures, strict inventories, unit/integration
partitioning, Shared registration, specialized gates and artifact links.
All 146 removed executable definitions retain exactly one receiving CTest
registration in the generated x64 configuration. The receiving sources and
assertions have their own reviewed inventory rows; registration alone does
not prove behavioral coverage.

Revise S94 before repairing the stale source-owner matcher: recognize current
app-nxvm components and normalize sources against their target SOURCE_DIR.
Apply the same directory normalization to the owner-test strict cohort; a
Shared-relative source must not be looked up relative to the repository root.
Restore verify-vm-machine-owner to the final specialized candidate list: its
earlier append was discarded when that list was reconstructed. Correct the
obsolete 80186/80286 manifest-runner comments without changing registration.

An ignored configure probe extracts the actual owner-check function, uses
relative sources from its distinct target directory, accepts two product
sources and rejects product/profile mixing. Its first output assertion missed
CMake's newline; the corrected whitespace assertion verifies the rejection.
Both root configurations regenerate successfully and pass machine-owner,
unit-registration (335 targets) and strict-CPU coverage (44 commands).
The final generated specialized list now includes machine-owner. Remove the
temporary probe source. MyNES has no scoped changes and is not built.

All 626 logical source/test/build/tool paths have reviewed dispositions bound
to current hashes. Reconcile the early qualification-pending labels against
the subsequent complete KBC, video, FDC/HDC, XT and fixture assertion reviews
and current dual-width full units/gates. The 99-row private-board intake retains
its concrete receiver mapping; CPU/indirect fixture review is recorded below.
Six manifest final-revision work remains explicitly assigned to S95, not hidden
as S94 implementation acceptance. Independent corpus and product artifact/boot
acceptance remain S95-S97.

Coordinator review accepts the S94 review result only: complete source/diff and
coverage records, 626 current hashes, dual-width 492/492 units, both specialized
aggregates, documentation governance and diff checks. No partial implementation
P is made. The original S93 implementation remains unaccepted until S97.

### Closure Verification In Progress

The current full x64 aggregate rebuilds 90 affected compile/link steps, then
passes 491/492 in 83.82s. The unchanged Lib desktop modal test fails at line 71:
its entered event does not signal within the original 3,000 ms after SC_SIZE.
This is not a passing suite. One isolated invocation passes in 0.95s wall time;
no timeout, assertion, Lib source or test is changed. Native startup signals
ready after showing/focusing the window; RUN_SERIAL already remains enabled.
These observations establish nondeterminism, not its root cause.
The complete x86 aggregate rebuilds 113 steps and passes 492/492 in 57.74s.
The current full x64 rerun passes 492/492 in 107.97s; the CPU boundary negative
alone takes 103.14s. An unrelated MySMB build/test was also active; do not claim
that this run proves intrinsic NXVM regression or terminate that other work.
No unit assertion, timeout or Shared modal implementation was weakened.

The complete specialized x64 gate initially finds three stale historical
fixture paths. Reconcile mantle/provider-composition, arbitration and entry-plan
with their actual Shared receivers, and add the two relocated constructors to
the discovery list previously covered by the App glob. Preserve 101 historical
identities and 133 direct constructor assertions. Re-read the receivers and
gate predicates; the repaired complete x64 specialized aggregate passes.
The repaired complete x86 specialized aggregate also passes. Both retain 518
strict-matrix rows (497 strict and 21 deferred). The default owner INI hash
remains A858BEE0D2145E539D69861A675FDDB10433B25EAD658A9DFAF0D69632989F13;
both MyNES executable hashes and its scoped Git status remain unchanged.
Default NXVM executable prerequisites were relinked inside the Debug build
trees. Source inspection confirms deployment occurs only in Release, so these
gate runs did not deploy new assets; S96 still owns their actual refresh.
These results do not substitute for S95 independent corpus or S96 eight boots.

Code-size method for final delivery: compare accepted S92 and the complete
current logical-path inventory, including untracked receivers and deleted
originals; count each physical source once, group relocations by their actual
source/assertion pairing, and report production, test and build/tool changes
separately. Pure moves are not deleted functionality or added implementations.
Tracked Git numstat alone omits new receivers and is not a final net-size claim.
S97 records final counts against the delivered tree, after S95/S96 verification.

## DOS Boot Probe Whole-Source Reconciliation

Read all 3,359 current lines in bounded portions, the complete 609-line accepted
baseline diff and the actual Core/common/AT/XT boot and composition fixtures.
The probe borrows the constructed Core and Board on the same executor thread;
its observers receive copied values and preserve write-time physical-route
effects. D4 observation reads the retained Profile owner, and refresh diagnostic
queries that owner's request without completing it. No diagnostic becomes a
paused-debug request, clocks a device or introduces a production API.

Retain reset and ROM mapping observations, FDC terminal/port histories, CMOS,
keyboard, IRQ/DMA/PIT, exception, Model40 RAM/video and loader checkpoints.
All 230 BOOT-PROBE marker literals match the baseline, including counts. Original
DOS prompt, date-input and installer-ready success predicates, F1 input, Core
quantum/deadline progression, wall/no-progress containment and common cleanup
remain. Timeouts and POST diagnostics remain failures, not success evidence.

Revise S94 before repairing two inherited diagnostic defects: the seven-bit
CMOS observer index could exceed its 64-item arrays; reject that observation
without aliasing the index or extending the machine's CMOS. The timeout trace
could read an uninitialized exception diagnostic when first-exception mode was
disabled; initialize the local and capture current Core diagnostic before that
report. No guest state or terminal predicate changes. The receiving file's
accepted-baseline diff is now 105 added/115 removed lines.

An actual-source temporary probe passes all 512 index/direction combinations
on x64 and x86. It compares the entire record object after selection, supported
and unsupported data, unrelated port, no-I/O and null inputs; unsupported indices
must leave all record arrays unchanged. The initial include probe failed to
link because the renamed external main retained unrelated dependencies; make
only that unused probe entry static, then both probes pass. Remove the temporary
source and both executables. Both real diagnostic targets pass strict incremental
builds (x64 3.36s, x86 2.84s) and no-asset invalid-argument exits. Source review
proves capture-before-timeout-read; this batch does not claim a runtime timeout
or external-ROM boot test. S96 retains actual boot acceptance once per case.

Qualify this path's current hash: 2 to 1 pending of 626. Product CMake and
complete S94 closure verification remain. MyNES has no scoped changes and is
not built. No partial commit, product artifact, owner INI or external asset change.

## Migrated Gate Consumer Universe Review

Read the complete controller, CPU/PIC, display, DMA/RTC, deadline, rational
clock, FDC, FPU and product-execution verifiers and their accepted-baseline
diffs. Migrating concrete receiver paths had not migrated all recursive scan
roots. Controller Board time/lifecycle predicates still matched the retired
devices directory; CPU/PIC scans omitted the new three IBM-PC roots. FDC
omitted AT/XT, FPU omitted neutral Core/Board consumers, and display/DMA caller
scans omitted their Shared receivers. The execution gate scanned product only,
making its machine-runner exception unreachable.

Repair the seven existing verifiers' input scopes rather than adding gates or
weakening constraints. Only the PIC bus's own implementation/header may import
its private bus layout; family consumers remain checked. FPU owner tests remain
outside the consumer-private-layout ban. Deadline/rational checks retain their
time and settlement predicates; their video receiver is the reviewed adapter
that forwards to the original chip advance.

All nine real positive gates pass. A copied repository-only source/test fixture
then injects and restores each violation separately: Board elapsed time,
lifecycle/READY, Core/CPU/PIC/FDC/FPU private imports, incorrect display/DMA
receiver and alternate App execution. All 24 controls reject with the expected
diagnostic, including common, AT and XT inputs. The existing CPU-boundary
negative unit also passes once on x64 in 48.96s, retaining all 173 mutations
(72 CPU, five bus and 96 migrated-test controls). Its subprocess cost is a
measured long unit, not compiler cost; removing those controls is not a speed
optimization. No production, API, artifact,
MyNES or external-media change is made in this batch. Inventory reconciliation
records 618 paths, zero stale hashes and 285 pending full-source dispositions.
Both x86 manifests, documentation governance and diff whitespace checks pass.
Current corpus review
and full dual-width closure validation remain open.

## Initial Construction And Ownership Inspection

Read complete current machine_board.c, machine_plan.c, machine_board_state.h,
machine_board_interface.h, board_profile_interface.h, parity.c,
parity_interface.h and x86 CMakeLists.txt. Inspect unchanged Core attachment
binding/destruction, and compare accepted Board construction/reset/finalization
and plan application/publication sections. Remaining baseline/current diff
portions are not yet qualified.

- Core binding rejects incomplete/replacement/frozen attachments. One Core
  destructor invokes Board finalization before CPU, ports and RAM cleanup.
  The unchanged Core implementation is not copied into Board.
- Board creation clears outputs, binds one candidate lifetime, then creates
  controllers/families. Later failure destroys Core and its bound Board.
  Allocation failure before binding destroys only Core; failed binding frees
  the unpublished Board explicitly. Plan outputs publish after all topology
  and timing declarations succeed, not after the first allocated device.
- Reset order retains the original phases. D4 memory, latches and refresh
  reset are Profile callbacks rather than common-board fields. Profile
  finalization precedes borrowed PIT destruction; the binding rejects
  replacement and requires all reset/NMI/refresh/deadline/finalize operations.
- Board state contains opaque KBC/XT/FDC/HDC/video members, not D4 RAM/latches.
  Common owns IRQ leases and supplies family line callbacks. Speaker wires
  have one common electrical owner rather than duplicate XT gate/data fields.
- Memory admission's parity observation preserves the original predicate:
  parity_observe.configured means memory_bytes is nonzero. Port-only parity
  wiring does not prohibit stopped RAM replacement. This apparent source
  difference is non-defective, not a reason to restore private state access.
- Read null-safe KBC/XT and FDC/HDC finalizer entry points. Their complete
  route/IRQ/DMA rollback internals remain in the controller review batch;
  entry-point inspection alone does not qualify those files.

## Scheduler And Profile Borrow Review

Read the complete unchanged x86/core/machine_scheduler.c and compare it
against d76d2b15e (no diff). Read current Board advance/deadline bodies and
their baseline relocation differences, the Model40 factory, plan publication,
App storage construction/finalization, and the video advance callee.

- Core alone updates elapsed_ticks. Its timeline, arbitration, readiness and
  peripheral phases retain their order. Board clocks convert the submitted
  source duration; they do not publish a second guest clock. FDC/HDC receive
  the same absolute due_tick, while PIT/RTC/KBC/video receive their existing
  domain conversions. This is source-path proof, not new physical timing proof.
- board_advance.c replaces embedded controller addresses with opaque objects;
  video advancement calls core_machine_vadp_advance, whose body forwards to
  the same adapter-owned x86_video_advance. No second chip or algorithm appears
  in this path. Complete video construction/rollback remains to be reviewed.
- board_deadline.c preserves existing controller queries. Its D4-specific
  pending test becomes the frozen Profile next_deadline callback, retaining
  the original now + 1 rule. The migration does not upgrade that timing claim.
  Existing addition-overflow behavior must not be called a new migration
  regression without a separate source/coverage disposition.
- Model40's factory clears the optional borrowed construction output before
  allocation; memory-configuration failure destroys its own candidate. After
  factory success, Board binds the lifetime or finalizes the rejected binding.
  Later topology/declaration failure destroys Core and its bound Board/Profile.
  The App error branch invokes storage_finalize, which clears model40_board
  after Core destruction. It does not dereference or separately destroy that
  borrowed object. The output is not a usable lease before whole construction
  succeeds; the reviewed production caller respects this restriction.
- Non-Model40 materialization never populates this pointer. The App object is
  zero-initialized and finalization clears the pointer, so it does not retain a
  Model40 candidate across failed construction/reuse. Remaining test callers
  must be checked before claiming all borrowers comply.

No new runtime defect is established by this batch. These concrete callee and
failure-path checks supplement, rather than replace, the unfinished whole-file
inventory and original coverage reconciliation.

## Controller Construction And Rollback Inspection

Read FDC configure/connect/initialize/finalize, HDC configure/connect/initialize/
clear and Board HDC/FDC publication, KBC initialize/finalize, Core port batch
registration/rollback, PIC source binding, and chip FDC/HDC destruction.
This qualifies those lifecycle sections, not their unread command bodies.

- Core port installation validates the batch, saves a list checkpoint, and
  removes all entries added since it on allocation/conflict failure. Rollback
  also clears registration_status; callers receive the original failure.
  Successful routes remain Core-owned until removal or Core destruction.
- FDC creates its chip before installing routes. Failed registration destroys
  the chip, then the configure failure finalizes copied connection/data state.
  The accepted baseline has the same finalization body; moving its storage to
  an opaque allocation does not create another controller execution route.
- HDC publishes configured only after chip and port installation, plus DMA
  channel binding for Xebec. DMA-binding failure removes HDC-owned port routes
  before clearing the chip/topology. Compaq's shared read contribution is
  explicitly wired-or; it does not replace FDC's primary read route.
- KBC allocates keyboard, mouse and 8042 in order, and finalizes partially
  created children on either child or port failure. No child pointer remains
  non-null after its finalizer.
- IRQ sources are PIC-owned list entries, not controller-owned allocations.
  FDC/HDC chip destruction deasserts IRQ/DRQ through their live connections
  before copied connection state is cleared. Board destroys these controllers
  before DMA/PIC, preserving the callback lifetime. Source binding/retry
  behavior still needs original-case reconciliation; this inspection does not
  claim arbitrary retries reclaim every PIC source immediately.

Located existing rollback receivers for DMA, PIC, XT, parity, video and port
ownership. Their assertions and complete mapping are pending actual test-body
review: search hits alone do not prove coverage or justify an all-green audit.

## Rollback Assertion Reconciliation Batch

Compare accepted planar_parity_nmi_s3 and ega_registration_transaction sources
with current receivers and read their owner-local fixture implementations.

- Parity retains the exact 0x1234 byte write, parity-bit corruption, unchanged
  read byte, blocked RAM replacement, latched fault and NMI assertions. Core
  owns the private parity-memory checks; Board observes the parity device.
  Publication failure still requires no parity buffer/owner and no published
  device, then removes the conflicting route and proves successful retry.
  The expected owner now is the actual opaque parity object, not old Board.
- Video retains eight initial CGA allocation failures and EGA/Compaq/VGA
  variant failure bounds 20/27/27. Every row still checks allocation position,
  original chip preservation, existing CGA/sentinel ports and memory routes,
  absence of partially installed extension ports, successful retry and cleanup.
  The fixture owns the video-private checks; the Core receiver owns memory/
  port-private checks. Neither replaces failure assertions with success-only
  smoke coverage. All scalar expected values and failure-loop bounds remain.
- PIC lifecycle source is byte-unchanged against the accepted baseline. Its
  allocation tests preserve null outputs, existing-source identity, forbidden
  cascade-source rejection; single/cascaded route conflicts retain sentinel
  reads and prove earlier attempted routes absent.
- DMA rollback body retains conflicting 0xd4 write ownership, absent attempted
  primary/page routes and allocation-failure retry. The fixture's new explicit
  controller_count selects 8 or 16 registers; old dual-controller callers pass
  2. Full channel-source diff remains pending; do not accept the entire matrix
  from inspection of this rollback function alone.

Executed the five corresponding existing independent x64 tests once, serially:
machine-port-ownership-board, PIC lifecycle S4, DMA channel, EGA registration
transaction and planar parity NMI S3. All five pass (0.14s total). This focused
set is only this batch's check, not a new fixed suite or S94 closure substitute.

## Mechanical Relocation Classification

A corrected read-only pairing pass classifies all 448 existing inventory paths
against d76d2b15e by the same path, or a unique baseline filename when moved.
Its first invalid PowerShell single-element-array run is discarded; the
corrected pass rejects failed baseline reads rather than treating them as empty.
The [mechanical pair index](t540-s94-mechanical-pairs.tsv) records 68 candidates:
five Git-blob-identical and 63 differing only in preprocessor include lines.
The comparison normalizes line endings only; it does not strip comments,
assertions, whitespace, literals or function bodies. Git blob identity includes
normal repository clean-filter handling. Recorded SHA-256 still binds actual
worktree bytes independently.

The other 279 paths require actual-diff inspection; 101 are new or ambiguous
and require explicit provenance pairing. The 161 deleted paths still require
receiver/disposition proof. Mechanical classification is not acceptance:
changed includes need public-boundary/dependency verification, and unchanged
moved bodies need build/source uniqueness and original coverage mapping.

Separately read the actual trace_interface.h/transaction.h diff: transaction
owner and kind enum values move unchanged into the copied trace contract;
private transaction state and phase stay private. No executable transaction
logic changes in this pair. Read App display/input/floppy/keyboard include
diffs: they select the new public Board or App value headers without changing
function bodies. Their build/consumer qualification remains in the full audit.

## Findings Requiring Disposition

1. x86/CMakeLists.txt describes a future CPU handoff and temporary warning
   exception despite accepted T539 ownership. Reconcile comments with the
   actual compiler policy; do not silently change warning flags.
2. machine_plan.c ends with an orphan arbitration-callback comment after
   its implementation moved elsewhere. Remove or relocate it to its owner.
3. machine_board.c contains detached Port-B/refresh comments and empty
   migration gaps. Reconcile with AT/Profile owners without changing algorithms.
4. machine_board_interface.h speaker/CMOS comments still name Core as owner
   of extracted board state. Correct wording without changing public values/API;
   apply the same semantic-comment sweep to the remaining moved headers.

The coordinator revised S94's bounded surface before repairing these comments.
All four classes are now corrected, including matching Model40 owner wording;
no compiler flags, API values or executable statements changed. The orphan
plan comment is removed; the refresh explanation is adjacent to its function.
Source manifest and inventory hashes are refreshed and manifest verification
passes. Whole-file qualification still remains pending; comment repair is not
an accepted clean audit. No new runtime bug is established here.

Read the complete DMA channel source diff: every changed call adds the explicit
2-controller argument, without changing data or assertions. The already-read
fixture chooses the former 16-register path for value 2; thus existing matrix
call semantics are preserved. App machine/profile construction diffs contain
the documented borrowed D4 output/clear and public include relocation only.

## Video And AT Keyboard Source-Diff Batch

Read complete current vadp.c/h/interface, machine_display.c and KBC c/h/interface;
compare complete accepted vadp.c, machine_display.c and kbc.c diffs. Re-read
Board IRQ binding/failure exits/finalization and the unchanged Core destructor.

- Video adapter state is only one chip handle, one borrowed Core handle and
  configuration admission. There is no second VRAM, mode or frame cache.
  Existing register dispatch, memory read/write/inspect/query, route tables,
  candidate configuration and rollback bodies are unchanged. New create/
  destroy/reset/advance/observation functions establish the opaque lifetime
  and return copied observations, not chip or framebuffer access.
- Display declaration validation moves from machine_display.c into the video
  adapter with the same port/personality predicates. Board publishes its copied
  topology/configured flag only after successful adapter configuration. The
  candidate's memory routes are removed on failure; earlier chip and routes
  remain. This is paired with the previously reconciled allocation/route
  failure assertions, not inferred only from a successful display capture.
- KBC protocol callbacks, command/data routes, reset, deadline, delivery and
  native-input bodies are unchanged. The substantive boundary change moves
  IRQ1/IRQ12 lease binding from KBC to Board. Board retains the same IRQ numbers,
  binds both before installing the callback and propagates either bind failure
  through whole-Core destruction. Its callback applies assert/deassert to the
  corresponding PIC-owned source; KBC has no PIC-private import or second IRQ
  state. Keyboard/auxiliary children are finalized before PIC sources.
- KBC destruction retains the original whole-machine teardown contract: Core
  has stopped dispatching and clears its port registry after Board teardown.
  Unlike video destruction, it does not independently unregister its routes.
  This batch therefore does not claim isolated KBC detach/replacement on a live
  Core; no such capability is added by the extraction.

The production bodies above have direct baseline/diff inspection; their build,
complete test-receiver/deletion reconciliation and final verification remain
open. The moved vadp.h introductory NXVM wording is recorded for the admitted
matching-header comment sweep, not a runtime defect, and corrected in the
following XT review batch. No production change or additional test execution
occurs in this video/KBC source batch.

Read complete kbc_controller_fixture.c, controller_fixture.h, kbc_fixture.h
and kbc_irq_fixture.h. Core owns the four allocation positions and conflicting
0x64 write-route checks, retaining 0x80 sentinel read, absent attempted routes,
unpublished child handles, successful retry and double finalization. AT protocol
tests construct the real adapter against an opaque executor, freeze/reset before
public bus access and treat bus errors as failures. The Board fixture binds real
PIC source leases at IRQ1/IRQ12; it does not mirror PIC state. The large protocol
test/diff combined output was truncated: its complete assertion reconciliation
is still pending and is not counted as reviewed from that output.

## Complete KBC Assertion Receiver Reconciliation

Finish the previously truncated controller test by bounded source reads and
five non-truncated consecutive diff slices against its accepted App source.
Read complete Board CPU/IRQ fixture and its registered CMake source list.

- Protocol cases retain mixed keyboard/AUX delivery, Set-2 translation,
  function/Pause bytes, unmatched breaks and typematic cancellation,
  self-test flush, line-release BAT, no second BAT on enable, FF/FA/AA ordering,
  5170 POST sequence, response polls/delays, A20/reset pulses and reply delivery
  behind full typeahead. No expected byte, delay or loop bound is weakened.
  Scan-set constants 1/2 become literal 1u/2u with the same public enum values.
- Core allocation/port-conflict assertions move to kbc_controller_fixture.c;
  the AT test still calls them and propagates their failure. The former inline
  CPU/IRQ1 test moves to kbc_cpu_irq_fixture.c, still called from main: identical
  guest code, 3/2-instruction budgets, EIP 6/0x100, pending OBF/source checks,
  ISR1, consumed AAh and cleared OBF. Core owns the reset-register patch;
  Board owns the actual KBC/PIC handles, with copied BAT readiness obtained
  from the AT fixture. This is not deletion of a hard-to-migrate assertion.
- Bus access now goes through the existing public Core interface; explicit
  freeze/reset replaces fabricated bare-port Core state for protocol cases.
  Each failed bus call terminates the fixture instead of supplying a success
  value. PIC IRQ assertions inspect the actual borrowed source, not a mirror.
- Independent CMake compiles the AT test, Core fixture and Board fixture into
  the same executable, linked to the real x86-ibmpc-common/x86-core owners.
  Rebuild that target in both tools-on independent trees; both builds finish
  successfully. Execute its registered case once per width: x64 passes in
  1.02s total and x86 in 0.90s total. This establishes this receiver only, not
  the complete S94 suite, independent corpus or product boot acceptance.

## XT Lifetime Defect And Bounded Repair

Read complete XT PPI implementation/private/public headers and its full
accepted-source diff; read complete XT controller/boot fixtures. IRQ lease
binding moves to Board while PPI byte/clear/DIP/NMI/speaker semantics retain
their old bodies. The adapter has no time axis; Board advances the separate
keyboard chip. Full original XT test reconciliation remains pending.

The finalizer sweep establishes a real pre-existing use-after-free in accepted
S92 as well as S93 carryover: Board releases shared PIT, then XT PPI finalization
calls reset, which publishes speaker gates through Board to that released PIT.
The PIT setter dereferences a non-null released handle; null checking does not
make it safe. Revise the active S94 repair boundary before implementation.

Move input-adapter destruction before PIT destruction in the single Board
destructor. Profile finalization still precedes borrowed PITs; Model40 D4
explicitly unbinds both output callbacks before release. FDC/HDC still precede
DMA/PIC, and PIT outputs still stop while DMA/PIC sinks are live. Do not add
late-callback guards, mirrored liveness flags, another teardown path or APIs.

Add an owner-local order assertion to the existing XT Board test. A real PIT
Mode-2 counter starts high and emits its final low edge during destruction;
the real PPI finalizer publishes its final speaker update. Test callbacks
record those actual events and require the speaker update before PIT's final
edge. Both current independent targets rebuild and their registered case runs
once per width: x64 1.13s total, x86 1.41s total, both pass. This is positive
regression evidence, not yet an executed old-order negative or full-unit proof.
The added observations exist only in the test; production retains its one
owner and direct lifetime order. All affected hashes must be refreshed.

## Complete XT Assertion Receiver Reconciliation

Read the complete accepted XT Board test, complete receiving test, and their
non-truncated no-index diff using the ignored baseline copy s94-review-xt.c.
Read complete Core/XT construction fixtures and Core time fixture; inspect the
receiving CMake executable's four source entries and real Board/Core links.

- PB7 remains the sole byte-latch acknowledgement; repeated PA reads retain
  1Eh, clearing deasserts readiness/IRQ1, and the following 9Eh is delivered.
  DIP selection, speaker gate/data/reset and absence of AT 64h stay unchanged.
- RAM parity and I/O-check retain PC7/PC6 observations, active-low PB4/PB5,
  masked and unmasked NMI execution, and reset clearing. Private state reads
  move to XT-owned comparisons with identical true/false failure predicates.
- BAT retains 12499/12500 reset thresholds, 300000/300300 delays, the exact
  elapsed+60 deadline and AAh delivery. Core's copied elapsed observation
  replaces its direct field read; the owner-local time helper calls the same
  Core advance operation. All sixteen FIFO slots, final FFh overflow and
  260-tick delivery intervals remain.
- All six refused-completion combinations remain in main, with identical
  occupied latch, 25-tick no-new-frame boundary, clock inhibit, mode-release,
  reset discard and 10000-tick no-duplication checks.
- The original eight allocation-failure positions still execute in the XT
  fixture. Core owns allocation injection/port inspection and retains absent
  60h-63h routes, unpublished PPI, 80h=5Ah sentinel, successful retry,
  A5h readback and double finalization. No assertion is dropped by splitting
  the fixture. The new teardown regression is additional, not a replacement.

This completes this test receiver's source/diff reconciliation. Earlier
dual-width focused passes remain narrow evidence. Fresh complete root unit
and gate runs are launched after the Board lifetime repair; their terminal
results must be recorded before they count as current verification.

## FDC And HDC Adapter Body Review

Complete the remaining FDC mark/format, IRQ/DRQ, timing conversion, DMA, port,
construction/reset/refresh/finalization source reads; read complete HDC media,
register, port, DMA and lifetime bodies and both adapters' public/private
headers. Compare complete non-truncated diffs against accepted App sources
using ignored s94-review-fdc.c and s94-review-hdc.c baseline copies.

The FDC diff adds opaque candidate allocation/destruction and configuration
rollback around the existing connect/initialize bodies. HDC adds the same
candidate lifetime, makes connect/initialize private, and names its reusable
rollback operation clear_configuration. All pre-existing media-result
conversion, register decode, clock conversion, data transfer and IRQ/DRQ
bodies remain unchanged. These are real allocation/failure boundaries, not a
second chip implementation. Public interfaces expose opaque handles and
frozen/copy configuration, not chip fields, sector cache or platform files.
Complete private-header diffs retain the original adapter fields behind named
opaque structs. Read both complete public headers alongside their complete
baseline headers: FDC retains DMA provider, reset, absolute advance, due query
and refresh; connect/initialize become one configure operation, and embedded
lifetime becomes create/destroy. HDC retains its DMA binding/providers, reset,
absolute advance, due query and IRQ observation; connect/initialize similarly
become configure. Its old finalize cleanup becomes clear_configuration for
failed route publication, while destroy releases the candidate. Common Board
removes a published HDC route batch after DMA-bind rejection, propagating a
removal failure before clear_configuration. Neither
public interface exposes the former connection/data/chip layout. This completes
declaration mapping, not all receiver coverage qualification. XT replaces its PIC-private lease with an injected IRQ line
and owner; common Board owns the actual lease. New public-header declarations
have source review but still require complete baseline declaration mapping.

FDC installs one atomic Core route batch after chip construction and destroys
the chip on route failure. HDC's composition publishes its provider through
Core; its clear_configuration contract requires revoked routes. Both retain
borrowed media/PIC/DMA lifetime, and Board teardown releases them before those
owners. This source review does not independently qualify every original
controller test receiver or claim live isolated hot-detach; complete coverage
reconciliation remains required.

## AUX State Assertion Reconciliation

Review every deleted aux_enabled predicate in the baseline-to-current test
diff. The KBC controller receiver at test/x86/ibmpc-at retains its actual-chip
AUX/scanning reset checks unchanged. Five App receivers instead lost distinct
actual AUX state checks: default PC/AT topology and composition, 5170
composition, Model40 initial integration, and two Model40 private-composition
checkpoints. A command-byte response is not equivalent to actual AUX state.

Restore all six distinct predicates through test_board_kbc_aux_enabled;
collapse only identical adjacent baseline duplicates in the 5170 and Model40
integration checks. Default PC/AT expects enabled; 5170 and Model40 expect
disabled. Existing later Model40 A8 rejection checks already retain the actual
state predicate. The existing Board fixture delegates to the AT-owner fixture,
which reads x86_kbc8042_aux_enabled(kbc->chip); no mirrored state or production
getter is added. Add those two existing fixture sources to the three PC/AT
test targets and include their existing header. This changes tests/build only;
production, artifacts, firmware, INIs and MyNES are untouched.

After this test/build repair, run build/s93-verification.ps1 once per width,
serially. Both processes finish with exit 0: x64 units pass 491/491 in 67.94s;
x86 units pass 491/491 in 58.25s. Both specialized targets pass 81 steps;
the strict matrix now covers 515 rows (494 strict, 21 deferred), reflecting
the six additional fixture compilations in the three PC/AT test targets, not
six new behavioral tests. The intentional T345 negative is followed by its
verifier self-test success. Inventory hash reconciliation reports zero
mismatches, documentation governance and diff checks pass, and MyNES status
has no changes. The complete source/coverage review remains open; this batch
does not accept S94 or deliver a partial implementation commit.

## Verification Cost Review And Repair

### A20, D4 And Keyboard Gate Source Review

Read all three complete verifier sources and their complete accepted-baseline
diffs, then reconcile the changed predicates against the already reviewed actual
owners. A20 changes only its KBC/Board paths; callback A20 publication and the
Core fallback route requirements remain. Keyboard transport changes the copied
input path and checks opaque pointer ownership instead of the retired embedded
layouts. Core has neither keyboard member; Board retains both, and the neutral
scheduler forbidden-access check and real input submission requirement remain.

D4 checks now locate Model40 memory, independent AT parity and Model40 platform
definitions rather than the former monolithic Board file. New exclusions reject
common-board layout imports in D4. The electrical-operation exceptions allow
their true AT-parity/D4 owners, not a second Board lookup. The removed old
D4 configure/reset caller pattern had treated both as Board-receiver operations:
actual configure now takes the neutral Core plus its memory owner, while reset
takes that memory owner alone. They are not retired Board methods. The atomic
two-route registration, parity observer publication, single RAM reconfiguration
path and Core memory-admission callback predicates remain. All three positive
gates pass. These structural checks are supporting evidence, not substitutes
for the reviewed production semantics or final behavioral qualification.

### Checked IRQ Setup Sweep

Complete the admitted 50-call/40-file positive-binding sweep. Replace 49
standalone ignored-status calls in 39 receivers with the existing PIC fixture's
new checked setup operation. It calls the same public bind with the same
arguments and terminates the test on failure before IRQ observation. Intentional
invalid-bind status assertions and assigned calls remain direct. Normalizing
only the new helper name back to the old operation yields identical complete
LF-normalized source hashes for all 39 files; no programs or assertions change.

The remaining PIT-IRQ0 receiver initializes its entire fixture, checks PIC
construction, all eight ICW writes, IRQ binding, PIT installation, provider
freeze and Core reset, returning the first failing status through its single
existing finalize path. PIT destruction releases its output callback before
PIC destruction; both tolerate absent allocations. Its twelve successful port
writes now use the existing checked helper. From program setup through main,
all original edge/counter/gate/reset assertions exactly match accepted source
after checked-write normalization. This does not alter production timing.

Both widths rebuild all 40 receivers with strict compiler options and pass
40/40 once each: x64 33.67s and x86 2.45s. These differing wall times are
observations, not a proven compiler/guest-performance diagnosis. The temporary
NULL-owner probe compiles against the real PIC fixture and linked shared code,
then exits 1 as required; temporary probe source/executable are removed.
The repeated full sweep finds zero ignored standalone binding calls. Update
41 test-manifest entries and the affected review hashes. Mechanical repair
qualification does not accept any still-pending full-source review row, and
these focused checks do not replace current full-unit closure validation.

### Renewed Performance Inspection

The renewed owner request removes the remaining per-target Ninja launches in
the same verifier. Read the full script and generated command/object paths;
validate all matrix rows first, query the 340 unique targets in one invocation,
then index only compiler commands by their actual CMake target object directory.
Every one of the 515 source rows retains its strict/deferred flag check. Unlike
the former dependency-wide lookup, a dependency's compiler command cannot
satisfy a caller's row. No successful result or command cache persists between
invocations. This changes only the NXVM verifier, not shared production code.

Single measured runs on the unchanged x64 matrix: 36.140738s before,
4.9107794s after (86.4% lower observed wall time). The x86 matrix also passes
all 515 rows (494 strict, 21 deferred), in 5.137477s. Four temporary build-tree
negative inputs specifically reject duplicate rows, missing source commands,
strict commands misclassified as deferred and dependency commands attributed
to their caller. These are verifier proofs, not whole-unit/S closure evidence.
Fresh confirmation of the optimized x64 matrix passes all 515 rows in 5.72s.
The current Shared x86 test increment compiles/links 39 steps in 5.78s; an
immediate unchanged-input build reports no work in 0.43s. This confirms that
unchanged objects are already reused; it does not measure a cold full build.

The prior complete unit runs cost 54.75s on x64 and 48.85s on x86, with the
recorded x86 modal-window failure still requiring a current full passing closure
run. Historical CTest mean costs are not a fresh benchmark. Incremental Ninja
already avoids unchanged compilation; the remaining S94 wall time includes
the actual-source/coverage review, not just compiler/test execution.
Keep temporary focused repair checks, full units/gates at S closure and the
owner's one-run-per-boot-group limit. Do not rebuild MyNES or lower coverage.

The owner's performance request keeps S94 active and preserves its complete
acceptance scope. The recent no-change build reports Ninja has no work; the
ordinary incremental compiler is not forcing a complete rebuild. The previous
unit runs take 67.94s on x64 and 58.25s on x86. A single file-only CPU-boundary
negative costs 46.10s/39.52s and has RUN_SERIAL, causing a global test barrier.
Read its complete script and gate: mutations occur only in its exclusive
build-tree copy; the gate reads that copy. Remove only its RUN_SERIAL property.
Native desktop and integration resource scheduling is unchanged.

Read the complete T344 direct-compilation verifier and its invocation. Its
515 rows query only 340 distinct targets. Retain every row validation, exact
duplicate rejection, command lookup and strict/deferred flag predicate; query
each target once per invocation, using a SHA-256 keyed local CMake variable.
No file cache or previous successful result is reused across runs. The same
x64 matrix passes before in 49.5379s and after in 35.1280s (one measured run
each, not a stable multi-run benchmark). Three same-target negative probes
still specifically reject a deferred classification of a strict command,
a missing source command and a duplicate row. Fixture inputs are ignored
build-tree data, not new product tests or a second gate implementation.

The verification workflow now uses temporary focused checks during repair;
complete dual-width units/gates remain mandatory for S closure. Source-only
audit batches or documentation updates do not justify rerunning identical
complete suites. This removes repeated work, not coverage. MyNES, artifacts,
owner INIs, media and production source are unchanged by this cost repair.

After the renewed performance repair, continue the admitted IRQ-binding sweep:
read the remaining 25 call contexts, completing the inspection of all 50 ignored
standalone calls in 40 test files. These are positive setup paths preceding IRQ
assertion/deassertion and execution; none is an intentional invalid-binding
assertion. Existing assigned/checked calls are outside that ignored-call set.
The shared PIC fixture is already included by these receivers. A checked
test-only binding helper can terminate before IRQ observation without changing
the positive arguments or assertions. The PIT-IRQ0 receiver additionally needs
its existing construction/freeze/reset/write cleanup reconciled as one setup
boundary. Context inspection does not qualify the full pending source bodies;
the flagged inventory dispositions remain open until repair and regression.

Run complete units once after the scheduling revision, with the same -j8:
x64 passes 491/491 in 54.75s, versus 67.94s previously (19.4% lower observed
wall time); both incremental build steps report no work. x86 finishes in
48.85s versus 58.25s, but passes only 490/491: library.kvm_window_modal fails
case 0 at line 74 because its exited event is already signaled. This is not
a full passing x86 result. Its RUN_SERIAL property remains unchanged and the
CPU negative has finished before this test starts. A single isolated rerun
passes in 0.84s. No Lib source/test change or weakening is admitted; preserve
the failed whole-run log and require a current full passing run at S closure.
The x64 complete specialized target passes all 81 steps. The separately run
x86 specialized target also passes all 81 steps with exit 0, including the
515-row strict matrix; its full-suite script correctly stopped at the unit
failure. These gates do not replace a full passing x86 unit run.

## Default PC/AT Receiver Review

Resume S94 by reading both complete vm_pcat_topology_s2_smoke and
vm_pcat_composition_s4_smoke sources and their full baseline diffs. Original
explicit IRQ0/1/12/8/6 and DMA2 routes, all profile port leaves, absent
62h/63h/3D6h/3D7h/3F3h routes, RTC provenance and controller configuration
checks remain. The topology descriptor's invalid read flag check and exact
route-table comparisons are unchanged. Composition retains F000:FFF0, zero
timeline/pending/sequence, NMI unmasked, genuine NOP progress, port70 NMI mask
and reset rearm assertions. The actual AUX-enabled predicates are restored.
The additional command-byte check does not replace them.

Read the complete Board composition fixture and its PIC/KBC helpers: copied
configuration comes from the same Board fields; PIC source verification uses
the same reset/program/assert/register-observe sequence as before. KBC commands
use public Core ports with existing owner-local reply/deadline handling. The
Core-owned port-presence and post-run write helpers perform the original
operations on the Core-owned port table. App sources no longer read those
layouts; no production getter is added. Whole build-list qualification and
other test receivers remain open.

## CGA/EGA/Model40 Video Receiver Review

Read the complete Model339 CGA topology and Model40 CECG S9/S11 tests,
their full accepted-baseline diffs, both owner-local video topology fixtures,
the bus fixture and the common video fixture. The eight original CGA port
direction predicates and four default EGA predicates are unchanged; numeric
port replacements match the baseline VADP definitions. Board aperture checks
read the same actual video chip, with explicit expected false/true values.
Text snapshot kind and A0000h write/read-FF assertions remain intact.

Model40 S9 retains control values 40h/7Fh/A5h, the lightpen write route,
status mask 06h before reset and 04h after reset. S11 retains every A0000h/
B0000h provider/ordinary-RAM transition, display-disable aperture assertion,
misc-output transition, graphics-map change and reset checkpoint. The removed
local route helper has exactly the same query size, read access, status and
expected-route predicate at its Core-owned receiver. Neither helper adds a
production getter or copies device state.

The bus helper uses existing paused/stopped Core bus operations, checks status
and does not advance guest time. The three CMake targets compile the matching
owner-local fixtures. All three test identities pass in the latest full-root
x64 and x86 logs; the separate x86 modal-window failure remains outstanding
whole-suite evidence, not a failure of these receivers. The common video
fixture's source is read, but its other consumers still need assertion/diff
reconciliation; this batch does not accept all video coverage.

Read the remaining Model40 CECG S10/S12/S13/S28 tests and complete baseline
diffs. S10 preserves B8000h text OK, snapshot kind, feature/environment value
03h and reset value 00h. S12 preserves the complete color-to-mono CRTC sequence,
inactive color read 00h, active mono 34h, environment 03h and reset color 25h/
inactive mono 00h. S13 preserves input-status E0h/F0h/E0h. Every numeric port
matches the old adapter macro, including environment 07C6h.

S28 preserves all four CRTC geometry writes, map 07h, pixels 15/0, reliable
generation and capture-required checks, and cleared reset pixels. Its byte
write now enters the existing stopped/paused public memory API, which delegates
to the original physical write owner; successful writes additionally invalidate
CPU prefetch and record trace, without advancing guest time. No predicate,
pixel expectation or execution path is removed. These four identities also
pass in both current full-run logs. This is preserved regression proof, not a
new hardware-manual or runtime-boot qualification.

## Executor, Competition And Rollback Receiver Review

Read complete current instance, checked-memory and 80386 competition tests,
their full original files and all line differences, plus complete Board
executor and DMA-competition fixtures. Instance and checked-memory differ only
in owner-local fixture include/name substitutions. Competition preserves every
trace predicate, 3-tick CPU budget, 8-tick device advance, A5h DMA byte,
CPU-begin/commit/retire before DMA-begin/commit/advance/PIT/PIC order, absent
FDC/HDC events and all HOLD/reset checks. The Board-owned fixture creates and
binds channel 2 before reset mapping registration instead of after it; these
construction operations have separate owners. Failed binding now destroys
the unpublished machine. No second DMA state or execution path is introduced.

Read complete port-assembly Board fixture/header, Core declaration header,
the current App test main and original main/injection code. Inspect original
and current FDC, RTC, HDC and Port-B transaction sections directly. FDC retains
chip failure plus seven route failures, zero adapter/topology, no routes/DREQ,
zero route allocation on chip failure and successful retry. RTC retains both
allocation failures, null chip/zero configuration, absent routes, collision
preservation and retry. HDC retains all protocols' 7/19/18 route matrices,
chip failure, conflicting route preservation, occupied DMA lease rejection,
zero adapter/topology/token, retry, Compaq 3F7h OR-composition and Xebec's
write-only 323h. Values are transient observations of the real owner, not
persistent mirrors or production getters.

Port-B keeps both planar parity and product D4 failure cases, both allocation
points, absent RAM parity binding/routes after failure, refresh reload 18 and
reset/port-data checks. The former D4 configured/latch fields are now an opaque
Profile lifetime: failed attach leaves no Profile binding and no port routes;
the App supplies its real D4 attach operation, never a Shared model switch.
The incomplete-binding rejection is additional coverage. Old injection
callbacks are unchanged apart from receiver paths and opaque adapter address
forms. All four original test identities pass in both latest root logs.

Remaining Core port-assembly helper sections and the D4 attachment body's
failure proof still need complete qualification; do not classify that entire
fixture as accepted from this transaction-section review. No source mutation,
new test run, artifact rebuild or partial P is performed in this batch.

## Remaining Port Helpers And D4 Attach Cleanup

Complete the preceding port-assembly review by reading all remaining current
helper bodies and comparing each against the accepted original monolithic
test. Range/batch transaction, timestamped lane/wired-OR read, Port-B live
time, DMA byte lanes, PIT publication and PIC rollback are exact function-body
matches after removing static linkage. Create-failure retains all three
default/aux-PIT/XT variants, every allocation failure through count+1, cleared
output pointers, attempt counts and attachment context. Its former Board
back-pointer check now executes inside test_board_construct before output
publication; a mismatch destroys the machine and returns INTERNAL_ERROR.
The original refresh reload-18 helper is also an exact body relocation.

Read both complete constructor fixtures and the original construction header.
The original projection, neutral fault-injected constructor, real Board
construction and success-only publication remain one chain. Board construction
owns Core cleanup on failure, so the fixture does not destroy it twice.
The independent port_assembly_core_smoke repeats the complete common matrices;
the App main retains the additional D4 cases with its product-owned attach.

Read all current D4 platform source and inspect Board construct_profile's
validation/factory/bind/finalize sequence. D4 create clears output, allocates
one zeroed candidate, publishes its port atomically and frees it on route
failure before attaching PIT outputs. Successful factory creation followed by
binding failure invokes the candidate finalizer, removing owned routes and
freeing it. Attach publishes its output only after successful bind; on every
failure it remains null. This proves the pending transaction cleanup path;
complete baseline/current D4 behavior reconciliation is still a separate open
review item. No source or verification input changes in this batch.

## D4 Platform Baseline Reconciliation

Compare the complete current d4_platform.c and both headers with the original
machine_board.c D4 construction, port read/write, NMI, refresh request/complete,
reset phases and observation bodies. All Port-B masks, clear-latch pulses,
single NMI signaling, failsafe edge, refresh address rollover and PIT reload
74h/18/0 remain. The former timer helper's elapsed-tick branch was planar-parity
only; D4 never selected it because construction rejects coexistence. Its
receiver therefore correctly reads PIT1/PIT2 outputs directly. Speaker writes
now call the frozen common Board line callback rather than reading Board state.

Configured checks become an existing opaque object's lifetime, not a guessed
default. Null platform observation retains the old unconfigured Board values
(configured/latches false, IOCHK/failsafe enabled). The added memory observation
reads the same Profile-owned memory object. Reset callbacks preserve memory
before devices, latches before PIT reset, outputs after PIT and refresh-state
clear after reattachment. The pending refresh deadline remains now+1 on Core's
source axis; this is unchanged fallback behavior, not a timing upgrade.

One Profile context owns latches and memory; destruction revokes owned memory/
port routes and PIT output callbacks before releasing it, while common Board
destroys the borrowed PITs later. New allocation/binding lifetime replaces
embedded Board fields without a second state path. Baseline reconciliation is
complete for this platform file and headers; the separate D4 memory source and
all Profile callers still require their own review.

## D4 Memory And Model40 Composition Reconciliation

Read the complete original D4 memory implementation and its current Profile
receiver, both receiver headers, Model40 composition and configuration, and
the original/current memory transaction test with the Core registry fixture.
The 80C00000h control window, extension-RAM range, low-nibble decode, byte
access rules, diagnostic masks, reset values and return classifications are
unchanged. The owner changes from embedded Board memory to one Profile memory
object; parity assertion and write-observer clearing call its frozen IOCHK
output, which reaches the same platform latch. No second RAM backing or new
timing classification is introduced. Configuration publishes that callback
only after atomic Core route/parity/observer installation succeeds.

All four original failure cases remain: full provider capacity, one free slot
for two routes, existing parity ownership, and full write-observer capacity.
The Core-owned fixture retains pre/post counts, parity identity, retry,
replacement and owner predicates, and complete owner removal. The product
callback retains configured/reset/diagnostic predicates and additionally
checks unpublished callback state on failure and actual parity/IOCHK output.
This is coverage relocation, not removal of the former transaction test.

Read the full D4 platform S4 test and its baseline diff. Port exclusivity,
refresh reload 18, Port-B masks, speaker transitions, masked/unmasked NMI,
auxiliary failsafe, shutdown reset without elapsed-time advance and final
reset predicates retain their original values and order. Opaque platform
operations replace Board-field operations; owner-local Core fixtures replace
cross-owner private executor accesses. No original predicate is removed.

Model40 composition retains its two 1.2 MiB mechanisms, Compaq HDC ports and
shared 3F7h decode, RTC configuration and memory alias/open-bus declarations.
Its selected factory creates the single D4 platform and memory owner, destroys
the candidate on memory-install failure and returns the frozen Board binding.
The App construction caller clears its borrowed Model40 pointer during failed
construction cleanup. This read does not accept the entire generic plan or
App lifecycle: their remaining source/rollback paths have separate inventory
rows. Correct one stale Model40 header comment: Profile owns D4 decoding;
Core owns route dispatch. This is owner wording only, not executable change.

## Common Plan And App Construction Reconciliation

Read the complete original/current machine_plan.c pair. Except for include
relocation, stale-comment removal and replacing the D4-specific topology/config
fields with the existing selected Profile factory, validation, timing/seam
classification, copied configuration, media/display bindings and controller
installation order are unchanged. The former D4 presence checks disappear
with those fields; Model40 owns their construction and memory configuration.
No machine name enters the Shared plan. create_from_plan clears both public
outputs first, validates, creates one Core/Board candidate, installs topology
and declarations, destroys the candidate on any failure, and publishes only
after success. No second executor or time axis is added.

Read Board bind_profile/construct_profile and their reset/finalize consumers.
The factory returns a copied callback binding. An incomplete binding is
rejected before publication; a successful factory followed by failed binding
is finalized. A bound Profile is finalized before borrowed PITs are destroyed.
Model40's optional construction output is a borrowed pointer, not an owning
lease; App clears it in failed creation cleanup and normal storage finalization.
Do not infer complete Board qualification from this selected-function read.

Read complete App machine.c and its three baseline diff hunks. The changed
surface is the Shared Board include, passing the borrowed Model40 output and
clearing that pointer on storage finalization. All other input, media, CMOS,
creation/reset and destruction bodies are baseline-identical. This file does
not acquire a second D4 owner. Separate lifecycle/display/debug files still
need their own review; baseline-identical bodies are not a new quality waiver.

Reconcile all differences of the 492-line original plan smoke with the
436-line receiver, and read the full Core plan fixture/header. The six moved
blocks retain attachment identity, allocation-failure steps 1--3 and null output
checks, all XT route presence/absence predicates before/after reset, PIT due
tick 1 and DMA due tick 3 with matching advance results. All other original
plan test bodies and markers are unchanged. Board-owned layout assertions
remain in the Board test; only Core-owned private assertions move to Core's
fixture. No predicate or timing expectation is dropped.

## App Copied Input/Display And Display Receiver Reconciliation

Read complete App display.c, machine_interface.h and machine_private.h with
their baseline diffs. display.c changes only its two owner-path includes;
cadence, snapshot generation, cursor/glyph/palette copy and frame conversion
are unchanged. The public Machine header relocates the copied input include.
The private header relocates the Board include and adds only the borrowed
Model40 pointer already reviewed with construction/cleanup. Its diagnostic
consumers are existing Model40 tests and the DOS boot probe, not another
production D4 owner. Both relocated copied input/display headers are identical
to their originals after line-ending normalization; dimensions, field types,
enum values and constants retain their ABI. Shared x86 has no App include or
use of these product input/display value names.

Read unchanged App lifecycle.c/control.c as consumers of the changed private
header. Common still owns the worker and lifecycle queue; App supplies the
bounded Core runner/reset/input/frame/debug callbacks. Construction-only
uncomposed reset/stop paths are unchanged from the accepted baseline, not a
new execution path introduced by this extraction. lifecycle, control, debug
and debug_adapter have no baseline diff in this working migration. This
consumer review does not approve unrelated baseline refactoring.

Read complete Shared machine_display.c and its original pair, then the
receiving VADP validator and observe/capture functions. Stopped/paused
admission, invalid/unsupported statuses, display publication and observation
remain unchanged. The original port/personality validator is at VADP's owner
with the same predicates; snapshot operations follow the same single video
chip through opaque adapter calls rather than reading its embedded fields.

Read complete vm_display_composition_s5_smoke.c and every diff hunk. The Core
port fixture calls the existing public bus without advancing guest time;
the replaced video address is the original B8000h constant. All original
text/CGA/EGA dimensions, pixels, palette, detached copied-frame generation,
snapshot acknowledgement/invalidation and reset assertions remain. No host
presenter or second video state is needed by this repository-only test.

## Board Advance/Deadline Source Reconciliation

Read complete original/current board_advance.c and board_deadline.c pairs.
All six device clock initializations/resets, retained rational phase, DMA/PIT
tick conversion, primary/auxiliary PIT order, PIC refresh/acknowledge, media
absolute due ticks, RTC conversion and peripheral ordering are preserved.
Changed FDC/HDC/KBC handles are the opaque receivers already reviewed; video
advance forwards the same settled ticks through its existing adapter. Host
presentation does not advance guest time. Read Core scheduler's corresponding
observation/arbitration/readiness/publication consumers; its settled order is
timeline, DMA/PIT/PIC arbitration, media/FPU/RTC readiness, then peripheral.
This selected-consumer read is not whole Core scheduler qualification.

Deadline minima, zero/immediate handling, timing qualification, DMA explicit
clock checks, FDC immediate fast-advance block and XT source-axis keyboard
deadline are unchanged. D4 pending-refresh now reaches the already reviewed
Profile next_deadline callback, which returns the original now+1 boundary.
The existing unqualified DMA L1 compatibility/fast-advance-block flags retain
their baseline predicates: no new fallback or precision upgrade is introduced.
Core still combines Board observation with timeline/FPU deadlines on its sole
elapsed axis; Board observation does not mutate a clock or publish ticks.

Read complete rational-clock test original/current. It now constructs neutral
Core with the same 80286, one retirement tick and provider 3/2 ratio/reset phase
1 instead of constructing unused PC components. All invalid/identity/reset
clock predicates and provider sequence 5,4,5,4 remain, with identical split
quantum 1/2/4 comparisons. Its reset-map fixture has a separate review row.

Read both boundary gates and baseline diffs. Deadline gate only changes owner
paths. Peripheral gate changes the direct video call to the opaque adapter
call and adds a check that the adapter forwards settled ticks to the chip;
readiness-before-peripheral and construction binding checks remain. They are
static ownership evidence, not substitutes for runtime sequence assertions.
Execute both CMake scripts against the current repository: each exits 0 and
emits its BOARD-DEADLINE or BOARD-PERIPHERAL success marker. Documentation
governance and diff whitespace checks pass for this documentary review batch.

## AT Parity Owner And Original NMI Coverage

Read complete parity.c/parity_interface.h and compare the original Board's
port-B timer helper, read/write, memory-fault, NMI, configuration and observation
functions. Refresh bit 4 keeps the same elapsed-tick quotient/parity formula
or PIT channel-1 output; bit 5 still follows channel 2. The writable low nibble,
latched bit 7, reset value 04h, enable/clear behavior and one-shot NMI admission
remain unchanged. Config/observation field types and enum values are retained.
Port-only wiring still reports no parity-memory producer. The new object owns
only the former parity fields; Core owns RAM/parity storage, and Board retains
refresh programming and speaker/PIT wiring. Candidate memory and port routes
are revoked on failed publication; destruction revokes routes before freeing
the callback owner. Whole Board constructor qualification remains separate.

Read the complete original and relocated planar-parity NMI test, plus both
Core fixture files. The original 1234h write/read and deliberate parity-bit
fault, frozen RAM-reconfiguration rejection, masked/unmasked NMI, 61h values
14h/84h, PIT advance of 19 ticks, clear/re-enable/reset predicates, conflicting
port rollback, null parity storage/owner and successful retry all remain.
Only Core-private RAM assertions move into the Core-local fixture; the
successful parity-owner identity now matches the actual opaque AT object,
not the former all-purpose Board. The original unbound-reconfiguration test
also keeps its construction/freeze/reset/rejection sequence and failure codes.
The deleted App test is paired with this Shared receiver, not dropped.

Read the independent AT smoke in full. Its four combinations cover memory
present/absent and PIT/elapsed refresh selection, conflicting route failure
and retry, speaker low bits, fault admission, latch clear and cold reset.
This source review identifies those assertions; it does not claim that the
new smoke directly measures both refresh waveforms, nor substitute it for
the preserved Board/PIT/NMI regression. No executable input changes in this
documentary batch; closure still requires the current full-unit proof.

## Board Public Construction And Reset-ROM Baseline

Compare eight complete Board functions: configuration validation, executor
projection, public create, reset-alias selection/registration and A20 port
read/write/registration. Each is identical to its S92 Board counterpart after
line-ending normalization. These mechanisms already lived in the accepted
App Board, not neutral Core; this working batch relocates them to Shared
Board. Neutral Core machine.c has no baseline diff. This direct comparison
corrects any impression that this batch invents or newly moves those eight
mechanisms out of Core. Whole Board construction/teardown remains subject to
its separate opaque-controller and family-owner comparisons.

Read complete original/current reset-ROM regression. Preserve both 286/386
far-jump-to-HLT cases, exact reset bytes/physical addresses, four ROM policy
modes, mapping counts 1/2/2/4, 14/15/16-byte source admission, explicit high-ROM
precedence and partial-window 2-byte boundary. The absent-memory allocation
failure, unpublished provider count, retry, ROM-priority byte 5Ah and adjacent
absent byte FFh assertions remain. The Board's configured bit is now read by
its tiny owner-local fixture; Core-private reset/mapping/allocation assertions
remain in the Core test. Two initialized-state fallback reads use existing
public memory_inspect, which admits non-running initialized backed memory
and reads the same physical addresses below 1MiB. No A20 wrap or guest I/O
side effect is part of these two byte assertions. This receiving review does
not claim whole memory-interface implementation qualification.

## Whole Board Main Source Reconciliation

Finish complete machine_board.c original/current review, including every
function rather than selected construction snippets. Function pairing finds
67 current definitions: 39 identical after line-ending normalization, 24
changed receivers/mechanisms and four additions. Fourteen former definitions
are removed from this file: six parity/NMI/port helpers are accounted for by
the AT owner, and eight D4 refresh/platform helpers by the previously reviewed
Model40 owner. This is a source/diff classification, not whole T acceptance.

The four additions are frozen Profile binding validation, Profile factory
construction/publication, a speaker-line callback and the AT IRQ1/IRQ12
electrical callback. Factory failure leaves the binding unpublished; binding
rejection finalizes the candidate, and successful publication uses the same
post-PIT reset phase. Model40 supplies the existing shutdown/refresh policy;
Shared Board does not name a product or decode D4 registers. AT and XT choose
the same IRQ lines as the original internal binding, with PIC leases now
owned by Board. AUX presence, reset output, repeat/response/serial timing and
XT's four duration values are unchanged. Partial creation remains owned by
the already-bound Core attachment and its null-safe finalizers.

Read all changed input, media configuration and reset/finalize functions with
their original bodies. Native-byte/scan-set/mouse admission and DMA token
validation retain their predicates; receivers are opaque. FDC configure owns
the former connect/initialize/failure-finalize sequence. HDC configure/clear
owns the former connect/initialize/finalize sequence; shared 3F7 wired-OR,
Xebec write-only port and DMA-binding rollback remain. Copied topology is
cleared on failure and configured is published only after success. Unchanged
DMA channel binding behavior is not claimed as a new rollback improvement.

Speaker state now stores only the two electrical lines; AT parity, XT PPI and
Model40 each feed that sink instead of Board reading their private latches.
Cold reset retains input, DMA/RTC, port latches, media, PIC/PIT, refresh and
video ordering. Model40's four callback phases preserve its old memory,
latch, post-PIT wiring and final-refresh resets. Finalization destroys parity
and Profile callbacks while their borrowed Core/PIT still live, then input
while its speaker/PIC targets live, followed by PIT/media/DMA/RTC/PIC/video.
This includes the previously recorded XT lifetime repair, not an unsupported
claim of byte-identical teardown. Header/ABI and individual runtime/test
receiver qualifications remain their own inventory rows.

## Board Headers And Profile Binding Contract

Read complete current Board interface/state/Profile-binding headers and every
baseline header diff. XT DIP/fault values, video port/config values and AT
parity values are now declared by their receiving component headers with the
same original fields/order/types/enums. Board config, clock plan, timing
rules, RTC defaults, media declarations and remaining public operations retain
their original definitions. The new Profile factory binding replaces D4-only
plan fields/API; D4 configuration/diagnostic declarations are Profile-owned,
not discarded. Model40 observation extends the old six latch fields with
copied memory diagnostics used by its existing tests, not a runtime layout
borrow. No new machine or product identity enters Shared Board.

The private Board state keeps opaque device pointers, PIC keyboard leases,
one parity handle, one Profile binding and two speaker line bits. Removed D4
RAM/latch/refresh and XT gate/data mirrors have their sole receiving owners;
the plan retains a construction-only factory/context, not an extra runtime
owner. Profile services borrow Core/PITs and the speaker sink only during
construction. Its callback contract explicitly excludes a worker or time
advance; reset phases, shutdown/refresh flags and finalize order match the
reviewed consumers. Bind/construct/factory operations have actual plan and
Model40 consumers. Header qualification does not replace separate review of
each device adapter or the remaining test/build inventory.

## Board Fixture And Small Receiver Coverage

Read the complete original Board fixture and its current receiver, then all
three included Core fixtures. NMI setup/execution and lifecycle-tail helpers
are unchanged. Reset mapping is Core-local and keeps the minimum 15-byte
prefetch window plus narrower 8086/186/286 aliases, now through the existing
public profile query/alias installation. Port helpers use the public Core
bus and fail the test on an operation failure, rather than reading private
port state. The delivery/handler second-round helper is byte-identical at its
Core receiver. The memory-provider helper constructs the same bounded route
through Core's public atomic installation. The former video route-query
helper is replaced by the reviewed CECG test's public memory_query; the PIT
programming helper is now local to its actual Board test, with identical
control/divisor bytes and explicit write-status checks. No replacement
fixture borrows another owner's private layout.

Read the entire original/current PIT-divider test: 286 reset mapping still
covers 15 bytes, PIT ratio is 1/4, and every original low/high transition,
four-/two-instruction run, partial phase reset and reproduced period remains.
Read the original scheduler's Board-timing subsection and the new isolated
receiver: all PIT programming, tick conversion, unqualified/qualified
deadline predicates and cleanup remain. Only machine->attachment.context
becomes the actual Board handle; the neutral scheduler's other assertions
remain assigned to its separate pending receiver review.

Read the complete new binding-identity test and both owner-local fixture
files. Board produces expected callback identities from declarations, not
from Core's stored attachment. Core verifies the sole non-self context and
four reset/clock/NMI/finalize callbacks. The fixture also executes the
null-safe NMI entry point and checks Board's borrowed Core identity. These
are extra extraction regressions, not replacements for original tests.
Build declarations register the identity test with the common Board target
and the separated timing receiver. Whole CMake qualification remains open.

## Core Scheduler Receiver And Extracted Board Subsection

Read complete current scheduler smoke and every original/current diff hunk.
Its only removed test body is the already paired Board-timing qualification
receiver. Every other callback, quantum/provider counter, zero-budget result,
three-tick retirement, media due tick, qualification input and
deadline/immediate/L1-fast-block disposition assertion remains. Original
direct calls to private Board functions become the matching callbacks from
the production attachment captured before instrumentation. The reviewed
Board attachment binds those same function identities and context. The
test restores that full attachment before destruction; it does not leave a
stack probe as the finalizer's owner.

This remains a Core-owned scheduler test constructed with the real Board;
it is not a newly neutral-only test. Core-private instrumentation observes
Core-owned fields, while Board state/function internals are no longer
included. The public construction/lifecycle helpers are reused without
borrowing another owner's layout. The original scheduler and PIT App
deletions are now paired to their complete Shared receivers; no original
predicate is silently removed by splitting the owner-specific subsection.

## Profile Plans And Remaining Public Value Contracts

Read the complete default PC/AT descriptor implementation and private header,
XT descriptor/source, Profile contract and floppy header, keyboard mapper,
and App machine-plan implementation/interface. Compare all baseline diff
hunks. Existing port/IRQ/DMA tables, nominal clocks, timing qualification,
drive/media distinctions, ROM copies, glyph conversion, CMOS input, optional
ROM handling and failure cleanup remain unchanged. Keyboard scan tables and
Pause sequences are unchanged. Machine identities and media policy remain
App-owned, not additions to neutral Core or Shared Board.

The sole executable machine-plan change passes Model40's construction-output
address to its already-reviewed selected factory. App supplies its own field
address; the factory clears it before construction and publishes the opaque
Profile object only after memory configuration succeeds. Board's binding
owns destruction; App's pointer is a borrowed diagnostic reference, not a
second platform state or independent finalizer. Non-Model40 materialization
and all input rejection paths retain their baseline bodies.

Correct three matching stale owner comments: the PIC and PIT1-to-DMA wiring
now belong to Board, not neutral Core. No constant, timing label, executable
statement or API is changed by this wording repair. This is the approved
matching owner-comment sweep, not a timing upgrade or new L1 finding.

Read complete public controller and FDC terminal-observation value headers.
The controller header differs from its original only by removal of an unused
PIT include and its blank line; the observation header is identical after
line-ending normalization. Read transaction state and copied trace headers:
owner/kind enum bodies and numeric values move unchanged into trace; private
phase/state stay in transaction. These reviews qualify the actual header
changes, not the still-pending complete build/dependency inventory.

## CGA Graphics And EGA Planar Receiver Reconciliation

Read complete CGA graphics, CGA 640 and EGA planar receiving tests and all
accepted-source diff hunks, using bounded slices for the larger planar diff.
Read the complete shared video fixture and the receiving CMake registration
loop: all three targets compile their actual common-Board source and link
x86-ibmpc-common plus x86-core, without App fixture or private Core includes.

CGA retains seven directional port predicates, CRTC masks and invalid-index
behavior, both lightpen transitions, 320/640 geometry, even/odd scanline
pixels, all normal/alternate/intense palettes, generation acknowledgement,
capture-required changes, reset status and text-disabled blank output. The
640 test retains its separate pixel pattern, palette, stable status reads
and return to text. No expected value or failure predicate is removed.

EGA retains text/planar display-enable sharing, 30h status alternation,
aperture bounds, sequencer/graphics/attribute readback, plane masks, latch
read/write modes 0/1/2, compare/don't-care behavior, invalid read-map masking,
ordinary-RAM fallback for unsupported mode and sequencer reset, provider
restoration, set/reset, aperture remapping, reset clearing and copied-frame
stability. Original literals, geometry and pixel/palette predicates remain.

The fixture constructs a real neutral Core, then freezes/resets on its first
guest-visible bus operation. Bus failures terminate the test. Before freeze,
directional exclusive registration probes actual route ownership and removes
only its temporary unused probe. Public paused/stopped memory calls delegate
to the same physical read/write/query implementation; successful writes also
invalidate prefetch and record trace but do not advance guest time. Private
VADP state inspection is owner-local in these common-Board tests, not a
cross-owner Core borrow. The original App deletions are paired to these three
receivers. All three passed in each latest root-unit log; that narrow proof
does not replace the outstanding current full x86 passing closure run.

Complete XT public-declaration reconciliation against the original Board and
PPI headers. Fault enum values, config fields and four observer/speaker/NMI
callback signatures remain; IRQ lease binding becomes the previously reviewed
Board-owned electrical sink. Create/destroy wrap the original initialize/
finalize lifetime, with unpublished allocation release on failure. The
interface does not add a clock, generic Mode-1/2 behavior or PIC state copy.
Its stopped-dispatch/context lifetime requirement matches actual Board teardown.

## Video Boundary Gate Receiver Review

Read all five complete display-authority, VADP memory/port and EGA controller/
sequencer gates and every accepted-baseline diff. Memory-route and EGA gates
change only receiver paths. Display authority additionally accepts an opaque
Board-owned VADP pointer and continues rejecting either embedded/pointer
Core-owned state and scheduler access. Its public guards, actual Board
receivers, App caller scan and plan/publication predicates remain.

Port-route ownership now checks the Shared source list and rejects either old
App or new Shared VADP source in the App list, instead of demanding the old
App runtime compilation. All raw port/memory prohibitions and route/rollback
predicates remain. These lexical gates support the source review; they do
not alone prove complete include closure or unique compiler linkage. Whole
CMake/source-list qualification is still recorded separately as pending.

Execute each gate once with the current repository as PROJECT_SOURCE_DIR:
all five finish successfully. No gate code changes, test suppression, full
suite rerun, artifact replacement or MyNES build occurs in this audit batch.

## EGA Port And Display Authority Receiver Reconciliation

Read complete controller, sequencer, external-port and display-authority tests
and every corresponding accepted App baseline diff. Controller masks, invalid
indices, four aperture selections, physical writes, attribute masks and reset
checks remain. Sequencer masks, exact generation changes and outside-aperture
non-changes remain. External direction/alias, feature-control, DAC, chain-4,
copied-frame and reset predicates remain; its separate feature-control coverage
comment predates this migration. Display authority changes only includes and
retains absent-board, configure/freeze/reset and invalid-operation checks.
The four original App deletions have these concrete Shared receivers, not
replacement smoke-only coverage. Repair one migration-only failure-cleanup
indentation after revising the S94 packet; no assertion or behavior changes.

Build the affected controller test incrementally in each root width and run
these four identities once per width. x64 passes 4/4 in 2.10s; x86 passes 4/4
in 0.37s. Neither command builds MyNES or replaces a product artifact. These
focused results do not supersede the outstanding full x86 closure proof.

## Shared Build And Corpus Boundary Source Review

Read the entire Shared CMake file, corpus gate and README, together with their
accepted-baseline diffs. One 16-file Core source list feeds production and
observable targets; one 13-file common-board list likewise feeds its two
variants. Each composition must select one variant, not both. Common-board
targets link chip/family capabilities but deliberately do not choose a Core
implementation. AT and XT targets use Types and public chip contracts; no App
source or native platform backend enters the Shared lists. The inherited two
CPU-file GNU warning options are unchanged. Correct README's stale future-CPU
handoff wording to that actual diagnostic contract, without changing warnings.

The gate adds the actual AT/XT/common-board allowed edges and canonical target
mapping while retaining private-header/platform/CRT rejection and the explicit
common-board-to-Core target prohibition. Run this gate once: it passes. Its
lexical check is not compiler/linkage proof. Read the NXVM runtime/observable
construction and Core link declarations: only Model40's two concrete extension
sources remain in the App runtime target. Complete root CMake and actual
per-product compiler/link uniqueness qualification remain pending, rather than
being inferred from those selected declarations. README's neutral fixture
claim has a concrete test/common/machine_fixture.c receiver in test/x86.

Refresh the two affected manifest entries; both complete source/test x86
manifest checks pass. Update the frozen path identities and reviewed
dispositions: 13 additional rows receive explicit review, including the four
deleted App tests and their four receivers. The 610-row raw-file identity check
reports zero mismatches; NXVM documentation governance and diff whitespace
checks pass. The MyNES source/test/artifact status remains empty. No
source-list, dependency, public API or guest timing change is
introduced by this review batch.

## Generated Link And CPU-Local Test Review

Inspect the current generated Ninja executable link declarations in both root
widths, using each rule's actual LINK_LIBRARIES rather than its repeated
order-only dependencies. Each tree declares 479 executable links: 301 consume
Core and 260 consume common-board. No executable names both Core variants or
both common-board variants. The initial stronger same-variant pairing probe
also flags three rows, which are not accepted as defects without source review:
media-provider and display-provider select a standalone archive member without
Core; timing-preview selects observable Core with the ordinary board. Complete
dependency qualification must distinguish archive-member usage and trace
requirements, not force an unnecessary Core dependency into provider tests.
Final eight production link proofs remain S96 work.

The compile-edge inspection finds the declared Core-local scheduler spy object
in addition to Shared target objects. Read its test CMake declaration: it
compiles the same scheduler with the outgoing prefetch-grant name substituted,
and calls the actual CPU grant through the spy. This is test instrumentation,
not an App production implementation. Its complete locality-test coverage and
link replacement reconciliation remain pending; a source-path scan alone does
not qualify or reject it.

Read complete CPU execution-context and FPU-interface-state tests; their
accepted Git diffs are empty. They retain five-profile copied debug/entry
snapshot checks and the FPU profile/prefix/LOCK/VM86 full-state predicates.
Read control-state, signal/prefetch and IDT-entry current sources and full
baseline diffs. Existing bodies and assertions remain; additions receive
protected-to-real cache fields, CLI/STI/HLT storage preservation, 8088 queue
capacity/stale-byte invalidation and NMI/failed-entry rollback. IDT checks
retain original interrupt/trap gate, 16-bit stack and descriptor access-byte
predicates; new cases explicitly verify stack/cache and delivered-exception
non-publication on failure. CPU-private state stays in same-owner CPU fixtures.
These three source-review dispositions are not yet complete original-Core-row
coverage acceptance: pair every transferred assertion with its original
receiver and current Core/public counterpart in the remaining reconciliation.

No executable source, test assertion, build dependency or artifact changes in
this batch. Do not rerun full units merely to append source-review evidence.

## Descriptor And 8088 Coverage Pairing; NMI Context Repair

Compare the complete original descriptor test and current Core receiver. Its
C7 segment-override program, zero-entry registers, 32-instruction budget,
waiting/no-fault status and physical FFFFh result remain through public Core
operations. The other original program leaves protected mode and reloads
segments. Pair its byte sequence and every CS/ES/SS/DS cache predicate with
CPU control-state: selector/base/limit/valid/access/type/DPL and code/data
attributes remain, including EBX=48h and CR0=0. The original cached-state
preparation is now CPU-local instead of borrowing Core's CPU. No original
descriptor case is silently dropped; Core retains memory/executor assertions
and CPU retains hidden cache assertions. Control-state's other transferred
CLI/STI/HLT coverage still requires its separate complete pairing.

Read the complete current profile-gate and original diff; reconcile original
main's ten non-queue calls, program bytes, opcode/fault and memory/PC outcomes.
They remain in Core. Terminal UD still uses the admitted unreadable-vector
fixture rather than private IDTR mutation; no new negative disposition is
introduced. Pair both removed 8088 functions to the CPU signal/prefetch test:
same programs, capacity/count, reservation transition/count/byte, stale
self-modified byte and invalidation predicates remain. The CPU-local fixture
uses real CPU execution and test-owned memory, not Core-private storage or a
second executor. Read its setup/provider definitions; the current fixture's
accepted diff is empty. This is explicit owner relocation, not an unchanged
Core memory-write API route claim.

NMI pairing exposes a context omission: the original external-origin case
uses same-CPL user code, while the added CPU pending-latch case only uses CPL0.
The Board still runs its user case and public state/diagnostic checks, but
private pending-bit coverage must also run under that context. Revise S94's
bounded coverage repair before editing. Parameterize the existing CPU NMI
case by user/kernel source, keeping both original CPL0 cases and adding CPL3
success/rejected-gate cases. The caller-selected descriptor has matching DPL;
check target selector/DPL, original stack delta and clear/retained pending bit,
rollback segments/registers and no second delivery. No public API or CPU
implementation changes.

The first CPL3 probe fails because it inherits the kernel HLT handler, which is
privileged; this is a test-construction error, not a new production finding.
Use the original Board loop handler for CPL3 and retain kernel HLT. The final
matrix passes once each on x64 (0.14s) and x86 (0.10s). Refresh its manifest and
frozen hashes: complete test/x86 manifest, documentation governance and diff
checks pass; all 610 frozen identities match. MyNES status is unchanged. This
focused repair does not establish current full-unit or
complete interrupt-entry/IDT coverage acceptance.

## CLI/STI And HLT Receiver Coverage Pairing

Read the complete current S48 CLI/STI receiver, S49 HLT receiver and their
shared CLI/STI interrupt fixture, plus all three accepted-source diffs. The
Board retains the original profile/opcode/prefix/LOCK matrices, real PIC
IRR/ISR checks, STI shadow delivery and stack IP=2, HLT stack IP=1 and six-byte
stack delta, IF-disabled pending IRQ, fault diagnostics and VM86 GP delivery.
The four 8088 guest programs still exercise PIC masks, byte comparison,
PIT-to-IRQ delivery and RAM POST writes through actual Core/Board execution.
These tests do not by themselves prove the separate guest-time contract.

Full CPU register/segment preservation and terminal-UD whole-CPU rollback
are paired to control-state's 60-case CPU-local matrix. All eight GPR and six
private segment comparisons remain; four real-mode profiles and all original
386 LOCK forms are covered. The negative vector is rejected by the existing
test-owned bus; Board producer negatives install an unreadable IVT overlay
before freeze, instead of mutating Core's private IDTR. CPL3 HLT rejection
retains the whole-CPU rollback predicate in the CPU receiver.

Protected/VM86 Board setup now executes LGDT/LIDT/LMSW, loads a real TSS and
uses IRET to enter valid user/VM86 state. Review the instruction bytes and
post-entry assertions. Accordingly the protected GP frame's source CS changes
from the old artificial selector 0008 with DPL=3 to a valid user selector 001B;
the handler, error code, source IP, flags and frame checks remain. Fixed FLAGS
bit 1 is included in the guest-created frame. This is an explicit fixture
correction, not evidence that the original private mutation route is retained.

The comparison finds a remaining private-state omission: CPL0 successful HLT
had only copied segment checks after migration. Revise S94 before repair and
add ten lines to the existing CPU HLT privilege test: retain CPL3 rejection,
and check CPL0 success, no exception, halted state, EIP=1, exact FLAGS and all
original private GPR/segment preservation predicates. No production/API change.
Incrementally rebuild only that target in each width. Both registered aliases
pass: x64 total 0.19s, x86 0.12s. Update test manifest and frozen identities;
this focused result is not current full-unit closure or Board-time acceptance.
The complete test manifest, documentation governance and diff check pass;
all 610 frozen identities match. MyNES source, tests and artifacts are unchanged.

## Control-State, Table-Register And Deleted HDC Header Review

Read the complete control-state Board receiver and accepted-source diff.
All original 15 static arrays and profile/LOCK forms remain, as do actual PIC
IRQ delivery, ISR, return IP=2/3/3, IF clearing, CLTS/SMSW/LMSW outcomes and
both early-386 MOV-CR read/write cases. Creation uses public reset/register
patching; the admitted unreadable vector-6 overlay replaces private producer
preflight. The removed reset callback did only fixture register setup, not a
hardware event. This Board test complements CPU-local storage checks rather
than duplicating the CPU executor or importing its layout.

Read every current table-register Board function and its complete baseline
diff. Apart from include ownership, IF's public constant and busy-TSS 0B's
equivalent value, bodies and assertions are unchanged. Retained cases include
four table operations with IRQ, memory LTR and descriptor busy-bit write,
SS/ES source selection, four source-limit/store-guard faults, both DOS SGDT
discriminator profiles, the loaded-GDT consumer and four real user-CPL rejects.
The public fixture/helper calls still execute guest instructions, read routed
memory and inspect copied debug values; they do not manufacture CPU CPL/cache.
No test deletion or assertion weakening is needed in this batch.

Close the outstanding deleted App HDC header review by reading its complete
original declaration/layout and the Shared private/public headers. Connection
members and sole chip pointer retain their owner. Connect/initialize are now
private steps of checked configure, with clear-on-failure; finalize is covered
by configuration clear and object destruction. DMA binding, providers, reset,
advance, next-deadline and IRQ observation have public opaque receivers. Read
the corresponding implementation and confirm old connect/initialize symbols
are static only in this one owner. This header reconciliation does not replace
the remaining HDC hardware/rollback test coverage qualification.

This is a source/diff review-only batch. No production/test inputs changed,
so unchanged full suites are not rerun. Update five frozen dispositions;
MyNES, artifacts, owner configuration and media remain excluded.
Documentation governance and diff checks pass; all 610 frozen identities
match, and the MyNES source/test/artifact status remains empty.

## Five String-Instruction Board Receiver Review

Read complete MOVS/STOS/LODS/CMPS/SCAS Board receivers and all five original
source diffs. The original 26 call-matrix cases remain: MOVS 4, STOS 6, LODS 6,
CMPS 6 and SCAS 4. Changes are only owner/include paths and equivalent public
FLAGS constants; compare CF/PF/AF/ZF/SF/OF/IF definitions to CPU's originals.
No program bytes, budgets, profile choices, initial values or assertions are
removed. These are actual Core/Board tests, not CPU-only replacements.

The original IRQ cases retain PIC vector programming and asserted/deasserted
sources, ISR/IRR checks and handler execution. Their stacked restart IP remains
0 for REP and 1 for the single operation. MOVS retains source preservation,
first destination byte and untouched tail; STOS retains byte/word content and
REP tail; LODS retains byte/word accumulator results and unchanged source;
CMPS retains both source images and arithmetic FLAGS. Count/index progress and
the remaining REP count are unchanged in all receivers.

Protected source/destination faults retain original DF/terminal diagnostics,
restart EIP, unchanged or first-iteration register results and CMPS/SCAS FLAGS.
Read complete limit fixture and original diff: descriptor images, writable and
exact-limit variants, guest LGDT/LMSW/segment-load bootstrap, budget/executed=10,
cleanup and all public setup operations remain. Removal of the unused private
CPU include does not replace guest descriptor loading with cache mutation.
CMPS's separate two-segment preparation likewise retains its guest bootstrap
and executed=11 check. No new abstraction or test merge is introduced.

Record the five receiving and five deleted tests plus both fixture paths.
This source-only batch changes no executable input, so no identical complete
suite is rerun; timing qualification remains separate from this preserved
instruction/IRQ/rollback coverage proof.

Also read complete timing-checkpoint and CPU/PIC lifecycle receivers and both
baseline diffs, the lifecycle helper and the reset-alias helper. Timing's 16
one-instruction observations retain ticks=3, cumulative ticks=3*(index+1),
both reset runs and equal CGA-status sequences; its diff is include-only. The
alias helper maps the real earlier-profile reset fetch window rather than
mutating CPU state. This is retained exact relative-tick coverage, not a claim
of a physical host pacing contract.

CPU/PIC lifecycle retains both reset-state/guest-port executions, every IRQ
source except cascade IRQ2, and both cleared IRR checks. Its helper retains
the original program, budget, reset PC and AL predicates. However the old
private CPU pointer equality is removed. Source reading alone does not prove
equivalent identity coverage; mark its three paths as source-reviewed with
that specific remaining qualification, rather than claiming complete parity.
After this batch documentation governance and diff checks pass; all 610
frozen identities match. MyNES source, tests and artifacts remain unchanged.

## CPU Instance Identity Owner-Local Repair

The lifecycle comparison above identifies a real dropped private identity
assertion, not a production reset defect. Read the complete Core executor-run
receiver and its original baseline: NOP/HLT, register OR, two reset calls,
execution budgets, EIP and wait-result checks remain. Core reset itself resets
the existing CPU context; it does not allocate a replacement. The active
packet now explicitly assigns the omitted identity check to that Core-owned
test rather than granting Board access to private Core state.

Capture its non-null CPU context immediately after construction and assert
the same identity after each of its two existing resets. Core's own private
header is permitted in its own unit test; no CPU private header, identity API,
production code or Board lifetime path changes. The Board lifecycle test and
helper still exercise guest reset/ports and cleared PIC IRR through public
interfaces. Together these receivers preserve both parts of the original
check. Incrementally build only core-machine-executor-run-smoke and run its
single registered unit once per width: x64 passes in 1.11s, x86 in 0.11s.
Refresh the test manifest and frozen identities. These focused results do not
replace the final full-unit requirement or close S94.

## Core Prefetch And Model40 Refresh Receiver Pair

Read the complete 523-line Core locality receiver, the 511-line original and
every line difference, plus the complete Model40 D4 receiver and original
executor helper. Guest instruction bytes, budgets, memory-data checks and
relative cycle assertions remain: fetch/port adds 6/1 ticks, read/write adds
4 ticks each. Explicit-overlap, cancellation, page-table read/write, DMA HOLD,
release and reset invalidation checks retain their original predicates.
Retirement wait still checks READY inhibit/resume, zero early retirement,
reset READY and refusal to publish unqualified physical ticks. The three
memory/port wait-window cases retain commit/cancel and reset checks.

The Core spy still calls the real CPU reservation function, counts only its
owned context and checks grant sequence 1,1,1,2, then 1 after reset. Its
refresh attachment now supplies request/complete callbacks rather than
mutating a private Board latch. Separately Model40 retains its actual latch
and D4 attachment: one-tick guest FNINIT retirements cross the original
20-tick PIT edge, and copied transaction traces require exactly one refresh
request, acknowledge, commit and release. It also checks the original
prefetch inhibition/resumption and reset latch/address clearing. Core's
direct refresh-pulse test proves page/pending invalidation and the next
read's two-tick miss. This is a decomposed mechanism/wiring proof, not a claim
that a synthetic attachment alone tests the PIT. No CPU-private import or
Board-private state is needed in the Core receiver.

The former executor helper's construction-failure destroy/null behavior is
retained by the neutral Core helper; high reset RAM is explicitly supplied by
a synthetic alias rather than relying on a PC board. Neither helper is a new
runtime route. Both existing locality tests pass once per width (2/2, x64
0.13s and x86 0.15s). Four frozen paths receive reviewed dispositions; no
production/test edit or new full-suite claim is made in this audit batch.

## Debug Delivery And Observe-Only Memory Receiver Review

Read the complete Board debug-state receiver (455 lines), original (459)
and each line difference. Only private reset-provider construction becomes
public reset/register patch, includes change and equivalent public FLAGS names
replace CPU-private names. All six groups remain: real 186/286 UD and 386 DB
delivery with stack IP/CS, MOV-DR/PIC order and ISR, four protected NOP prefix
traps, nine legacy prefix rejects and four 386 LOCK rejects, TF trap before
pending PIC IRQ with ISR clear/IRR set, and the original protected DR0/DR7
breakpoint program. Guest descriptor setup, vector/handler bytes, budgets,
diagnostic masks, handler PCs and stack predicates are unchanged. It imports
no CPU private header and retains only Board-owned PIC private observation.

Read both complete memory-inspection versions. Apart from the include and
neutral-create/config replacements, their bodies match: mixed RAM/provider
reads, observe-only suppression of device and parity side effects, high reset
preview, provider failure and unavailable lexeme, A20 wrap, zero-byte reject,
and paged preview leaving both directory/table entries unchanged. Core owns
the intentionally corrupted memory fixture. Its CPU use remains public
preview/invalidation, not a private CPU layout import.

Also read the complete existing Core debug API smoke/fixture and App debug
adapter plus its complete mapping regression. Core covers pre-reset rejection,
bounded copied observations, step/continue, port errors leaving the caller's
value intact, register patch failure atomicity and all three watch kinds. App's
mapping test changes only an unused Core-private include; its paused-lease,
register matrix, real-memory, watchpoint, execution-plan/result and stale-lease
checks remain. The adapter has no migration diff: it still copies snapshots
and routes product requests through public Core debug APIs. Its historical
status-classification comment describes obsolete separate domains; this is an
inherited wording issue, not evidence of a new status ABI or a migration fix.
Keep that collaborator qualification explicit until the comment is reconciled.

The three existing Board/Core debug and inspection units pass once per width:
x64 3/3 in 0.15s, x86 3/3 in 0.14s. Five changed-path rows receive source/diff
dispositions. App mapping's unchanged body is not assigned new runtime proof
by those three tests. No production change or complete-S acceptance is claimed.

## Board Boundary Gate Baseline Review And Comment Cleanup

Read all eight deadline, DMA arbitration, peripheral, PIC/CPU locality,
PIT/PIC tail, Port B, readiness and refresh-request scripts completely and
compare their actual baseline diffs. DMA, PIT/PIC tail, readiness and deadline
change only the actual receiving source paths; they retain every original
forbidden/required predicate. Readiness still requires media before FPU before
RTC, and peripheral still requires readiness before peripheral advance. Video
now checks both the Board adapter call and its forwarding to the real chip.

PIC/CPU locality follows the moved Model40 refresh pulse while preserving the
ban on Profile directly invalidating Core locality. Refresh-request retains
Core hold/request/acknowledge/commit/release and Board callback checks; it now
requires D4 latches only in Profile, forbids them in common Board, and checks
both Profile binding and address increment/latch clear. Port B checks the
actual AT parity and Model40 construction functions separately, preserving
memory-before-port preparation, rollback and publication ordering, raw-route
exclusions and the one parity-release mechanism. These source-shape gates
cannot prove execution ordering; the reviewed mechanism/wiring tests supply
that separate runtime proof. All eight static scripts pass their current
invocations; six previously pending gate rows now have full-source/diff review.

Resolve the inherited status-domain wording identified in the previous batch.
The adapter already accepts lib_status on both sides. Change only its comment
to describe the existing policy: recognized classifications pass through,
internal/unrecognized failures become IO_ERROR. No status enum, switch case,
return value, function, API or runnable input changes. Add that newly changed
path to the frozen universe rather than hiding it outside the audit. Its
complete source and previously inspected caller/lease regression remain the
same; no product EXE rebuild is required for a comment-only edit.

## RTC, DMA/FDC And HDC Static Receiver Review

Read five additional complete scripts and their baseline diffs. RTC, DMA/FDC
and ATA feature gates only relocate the real Board adapter and test input
paths; no original forbidden/required predicate is removed.
RTC retains host-time exclusion, shared IRQ state, one atomic port route and
profile clock binding. DMA/FDC retains the ban on FDC raw RAM/DMA access and
requires Board channel binding through the shared DMA memory cycle. ATA keeps
the actual chip's sector/IRQ/control features, injected media path and copied
profile personality, excluding the retired duplicate CHS/LBA sector loaders.

HDC route ownership now recognizes its opaque pointer instead of an embedded
controller; the Core exclusion accepts either spelling so embedding cannot
evade it. Atomic port install/removal, wired-OR read and owner-scoped removal
checks remain. The portal scan has 16 real current Product C/header inputs,
not an empty obsolete directory; its moved common plan still must apply HDC
configuration. The initial run passes the first four scripts but fails the
portal gate: it expects App submission in machine_devices.c, although the real
production submission now lives in profiles/machine_plan.c. This is a stale
gate input, not a missing production HDC setup. Revise the active packet before
repairing only that read source and local variable; keep the same required
submission symbol and both retired-portal/Shared-application checks. Their
source-shape results do not claim media/command/IRQ timing completeness; the
controller runtime coverage rows still need their own receiver review.

After repair the real current portal invocation passes. A minimal ignored
build-tree probe retains a nonempty clean Product input and the Shared
application symbol, but omits only the App submission symbol; the same gate
rejects it with the intended missing-submission message and nonzero exit.
Delete the three temporary probe files afterwards. The four preceding real
gates and corrected portal gate now pass; this does not erase the initial
failure. The changed gate hash is refreshed in the frozen inventory.

## HDC Runtime Receiver Review

Read the complete three HDC receivers, their accepted-baseline sources and
full diffs, plus the old HDC support header and current owner-local fixture.
The general HDC receiver retains every WD1003/ATA assertion: diagnostic,
CHS/LBA data, step timing, invalid IDENTIFY, interrupt mask/unmask and status
acknowledgement, write without readback, media generation, range, readonly and
absent-media failures. Its change is includes and opaque HDC access, not new
instant-completion behavior. The fixture retains next-deadline servicing and
adds checked public bus access in place of unchecked private port calls.

The Compaq standalone receiver replaces embedded Core/HDC and raw port
registration with two actual neutral Core instances, independent PIC pairs,
opaque HDC instances and frozen public routes. Cleanup releases HDC before
PIC and Core. Original master/slave words, alternate-status IRQ retention,
primary-status IRQ acknowledgement, verify, reset, empty fitted drive and
3F7 wired-OR assertions remain. The assembled-machine receiver retains its
FDC/HDC media IDs, DMA channel 2 and shared 3F7 low-bit assertion; only the
construction imports and checked public port access change.

Incremental builds for these three targets report no work. Run each receiver
once per width: x64 passes 3/3 in 0.11s and x86 passes 3/3 in 0.08s. This
records eight original/receiving/fixture paths; it neither qualifies unreviewed
FDC receivers nor substitutes for complete S94 closure verification.

## FDC Topology And Media-Change Receiver Review

Read both complete current sources and their complete accepted-baseline
diffs. All topology assertions remain: frozen registry rejection, deferred
reset IRQ and four sense results, distinct drive bytes/read counts, wrong
selection, absent DS2 sense/recalibrate/seek/read results. All seventeen
media-change checkpoints remain: READY tied high independently of DIR,
generation and drive-specific change latches, real stepping versus no-motion
recalibration, motor-off and reset cancellation of DMA and pending IRQ.
Changes replace embedded FDC access and private Core port access with the
opaque Board FDC and checked public bus helper; no expected result is weakened.

Read the complete old fixture and its two receiving headers. Wire constants
move unchanged into fdc_values.h. Deadline advancement, overflow rejection,
bounded seek completion and chip/IRQ agreement helpers retain their bodies;
new port helpers terminate on a failed public bus operation rather than
accepting an unobserved read value. Incremental builds report no work; both
receivers pass once per width, 2/2 in 0.08s x64 and 0.06s x86. Seven paths are
qualified here. The larger general FDC receiver remains pending: a truncated
combined source read is not complete review evidence.

## Complete General FDC Receiver Review And DMA Fixture Finding

Read all 1,169 current lines in four untruncated chunks and compare the
complete 1,171-line baseline through all 256 added/removed diff lines.
Readiness retains ten command forms across six input conditions before and
during transfer; result identity retains eight unit/head values. Write TC
retains six length/action cases across normal/deleted and timed/untimed
variants. Terminal CHRN retains four commands across sixteen combinations.
No-implied-seek, physical Track0 versus PCN, 77-pulse recalibration, READY
loss and DOR mechanism selection remain. The main DMA/non-DMA/deleted-data,
scan, format, reset, empty drive, byte-deadline, media failure and unqualified
handshake assertions remain unchanged. Two RAM writes now use the public
physical-memory operation; inspection confirms it calls the same physical
write and invalidates prefetch on success while stopped/paused. No CPU runs
during this setup, and byte/transfer expectations are unchanged.

Read the complete old/current DMA fixture. Its transfer/register observation
loop retains its sixteen-clock bound and transfer/release termination. The
new public bus helper, however, ignored read/write status and initialized
failed reads to zero. Revise the packet and repair both operations to fail
the fixture immediately, matching FDC/HDC/video helpers, with no production
or public API change. Rebuild and run the general FDC receiver once per width:
x64 passes in 4.43s, x86 in 3.77s. Update the test manifest and verify it.

The same-fixture sweep finds one additional unchecked public bus read in
pic_fixture.h. This is a real remaining finding, not accepted coverage; its
repair and failure-path qualification remain next work. DMA fixture's original
synthetic-owner replacement and explicit failure probes also remain pending.
Complete suites must be refreshed after these test-input changes at S closure.

## PIC/DMA Fixture Failure Qualification

Revise the packet before the PIC fixture repair and add its complete-source
review to the inventory. Its public bus read now checks status and terminates
on failure; register captures, source routing and vector programming are
unchanged. Inspect the complete former DMA port-owner adapter: it allocated
a shallow synthetic Core and copied the port table in and out. The current
DMA fixture instead constructs an actual neutral Core, initializes DMA on
that owner, freezes providers and resets. The transfer loop's register-change
and release predicates and sixteen-clock bound remain; no copied Core layout
or second port table is retained.

An ignored temporary x64 probe compiles the actual DMA/PIC fixture headers
with injected public bus outcomes. The successful branch verifies unchanged
read and word values and exits 0. Separate DMA-read, DMA-write and PIC-read
failure branches each exit 1; reaching the caller after a failure would
instead exit 0 and fail the probe. This is helper failure injection, not
production bus proof. Remove the temporary source and executable afterwards.
Real PIC OCW3, PIT IRQ0 and DMA/RTC receivers pass once per width: 3/3 in
3.31s x64 and 0.21s x86. Test manifest verification passes.

This closes the two helper findings recorded above, not all unchecked calls
in the test corpus. Direct call-site status handling remains part of the
pending individual receiver audit; complete current suites remain mandatory
at S closure. MyNES, production APIs and owner assets are unchanged.

## Core PIC Transaction Receiver And Existing PIC Context

Inspect the current OCW3, command-priority, IRQ-lifecycle and lifecycle-S4
sources completely. Discover their actual accepted-baseline paths through
Git tree inspection: all four already reside in test/x86/ibmpc-common and
their baseline diffs are empty. They are existing context, not four new
migrations or four additional frozen review dispositions. Their poll,
special-mask/SFNM, programmable cascade, multiple level sources, PIT reset
and destroy, empty IRQ, allocation failure and route rollback assertions
remain. Initial lookup under the former App path was invalid; no such lookup
is used as baseline or evidence.

Read the complete former 212-line PIC-phase source and the current 265-line
Core-owned receiver, then its entire diff. Neutral Core creation plus public
PIC initialization/attachment replaces Board creation. Attachment owns PIC
pending, acknowledge, reset and finalize; failed construction releases PIC
before Core. Core-private transaction/CPU-bus observations remain same-owner.
Original DMA-conflict refusal keeps vector FF, no trace and no ISR; the
successful cascade acknowledgement keeps vector 2E, two transaction records,
master/slave ISR 04/40 and idle transaction. Real IRQ0 execution retains
acknowledgement-before-stack-frame ordering, HLT EIP 0101 and reset counters.
No assertion or Core transaction responsibility moves into PIC.

Incrementally rebuild and run PIC-phase and the three existing dependent
PIC receivers once per width: 4/4 pass in 3.50s x64 and 0.29s x86. Reuse the
unchanged OCW3 result from the preceding helper-repair batch, not a new run.
This qualifies two frozen paths. Existing context also exposes unchecked
construction/setup returns in the older standalone PIC fixtures; those are
not proved safe by successful runs and remain a failure-path review finding
for subsequent bounded repair. No production or new test API changes here.

## Port Ownership Receiver Review

Read the complete accepted and current `machine_port_ownership_board_smoke.c`
sources. Both have 266 logical lines; a line-by-line comparison finds exactly
four include replacements and no body differences. The receiving test keeps
duplicate and built-in port rejection, provider freeze/reset, actual CPU IN/OUT,
unchanged output on provider read failure, write failure CPU diagnostic, FDC
independent DOR read routing and write-conflict rollback assertions. Its own
setup checks status and destroys partial construction on failure.

Read the deleted `support/port_owner_fixture.h`: it allocated a shallow Core,
copied an executor port table and copied the table back on close. No current
source/test calls its open/close names. That absence alone does not qualify all
former consumers; its deleted inventory row remains pending until every
receiving fixture is reconciled. This port ownership test did not use that
helper and still exercises actual machine construction.

Run the existing port ownership test once per width without a rebuild: x64
passes in 0.05s total, x86 in 0.06s. These focused results qualify this unchanged
body, not complete S94 acceptance. The separately recorded PIC setup-status
finding remains unresolved; no source or public API is changed in this batch.

## PIC IRQ Lifecycle Setup Repair

The previously recorded ignored setup-status defect is repaired in the
existing IRQ lifecycle test. PIC construction, provider freeze/reset and
source bindings are checked before any dependent assertions. The existing
ICW helper now returns the combined write result and drops two unused PIC
parameters. All thirteen subsequent bus writes contribute to the failure
result. Failed construction or binding reaches the single cleanup label;
there is no fixture framework, public API or production change.

Read the actual PIC constructor/finalizer: failed construction leaves output
handles null and destroys unpublished endpoints internally; finalize tolerates
null endpoints and frees all published source leases before Core destruction.
Thus the new test cleanup uses the existing resource owner, not a second
partial-resource cleanup implementation. Review the complete repair diff
against accepted S92: all original edge/level, mask, cascade, ISR and vector
assertions remain. Git numstat reports 38 added/36 removed test lines, net two.

Incrementally compile this test and execute it once per width: x64 passes
in 0.56s total; x86 passes in 0.12s. These runs exercise the real success paths;
they are not injected construction-failure proof or a full unit result. The
other three standalone PIC setup findings and failure-injection qualification
remain pending within S94. This test-only change requires no product EXE
rebuild; MyNES and all production sources remain unchanged in this batch.

## Remaining PIC Setup-Status Repair And Failure Proof

Read the complete OCW3, command-priority and lifecycle-S4 sources and their
repair diffs. Each initializer zeroes its owned handles, checks neutral Core,
PIC, optional PIT port installation, freeze and reset before dependent use.
Failure releases the existing PIC/Core owners and exits with failure. Existing
raise/bind helpers check binding before dereferencing the source; lifecycle
uses one private checked bind helper for successful setup only. A private
`test_pic_port_write` counterpart to the existing checked read helper handles
successful register writes. No production or public interface is changed.

All original poll, auto-EOI, special-mask, SFNM, programmed cascade, priority,
two-source level counts, PIT reset/destroy callback and empty-vector assertions
remain. The intentional IRQ2/IRQ16 invalid bindings now also verify their error
codes. Allocation-failure, idempotent-bind and port-conflict rollback keep
direct raw status assertions; the success-only helpers do not intercept them.
The EOI helper's slave selector uses the existing `lib_bool` vocabulary.

Git numstat for these three tests reports respectively 36/28, 39/26 and 49/31
added/removed lines. Including the preceding IRQ lifecycle and fixture repairs,
the five PIC test paths add 172/remove 123, net 49. This increase makes setup
failure visible and removes undefined partial-fixture continuation, not a new
test framework or shared production path.

Incrementally build the three targets, then execute each once per width:
x64 3/3 passes in 3.14s; x86 3/3 passes in 0.29s. A temporary ignored native
probe includes the actual lifecycle-S4 source (renaming its original main),
links its existing target libraries and uses its existing allocator seam.
Original full lifecycle and a successful bus write exit 0; initialization
allocation failure, subsequent IRQ binding allocation failure and an unmapped
`ffffh` bus write exit 1. Unexpected continuation would exit 7. Thus failure
termination is exercised against the actual helper, not a copied model.
Remove the owned probe source and EXE afterwards and confirm both absent.

This closes the four specifically identified PIC fixture setup findings;
it does not qualify all unreviewed tests or full S94 exits. Test manifest and
inventory hashes are updated. Test-only edits do not require new product EXEs;
all production source, owner INIs, MyNES source/test/artifacts stay unchanged
in this batch. Current full-unit and complete corpus review remain pending.

## DMA Binding And RTC Receiver Review

Read the complete accepted and current sources for DMA binding-token,
RTC-CMOS-S3, RTC-storage-S4 and DMA/RTC-authority tests; compare every differing
line with an LCS diff. Binding-token changes only includes: both unique nonzero
instance tokens, rejected cross-instance FDC bindings, valid same-instance
bindings and both freeze/reset assertions remain. RTC-storage likewise changes
only includes: one CPU retirement advances RTC to guest tick 3, no FDC/HDC
advance trace is allowed, run-boundary/CPU-retire events and zeroed reset
timeline assertions remain.

RTC-CMOS-S3 replaces an embedded shallow Core/port table with a real neutral
Core. All PF/AF/UF/IRQF enable/mask/clear, IRQ8 vector70, RTC destroy-deassert,
equipment-default, NMI-mask and invalid/valid rational timing assertions remain.
The fixture already propagates creation statuses and shares its finalizer for
failure; repair the remaining unchecked PIC ICW writes to return the actual
first bus error through that path. A local eight-row port/value array retains
the original register order and values without eight duplicate error branches.
Both EOI writes now contribute failure to the existing assertion result.

DMA/RTC-authority preserves invalid topology/channel arguments, duplicate
bindings, FDC/refresh tokens, equipment defaults, NMI masks, five-second RTC
advance, IRQ8 and reset retention. The original manual time advance of 3 then
1 tick is now real CPU retirement of three then one `DB E3` instruction, with
explicit execution/tick counts. The same PIT1 output and DMA request-bit
observations are required after each boundary. No device deadline is skipped
or replaced by a successful boot checkpoint. Repair its ignored PIC/CMOS
setup accesses through the already checked test bus helpers, removing the
zero-initialized read fallback.

Build the affected receivers incrementally and run each of these four tests
once per width: x64 4/4 passes in 3.78s, x86 4/4 in 0.25s. Update eight old/new
inventory dispositions and changed test/manifest hashes. The large DMA-channel
test and remaining RTC receivers still require full source qualification;
the earlier truncated combined output is not accepted as a complete read.
Production, MyNES and owner INIs are unchanged. These test repairs neither
require a new product EXE nor close S94.

## Verification And Remaining Work

### FLAGS And Arithmetic Board Receiver Review

Read all six direct-FLAGS, LAHF/SAHF, PUSHF/POPF and three INC/DEC group
receivers. Compare the entire accepted App bodies in order, not only counts:
after removing include lines and substituting the equal-valued public EFLAGS
names plus original IOPL=3000h and RESERVED=FFFC802Ah, all six bodies are exactly
equal. The second RESERVED define in cpu.h is commented-out, not the original
contract. No instruction bytes, expected flags, descriptor comparisons,
IRQ frame/IRR/ISR, protected faults, divide/AAM delivery or XLAT rollback changed.

Read their five complete fixture bodies against the original App fixtures:
protected flags and fault setup are unchanged; limit preparation retains the
real GDT and ten-instruction protected bootstrap; DE preserves register/FLAGS
rollback and 16/32-bit delivered frames. IRQ retains real PIC vector 20h,
handler/stack bytes, CPU snapshots, IRR/ISR and frame observations. Its ignored
source-bind status now routes failure through existing machine cleanup before
assertion. PIC fixture programming/capture now checks status using its existing
test-failure convention rather than silently returning zero state. No production
API or behavior changes.

Rebuild and run all six once per width: x64 6/6 in 3.39s; x86 6/6 in 0.29s.
A temporary native probe calls the actual PIC helper with no bus: invalid
vector programming and register capture both exit 1 as required. Remove probe
source/EXE after exit. Header changes invalidate earlier full-suite evidence;
complete S94 units/gates remain required after all review repairs.

The similar-issue scan across test/x86/{ibmpc-common,core,ibmpc-at}, matching
standalone core_machine_pic_irq_source_bind calls through their statement
semicolon, finds fifty further unchecked setup sites. They are not classified
as accepted by the preceding six-test result; their containing inventory rows
retain an explicit unresolved setup-status suffix for the next repair batch.
Conditional/assigned calls and intentional error-status assertions are not
this syntactic class. Actual caller/source review must establish their final
dispositions; no production or MyNES path is mutated by this test-only scan.

### App FDC Authority And ATA Port Receiver Review

Read both complete sources and all migration diff hunks against S92. FDC's
eight former private binding predicates are unchanged inside the Board-owned
fixture: first/second media IDs, DMA token/channel, IRQ lease, both PIC handles
and Core identity. ATA preserves selected profile/ports/IRQ14/200-tick service,
media geometry, IDENTIFY/CHS/LBA byte data, two-sector count/address progression,
IRQ acknowledgement versus alternate status, zero-count 256 semantics,
software reset, invalid LBA, absent slave/unknown-command ABORT, NIEN masking
and reenabling, and absent-media failure. Board fixture operations call the
same HDC service/observation/IRQ mechanisms; no immediate-completion fallback
or replacement media path is introduced. Original assertions and byte values
remain; this review does not qualify other controller-fixture consumers.

Rebuild and run both once per width: x64 2/2 in 3.54s, x86 2/2 in 0.16s.
No source repair or new production API is required for these two receivers.

### Board Composition And Diagnostic Fixture Review

Read the complete Core/Board composition and boot fixture pairs, AT boot,
command and state fixtures, XT boot pair, Board controller pair and Board KBC
state pair. These are separately compiled test operations at each real owner:
Core reads physical routes/transaction/CPU/ROM values, Board reads its topology
and bindings, AT reads keyboard/BAT/repeat state, XT reads PPI/byte readiness.
Their callers receive copied values or perform bounded test-only operations;
no production diagnostic interface or mirror object is added. This source
review does not accept every yet-unreviewed caller's preserved coverage.

The complete 5170 composition source/diff retains font bytes, disabled Core
transaction timing, speaker, CPU, actual AUX disable, A20, 512KB/open-bus RAM,
parity, option ROM, HDC ports/service periods, 120-tick PIC unmask, native and
compatible floppy geometries, rejected 720K, real CPU refresh calibration,
DMA word-port conversion, external ROM bytes and default HDC presence.
Its previous two identical AUX queries become one identical observation, not
an omitted distinct checkpoint. All seven Core timing-disable predicates are
retained inside the Core-owned fixture.

Repair the moved PIC-unmask helper's five unchecked register writes and source
binding: return failure before source assertion/deadline comparison on failed
setup. Preserve the exact 120-tick expectation, signatures and error bit 12h.
This is a test-only +8/-8 replacement, with no product executable-input change.
Rebuild and run the 5170 composition receiver once each: x64 1/1 in 0.37s,
x86 1/1 in 0.16s. A temporary native probe links the actual compiled Board
fixture and passes a Board with no PIC binding: the helper returns failure
without attempting source delivery. It exits zero; remove source and EXE after
exit. Refresh the Shared test manifest. Remaining fixture consumers and complete
S94 closure proof remain open.

### Controller, Attachment And Input/Display Receiver Review

Read the complete controller authority, auxiliary PIT and input/display
receivers against their complete accepted App sources. Controller construction
failure still destroys the unpublished Core; FDC-before-DMA and duplicate
controller rejection, DMA token/owner, IRQ6/14 routing, media identity, absent
NDMA read/write results, ATA BSY-to-ABORT and reset states remain. The removed
attachment block is retained in Core's attachment-phase receiver, with Board
binding identity separately checked by the existing binding fixture/receiver.
The phase receiver preserves binding argument/copy/freeze rejection, device
reset at tick 17 before clock reset to zero, NMI refresh and finalization while
CPU remains alive. No original assertion is silently retired.

Auxiliary PIT retains independent 1:4 clock conversion, exact CR-to-CE/latch
values and reset cancellation for both PITs. Actual checked bus operations
replace raw route-table observations. Its arbitrary-time operation resides in
the Core-owned test fixture, not an added production clock API. Input/display
retains both keyboard topologies across five lifecycle states, null arguments,
firmware-operation mutation guards, readonly observations, exact 3-tick NOP,
CPU/KBC/video/boundary trace order and timeline reset. Core-owned fixture setup
replaces cross-owner writes without removing these ten guard contexts.

The first build invocation used a nonexistent input/display target and failed;
the three subsequent existing-test passes are not build acceptance. Correct
the target, require build success before CTest, and run all four once per
width: x64 4/4 in 1.22s; x86 4/4 in 0.16s. This is batch evidence only.
Also read the four-row Board smoke (8042/XT times 8253/8254): it checks public
plan construction, freeze/reset, native scan set and PIT route installation;
exact counting belongs to the dedicated PIT tests, not that construction smoke.
Remaining fixture consumers and complete closure validation stay open.

### AT KBC Serial And AUX Port Receiver Review

Read both complete receivers, their complete original App sources and the
KBC/IRQ fixture pair. Serial cadence retains the two-byte 2-tick delivery,
inter-byte OBF absence, pending-byte reset cancellation, no stale delivery
after ten ticks and BAT scan translation 05h to 3Bh. Its old raw port fixture
is replaced by a real opaque Core, frozen/reset before first guest bus access.

AUX retains source-tagged OBF, IRQ12 vector 74h/EOI, controller/keyboard
separation, F2/F4/F5/F6/FF, A7/A8, F3/E8 valid/invalid parameters, E9 ordered
status and report bytes, IRQ masking, 2-tick response, overflow packet,
unknown-command resend, packet-atomic full FIFO, independent 64-byte keyboard
queue and teardown deassertion. The same IRQ source leases now belong to a
test wiring record rather than private KBC fields. Construction/binding
failure stops before protocol use; three early failures now share final
KBC-before-PIC cleanup rather than leaking both owners. All protocol assertions
and diagnostics remain. No production/interface or product artifact changes.

Run both once per width: x64 2/2 in 0.65s; x86 2/2 in 0.11s. The checked KBC
fixture has no synthetic Core copy or cross-owner raw port table; the IRQ
fixture binds keyboard 1/AUX 12 and returns first setup error. This does not
accept unreviewed consumers of other old KBC fixture variants.

### Complete DMA Channel Receiver And Checked Writes

Read all 1,247 lines of the DMA channel receiver and its complete accepted
baseline, plus the whole migration diff and DMA fixture. The migration adds
the existing two-controller argument to 34 transfer-helper calls only. The
126 first-service rows retain seven channels, normal/TM, demand/single/block
and verify/write/read; each phase still checks callbacks, registers, TC and
RAM before and after S4. Subsequent cases retain inclusive count, masking and
deassertion, auto-init/decrement, held DREQ, M2M phases/directions, all sixteen
page latches, sparse secondary ports, byte/word wrap, software request/cascade,
priority/rotation/starvation, controller-disable, EOP, failed physical-route
preflight, M2M auto-init/termination and reset-binding retention.

Repair 112 unchecked successful bus writes using one checked test-only helper.
The three raw rollback writes still assert their expected status directly.
No guest assertion, production behavior or API changes. The helper adds seven
lines; replacements are line-count neutral. The complete accepted-baseline
diff for these two files is +161/-152, net +9, including preceding fixture
failure and explicit-controller-count work, not only this repair.

The DMA receiver passes once each: x64 1/1 in 1.36s, x86 1/1 in 0.11s.
A temporary native probe calls the actual helper against an initialized Core:
valid 0Ch write exits 0, unsupported FFFFh write exits 1. Both probe files are
removed. This proves rejection, not just source shape. MyNES and product
inputs are unchanged; complete closure verification remains pending.

### General RTC And App CMOS/DMA Receiver Review

Read the complete general RTC receiver and its original App baseline, both
App CMOS/Model40-DMA diffs, and the complete CMOS fixture pair. The RTC
receiver retains UIE after 50,000 ticks, vector 70h, IRQF/UF clearing and
equipment-byte reset retention. Its unchecked eight PIC writes and IRQ8
binding now fail through existing cleanup before RTC construction. No new
production API or hardware assertion is introduced.

The App CMOS test retains all fifteen checkpoints: external-seed configuration,
checksum, VRT, update/periodic/alarm IRQs, SET freeze, data mode, NMI mask and
reset retention. Its removed private RTC/PIC operations reside in the existing
owner-local test CMOS fixture, not a second production path. Model40 DMA keeps
the two-controller topology, FDC channel 2, cascade channel 4, configured wait,
gate/ready signals, duplicate-binding rejection, D4 programming and reset
request clearing; copied composition observations replace raw Board fields.

Rebuild and run these three receivers once on each width: x64 3/3 in 3.50s,
x86 3/3 in 0.24s. The DMA/RTC authority gate diff changes only five receiving
paths to Shared ibmpc-common; its substantive checks are retained. These are
batch proofs, not complete S94 unit-suite acceptance or a partial P delivery.

Earlier root evidence is 491/491 per width with both specialized gate targets
passing; independent tools-on x64 finished 291/291 in 119.40s. The subsequent
Board lifetime repair changes executable inputs, so those results are not
current full-suite proof and cannot be reused for final acceptance.

After the lifetime repair, execute build/s93-verification.ps1 -Width x86:
the process finishes with exit 0; full units pass 491/491 in 77.48s and the
specialized target passes all 123 build/static steps, including the 509-row
strict matrix (488 retained, 21 deferred). Its T345 negative emits an expected
rejection and then the explicit verifier-self-test success; it is not an
unhandled gate failure. The corresponding x64 process also finishes with
exit 0: full units pass 491/491 in 254.64s and all 123 specialized build/static
steps pass, including the same strict matrix. These were one complete run
per width, not repeated boot trials. No MyNES product build is invoked.

The path inventory check initially detects only two stale hashes: the source
and test x86 manifests changed in the preceding lifetime-repair batch. Refresh
those inventory identities and repeat the full extant-file hash comparison:
zero mismatches. Source corpus, test manifest, documentation governance and
changed-document whitespace checks pass. No implementation P or S acceptance
is made while the remaining actual-source/coverage rows remain open.

Continue actual source/diff review of Board advance/deadline, controller/family
mechanisms, D4 lifetime, App adaptation, all test receivers/deletions, source
lists and gates. Reconcile the frozen 99-row intake and CPU/indirect fixtures.
S94 remains open; no P, component or T acceptance is claimed.

### Continued Audit After Performance Inspection

The current copied-frame failure probe compiles with strict C11 warnings and
passes: INVALID_STATE and IO_ERROR must reject prompt/text success even when
the failed capture supplies prompt-shaped characters; OK retains success.
It includes the actual four predicates from the three DOS tests, replacing only
capture. This is a failure-path proof, not an external-media boot result.
The three actual integration targets compile/link incrementally on x64 in
3.74s and x86 in 5.77s (six steps per width). No MyNES target is built.
The source audit continues; full S acceptance is not claimed by these checks.

### Additional Migrated Integration And Gate Review

Complete current source and baseline diff inspection covers nine additional
gates: default-profile closure, T332 lifecycle, T344 historical shapes, 286
Appendix-B/LSL, Jcc lexeme, physical eligibility/inventory and collaborator
plan boundary. Positive checks pass. T332 retains 44 inventory owners; T344
qualifies its finite 133 classified historical owners, not every constructor
in the repository. Original predicates remain; receiver paths and opaque
fixtures account for the migration.

Complete source/diff inspection also covers default-profile, CGA, FDISK,
keyboard, video-port, EGA and mouse DOS receivers, plus FDC READ TRACK,
ATA PIO, HDD firmware handoff and INI CMOS-seed receivers. Apart from the
admitted capture-status repair, changes relocate imports or substitute
owner-local observation/I/O fixtures. ATA still waits for BSY/DRQ at each
sector boundary; FDC retains the one-instruction/128-quantum DMA and result
comparison; HDD retains partition-VBR identity and controller-command checks;
CMOS retains every selected byte expectation. Waiting still advances through
the existing machine deadline path. Overlay transformations use the existing
media registry, not another image file or firmware path.

The four additional FDC/ATA/HDD/CMOS targets compile/link on x64 in 2.01s and
x86 in 2.12s, seven steps per width. This proves build compatibility only;
external-media runtime qualification remains S96. Reconciled inventory has
620 paths, zero stale hashes and 238 pending full-source dispositions.

### Windows Receiver Diagnostics Review

Read the complete current checkpoint, setup and HDD-admission sources and
their S92 diffs. Observation substitutions preserve their guest command,
MBR/VBR, geometry and setup-success predicates. The checkpoint's DIR window
is the previously diagnosed containment correction recorded in Current:
6.42s exceeded the old 5s window; the separate 60s window does not change its
success predicate or outer CTest limit. It is not classified as path-only.

Actual HDD-admission source reveals an inherited diagnostic use-after-free:
entry points into hdd_overlay, released before the error report reads entry[4].
After packet revision, move the single release after both final report branches.
Initialize its run result because failures before the first Core call reach the
same diagnostic. The HDD firmware-handoff and BYOB boot probes also report a
run result when Core rejects before writing it; initialize these two results.
The Core implementation confirms its ownership/argument rejection precedes
result initialization. No success criteria, guest bytes, deadlines, runtime
API or external inputs change. The BYOB whole-source review remains pending.

The HDD/Windows result-reader sweep excludes ATA, FDC and INT13 trace's
short-circuit-only result readers: they do not consume a rejected run's result
in diagnostics. Overlay-transform readers in CGA/EGA/FDC/ATA/mouse finish
all entry/data use before release. This is the inspected diagnostic class,
not blanket qualification of the larger BYOB/Model40 sources.

The three Windows targets, HDD handoff and BYOB probe compile/link on x64
in 4.02s and x86 in 3.32s, 15 steps per width. These results qualify compilation,
not a fresh Windows boot or runtime failure injection. Inventory retains
620 paths, with the three Windows rows reviewed and 235 still pending.

### Core Lifecycle And CPU/PIC Receiver Review

Read the complete current reset-identity, neutral configuration and INT/IVT
receivers, both protected far/data PIC receivers and their whole S92 sources.
Also read the protected PIC fixture's complete S92/current implementations.
The far/data bodies remain identical: selector/IP, saved frame, EAX/ESP and
PIC ISR/IRR observations survive. The fixture replaces its embedded raw Core
port owner with the existing owner-local constructor/destructor; IRQ-bearing
CPU bus callbacks, protected layouts, gate and handler bytes remain unchanged.
Reset identity retains the same actual CPU allocation across both resets and
the complementary public PIC bus program. Configuration retains freeze,
post-freeze rejection, reset and memory-write checks through a neutral Core
constructor with explicit reset alias instead of implicit board mapping.

Review discovers two inherited false-success/unchecked-setup defects. Admit
them before editing: INT/IVT construction failure now sets failure before
cleanup, and protected PIC raise uses existing test_pic_bind_source before
asserting/deasserting. The latter's exactly two callers keep identical CPU and
PIC assertions. Actual included-source failure probes reject creation failure
for 8086 and 80386 and terminate on failed PIC binding before assertion.
All five real receivers build and pass once per width: x64 5/5 in 3.21s,
x86 5/5 in 0.57s. Refresh the two affected test manifest entries.

Read the whole exception fixture and AT controller declarations, Model40
refresh fixture declarations/definitions, synthetic CMOS/ROM and copied-frame
test helpers, Model40 SKEY/compatibility receivers, Shared test README and the
two NXVM boundary-tool files. These retain owner-local private assertions;
synthetic unit asset bytes remain in code and presentation observes the sole
Common frame. Model40 preserves A20/reset/high-ROM and 19-tick PIT observation
through the existing Core/Board fixtures. Its two receivers pass once per
width (x64 2/2 in 2.66s; x86 2/2 in 0.18s). Boundary-tool positive checks pass;
the reduced DAG allowlist removes retired App-device edges, not new exceptions.

Map the five deleted Core/PIC sources/helpers to these reviewed receivers,
without treating deleted-file absence as coverage. This batch qualifies 22
additional inventory paths; independent corpus, full current units/gates and
product qualification remain open. No production, MyNES or artifact change.
Current inventory is 620 paths, 213 pending and zero stale hashes. Test x86
manifest, source x86 corpus boundary, NXVM documentation governance and full
worktree diff checks pass. MyNES source/test/document/artifact status is empty.

### Build And Verification Cost Recheck

The owner requests checking S latency before continuing. Recheck the existing
batched T344 implementation rather than introduce another cache or verification
path. Its current x64 invocation validates all 515 rows (494 strict, 21 deferred)
in 5.09s, versus the previously recorded 36.14s pre-batch invocation. The current
eight-receiver x64 incremental build reports no work in 0.40s. The corresponding
x86 build updates two compile/link steps in 1.42s. These are single measured
checks, not cold-build or sustained-performance benchmarks.

CTest cost history identifies the CPU boundary negative as a long file-only
test; its current registration already lacks RUN_SERIAL and retains the full
negative corpus. Native desktop resource serialization remains unchanged.
History also contains firmware and decoder runners costing roughly 10-19s;
historical averages do not establish current individual timing. No test budget,
success predicate, compiler option or required S-closure suite is reduced.

### Renewed S-Latency Experiment

The owner again requests checking and optimizing build/test cost before
continuing S94. The current x64 four-stack-target incremental build reports
no work in 0.45s. Existing Ninja object reuse is working; this is not a cold
build measurement. Historical CTest costs identify firmware and decoder
runners, but cannot establish current individual performance.

A full x64 unit experiment with `-j16` instead of the existing `-j8` is
unsuccessful: firmware-floppy and segment-selector tests time out despite
emitting success markers. Several ordinary cases also take substantially
longer than their recorded earlier runs. The experiment is stopped by its
verified CTest PID and process tree; its partial results are not a whole-unit
pass. This observation does not prove a specific compiler, scheduler or host
root cause. No timeout, predicate, test count or desktop serialization is
changed. Retain eight-way scheduling rather than assume more concurrency is
faster on this host.

After stopping the experiment, the unchanged four stack receivers pass once
per width at `-j8`: x64 4/4 in 0.10s, x86 4/4 in 0.24s. The x86 incremental
build also reports no work. Continue source review of these actual receivers;
these runs establish their current normal baseline, not the pending
failed-output repair proof or source qualification. Source-only audit batches
reuse unchanged passing checks, while changed tests receive their temporary
focused build/run and the complete required suites remain at S closure.
The frozen source audit still has 214 pending paths; its reading and coverage
reconciliation is a separate cost, not evidence of slow compilation. MyNES,
owner INIs and deployed binaries are not touched by this experiment.

Continue the current instruction-limit and VM86 coverage audit: six instruction
receivers plus board VM86 delivery/LGDT-LIDT and the CPU-private VM86 rollback
receiver pass once on both widths. The x64 eight board receivers take 5.81s;
its CPU receiver takes 0.14s including CTest overhead. All nine x86 receivers
take 0.58s together. The CPU source retains breakpoint/RF, cache invalidation
and invalid-gate/TSS/stack prepublication checks; these are not silently removed
from the board receiver. Complete receiver-map qualification remains open.

The practical cost policy remains incremental selected-target builds during
repair, temporary change-specific checks, complete units/gates at S closure,
and one run per admitted boot group. Actual source/diff/coverage review accounts
for substantial remaining S94 work; compilation is not forcing repeated full
rebuilds. No new production, Shared, MyNES, INI or executable change is made by
this recheck. S94 and T540 remain open.

### Instruction-Limit And VM86 Receiver Reconciliation

Read the six complete instruction-limit receivers and S92 originals. An exact
text comparison after only replacing the fixture include path and the FLAGS
prefix succeeds for bit-scan, bit-test, double-shift, IMUL2, rotate and SETcc.
Their instruction bytes, initial values, operation/pass loops, fault reason,
register/PC/memory nonpublication and flag assertions are unchanged. The
previously qualified descriptor-limit fixture still supplies the same genuine
protected guest bootstrap; these consumer bodies now receive their own review.

Read both complete VM86 board receivers, original delivery/LGDT-LIDT sources
and the complete CPU-local VM86 state receiver. Board retains UD/GP/NM, TF,
actual PIC IRQ0 entry and IRET round trip, and mapped/unmapped-source paging
frames. Public GDT/IDT setup, LGDT/LIDT/LMSW, LTR and IRET replace the historical
private CPU seed. The available TSS descriptor becomes busy through LTR; copied
observations verify TR selector/type/base/limit and VM86 entry before the cases.
The delivery helper executes the handler after an acknowledged delivery round;
it does not bypass exception execution or fabricate a frame.

The CPU-local receiver preserves full cache invalidation, execution-breakpoint
DR6/RF, and invalid gate, access, TR/TSS, SS0 and short-stack rollback. Its
prepublication comparisons cover the complete CPU and 40 stack bytes. Synthetic
CPU IRQ coverage complements rather than replaces Board PIC ISR/IRR checks.
LGDT/LIDT retain both forbidden-in-VM86 instruction cases, all GPR and GDTR/IDTR
observations, unchanged six operand bytes and the complete ten-word fault frame.
The deliberate board/CPU split is not classified as a path-only migration.

The LGDT/LIDT test has one inherited failed-output read: a failed run can skip
diagnostic capture, but its separate assertion previously read that output.
Revise S94 before repair, then guard the post-run predicate behind successful
setup/run/capture. This one-line change adds no mirrored state, API, guest bytes
or alternate test path. Read the remaining VM86 delivery and IRET readers:
their diagnostic assertions are in the same checked short-circuit expression,
and register snapshots are checked by their capture helper. Whole IRET migration
qualification is separate and remains pending.

Normal LGDT/LIDT reruns pass on x64 (1.27s CTest wall) and x86 (0.10s).
An ignored build-tree probe copies the actual case body byte-for-byte, omitting
only its main entry point to avoid its nested main macro. Inject IO_ERROR without
writing the run result, execute both real guest setups and verify both reject
the failed run (two injected calls, exit zero). Its strict C11 build passes.
The first probe attempt fails to compile because of that nested main macro;
it is not counted as verification. The corrected probe passes.

Together with the previous nine dual-width receiver runs, this qualifies 17
additional source/deletion paths. Update the changed test hash in its manifest
and review inventory. The 620-path inventory now has 223 pending-source rows; full
current units/gates, independent corpus and product acceptance remain open.
There is no production, MyNES, INI, firmware, media or executable-input change.

Correct the previous pending-count method: rows annotated `pending;...` are
still unqualified source reviews, not completed rows. Before this batch there
were 240 such or plain pending rows; 17 are qualified here, leaving 223 (197
plain pending and 26 annotated pending). Earlier reports of 213 and the initial
196 estimate excluded annotated rows and must not imply their acceptance. The
frozen universe remains 620 paths. Refreshing the verified manifest's inventory
hash leaves zero stale rows; its final corpus revision remains S95 work.

### IRET Core And CPU Context Review

Read complete original VM86/protected/S51 IRET sources and their Core/Board and
CPU-local receivers, including the already-qualified CLI/STI includer boundary.
VM86 retains CF/67-CF success, high-half selector truncation, all six segment
cache attributes, full CPU short-stack rollback and paging composition. Core
now enters protected mode through guest LGDT/LMSW/segment loads/far JMP; the CPU
receiver retains the original private cache-valid/kind and full-state facts.
Its synthetic memory bus is not claimed as paging translation proof: the public
Core receiver retains the actual paged execution and mapped stack/code path.

Protected IRET retains the five same-level operand/address/small-stack/conforming
success cases, CPL3 FLAGS restrictions and five descriptor/stack rejection
cases. The public setup validates guest-created CS/SS, PC and SP, uses real
IRET to enter CPL3, and mutates only the intended descriptor after bootstrap
for rejection. CPU ownership retains the complete original cache equality and
descriptor-access-byte nonpublication checks. Register values, stack frames,
fault masks/codes and FLAGS expectations are paired, not weakened by copying.

S51 keeps twelve real-profile/wrap/prefix cases, ten protected cases and both
PIC IF-restoration cases. Its CPU receiver retains the 286 stack-boundary case,
twelve legacy attribute rejections and four LOCK forms with original full CPU
and sixteen stack-byte nonpublication checks. Board still tests actual PIC
IRR/ISR, vector/frame IP, HLT and IRQ disposition, not just a synthetic CPU IRQ.

The failed-output sweep finds the inherited protected user-FLAGS separate
diagnostic predicate and S51 real/PIC result predicates. Revise the packet before
guarding these five predicate sites with existing success state. No success
assertion, guest byte or fault expectation is removed. Normal six-receiver runs
pass x64 in 3.10s and x86 in 0.37s. After changing the S51 sites, rerun only that
changed receiver: x64 1.60s and x86 0.12s, both passing.

Ignored strict-C11 probes execute the actual protected user case and an exact
copy of the S51 case bodies, leaving their real guest setup intact. IO_ERROR
without writing the run result is rejected by the user-FLAGS, S51 real and S51
PIC paths. The first S51 probe build reports unused copied entry helpers;
referencing those helper identities fixes the probe without suppressing warnings.
Only the corrected build/run counts as evidence.

This batch qualifies nine additional paths, including three paired deletions;
the source-review pending count falls from 223 to 214 of the same 620 paths.
The four adjacent PUSH/PUSHA/ENTER/GPR-stack sources remain pending: partial
reading exposed separate failed-output readers, but complete source/baseline
review and guard repairs are not yet finished. They are not hidden by this
passing IRET batch. Test manifest and inventory hashes are refreshed. No product
source, executable input, MyNES, owner INI, firmware or media changed. Full S94
closure units/gates and S95-S97 acceptance remain required.

### Stack Receiver Failure-Output Review

Complete the four adjacent PUSH-immediate, PUSHA/POPA, ENTER/LEAVE and GPR
PUSH/POP source bodies and their complete S92 originals. The normalized full
bodies match exactly after removing include-path changes, checked PIC binding,
the physical-memory read-to-public-inspect substitution and this batch's
failed-output guards. This mechanical comparison supplements the body review;
it does not independently prove architectural semantics.

All twenty original execution contexts remain: four immediate PUSH, four
PUSHA/POPA, four ENTER/LEAVE and eight GPR PUSH/POP contexts. Protected cases
retain original invalid-stack/data limits, sentinel words, full register and
segment rollback. ENTER retains its two ancestor writes, old-BP write and
unwritten third word. IRQ cases retain exact return IP, SP, low-word register
effects, push order, destination memory and PIC ISR/IRR predicates. Bootstrap
programs and paused descriptor preparation are unchanged.

Add ten guards on separate diagnostic/frame predicates, plus guards on seven
following IRQ variant branches. Failed execution or snapshot capture no longer
permits reading output that its producer did not initialize. Successful-path
predicates are unchanged; no substitute zero snapshot, production helper, API
or secondary execution route is introduced.

Rebuild only these four targets per width: eight compile/link steps each.
Normal current tests pass 4/4 on x64 in 4.03s and 4/4 on x86 in 0.27s. Strict
C11 ignored probes include the actual source functions and inject either
IO_ERROR without writing the run result or a failed post-run snapshot without
writing its output. All 26 actual-case failure checks reject correctly: four
immediate-PUSH, six PUSHA/POPA, six ENTER/LEAVE and ten GPR checks across the two
failure modes. ENTER and LEAVE are selected separately; the GPR protected
forms are invoked individually. These probes are temporary diagnostic inputs,
not new registered tests or replacement fixtures.

Qualify four receiver paths and their four paired original deletions. The same
620-path inventory now has 206 pending rows and zero stale hashes. Refresh the
test x86 manifest and its inventory identity; current manifest, source x86
corpus and whole-worktree diff checks pass. No production or executable input,
MyNES, owner INI, firmware or media changed. This is another completed review
batch within S94, not S94 closure or T540 acceptance.

### Neutral Memory, Reset And Time Receivers

Read complete originals and receivers for immutable ROM, memory device-route
registration, RAM/port context isolation, 386 real-mode 32-bit address execution,
real-mode instruction ticks and explicit time. Complete normalized bodies match
after the named include/constructor substitutions, RTC private read-to-public
bus read and diagnostic initialization; no other semantic hunk is present.

ROM overlap rejection, frozen registration rejection, ignored guest writes,
reset preservation and executable reset alias remain. The device route keeps
its three provider callbacks and all overlay-enabled/fallback/late-registration
checks. RAM/port isolation retains installed bounds, A20 alias versus enabled
access, rejected overlapping mapping count, high reset span, freeze rejection,
port last-write isolation and reset clearing. The 386 program retains all bytes,
its four-byte REP MOVSD transfer, no-fault predicate and reached-PC assertion.

The six tick rows retain MOV=2, OUT=3, INT=25, segment prefix=3, HLT=2 and
rejected 286 operand prefix=0, with original statuses, stop reasons and
retirement counts. Initialize only the error report's run result, observation
and actual-profile destination so failed producers cannot leave it reading
uninitialized objects. Success checks still require each producer's status.
Three strict-C11 actual-source probes independently fail run, profile query and
observation capture without writing output; each returns failure and emits its
existing diagnostic. Temporary probes are not new registered tests.

Explicit time still intentionally links the common board: its RTC observation
now uses ports 70h/71h instead of private shared_rtc access. It retains halted
no-retirement/no-autonomous-tick checks, timeline now/sequence invariance,
five-tick advancement, RTC advancement, reset clock versus persistent RTC,
unstarted lifecycle rejection and UINT64_MAX overflow rejection. The private
elapsed_ticks injection remains in a Core-owned test to exercise overflow; it
does not justify a new production debug setter. This composed RTC case is not
claimed as a pure neutral-Core-only dependency proof.

Normal six-target runs pass 6/6 per width: x64 2.45s, x86 0.35s, after two
compile/link steps per width. Qualify six receiver paths and six paired original
deletions, leaving 194 pending in the same 620-path inventory. Refresh the tick
test and test-manifest inventory hashes. No product source, EXE input, MyNES,
owner INI, firmware, media or timing precision claim changes. S94 complete
units/gates, independent qualification and T delivery remain open.

### CPU/FPU Metadata, Diagnostic And Decoder Consumers

Read complete current/S92 bodies for CPU/FPU profile choice, metadata closure,
fault diagnostic, execution context and all three 80186/80286/80386 decoder
inventory runners. The first three retain their function bodies exactly and
replace only private CPU/old Board includes with their public interfaces. The
execution-context receiver also retains its complete body, changing its fixture
include path only; qualify its remaining original-deletion row rather than
count its already-qualified receiver twice.

Retain five explicit CPU/FPU configurations plus default 386/no-FPU selection,
the PUSHA/Jcc/ESC/MOVS/invalid-CPUID metadata checks, and all 68,096 metadata
queries in the closure matrix. The fault test retains guest LIDT setup, the
original NOP window, D6 invalid opcode, exact fault PC/bytes/mask and zero
development-history contract. Five CPU profiles retain copied entry/current
snapshots, register/segment values, invalid patch/read rejection and snapshot
nonpublication on invalid point. These are consumer regressions, not a new
claim of full CPU functional/timing conformance.

The three lexical runners retain their exhaustive opcode/ModRM loops and JSON
schemas, masks, historical accepted counts and success markers. Their discovered
stream lifetime defect is corrected at the existing runner owner: each opened
stream has one failure cleanup label; failed final fprintf no longer skips
fclose, and 386 intermediate write failures also close before returning.
Across the three current files this cleanup removes 35 net lines relative to
their pre-repair bodies. No serializer framework, helper API or production
storage dependency is added.

All seven normal tests pass per width: x64 1.87s, x86 0.41s, after six
compile/link steps per width. All six generated JSON hashes equal their
pre-repair hashes and corresponding other-width hashes: 80186 C0904EC4..CD2D2,
80286 AF39B970..34B2B, 80386 7A76B467..B17A3. Fourteen strict-C11 probes include
the actual runner sources and independently fail header, primary mask, escaped
mask where applicable, footer or fclose. Every failure returns rejection and
closes exactly once, including failures during final output. Temporary probe
outputs remain ignored and are removed after this batch.

Qualify seven additional frozen paths, leaving 187 pending of the same 620.
The three modified runner hashes are refreshed; Shared manifests are unchanged
because no Shared source/test file changes in this batch. Production, executable
inputs, MyNES, owner INIs and external assets remain unchanged. This completes
the batch review only; S94 full closure checks and T540 delivery remain open.

### CPU Manifest Verification Cost And Audit Continuation

The owner requests optimizing S execution cost before resuming review. The
two measured slow x64 cases pass once: firmware floppy 15.85s and decoder
ledger 7.12s, combined CTest wall time 16.50s. Its incremental firmware build
costs 0.92s (one link); unchanged audit targets report no work. The eleven
firmware cases exercise real guest firmware, including timeout/recovery; do
not remove their instruction-boundary observation to manufacture a speedup.

Read the complete 182-line CPU manifest verifier and its actual baseline diff.
Its generated-key check projects and scans all existing keys per row, and
array appends copy prior records. Replace only these growing collections with
invocation-local lists and a hash index. Retain the final cross-profile check,
base duplicate check, status/count validation and derived-profile semantics.
The bounded derived-manifest/suffix arrays need no new machinery. The script
diff adds 13/removes 9 lines (net +4); no persistent cache or second verifier.

Single same-host measurements: canonical emission 4.388s before, 2.012s after,
54.2% lower observed wall time. Separate-process raw hashes initially differ
because the existing base Hashtable enumeration is not cross-process stable;
this is not ignored as equivalence. Execute accepted baseline and current
script bodies within the same process: all 4,906 records, complete JSON fields
and emitted ordering are byte-identical. Key-sorted canonical SHA-256 is
6124BB7F939757ABF2C7D93C7BDB5D847FFB0AE61B0E6274E90204D495137D6A.
In-memory malformed manifests produce the identical original rejection for
duplicate base, generated context, cross-profile key and invalid status.

All eight selected runner/8086/8088/decoder consumers pass once per width:
x64 10.94s, x86 10.31s. The decoder check itself now costs 3.40s/3.38s.
The additional 186, 286 and 386 result-verifier scripts also pass per width
(616, 771 and 1,413 conforming records). These are focused equivalence proofs,
not a new complete-unit result or S acceptance. Add this newly changed NXVM
tool to the frozen universe, now 621 paths; Shared manifests remain unchanged.

Then resume actual source review: read complete original and current sign
extension and prefix-attribute Board receivers. Normalized bodies are identical
after include relocation, checked PIC binding and the public IF constant
(both constants are 00000200h). Preserve both CBW/CWD register/IRQ cases and
the prefixed MOV no-shadow frame/register/ISR/IRR case. Run/capture/frame
predicates already short-circuit on failed producers; no repair is needed.
Both tests pass per width (x64 0.08s, x86 0.14s); neither rebuild has work.
Qualify their two receivers and paired deletions, leaving 183 pending of 621.
No MyNES, owner INI, Shared production, firmware or external asset changes;
S94 and T540 remain open without partial delivery.

### LEA, MOV, MOFFS And XCHG Failure-Output Boundaries

Read the complete four original S92 sources and complete current receivers.
Outside the existing include/checked-binding/public physical-inspection changes,
the migrated bodies retain all guest bytes, descriptor limits, register seeds,
full register/FLAGS rollback, memory atomicity and PIC/frame predicates.
Normalize those explicit mechanical changes and this batch's guards: all four
whole bodies equal their originals. The Board-private PIC fixture remains
owner-local; no Core/CPU layout or production setter is introduced.

The producer-failure sweep finds nine separate result consumers: LEA's frame
check; MOV and MOFFS protected diagnostics and IRQ frames; XCHG's write/read
atomicity and memory/accumulator IRQ checks. Guard these existing predicates
with successful prior status, rather than initialize unknown values and then
pretend to verify them. XCHG's initialized failure report additionally checks
Board presence before reading PIC registers after failed construction. Only
the unavailable diagnostic PIC fields use zero; failed construction still
rejects the case. No successful assertion or execution path is removed.

Fourteen original execution contexts remain: LEA one, MOV four, MOFFS four,
XCHG five. Final-source normal tests pass 4/4 per width, x64 3.82s and x86
0.24s. Nineteen strict-C11 actual-source failure checks cover failed short run
and unwritten after-snapshot for all nine functions, plus failed construction
of the accumulator IRQ reporter. The real protected ten-instruction bootstrap
still executes; no substitute CPU or rewritten test body is used. Every probe
rejects failure and the null-Board report completes without a crash. Owned
ignored probe source/executable are removed after verification.

All eleven repairs are in-place line replacements (source net zero versus
the pre-repair receivers); no helper, persistent probe or public API is added.
Refresh the four Shared test manifest entries and inventory hashes, qualify
the four receivers plus paired deletions, leaving 175 pending of 621 paths.
The complete test manifest and production corpus boundaries pass. No production,
MyNES, INI, firmware/media or executable-input changes occur. This completes
the batch review only, not S94 full-unit acceptance or T540 closure.

### Renewed S-Latency Check And CECG Failure Rejection

The owner again requests checking execution cost before continuing S94. Query
the current CTest cost data and native incremental build rather than treating
historical averages as a fresh benchmark. The affected x64 target build reports
`ninja: no work to do` in 0.419s. The unchanged firmware floppy and decoder
checks pass once together in 14.65s: respectively 14.62s and 3.26s. The firmware
test still executes its eleven real guest read/write/error/recovery contexts;
do not skip instruction checkpoints or change guest timing to reduce this cost.

Retain the already verified invocation-local strict-command batching and CPU
manifest indexing. Continue affected-target incremental builds and transient
checks during review, with complete dual-width units/gates at S closure. No
extra cache, new build tree, timeout increase, repeated boot round or MyNES
build is introduced. Remaining whole-source review, not an incremental compile,
is the current main work item; 175 inventory dispositions remain pending.

Resume that review with a strict-C11 probe including the actual CECG S9
receiver. Unexpected acceptance of invalid configuration, failed generic retry
on the same owner, and failed construction all reject. All three checks pass;
the owned ignored probe source and executable are removed after execution.
The remaining video/interrupt/FPU batch verification and inventory dispositions
are not yet complete. This is progress evidence, not S94 acceptance or a P.

### Video, Interrupt Return, Call Gate And FPU IRQ Receivers

Complete original/current body and diff review covers the three CECG receivers,
interrupt-return composition, call gate, and the Board IRQ part split from the
mixed FPU interface test. Retain the complete 19-row directional port matrix,
CECG provider/RAM routing and reset transitions, planar page/frame changes and
B0000/B8000 alias checks. The opaque Core video fixture owns the real VADP
adapter; its increased RAM capacity does not change any tested address or route.
The original mixed FPU source deletion and its other receivers remain pending;
this batch qualifies only its fully inspected Board IRQ receiver.

Preserve the original INT31/IRET/IRQ0 program, eight-instruction budget, complete
IRQ frame and PIC assertions; the 286 call-gate GDT/TSS/program/marker bytes;
and both WAIT/FNINIT IRQ cases with their seven-register preservation checks.
Public entry/register patches and copied snapshots replace private CPU storage,
without a new production API or a substitute execution path.

Correct four existing failure boundaries: CECG invalid configuration must
actually reject before retrying generic configuration on the same owner;
interrupt-return's separate output predicate runs only after successful
producers; call-gate failure reporting has initialized diagnostic/result values;
FPU oversize rejection has a zero-initialized fixture and non-overflowing bound.
No successful-path assertion is weakened or omitted.

Six normal receivers pass on both widths: x64 3.24s, x86 0.34s. Nine strict-C11
actual-source fault checks pass: three CECG outcomes; failed run and failed
diagnostic for both call gate and interrupt return; FPU lengths eight and the
maximum lib_size. Failed producers leave outputs unwritten. Both call-gate
reports complete safely and still reject; oversize FPU cases reject before
construction/copy. Owned ignored probe sources/executables are removed.

Refresh four test manifest entries and their inventory hashes. Qualify the six
receivers and five fully reviewed paired deletions, leaving 164 pending of 621
paths. No production, MyNES, INI, firmware/media or executable input changes.
This is another bounded review batch, not S94 closure or a partial P delivery.

### Complete FPU Interface Split And Failure-Output Review

Read complete current/original 8087 and retained App ESC bodies and the entire
original mixed S65 source in bounded chunks. Review its Core, CPU and Board
receivers together, not its deletion in isolation. The original 123 S65 call
contexts retain 121 Core execution/transaction/deadline/exception contexts and
two Board WAIT/FNINIT IRQ contexts. The already reviewed CPU receiver adds the
full private-state/cache complement (91 contexts), including all original
prefix/LOCK/incompatible rejection and VM86 storage-preservation conditions.
No arithmetic, stack/reset, unmasked FWAIT or profile gate is removed from 8087;
all six App ESC calls, including both delivered #NM cases, remain.

All 27 static S65 guest recipes/data declarations and seven 8087 recipes are
retained verbatim after whitespace normalization; only their removed reset
provider declarations are absent. Public entry patches replace those callbacks.
S65 blocks vector 6 using the existing explicit exception fixture instead of
private IDTR mutation. Its protected bootstrap starts at 0200h rather than
overlapping that IVT provider, retires the exact nine setup instructions to
the same protected entry, and no longer requires incidental execution of blank
memory followed by private halted-state mutation. Target #NM still has zero
retirements/ticks, the exact original frame and GPR/segment assertions, then
executes the original HLT handler. This is a test construction correction,
not a guest timing change or new production path.

The output sweep repairs nine in-place line replacements across three tests:
initialize the 8087 failure reporter's run result; guard its unmasked exception
predicate, S65 handoff/#NM/protected diagnostic/result readers, and App ESC's
separate diagnostic/state predicates after successful producers. Original
successful-path assertions remain intact. Helper-based success/rejection checks
already short-circuit on failed capture; checked snapshot helpers terminate
rather than return invented data. The two split Board IRQ checks were reviewed
and qualified in the preceding batch.

Four normal FPU tests pass once per width: x64 2.86s, x86 0.29s, after six
compile/link steps per width. Thirteen strict-C11 actual-source fault checks
pass: failed run/diagnostic for the 8087 unmasked case, S65 handoff, real #NM
and protected #NM; failed run/diagnostic/state for App scalar ESC and failed
run/diagnostic for its delivered #NM. Failed producers leave outputs unwritten;
the protected nine-instruction bootstrap still executes. The 8087 report remains
safe and every tested failure rejects. Owned ignored probes are removed.

Refresh the two Shared test manifest entries and three inventory hashes;
qualify both Core receivers, their two original deletions and retained App ESC,
leaving 159 pending of 621. The CPU and Board split receivers retain their prior
review dispositions. Production, MyNES, INIs, firmware/media and executable
inputs remain unchanged. S94 full-suite acceptance and T540 delivery remain open.

### Current Build And Test Cost Recheck

The owner asks to optimize slow S execution before resuming the active review.
Retain the already verified strict-matrix batching (36.14s to 4.91s) and CPU
manifest indexing (4.388s to 2.012s); neither is a newly measured speedup here.
An initial build measurement used an incorrect transaction target name and
failed Ninja target lookup. Its timing is not build evidence. After resolving
the names from the actual test registration, the five current Core receiver
targets build successfully in 0.371s without recompilation.

The two slow selected checks each pass once on x64: firmware floppy 14.95s,
decoder ledger 3.07s. Their four-test invocation, including RAM construction
and ROM rollback receivers, completes in 14.98s. The remaining transaction,
lifecycle and timeline receivers pass once together in 1.33s. These are current
focused results only, not complete dual-width acceptance.

No further cache, build tree, parallelism increase, timeout change or removal of
guest checkpoints is justified by these measurements. Keep affected-target
incremental builds, temporary affected-test selections during repairs, and
complete dual-width units/gates at S closure. Source/diff/coverage review is
still the substantial pending work; the full baseline transaction and lifecycle
bodies are read next, including construction failure and unwritten run outputs.
No production, MyNES, INI, firmware, external media or executable changes are
introduced by this cost recheck. S94 remains active.

### Core Memory, ROM Rollback, Transaction And Timeline Receivers

Read the complete five baseline App tests and current Core receivers, their
actual body differences, and the complete construction/binding fixtures. RAM
retains two successful sizes, two allocation failures, seven malformed ratios,
two null-argument preflights, raw RAM reallocation/read/write and three board
publication cases. The board backpointer assertions now execute inside the
board-owned construction/binding fixtures; the Core receiver only compares its
own attachment with the copied expected context. No assertion is dropped.

ROM rollback's body is unchanged apart from its relocated include. All four
failure modes preserve existing bytes, mapping/provider counts and unbound
firmware, then prove a successful retry and high reset alias on the same owner.
Transaction S2 retains memory/port pairs, prefetch provenance and wrap counts,
reset cancellation, five-phase DMA service and invalid/success/provider-failure
DMA cycles. Its DMA port is the actual Core executor port required by the new
bus boundary; the stack Core has an explicit initialized lifecycle. The device
callback's memory inspection uses the Core API and observes the same byte.

Lifecycle retains all 18 port and six memory outcome rows, observe-only and FPU
extension admission, provider side effects on failure, reset cancellation and
CPU commit/retirement ordering. Timeline retains immediate-progress reporting,
three retired ticks, trace ordering, stable same-time nested callbacks, earliest
due queries, cancellation, rejection of past scheduling and reset clearing.

Admit and apply eight in-place line replacements in the three transaction/
lifecycle/timeline receivers: failed construction returns before dereferencing
the absent machine; run/observation predicates do not consume unwritten outputs.
The original successful assertions and execution inputs remain intact. The five
normal receivers pass once per width: x64 4.32s, x86 0.34s; each rebuild compiles
and links only the three changed tests. No production or executable input changes.

A strict-C11 probe includes the actual receivers and injects unwritten-output
failures: S2 construction and first/second runs (three checks), lifecycle
construction/run (two), timeline construction/run/observation (three). All eight
reject with test failure. Reusing one just-exited probe EXE caused a host link
permission error for the third compilation; a distinct timeline output links
and passes, without restarting any live test or treating the failed link as proof.
Owned ignored probe files are removed after verification.

Qualify the five receivers and five paired original deletions: 159 to 149 pending
of 621 review paths. Refresh the three test manifest entries and current hashes.
This is one source/coverage batch, not full S94 acceptance or a partial P. MyNES,
owner INIs, firmware, external media and production artifacts remain unchanged.

### Board Segment-Pointer Fault And IRQ Receivers

Read complete original/current LES/LDS, S41 LES/LDS and LSS/LFS/LGS sources.
The first combined source read was truncated; bounded rereads cover the missing
current S41/LFG bodies and original bodies before qualification. Actual body
comparison separates include relocation, checked PIC binding, public memory
inspection and S41's added stage-specific failure text. Guest programs, bootstrap
instruction counts, register/segment seeds and assertions remain unchanged.

Retain four base LES/LDS contexts (two source-limit faults, two IRQ/no-shadow
frames); ten S41 contexts (two opcodes times three bad selectors, two source
limits, two IRQ frames with unaffected GPR/segment/flags observations); and six
LSS/LFS/LGS contexts (three source faults and three IRQ cases). LSS still observes
its one-instruction shadow and frame IP 6, whereas LFS/LGS observe no shadow and
frame IP 5. Protected setup still executes genuine guest LGDT/LMSW/segment loads.
Board PIC state stays Board-owned; memory/CPU observations use public Core APIs.

Revise the packet before ten in-place failure-handling replacements in the three
tests. Guard snapshot/diagnostic/result predicates and frame-address consumers
after failed producers; initialize LFG's diagnostic-print values and exclude
absent-board access in its failure report. There is no new helper/framework or
production behavior/API. Successful predicate evaluation is unchanged.

All three normal receivers pass once per width: x64 3.28s, x86 0.23s, with only
three test recompilations/links per tree. A strict-C11 probe includes each actual
receiver and injects unwritten-output construction, one/two-instruction run,
diagnostic and first/second snapshot failures. Thirty affected helper contexts
reject correctly: base LES/LDS eight, S41 fourteen, LSS/LFS/LGS eight. It also
proves LFG construction failure reporting no longer dereferences a missing board.
Distinct probe executables avoid relinking an output still being retired by the
host. Owned ignored probes are removed after execution.

Qualify three receivers and three paired original deletions, 149 to 143 pending
of 621 paths. Refresh their manifest/hashes. This is a bounded coverage review,
not full S94 acceptance or partial delivery. No MyNES, owner INI, firmware, media,
production or product executable input changes.

### Firmware Capability And Legacy Timing Normalization

Read complete original/current firmware-capability and legacy-normalization
receivers, then their actual differences and the removed executor constructor
plus retained public debug/bind fixtures. Truncated combined reads are followed
by bounded rereads before qualification. Retirement observation was initially
selected too but remains unqualified: its complete body and coverage review are
still required, not inferred from this batch's results.

The removed firmware executor helper only created the same minimum-RAM Board
configuration; direct construction preserves that input. Former configuration
memory-query helpers become owner-local physical queries before freeze, with the
existing public query for the later alias-boundary assertion. Retain failed
configure rollback of E0000/F0000 with prior D0000 preserved, expired capability
rejection, configure/reset reentry exclusion, immutable alias bytes/length,
reset/run failure lifecycle transitions, port-error propagation, after-run
callback and provider read tick equal to Core elapsed time.

Timing normalization preserves all eight 8086 and seventeen 80186 arithmetic
rows, including odd-address/segment overhead and the exact ticks/provider
advance assertions. Public word writes preserve the original low-half mutation;
public EIP reads retain the divide-by-zero vector/handler assertions: zero
retirements and zero published time on delivery, then one HLT/two ticks.
No timing evidence classification, production clock or CPU behavior changes.

Revise the packet and make four line replacements: initialize diagnostic run
results in both tests, reject failed firmware construction before dereference,
and skip null timing-case diagnostic dereference while retaining failure return.
Both normal tests pass once per width (x64 2.76s, x86 0.28s), four compile/link
steps per tree. Strict-C11 probes include actual sources and prove seven failures
reject: firmware construction/run; timing null case, construction/run; divide
delivery construction/run. The failure print values are defined even when no run
result is written. Owned ignored probes are removed after execution.

Qualify two receivers and two original deletions: 143 to 139 pending of 621;
refresh test manifest and inventory hashes. This is bounded S94 review evidence,
not complete acceptance or a partial P. No MyNES, owner INI, firmware, media,
production or product executable inputs change.

### Complete Retirement-Observation Receiver Review

Read all 755 baseline and 767 current lines, including every helper and main
predicate, and compare the original private reset-mapping fixture with the
current public alias installation. Both install the 16-byte high reset alias
and the narrower 8086/8088/80186 or 80286 physical alias into the same backing.
The current helper queries the public profile instead of reading Board-private
construction state. The FPU wait setup remains an intentional Core-owner test.

After normalizing includes, that helper and two newly admitted guard prefixes,
the entire remaining test body is identical. Retain all five unallocated-profile
cases, scalar/ModRM/stack/branch/string timing contexts, repeat continuation,
pre-mode snapshots, callback reentry exclusions and disabled-provider/reset
checks. The actual normal receiver executes 116 Core run calls.

Two separate main predicates read a run result even when its producing call
failed without filling it. Add only `if (!failed)` to those predicates; all
successful-path observation assertions are unchanged. Normal x64/x86 receivers
pass once (CTest wall time 2.75s/0.54s). A strict-C11 probe includes the actual
receiver: its normal pass counts the 116 calls, then injected first-run and
last-run failures leave output unwritten and are rejected; failed construction
is also rejected. The probe's normal pass is a context-count control, not a
second boot qualification or complete-suite claim.

Qualify the receiver and original deletion: 139 to 137 pending of 621. Refresh
the manifest and inventory hashes. Remove this batch's owned ignored probe
after execution. Production, API, timing classification, MyNES, owner INIs,
firmware/media and executable inputs are unchanged. S94 stays active; this
batch neither delivers a partial P nor proves full closure.

### Real-Mode 386 REP String Receiver

Read complete original and current sources and the already-qualified reset
alias fixture. The entire body is identical after normalizing includes,
Board config to neutral executor config and Board create to neutral create.
All original explicit memory, CPU, FPU and tick inputs remain. No port, IRQ,
device callback or Board observation occurs in this test; neutral Core is its
correct owner, not a replacement Board execution path.

Retain the three programs: REPE CMPS stops at the second-byte mismatch with
ECX=1/ESI/EDI advanced twice and ZF clear; REPNE SCAS stops at its second-byte
match with ECX=1/EDI advanced twice and ZF set; CS-override CMPS uses 1000h
instead of the distinct DS-default 11000h with ECX=0 and ZF set. Each run,
diagnostic and copied snapshot predicate short-circuits on failed output
production. Setup destroys a partially constructed owner before rejection;
later cases run only after prior success. No further failure guard is needed.

Normal receivers pass once per width (x64 CTest 0.05s, x86 0.10s); both
incremental builds report no work. Qualify receiver and deletion, 137 to 135
pending of 621. This source-only review does not change any source/test input,
manifest, product artifact or MyNES path, and does not close S94.

### CPU/PIC Task, Outer-IRET And IDT Privilege Receivers

Read all three original/current bodies, their CPU task/outer-return setup,
checked PIC operations and the bare Core registration owner. The latter replaces
only the original embedded initialized Core/port owner: it allocates that same
registration state and does not create or run another CPU. CPU execution remains
the CPU fixture's sole execution context; PIC acknowledgement follows its bus.

Normalize include relocation, opaque registration-owner creation/destruction,
the public IF constant, the three binding guards and null-PIC failure printing:
the complete remaining bodies are equivalent. Task16 retains the direct task
transfer, target IF and IRQ handler 0180h/HLT at 0181h plus ISR/IRR checks.
Outer IRET retains the 32-bit outer frame, user NOP, four refreshes and kernel
IRQ return CS/SS/IP/HLT and ISR/IRR assertions. IDT privilege entry retains the
CPL3 descriptor/TSS setup, ring-zero stack 8FECh, cleared IF and PIC service
checks. None of those successful contexts or expectations changes.

The earlier checked-binding record's zero-ignored-call statement was too broad:
these three still-pending CPU/PIC paths retained raw ignored calls. The current
all-test search exposes them. Revise the packet before repair, check each bind
through its existing cleanup and skip execution on failure. Outer-IRET failure
printing also avoids reading an absent PIC after failed construction. The three
files add 16/remove 5 lines for this repair, net +11; no new state or helper.
The remaining raw multiline calls found by the sweep compare their status.

All three normal receivers pass once per width (x64 0.48s, x86 0.21s). A
strict-C11 probe includes the actual three sources and injects missing Core
owner, rejected PIC construction and rejected IRQ binding: all nine contexts
return failure with zero CPU refreshes. Their existing finalize paths remain,
and failure diagnostics no longer prevent cleanup on an absent PIC. Remove the
owned ignored probe source/executable after verification.

Qualify three receivers and three deletions: 135 to 129 pending of 621. Refresh
test manifest and review hashes; no production/API, MyNES, owner INI, firmware,
media or product executable input changes. Full S94 closure remains pending.

### Software INT Complete CPU And Board Receiving Pair

Read the complete 604-line original mixed test, 207-line CPU receiver and
Board receiver, including their real-entry, snapshot, delivered-exception and
guest-mode setup helpers. Reconcile all 90 original contexts: 24 INT3/INT n/
INTO-taken transfers and eight INTO-clear cases; 36 legacy invalid-prefix and
12 386 LOCK rejection contexts; six protected success/DPL contexts; three
protected-fault/VM86 contexts; and one PIC pending-boundary context.

CPU owns all 48 private whole-state/stack rollback negatives. Its constrained
IDTR reproduces the original terminal-before-exception-frame-write preflight;
`cpu_instruction_run` always copies the CPU state even on terminal error.
It also retains 32 positive GPR/segment-state contexts. Board keeps those 32
contexts for their distinct Core delivery, FLAGS/width and real stack-frame
assertions, plus the ten protected/PIC rows. The overlap observes different
owners and does not duplicate a production instruction implementation.

The original IF/TF clearing and GPR/segment comparisons, return IP, CS and
known FLAGS masks all remain. The 16-bit flags-image mask 802Ah equals the low
word of the original private reserved mask. Physical stack reads become public
inspection of the same low ordinary-RAM ranges. The original fabricated VM86
cache writes become genuine LGDT/LIDT/LTR/IRETD entry, confirmed by the existing
mode helper; an empty GP gate preserves the terminal DF and before/after state
check. CPU-private rollback cannot move to a public Board snapshot, so its
receiver is retained rather than silently removed with the mixed test.

Separate Board result/diagnostic and protected snapshot consumers still read
unwritten outputs after producer rejection. Revise the packet, add guards
without changing successful-path predicates, and correct one unindented CPU
fixture declaration. Both receivers pass once per width (x64 1.49s, x86 0.18s).
The actual-source strict-C11 failure probe verifies 17 rejected contexts:
construction/run/diagnostic rejection for real transfer, INTO-clear, protected
and first protected-fault paths; construction/run for PIC; and later diagnostic
rejection in protected DPL, second protected-fault and VM86 paths. PIC does not
consume a diagnostic, so that non-call is not counted as a failure probe.

The initial probe compilation failed because the includer's nested `main`
macros defeated an outer rename. That is not test evidence. The corrected
ignored GCC probe invokes its driver before the unmodified test entry point;
the final 17-context run passes. Remove its owned source/executable afterwards.
Qualify the pair and original deletion: 129 to 126 pending of 621, refreshing
manifest and inventory hashes. The separate interrupt-entry includer still
requires complete source qualification. No production/API, MyNES, INI,
firmware/media or product executable changes; S94 remains open.

### Incremental Cost And Interrupt-Entry Continuation

The renewed owner request checks actual build/test cost before continuing.
The first build command names a nonexistent target and is not build evidence.
After resolving the actual registration, the unchanged firmware and interrupt
targets report no work in 0.364s. Current single x64 test runs pass: firmware
floppy 15.14s and decoder ledger 3.42s, overlapping for 15.17s total. The first
selection did not include interrupt entry because its test name was wrong;
that invocation proves exactly two tests, not three.

Keep the already verified matrix batching and CPU-manifest indexing, incremental
trees, eight-way scheduling and transient affected-test execution. Do not add
a compiler cache to a no-work build, raise concurrency after the recorded
sixteen-way timeout experiment, or remove firmware checkpoints. The firmware
test executes eleven synthetic-media BIOS scenarios with exact PC stops; no
production speedup or cold-build improvement is inferred from these timings.
The large actual-source/diff/coverage review remains distinct from build cost.

Continue S94 by reading all 616 original and 671 pre-repair receiver lines of
interrupt entry. Its 26 original contexts remain: two gate successes, prefix
width, eight rejection cases, three software frontends, four IRQ/NMI origins,
three delivered faults, T305 delivery and four failed-delivery rollbacks.
Real LGDT/LIDT/segment loads and IRET replace fabricated CPU caches; ordinary
Core snapshots/frames remain Board observations. CPU-private latch/cache
coverage retains its previously reviewed owner, not a public getter.

Three separate preparation stages still allowed later execution/capture after
failure. Revise the active packet and add existing-condition guards around
external-origin run, fault-delivery setup and failed-delivery rollback capture.
No successful assertion, guest program, budget or production input changes.
The affected receiver rebuilds and passes once per width: x64 0.85s test time,
x86 0.09s. Refresh its manifest/inventory hash; retain pending disposition
until failed-preparation probes and final CPU/Board pairing are qualified.
This batch does not close S94 or justify a partial P/commit.

The real-source preparation probe wraps memory writes, direct execution and
snapshot capture, retaining actual Board construction and the guest bootstrap.
Eleven external-origin/delivered-fault/failed-delivery scenarios first pass
normally; reject each of their 145 individual write positions in turn. Every
rejection returns failure without subsequent intercepted execution, write or
capture. This includes the later user-IRET and limited-stack reload setup,
not only the first construction write. Strict C11 compilation and the probe
pass. Remove its owned ignored source/executable after recording the result.

The normalized original/current `main` bodies match exactly after replacing
the two gate constant names, preserving all 26 original invocations. Finish
CPU/Board pairing by rereading the CPU latch/cache receiver: successful and
failed CPL0/CPL3 NMI latch consumption, four delivered-fault cache/stack
rollbacks and six full software rollback cases remain CPU-owned. Board keeps
PIC ISR/IRR, public diagnostics, accessed bits and real memory frames. The
CPU receiver passes once per width (x64 0.02s, x86 0.03s); its source bytes do
not change. Qualify the Board receiver and original deletion, remove the
CPU row's remaining-pairing note, and reduce pending paths from 126 to 124.
The complete 621-row review still has remaining work; no S/P acceptance.

### MOVX, FS/GS And Legacy Divide Receivers

Read the complete original/current MOVX and FS/GS Board bodies, both retained
CPU siblings, and complete original/current legacy divide receiver. MOVX keeps
sixteen older-profile opcode/ModR/M rejection contexts and the protected data
limit fault with zero unauthorized source reads. Its two former memory-helper
registrations become one public route batch with the same two ranges,
callbacks and owner. The unchanged CPU sibling retains sixteen successful
extension/size/source combinations and address-prefix semantics.

FS/GS retains both protected POP stack-fault cases through genuine descriptors;
the existing CPU sibling retains successful/rejected real-mode width matrices,
protected POPs and deliberately inconsistent private selector/cache contexts.
No CPU sibling code changes. The two-profile divide receiver preserves vector,
stack frame, GPRs and generation-specific known FLAGS. Its whole normalized
body exactly matches baseline after public include/flag-name substitutions;
the reserved-mask literal equals the active CPU definition 0xfffc802a.

MOVX and FS/GS had separate failed producers followed by unguarded output
consumers. Revise the packet and use one short-circuit chain at each operation
sequence; FS/GS reads selectors/FLAGS directly from its successfully captured
before snapshot rather than retaining two redundant local copies. Preserve
every successful-path comparison, program and budget. Three Board plus two
CPU cases pass once per width (x64 2.40s total, x86 0.32s). The final indentation
cleanup does not change those semantics.

An actual-source strict-C11 probe includes the three final Board bodies and
rejects each construction, write, patch, run, diagnostic and capture call in
turn, leaving failed outputs unwritten. All 167 failure positions reject with
zero later intercepted operations. Remove owned ignored probe products.
Qualify the three Board paths: 124 to 121 pending of 621. No production/API,
Shared CPU sibling, MyNES, INI, firmware/media or product executable change;
S94's complete current unit/gate and remaining source review still remain.

### Legacy LOCK And Operand/Address Receivers

Read both complete baseline/current bodies and actual diffs. LOCK retains
three older-profile LOCK OUT contexts and three protected privilege contexts,
including the original allowed/rejected LOCK CBW behavior and exception frame.
Operand/address retains both data-limit fault contexts and the two REP port
string contexts with their source, destination, count and provider assertions.
These are preserved tests, not new instruction or timing qualification claims.

Whole-body comparison is exactly equal after public include-path substitutions.
Existing preparation chains reject failure before consuming unwritten outputs;
initialized diagnostic values preserve failure reporting. No source repair,
new probe, API or timeout change is needed. Both target builds report no work;
both tests pass once per width (x64 0.06s total, x86 0.15s).
Qualify these two paths: 121 to 119 pending of 621. Complete S94 review and
current dual-width unit/gate closure remain open; no partial P is delivered.

### Exception Delivery And Segment-Selector Receivers

Read the complete current bodies and their actual baseline diffs. Protected
and real #UD originally change only their public Board include; segment-selector
is exactly baseline after its two include substitutions before this repair.
Read both full real #GP bodies: its private CPU reset-provider patch becomes
the same entry patch through stopped public Core debug after reset. No guest
instruction, table, stack, flags or expected delivery behavior changes.

Preserve six protected #UD contexts, six real #UD contexts and both real #GP
contexts, including failed delivery. Retain both 256-opcode metadata matrices,
lexeme neighbors and operand/group negatives. Segment-selector retains four
286 cache rejections, five LXS rollbacks, four MOV rejections, two POP rejections
and final POP rollback. All 47 matched byte-array declarations are identical
to baseline (17/4/9/17 respectively); actual-diff review preserves every
successful frame, register, segment, exception and metadata predicate.

Failed before-capture, run or diagnosis could previously reach later operations
or unwritten result readers. Express each producer/consumer sequence as one
short-circuit chain. Remove the two real-mode run wrappers, so expected status
is checked before capture/diagnosis. Do not run the real #UD handler after its
delivery check fails. Initialize fault run results and the segment bootstrap's
failure report. Segment setup-error returns now destroy their existing Core;
failed cases terminate their local matrix after cleanup. Successful matrices
remain complete. No production, API, timing or product-artifact input changes.

Three exception tests pass once per width (x64 2.95s total, x86 0.18s); the
segment test passes once per width (x64 0.88s test time, x86 0.08s). A strict-C11
probe includes all four actual final test bodies, wrapping construction,
freeze/reset, patch/write, execution, capture, diagnosis and linear reads.
Fourteen exception cases plus four segment matrix functions first pass normally.
Reject each of their 514 intercepted call positions while leaving failed
outputs unwritten: every scenario rejects, makes no later intercepted operation
and retains zero live Core allocations. Expected diagnostic messages remain;
owned ignored probe source/executable are removed after the result.

Qualify four inventory rows: 119 to 115 pending of 621. Full current dual-width
units/gates and remaining review still block S94 acceptance. Whole-baseline
tracked diff for these four test paths is 108 added/124 removed lines, net -16,
including the pre-existing migration and this failure-boundary repair. MyNES,
owner INIs, external firmware/media and Shared source remain unchanged here.

### Migrated CPU/FDC Negative Controls

Read complete original/current CPU and FDC copied-input negative scripts and
their actual diffs, then reread the current consuming CPU/PIC, FDC-state and
DMA/FDC verifiers and their CTest registrations. CPU retains nine files times
eight injections (72), five bus-layout injections and 24 test receivers times
four injections (96): all 173 controls retain specific rejection checks.
The receiver path selection follows the actual Shared/App split, not a second
test implementation. FDC retains its baseline checks and all seven chip,
Board, scheduler, DMA and media-ownership injections at their new real owners.
No additional repair or reduced negative coverage is needed.

Both registered tests pass once per width: x64 CPU 45.89s and FDC 2.68s;
x86 CPU 44.94s and FDC 3.08s. Registration uses separate owned directories;
validate the CPU fixture's resolved cleanup path beneath build before launch.
Only copied inputs are mutated. Preserve the already admitted removal of
unnecessary CPU-negative global serialization; no new timeout or scheduling
change is made. Qualify both paths: 115 to 113 pending of 621.

Read all current 80286 protected-mode source and its actual migration diff.
Its nine original contexts still have separate failed producers and diagnostic,
state or frame consumers, including uninitialized failure-report fields.
That path remains explicitly pending: the next bounded repair must retain its
nine programs/assertions, public stopped entry setup, and genuine fault delivery,
then prove failed-producer/reporting behavior before qualification. Merely
having read the file or passing the boundary scanner does not accept it.

### 80286 Protected-Mode Failure Boundary

Complete the above pending repair after reading the complete current body,
original body and actual diff. All nine contexts retain their instructions,
descriptor tables, exception/frame/register predicates and markers; all 24
matched constant byte arrays are identical to S92. The old private CPU reset
callback becomes the existing stopped public Core register patch after reset.
Validate helper arguments before optional fault-table setup; chain failed
producers with their consumers instead of performing later checks independently.
Initialize the positive case's diagnostic/state and report flag. No production,
API, timing or product input changes.

The registered normal test passes once per width: x64 0.75s, x86 0.08s.
A strict-C11 probe includes this actual final body and its actual inline fixtures.
Wrap construction, bind/freeze/reset, entry/register patches, memory writes,
execution, diagnosis, state, snapshot and reads. Every context passes normally;
all 141 intercepted failure positions reject with failed outputs left unwritten,
zero later intercepted operations and zero live Core allocations. The expected
failure diagnostic remains; remove the owned ignored probe source/executable
after terminal completion. Qualify this path: 113 to 112 pending of 621.
The whole-baseline path diff is 49 added/69 removed lines, net -20 including
migration. Current complete dual-width verification and remaining source review
still block S94 acceptance. MyNES is untouched and unbuilt.

### Protected Bootstrap, Call Gates And External Returns

Read complete original/current bodies and actual diffs for the protected-16
bootstrap and all six gate/external/outer/return/call-gate consumers. The fixture
diff changes only the public Board include and introduces six guest descriptor/
FLAGS encoding constants. Check each value against its former CPU definition.
The complete fixture body is identical after removing that enum and normalizing
the include. No guest-only bootstrap, GDT/IDT/TSS image, LTR/IRET privilege entry,
default-size setup, register/frame predicate or cleanup changes.

The six tests retain all 43 contexts: five 32-bit call-gate entries/rejections,
five 16-bit call-gate contexts, ten interrupt/trap gate and encoding/present/entry
contexts, ten software/DPL/NMI contexts, six outer-NMI profile/TSS/gate rows and
seven IRET/RETF variants. Remove six redundant unchecked failure-report queries
in three receivers; initialized/copied diagnostic values still produce the
original report. All six complete bodies are baseline-identical after only
public include/constant substitution and those six query removals. Every guest
program, successful assertion and marker remains unchanged.

Both call-gate tests pass once per width: x64 1.71s combined, x86 0.14s.
The four other tests pass once per width: x64 2.52s combined, x86 0.28s.
A strict-C11 actual-source probe includes all six bodies plus their common
fixture. All original contexts and two additional parity/bootstrap selections
pass normally. Inject failure at each of 914 construction, parity configuration/
fault/observation, freeze/reset, register patch, write/run/diagnosis/capture/read
positions with outputs left unwritten. Every case rejects, with zero later
intercepted calls and zero live Core allocations. An initial two-receiver probe
covered 251 positions; the expanded probe supersedes it. Remove the owned ignored
probe source/executable after terminal completion.

Qualify seven paths: 112 to 105 pending of 621. Whole-baseline diff is 73 added/
69 removed lines, net +4 including the existing ten-line encoding enum and
migration. No production/API, timing, MyNES or artifact input change. Complete
current dual-width units/gates and remaining actual-source review remain open.

### ARPL Board Receiver Reconciliation

Read the complete 292-line original and Shared receiver, including the two IRQ
variants. Full body comparison is exactly identical after four include-path
substitutions, public ZF/IF constants and the already admitted checked IRQ-binding
helper. Reread that helper and its public PIC programming/capture operations.
No program, GDT/IDT/bootstrap byte, memory prefix/limit rollback, register/segment,
IRQ frame or PIC predicate changes. All four original execution contexts remain.
The private Board layout access is owner-local at test/x86/ibmpc-common, not an
App import. Registration links the receiver to x86-ibmpc-common and x86-core;
there is no duplicate App executable/source. Both normal tests pass once per
width: x64 0.04s, x86 0.03s, both incremental builds have no work.
Qualify receiver and original-deletion rows: 105 to 103 pending of 621.
BOUND's original deletion/receiver remains pending; its earlier oversized
combined diff output was truncated and does not constitute full review.

### BOUND And Port-I/O Receiver Reconciliation

Complete the formerly truncated BOUND review through bounded reads of all
339 original/receiver lines. Also read all 224 scalar port-I/O and 271 string
port-I/O original/receiver lines. Each complete body is identical after four
include substitutions, the public IF constant and the previously admitted
checked IRQ-binding helper. No new source repair is needed. All programs,
descriptor/interrupt tables, register/frame/partial-transfer checks and markers
remain. Failed producer outputs are initialized and their later readers are
guarded; intentional double-fault status handling is retained.

BOUND retains eight contexts: four real-mode bound limits, three protected
BR/GP/SS-to-DF paths and IRQ delivery. Scalar I/O retains eight provider-failure,
IRQ and timestamp contexts. String I/O retains eight INS/OUTS REP/non-REP IRQ
and protected-limit contexts. Their private Board reads now belong to the
Shared Board test owner, with one registration and no duplicate App source.
All three normal tests pass once per width. BOUND takes x64 0.04s/x86 0.03s;
the two port tests together take x64 0.98s/x86 0.16s. Incremental builds report
no work. Qualify six receiver/deletion paths: 103 to 97 pending of 621.

The renewed latency check reuses the recorded strict-matrix batching and
manifest indexing rather than claiming another optimization. Current port
target builds finish in 0.82s/0.80s without compilation. Retain eight-way
scheduling, transient affected checks and mandatory complete S closure suites;
do not introduce a persistent cache, raise timeouts or trim coverage. The
remaining source review is separate from build/test cost. S94 remains active,
with no partial P delivery, MyNES rebuild or owner-input change.

### Protected I/O And TSS Permission Boundary

Read the complete protected-I/O original/current bodies, all 83 port-mode
fixture lines and both complete TSS I/O-map bodies. Protected I/O retains
24 scalar forms (eight opcodes across kernel/user/VM86), one denied scalar,
eight INS/OUTS permission cases and the insufficient/sufficient budget sequence.
Its former fabricated caches are replaced by guest LGDT/LMSW/far jump and,
for user/VM86, LTR/IRETD through public Core operations. Copied snapshots verify
PE, selectors/bases, CPL/VM and busy TSS. Setup execution remains on Core's
single timeline; row ticks and provider counters retain their original exact
expectations while elapsed ticks include the separately captured setup time.
No fixed instruction costs, permission predicates or markers are weakened.

TSS I/O-map retains all seven 286/386 allow/deny/IOPL/truncated-word cases.
All eleven original byte arrays are exactly identical. Replace the old private
reset callback with the already implemented public stopped register patch;
tables, privilege transitions and genuine delivered handlers remain. Review
finds an inherited failure chain: failed install/run/read/diagnosis could still
execute later operations and report unwritten values. Revise S94 first, then
chain those producers, guard success predicates and initialize report values.
No production or API changes. The two normal tests pass once per width: x64
2.04s combined, x86 0.24s combined. A strict-C11 probe includes the actual TSS
body and fixture, with failed outputs left unwritten. All 155 intercepted
creation/install/freeze/reset/register/write/run/read/diagnostic positions across
seven cases reject with zero later intercepted operations and zero live Core
allocations. Expected failure reporting is suppressed only in the probe.
Remove its owned ignored source/executable after terminal completion.

Refresh test manifest and inventory hashes. Qualify five paths: 97 to 92 pending
of 621. The other port-mode consumer remains pending its own full review; fixture
qualification does not accept that caller. Current full dual-width units/gates
and remaining actual-source review still block S94 acceptance. MyNES is untouched.

Read the complete hardware-delivery S3 original/receiver next. Its four real,
protected and masked/unmasked VM86 priority contexts retain frame and PIC
assertions, but failed NMI admission/mask setup can still reach PIC preparation
or execution. Revise S94 for that bounded guard repair. Its retired private
NMI-pending assertions require CPU-owned coverage reconciliation; the initial
combined supporting-source output was truncated, so neither that mapping nor
this receiver/deletion pair is qualified. Continue with bounded reads and
matching failure-path verification, not a new public pending-state getter.

### Hardware Priority And NMI Failure Boundary

Complete the bounded NMI-owner reads. The real-mode private consumed/pending
bit is covered by cpu_signal_case across all five profiles, including pending
retention while masked and clearing on delivery without IRQ acknowledgement.
The protected bit is covered by idt_test_nmi for CPL0/CPL3, with valid delivery
and failed-gate rollback. VM86 vm86_state_nmi explicitly checks both masked
retention and unmasked consumption. The Board still owns the actual simultaneous
NMI/IRQ priority, PIC pending/active and exact real/protected/VM86 frame checks.
No private CPU reader or new public pending-state query is restored.

The complete hardware original/current bodies retain four priority contexts and
all five byte arrays exactly. Guest setup/public snapshots replace the former
borrowed CPU cache; original frame/PIC/register predicates remain. After the
packet revision, guard PIC preparation/execution on NMI admission/mask success.
Destroy the already-created real-case Core when bind/freeze/reset preparation
fails. This bounded repair adds nine lines/removes two relative to the reviewed
receiver and changes no production/API, instruction cost or product input.
Hardware plus its three CPU owners pass once per width: x64 1.91s total,
x86 0.19s total.

Strict-C11 failure probing uses the exact final hardware body with only its
terminal main renamed, verified by whole-body equality; it includes the actual
interrupt/VM86/PIC fixtures. The first direct include cannot override nested
main macros and fails compilation, so it is not verification evidence. The
corrected probe covers all 21 construction/bind/freeze/reset/NMI/mask positions
across four contexts. Every failure returns rejection, performs zero subsequent
intercepted PIC/register/memory/execution/diagnosis/snapshot operations and leaves
zero live Core owners. Remove the owned ignored probe body/source/executable
after terminal completion. Refresh manifest/hash and qualify receiver/deletion:
92 to 90 pending of 621. Current complete S94 review/units/gates remain open;
there is no partial P or MyNES rebuild.

### Secondary Integer And System Timing Receivers

Read complete T359 S5/S6 original/current bodies and their actual ownership/
setup changes. All 21 S5 byte arrays and all twelve original S6 arrays are
identical. S5 retains sixteen secondary-integer rows, four attribute rows,
insufficient/sufficient BSR budget and illegal LOCK rejection; each original
exact instruction/elapsed/provider cost remains. Its private FS base seed
becomes the existing public real-mode selector patch. Neutral construction
installs the same three alias ranges through one public operation. Explicit
vector-6 refusal replaces the old private IDTR-limit preflight for the retained
no-retirement negative; copied diagnostics additionally prove the UD cause.

S6 retains ten system/privileged instruction rows and rejected LOCK. Its
former fabricated protected caches become guest LGDT/LMSW/segment loads/far
jump with copied descriptor/base/limit/CPL observations. Two additional setup
arrays implement that transition; the original GDT and every test instruction
remain. Setup time is measured on the existing Core timeline and included in
elapsed/provider totals, while the tested instruction's tick cost is unchanged.
No App/Board dependency remains in these neutral Core receivers.

Remove one redundant unchecked snapshot query from S6's setup-failure report
after revising the packet; its initialized result/snapshot still report the
failure. A strict-C11 probe includes the actual final body. Inject failure at
all six write/patch/run/capture setup positions, leaving failed outputs
unwritten. Each path rejects without subsequent intercepted operations and
without publishing the setup-tick output. Cleanup follows each attempted setup;
remove the owned ignored probe source/executable after terminal completion.
Both normal registered tests pass once per width: x64 2.23s total, x86 0.18s.
Refresh hashes/manifest and qualify four paths: 90 to 86 pending of 621.
Full current review/units/gates and later independent/product delivery remain
open; no production/API, owner INI, MyNES or product executable input changed.

### Execution Cost And String-I/O Timing Reconciliation

The owner asks to inspect slow S execution before continuing. Current no-work
builds of the S4 timing target take 0.414s on x64 and 0.415s on x86; no compiler
or linker runs. Retain the earlier measured strict-query batching and manifest
indexing improvements, eight-way scheduling and affected-target incremental
builds. Complete dual-width units/gates remain mandatory at S closure. These
measurements do not justify another cache, a higher concurrency setting, larger
timeouts or fewer acceptance tests. Actual remaining source/coverage review is
distinct from compilation cost.

Read all 567 original and 542 receiver lines of T359 S4, including the real,
protected/user/VM86 string-port rows, REP continuation/reset, insufficient
budget and failed-port rollback. All 24 original static arrays/timing tables
match the receiver after whitespace normalization. The original main's profile
and scenario selections remain. Replace private register seeds with the checked
public register fixture; its failed read/write terminates instead of continuing
with an unwritten value. Replace borrowed PE/CPL/TSS caches with the already
reviewed guest LGDT/LMSW/LTR/IRETD fixture. Count setup time on the same elapsed
and provider timeline, separately from the unchanged instruction tick costs.
The neutral executor preserves profile-specific reset aliases and the local
port provider; no App/Board owner is imported.

The first CTest selection omitted the registered `unit.` prefix and matched
no tests; it is not validation. Resolve the actual registration and run the
one normal case once per width: x64 passes in 0.21s total, x86 in 0.24s total.
Qualify the deleted original and current receiver: 86 to 84 pending of 621.
No additional source, production/API, manifest, artifact, INI, protected input
or MyNES change is made. S94 and T540 remain open; this is not partial delivery.

### IMUL Immediate Board Failure Boundary

Read the complete original and receiver, both 215 lines, and the previously
qualified descriptor-limit and PIC fixtures. The whole receiver equals the
original after enumerating include relocations, equal public FLAGS constants,
the exact original reserved mask `0xfffc802a`, checked PIC binding and the
single failure guard below. Protected source-limit rollback retains the same
guest program, fault/DF, EIP, EAX, nonparticipant registers, FLAGS and all six
segment snapshots. Register and memory forms retain their guest bytes, result,
interrupt frame, PIC IRR/ISR and no-shadow checks. The shared Board genuinely
owns descriptor loading, memory and PIC integration; this is not a CPU-only
test mislabeled as Board coverage.

The memory form previously wrote its source even when preceding code/vector/
handler setup failed. Revise the active packet first and guard that write with
`!failed && form`. No extra state, helper, public API or production path is
introduced. A strict-C11 temporary probe includes the actual receiver after
preincluding its guarded fixtures. Normal execution makes nine intercepted
writes across the two forms. Inject failure at each of those nine positions;
each rejects with zero subsequent intercepted writes. Each attempted case
destroys its machine through the actual test cleanup. Remove the owned ignored
probe source and executable after terminal completion.

Normal registered tests pass once per width, x64 0.82s total and x86 0.08s total,
with two compile/link steps per width. Refresh the test corpus manifest and its
inventory hash; the complete manifest verifier passes. Qualify two paths:
84 to 82 pending of 621. Full current S94 units/gates, remaining actual review
and later independent/product delivery remain required. No MyNES, owner INI,
firmware/media or production executable input changes; no partial P is delivered.

### Legacy Segment-Stack Board Failure Boundary

Read all 286 original and 286 pre-repair receiver lines. Include relocation,
checked PIC binding and public physical/operational memory reads replace the
old App layout access. Guest addresses are unpaged in these contexts, so the
observations retain their original bytes. All eight static arrays remain exact,
including GDT/bootstrap, invalid selectors, PUSH/POP forms and IRQ frame offsets.
Retain sixteen contexts: null SS, nine target/invalid-selector combinations,
two stack limits and four IRQ forms. Every successful-path register, FLAGS,
segment snapshot, unchanged memory, DF/fault and IRQ/frame predicate remains.

Setup failures previously flowed into execution and copied-value consumers.
Revise S94 first. Chain writes, before-snapshot, expected-fault run, after-
snapshot and diagnosis with short-circuit OR. Remove the now-unnecessary three
local status variables. Guard IRQ writes after failed patch, PIC admission
after failed before-snapshot, and after-snapshot/memory reads after failed run.
The final receiver is 285 lines, one fewer than the original; no helper/state,
production path, public API, guest timing or extra failure report is added.

A strict-C11 temporary probe includes the actual receiver, with its guarded
fixtures preincluded. Intercept create, freeze, reset, patch, memory write/read/
inspection, execution, snapshot and diagnostic operations. Across all four
scenario groups, inject failure at every one of 241 normal producer positions,
leaving failed outputs unwritten. Each case rejects, performs no subsequent
intercepted operation and destroys every successfully created machine. The
actual PIC fixture's native programming/binding calls are not intercepted;
the reviewed caller now reaches them only after successful before-snapshot.
Remove the owned ignored probe source/executable after terminal completion.

Normal cases pass once per width: x64 0.83s total, x86 0.09s total, two build
steps each. Refresh corpus manifest/inventory and qualify the paired deletion
and receiver: 82 to 80 pending of 621. Complete current S94 unit/gate closure
and independent/product delivery remain open; MyNES and protected inputs are
unchanged. This bounded audit is not a partial P or T acceptance.

### Segment MOV Board Failure Boundary

Read all 274 original and pre-repair receiver lines. The migration changes
four include paths and replaces unchecked PIC-source binding with the existing
checked fixture. Retain all seven static arrays exactly and every successful
predicate: six protected selector/load/store limit failures and three SS/DS/FS
IRQ-shadow forms. Guest descriptor loading, preserved segment snapshots and
FLAGS/ESP, first-fault DF, frame IP, target selector and PIC ISR/IRR checks remain.
This receiver still proves genuine Board memory/descriptor/PIC integration,
while independent instruction behavior retains its separate CPU-owned test.

The IRQ body continued after failed register patch and read its frame after
failed execution/snapshot. Revise S94 first, then separate PIC admission into
the next successful stage and chain run/snapshot/frame read with short-circuit
OR. The final receiver is 276 lines, net two more than the original. No new
state, API, helper, guest program, timing claim or production path is added.
The protected-fault body already chains its producers and needs no repair.

A strict-C11 temporary probe includes the actual receiver after preincluding
guarded fixtures. Intercept create/freeze/reset, register patch, memory write/
read, run, snapshot and diagnosis. Inject failure at all 136 normal producer
positions across both groups, leaving failed outputs unwritten. Every failed
attempt rejects, performs zero later intercepted operations and leaves zero
live created machines. PIC fixture calls are not intercepted; the reviewed
stage guard prevents reaching them after failed register patch. Remove the
owned ignored probe source/executable after terminal completion.

Both normal registered cases pass once per width: x64 0.89s total, x86 0.09s
total, two compile/link steps each. Refresh manifest/inventory; qualify the
paired deletion and receiver: 80 to 78 pending of 621. Full current S94 suites,
remaining source review and later independent/product delivery stay open.
MyNES, INIs, protected assets and product executable inputs remain unchanged;
this is not partial delivery or T540 acceptance.

### Xebec Board Wiring Receiver

Read all 319 original and 344 pre-repair receiver lines and its HDC/DMA/PIC
fixtures. All eight DCB/response/sense arrays remain exact. Protocol, geometry,
IRQ5, DMA3, controller count, service delay and media providers retain the same
configuration. The original ten port read/write ownership directions become
public registration-conflict probes, including write-only 323h and absent ATA
1F0h/1F7h. A successfully installed probe is removed before freeze; unexpected
registration/removal status rejects. The receiver explicitly freezes/resets
construction before guest bus operations. Direct private port/RAM accesses
become checked public bus/memory operations; opaque HDC access retains the
same owner-local copied observations and DMA callback binding.

Retain initial selection, pending deadline, two-sector DMA read through RAM,
early terminal count, DMA write/persistence, sense/invalid command, IRQ assert/
read-clear, DRQ mask enable/disable, mask byte, initialize and reset checks.
The migrated DMA transfer fixture follows the existing clocked DMA owner,
observing actual channel register progress instead of borrowing chip storage.

Revise S94 to reject failed binding/phase/admission/source-memory setup before
later commands or transfers. Nine early failure branches use one label at the
already-existing machine/registry cleanup; no extra owner or cleanup helper is
created. Remove the redundant `!failed` condition after the binding guard.
Final receiver: 354 lines, net ten added against carryover and 35 against the
original; most migration growth is the explicit public ten-direction route
probe replacing private port-table reads. No production/API/timing change.

A strict-C11 probe includes the actual final receiver after preincluding its
fixtures. Inject six failures: machine create, registry create/bind/freeze,
DMA destination read and source write. Monitor subsequent configuration, port,
service, transfer and memory calls; every case rejects with zero monitored
operations after failure and zero live created machines/registries. Fixture-
internal operations are not intercepted, and assertion-only phase failures
are reviewed guards rather than claimed injected cases. Remove the owned
ignored probe source/executable after terminal completion.

After the final redundant-condition cleanup, normal tests pass once per width:
x64 0.70s total, x86 0.20s total, two build steps each. Refresh manifest/inventory
and qualify the paired deletion/receiver: 78 to 76 pending of 621. Full current
S94 suites, remaining review and later independent/product delivery remain
required. MyNES, owner INIs, protected inputs and product executable inputs
are unchanged; no partial P or T540 acceptance is delivered.

### 8086 Real-Mode Corpus Receiver

Read all 309 original and pre-repair receiver lines and the already qualified
memory-alias fixture. This is four 8086 contexts, not a cross-generation matrix:
segment override, REP forward/backward storage, INT/IRET and four ordered port
transactions. All eleven static arrays remain exact. Retain reset jump, vector
6 and handler, delivered-UD diagnosis/opcode, halt result, AX/SP/CF, source and
destination byte checks, ordered OUT/IN values and the same run budgets. The
neutral constructor replaces Board construction; the existing mapping helper
supplies the same high and narrower reset aliases. No Board/App dependency
or board wiring is required by these CPU/Core behaviors.

Revise S94 before repairing the old unguarded producer chains. The UD helper
now rejects setup/run/diagnostic/handler failure immediately and publishes its
fault snapshot only after complete success. All four callers chain their
producers before fault/memory/event predicates. REP's existing failure report
uses its already initialized byte buffers. Remove the redundant local status
and accumulated failure from the helper; final receiver is 303 lines, net six
fewer than the original. No new state, helper, public API or production change.

A strict-C11 temporary probe includes the actual receiver. Across all four
groups, inject failure at every one of 45 monitored create/freeze/reset/port-
install/memory/read/run/diagnostic positions, leaving failed outputs unwritten.
Every case rejects with zero later monitored operations and zero live machines.
Reset-alias fixture internals are preincluded and not injected here. Separately
inject all seven producers in the actual UD helper; a sentinel output remains
byte-identical after each failure, no later producer executes, and caller-owned
cleanup destroys the prepared machine. Remove the owned ignored probe source
and executable after terminal completion.

Normal registered cases pass once per width: x64 0.81s total, x86 0.08s total,
two build steps each. Refresh manifest/inventory and qualify the paired deletion
and receiver: 76 to 74 pending of 621. Complete current S94 suites, remaining
review and later independent/product delivery remain required. MyNES, owner
INIs, protected inputs and product executable inputs stay unchanged; no partial
P or T540 acceptance is delivered.

### EGA Registration Wiring And Incremental Cost

Finish the complete 365-line original/360-line receiver reconciliation and
read the unchanged video registration fixture. Its owner-local checks retain
the original null chip after initial allocation failure, chip identity after
failed configuration, configured flag and EGA aperture assertions. The main
receiver retains allocation attempts, provider/observer limits and counts,
overlap priority and decline, freeze rejection, unrelated sentinel routes,
all eight initial CGA failures, EGA/VGA/Compaq allocation failures at the
original 20/27/27 positions, retry and final Compaq route collision. Teardown
uses the explicit machine rather than borrowing it through the adapter.

Build wiring adds the fixture owning the real adapter and main to the one
registered receiver; it does not introduce a second production video path.
The registered case passes once on each width: x64 0.03s (CTest total 1.19s),
x86 0.04s (total 0.09s). Incremental selected-target builds report no work
in 0.410s/0.401s respectively. Qualify both paired paths. The inventory has
73 `pending*` rows (74 before this batch); the receiver previously used a
distinct build-wiring-pending disposition. Across all dispositions containing
`pending`, 98 rows still record remaining work, including final verification
and manifest revision. All 621 recorded file hashes/deletions match; no source
or manifest bytes change in this batch. Earlier prefix-only totals are not
whole-task closure counts.

This renewed performance inspection confirms object reuse, not a new speedup.
Retain the measured strict-matrix batching and CPU-manifest indexing already
recorded above, eight-way scheduling and change-specific incremental checks.
Do not repeat unchanged full suites per source-reading batch, raise timeouts,
trim assertions or build MyNES. Complete current dual-width units/gates remain
mandatory at S closure. Remaining source reconciliation, rather than repeated
compilation, is the dominant unfinished work; S94 and T540 stay open.

### NXVM Video-System Producer Boundary

Read complete original/current CGA 640, CGA graphics and EGA sequencer system
tests. CGA changes only the board header location; preserve both byte programs,
pixel/palette assertions, 640-row addressing and BDA mode/text-return checks.
Neither retained CGA source currently has a build/CTest registration. They
remain pending coverage reconciliation with the actual receivers, not credited
as normal passes or silently removed from the audit universe.

The EGA migration adds an unused bus fixture. Remove it and repair the original
unchecked reset, accumulated producer failures and construction early return.
Use the existing cleanup and one short-circuit chain; no new state/API/helper.
Retain reset sequencer values, index/data writes, the 0x0e register mask and
aperture write/read equality. The source remains the same one registered test;
against S92 it adds 12/removes 10 lines (git diff --numstat), including the
header migration. No production or product executable input changes.

A temporary strict-C11 probe includes the actual receiver with controlled
producer responses. Nine injected failures (create, reset, three port reads,
two port writes and two memory operations) reject with zero later operations
and one destruction. Three invalid construction results (null session,
inactive session, absent Core) reject before reset and destroy any returned
session. Failed output producers leave their outputs unwritten. This proves
test control/cleanup, not simulated hardware semantics. Remove both owned
ignored probe files after completion.

The real registered test passes once on x64 (1.10s, total 2.23s) and x86
(0.08s, total 0.11s), with two incremental compile/link steps each. Qualify
only the EGA path: prefix-pending 73 to 72 of 621. Keep both CGA sources
pending. MyNES, INIs, firmware/media and executable inputs are unchanged.
Complete remaining review and full current dual-width unit/gate closure are
still required; no partial P or T540 acceptance is delivered.

### Retained CGA Guest And BIOS Coverage Registration

Reconcile the two dormant system sources against the registered 640/320 port
receivers and CGA DOS integration. Port tests prove adapter memory/palette
behavior but do not execute the original guest instructions. DOS integration
proves 320 pixel order, not the original BIOS mode 06h/BDA/text return. The
T500 inventory originally classified both sources retain-route-to-unit;
later firmware cutover made the BIOS-dependent synthetic unit setup invalid.
Do not treat dormant source or nearby predicates as equivalent coverage.

Register vm-cga-graphics-system-smoke as unit with the original 8086 entry plan,
program, even/odd row pixels and palette-1 assertion. A failed snapshot capture
now rejects immediately; a successful non-graphics snapshot still waits. An
actual-source strict-C11 temporary probe rejects all four failed producers
(construction, entry plan, run, capture), leaves failed outputs unwritten,
performs no subsequent producer and destroys the returned session. Its normal
control response reaches all original success predicates. Remove the ignored
probe source/executable after completion.

Move the 640 source to integration/dos under the same target name. It loads
the real build-selected default-profile firmware through the existing INI
integration composition, not a unit-generated BIOS or injected host service.
The original boot byte tokens are identical; preserve 500000 containment,
640 pixel values, black/white palette, BDA 0449h values 06h/03h and text return.
The original unit requested 8086; this firmware test instead uses the INI's
fixed 386 product and makes no new 8086 BIOS-composition claim. CPU-independent
video assertions remain identical. Its only media modification replaces sector
zero in the existing discard-only overlay; the external file remains read-only.
Use the existing waiting/deadline helper if BIOS enters HLT. Register the
integration only for the default product, matching its actual firmware role.

Normal x64 unit/integration pass (0.42s/3.11s, total 3.51s) and x86 pass
(0.14s/1.83s, total 2.08s), with no skip. After the snapshot-failure repair,
rerun only the changed x64 unit: 0.24s, total 0.31s. The unaffected integration
result is reused; it is not run multiple times. Original 640 source is 83 lines,
receiver 92; 320 source remains 80 lines. This batch adds nine CMake registration
lines and nine net test lines versus its read carryover. No new production/API,
framework, executable input or Shared test corpus changes.

Expand the frozen path inventory only for the receiving integration source:
621 to 622, deletions 161 to 162. Qualify both original paths and the new receiver:
prefix-pending 72 to 70. CMake whole-file review remains pending despite these
verified registrations. Complete S94 review/units/gates and later independent,
product and delivery acceptance remain required. MyNES and owner inputs remain
unchanged; no partial P or T540 closure is claimed.

### Terminal Fault And Provider Composition Failure Boundary

Read both complete original/current sources and their full baseline diffs.
The fault runner retains guest LIDT then invalid opcode, terminal #UD mask and
detail, faulted lifecycle/run, and reset clearing the outcome/diagnostic and
returning Core to stopped. Replace accumulated independent observations with
two checked short-circuit chains and check reset status. Keep existing cleanup;
the source is 74 rather than 76 lines, with no new helper/API or production change.

The 123-line mantle fixture is not NXVM-specific: it composes the Shared board,
Core execution provider, RTC and media registry without profile, host or asset
input. Move it to ibmpc-common/machine_provider_composition_smoke.c and retain
the target/marker. Remove App source/registration; register it once with
x86-ibmpc-common and x86-core in the Shared test corpus. Initialize run result
for the existing error report and guard later bindings, freezes, reset,
entry plan, run and media query after earlier failure. Preserve stage bits,
8086 HLT/IF, copied media presence, config values and destructor order; source
line count stays 123. CMake moves three App lines to six Shared registration
lines. Relative to read carryover, test net -2 and registration net +3; the
one extra line makes the test independent rather than adding a production layer.

Actual-source temporary probes verify twelve fail-capable fault-runner positions
(the void start is not injected) with failed outputs left unwritten; each rejects,
performs no later monitored operation and destroys a returned session. The
expected faulted-run response remains INTERNAL_ERROR with a populated fault
result; injection uses a mismatching status rather than pretending that expected
error is success. The fixture's normal control responses are test-flow evidence,
not real CPU fault emulation. An earlier probe compile failed due to local naming
collisions and incorrectly shifting the already-encoded #UD mask; correct the
probe, not production, before its passing run.

The composition probe instead wraps real constructors/destructors and source
operations. All eleven failures reject with no later monitored operations and
zero live top-level Core/RTC/registry owners. Internal constructor callbacks are
not separately injected. An initial PowerShell linker-argument parse failure is
not a test result; the correctly quoted strict-C11 compile and probe pass.
Remove all owned ignored probe sources/executables after completion.

Real registered cases pass once per root width: x64 2/2, total 2.39s; x86 2/2,
total 0.30s. The composition receiver also builds and passes independently in
the existing x64 tools-off test/x86 tree, 0.46s (total 0.81s), with no App source
or NXVM target dependency. This is not whole independent-corpus acceptance.
Refresh test/x86 manifest and inventory: 623 paths/163 deletions, prefix-pending
70 to 68. Both entire CMake files still await whole-file qualification. No
MyNES, INI, firmware/media or product executable input changes. Complete S94
review/units/gates and S95-S97 acceptance still remain; no partial P is delivered.

### Renewed Scheduling Check And Default PC/AT Apply Review

The owner asks to optimize S execution before continuing. Existing local x64
and x86 CMake caches had PROJECT_UNIT_TEST_JOBS=12 and 4 respectively, unlike
the accepted explicit eight-way closure policy. Configure both existing trees
with PROJECT_UNIT_TEST_JOBS=8; do not alter registration, timeout, coverage or
product inputs. Configuration succeeds in 4.14s/4.11s. This aligns the aggregate
route with direct CTest commands; it is not a measured full-suite speedup.
Retain affected-target Ninja builds and unchanged-input result reuse, with
complete units at S closure. Do not repeat whole suites per source-reading batch
or build MyNES. No-work Ninja and the unchanged default apply test together
complete in 2.30s, with CTest runtime 1.37s. The preceding affected fixture
incremental compile/link took 1.66s. Compilation is not the dominant remaining
cost; full original/receiver reconciliation is still pending across the ledger.

Resume that reconciliation with the complete default PC/AT apply source,
original source and diff, and composition/CMOS/time fixtures. Construction
failure previously entered a report dereferencing a null session; the 80186
poll loop could treat failed port reads as a successful exit and continue guest
setup. Separate construction from observations, copy the CMOS type once and
use one format-helper cleanup. Check failed polling reads and destroy returned
owners on construction errors. Preserve four media formats, checksum, refresh
edge, 80186 guest bytes and original marker; no production/API changes.

Registered normal cases pass once per width after affected-target incremental
builds: x64 1.15s (CTest total 2.30s), x86 0.11s (total 0.24s). Refresh the
inventory hash but keep this receiver pending until failed-producer proof is
complete. Current full-suite, independent, artifact and boot acceptance remain
open. These test-only changes do not require a new product EXE. MyNES scoped
status remains empty; no partial commit, push or S94 closure is claimed.

The default-apply actual-source controlled-producer probe now passes 18 checks:
four fail-capable positions in the format helper, seven in 80186 polling,
successful-null/error-null construction for both helpers and main, and main
construction error with a returned owner. Failed reads/run leave outputs
unwritten. Every injected error rejects before later monitored status calls or
CMOS/composition observations; returned owners are destroyed once. The probe
uses synthetic control responses, not real construction/CPU execution; the
registered dual-width cases above provide normal-path evidence. Remove its
owned ignored source/executable. Qualify the receiver, leaving 67 prefix-pending
rows out of 623; other final-verification dispositions still prevent closure.

### Shared Board Entry-Plan Receiver

Read the complete 107-line original, carryover and baseline diff, then the
complete receiving body. The test depends only on Shared board composition
and Core APIs, not NXVM profiles or assets. Move it from App devices to
ibmpc-common/machine_entry_plan_smoke.c, preserving constructor semantics,
target, marker, configuration, guest bytes and all assertions. Core-independent
construction is not substituted for the original board composition.

Replace fifteen accumulated-failure assignments with short-circuit assignments:
later API calls and unwritten state/result observations do not execute after
failure. Keep ROM route entry, running-state rejection, invalid ROM preload
rollback, duplicate-preload rejection, RAM entry, HLT and reset-state checks.
No line-count growth, helper, public API or production change. Remove five App
definition/list lines and add six Shared registration lines; compile flags and
unit registration remain effective through the existing Shared entry.

The actual-source probe wraps real constructors and API functions and injects
an unexpected IO_ERROR at each of nineteen normal call positions, including
the three intentional entry-plan error responses. Each rejects immediately,
makes no later monitored calls and destroys the returned Core. Failed state,
memory and run operations leave outputs unwritten. The normal real run also
passes. Remove the owned ignored probe source/executable.

Registered tests pass once per width: x64 0.73s (CTest total 1.34s), x86 0.08s
(total 0.21s). The receiver independently builds and passes in the existing
x64 tools-off Shared tree, 0.58s (total 1.23s), without an App library. Update
test/x86 manifest; complete verification passes. Inventory expands to 624 paths,
164 deletions, with 66 prefix-pending paths after qualifying both original and
receiver. Whole CMake reviews and other final verification remain pending;
these focused proofs do not close S94 or T540. MyNES scoped status is empty.

### Arbitration And D4 Refresh Producer Boundaries

Read the complete 104-line original/current arbitration body and baseline diff.
Move this App-independent board/Core trace test to ibmpc-common, retaining the
target, marker and original construction. Its fixture include becomes adjacent;
short-circuit subsequent operations/observations after failed producers. Keep
the 3-tick retirement, exactly one DMA/PIT/PIC group at tick 3, CPU retirement,
last run-boundary event and reset timeline checks. Source line count stays 104;
replace four App definition/list lines with seven Shared registration lines,
using the existing observable Core/Board libraries, not a new trace path.

Both registered root cases pass once: x64 1.02s (total 2.06s), x86 0.09s (total
0.22s). The actual-source probe wraps real functions and injects errors at ten
call positions. Failed run/timeline outputs remain unwritten; every error
prevents later monitored operations and leaves zero live Core owners. The
receiver also independently builds/passes in x64 tools-off Shared, 1.03s
(total 2.05s). Refresh manifest, add its receiver to the inventory and qualify
the original deletion/receiver; whole CMake qualification remains pending.

Read the full original/current D4 refresh source in two complete chunks and
its full semantic diff; inspect the Model 40 refresh fixture and the relevant
Core/Board/time owner fixtures. Retain the Model 40 test in App. The D4 fixture
checks the same pending/address and reset pulse fields now owned by D4; Core
fixtures retain A20, DMA wait and the exact attachment refresh callback.
Existing raw DMA programming becomes checked Core bus writes with unchanged
ports/values. No owner state is mirrored or a public private-state getter added.

Short-circuit accumulated failures and guard subsequent DMA programming,
request assertions and A20 port writes. Preserve 19/11 tick BUSRDY boundaries,
pending/address transition, unchanged-address rejection, compatibility deadline
classification, transferred byte, refresh HOLD request/ack/transaction/release
before DMA, no intervening CPU transaction, reset clear and non-D4 contrast.
Both normal cases pass once: x64 1.04s (total 2.11s), x86 0.08s (total 0.11s).
Its actual-source probe wraps real creation/attach/binding/reset/timing/read
functions: 22 error positions all reject, perform no later monitored status or
port/request/state observation and destroy returned Core owners. Failed outputs
are left unwritten. Remove both owned temporary probe sources/executables.

Inventory now has 625 paths, 165 deletions and 64 prefix-pending dispositions;
all recorded current hashes/deletions agree. MyNES and product executable inputs
remain unchanged. Independent whole-corpus, complete units/gates, artifact/boot
and delivery verification still remain. No partial P or S94/T540 closure.

### DMA Competition And HOLD Failure Boundaries

Read complete original/current competition bodies and semantic diff. Retain
this D4-dependent test in App. Short-circuit failed producers and guard later
programming/request operations; preserve CPU/DMA/PIT/PIC ordering, BUSRDY,
wait-state, transferred-byte and reset checks. The shared HOLD fixture now
short-circuits acknowledgement/transaction start after failure, retaining its
unconditional release cleanup. Its two production-test callers are both in
this competition test; no production implementation or API changes.

Actual-source probes reject all 30 competition producer failures and all five
HOLD helper failures, with no later monitored operations and correct cleanup.
Remove the two owned probe sources/executables. Competition and D4 regression
cases pass once per width: x64 2/2 in 3.25s, x86 2/2 in 0.17s. Read and qualify
the complete negative-verifier diff: its two keyboard edge probes use the
canonical AT header and still reject forbidden chip dependencies. Both root
invocations pass. Refresh the fixture manifest identity. Inventory retains
625 paths/165 deletions, with 62 prefix-pending rows; this is not whole-task
completion, because qualified rows may still require final verification.

### Renewed Build And Verification Performance Check

The owner requested performance optimization before further S94 execution.
Both retained root caches now use PROJECT_UNIT_TEST_JOBS=8 (previously 12/4).
This is ignored local configuration, not a new project default or a measured
full-suite speedup. Keep eight-way execution: higher concurrency previously
caused timeouts. Do not change test coverage or timeout budgets.

Current selected-target no-op Ninja measurements are 0.158s x64 and 0.191s
x86; both report no work. Incremental objects already work correctly. Historical
CTest cost data identifies firmware-floppy and CPU decoder cases as long unit
cases, but its accumulated estimates are not new benchmarks. Retain previous
strict-matrix batching (36.14s to 4.91s) and CPU manifest indexing (4.388s to
2.012s). Use changed-target builds and transient relevant checks while reading
source; repeat full current dual-width units/gates at S closure, not after
every review batch. Existing ccache availability alone proves no benefit; no
compiler launcher or new cache framework is added. Whole-source reconciliation
remains the principal unfinished work. MyNES is neither rebuilt nor changed;
S94/T540 remain open, with no partial P.

### No-Media BIOS Coverage Restoration

Read the full original/current no-media video source and baseline diff, its
synthetic session fixture, INI integration support and FDD/HDD remove owners.
The old source was not registered, and its current unit constructor maps only
a synthetic HLT ROM. Its original BIOS text/INT10 checkpoints therefore require
the real Default firmware. Move it to integration/dos and restore one Default
INI integration target. Before reset, the existing transform removes all
session-owned floppy/fixed-disk media; no asset master or INI changes.

Preserve the 100000-instruction budget, INT10 functions, absence of INT F2,
keyboard-wait opcode, Invalid boot disk text, BDA cursor 0600 and visible cursor
at (0,6). Guard null/inactive construction and reset failures; failed capture,
memory, register or run operations immediately close the session rather than
continuing with unwritten values. Check the text pointer before length. No
production change, helper API or replacement ROM is introduced.

Final source passes once per width: x64 0.24s (CTest total 0.32s), x86 0.09s
(total 0.12s). Actual BIOS marker reports INT10=21 F2=0 CURSOR=0600 AH=00020E.
An actual-source probe injects nine failed producer classes, leaves failed
outputs unwritten and verifies no later monitored calls. The seven post-open
failures each close once; media-transform failures use existing open rollback.
Both null-text helper cases reject safely. Its initial harness-only extra
snapshot exceeded stack capacity; use static probe storage, not a product
change, then obtain the passing result. Remove the owned probe source/EXE.

Inventory now has 626 paths, 166 deletions and 61 prefix-pending rows. Whole
CMake review/final verification remain pending; this restored checkpoint does
not prove complete units or the eight profile boot groups. MyNES and executable
inputs stay unchanged; no artifact rebuild or partial S94 commit.

The complete root registration gate initially fails with three extra actual
routes: core-mantle-shape, entry-plan and arbitration. Prior migration retained
Shared registration but omitted these targets from the root aggregate inventory.
Restore all three inventory entries, using the existing Shared-route detection
instead of registering duplicate tests. Both root gates now pass with 335
canonical routes (the separate single boot row remains accounted independently);
the INI integration boundary gate passes on both widths. No verifier relaxation.

### Direct Profile Plan Reconciliation

Read the original/current IBM5170 direct-plan test and its one-line header
migration. All original CPU, RAM, cascaded PIC, DMA count, time axis, route count,
display/DMA/RTC topology and firmware/media policy assertions remain. Preserve
1536 KiB admission, 1024 KiB/4 MiB rejection, CMOS base-memory, Model40 2 MiB/386
and Default 32 MiB CPU/FPU/floppy choices. Plan setters remain status-gated and
destroy the returned plan on every exit. Keep this product-specific matrix in
App. Both actual cases pass: x64 0.23s (total 0.34s), x86 0.08s (total 0.15s).
No new source edit or artifact input change. Qualify this path, leaving 60
prefix-pending rows; the full task's final-verification qualifiers remain open.

### Model339 Clock And Keyboard Contract Reconciliation

Read complete original/current Model339 clock source and semantic diff, copied
plan/board fixture bodies and both AT/board keyboard-state fixtures. Retain every
descriptor/plan/board DMA/PIT/RTC/video ratio, phase, memory, timing-rule,
disposition and macro-pacing/physical-unavailable assertion. The AT-owned cadence
fixture retains the original direct keyboard make, initial-1/no-repeat, +1/1Ch,
repeat deadline and break checks. The App test additionally traverses real KBC
serial translation to 1Eh, OBF and the 4000000/1/800000 tick checkpoints. No
timing-grade change or production state getter. Keep the profile test in App.

Short-circuit accumulated failures in both test and cadence fixture and guard
the two later void KBC advances. Preserve original checks and markers; no line
growth in this repair. Caller sweep finds only this App test consumes the
board cadence wrapper. Normal registered tests pass x64 0.50s (total 0.97s),
x86 0.07s (total 0.10s). Actual-source real-object probes inject errors at all
16 App producer positions and seven owner-local cadence positions; they reject,
make no subsequent monitored calls, leave failed outputs unwritten and clean
up owners. The App constructor probe returns an owner even on its error to
verify destruction. Remove both probes after completion, update the fixture
manifest and qualify this App path. Inventory remains 626 paths/166 deletions,
with 59 prefix-pending rows; final verification remains separate and pending.

### Runner Error And Removed KBC Fixture Reconciliation

Read complete original/current runner-error test, its complete diff, Common
state-wait fixture and the two Core mutation functions. Preserve both ERROR
paths: non-fault invalid lifecycle and active-reset firmware failure. The
firmware injection now waits for PAUSED before replacement, then resumes before
reset, eliminating a running-thread data race. Core fixture functions mutate
only the original lifecycle/provider facts; no production debug API is added.
All status/state waits reject through cleanup; executor destruction precedes
waiter destruction so its callback context remains live. Actual real-thread
cases pass x64 1.20s (total 2.41s), x86 0.09s (total 0.11s). No additional edit.

Read both original deleted App KBC fixture headers. The device fixture's
temporary port-owner wrapper has the real neutral executor/AT adapter receiver;
its controller and serial consumers retain already recorded full assertion
reviews. The machine fixture's bounded eight-attempt reply and masked command
helpers live in the AT command fixture, with checked Core bus access and the
same configured deadline advance. Board wrappers feed the three original
profile consumers. Their remaining body audits stay independently inventoried;
qualifying header removal does not qualify those App tests. No old include or
duplicate helper remains in App. Qualify both deletions and the runner path;
inventory retains 626 paths/166 deletions, with 56 prefix-pending dispositions.

### Model40 CMOS Seed Producer Boundary

Read complete original/current seed test and semantic diff. The original
port mechanics are now test-only Core-owned fixture calls with the same 70h/71h
addresses and index/data sequence; original in-code ROM/CMOS inputs remain,
without external assets. Preserve default floppy/fixed/equipment/base/extended
memory fields, checksum, independent second session, reset-retained 5Ah equipment,
and custom seed's 33h/checksum bytes. The custom 33h value is a unit sentinel,
not a replacement production CMOS seed or a newly asserted hardware meaning.

Short-circuit later constructors after failure and reject a successful null
custom-seed object; all returned owners still reach teardown. Seven exact
constructor/reset error or null-output cases in the actual-source probe reject,
perform no later monitored port/producer calls and leave zero live owners.
Errors deliberately return real allocated owners to exercise cleanup. Final
normal registered tests pass x64 0.76s (total 1.53s), x86 0.08s (total 0.10s).
Repair adds one continuation line for readable null checking, no helper or
production/API change. Remove the owned probe. Inventory has 55 prefix-pending
rows, with all 626 hashes/deletions current; complete S94 closure remains open.

### Instance Ownership And Producer Boundaries

Read both complete original/current instance-isolation and PCAT-ownership
sources, their semantic diffs and the owner-local isolation fixtures. Preserve
Core/CPU/memory/port and RTC/FDC/HDC independence, separate RAM/EAX values,
first-only read watchpoint and independent FDC DMA tokens. These remain App
composition tests, not a revival of multi-session product UI. The PCAT test
retains port70 NMI mask/unmask and replaces the old private connection-pointer
assertion with real FDC command/result locality, including the surviving
composition after its peer is destroyed. No production state getter is added.

Reject successful null constructors, short-circuit producers after failure and
destroy both returned owners on every exit. In actual-source real-object probes,
all 15 isolation and 23 PCAT producer/fixture failure positions reject without
later monitored calls or live owners. Both tests also reject two successful-null
constructor positions each; constructor errors intentionally return allocated
objects to prove cleanup. Remove the temporary probe source and executable.

Current normal registered tests pass once per width, two of two in 0.09s for
both x64 and x86. Unchanged incremental builds report no work in 0.41s/0.42s;
this is not a cold-build or full-suite benchmark. Qualify both paths, leaving
53 prefix-pending rows in the 626-path inventory. Full S94 verification and
remaining source dispositions stay open; no partial P, MyNES, INI, asset-master
or product executable change is made by this batch.

### Four-Profile DMA Deadline Producer Reconciliation

Read complete original/current timing-qualification source and semantic diff.
Retain each Default, Model339, Model40 and XT materialization, nonzero DMA clock,
controller/cascade/FDC-channel wiring, nonzero request token, provider freeze,
reset and valid next-deadline/progress disposition assertion. Core owns time;
the board-owned fixture asserts the existing DREQ, with no second scheduler or
new timing classification. This App profile-assembly matrix remains in App.

The full-source audit finds two failed-producer defects: an observation error
still prints fields the producer has not written, and the port fixture exits
instead of reaching object teardown on write failure. Use the existing checked
Core bus operation before DREQ, print observation fields only on success and
send failed/null plan/Core construction through cleanup. Remove the unused bus
fixture include. No production, public API or Shared fixture changes.

Actual-source probes use real plan/Core objects for all four profiles. Each
profile rejects seven producer failures plus null plan, null Core and null board
success outputs: 28 failure positions and twelve null cases total. Errors may
return allocated owners; every case performs zero later monitored calls and
leaves zero live objects. Observation errors leave outputs unwritten. Remove
the owned temporary probe source/EXE after the passing checks.

Normal registered cases pass x64 in 1.04s (CTest total 2.16s) and x86 in 0.08s
(total 0.11s). Qualify the receiver, leaving 52 prefix-pending dispositions;
the complete task's full suites and final verification remain required.

### Deleted Port Owner Fixture Reconciliation

Re-read the complete original shallow port-owner header and both original
callers: the App DMA and KBC initialization fixtures. Read both complete Shared
receivers again and check their recorded, hash-current qualifications. Each now
constructs a real neutral Core owner and uses its registration/freeze/reset
and checked bus access rather than copying a port table through a temporary
synthetic Core. The DMA transfer/register loop remains accounted separately.
No current App or Shared call uses the retired open/close helper.

This supplies the missing caller reconciliation named by the earlier deletion
review. Qualify that header deletion; individual consumer bodies and final
verification retain their own dispositions. Inventory now has 51 prefix-pending
rows, not complete S94 acceptance. No source, test input or artifact changes
in this deletion-review step justify rerunning unchanged passing suites.

### Model40 Private Composition And KBC Reconciliation

Read complete original/current Model40 composition source and its full semantic
diff, the in-code ROM/CMOS construction fixture and the receiving CMOS/KBC
helpers. Preserve missing-ROM rejection, deterministic retirement, page-locality
and access-wait windows, BUSRDY/prefetch/overlap policy, all clock ratios, 386/2MiB
configuration, four CMOS memory bytes, D4 registers, port61 speaker selection,
reset-ROM byte and plan ownership. The original one-instruction result remains
one retirement/three ticks. Mouse submission must leave OBF clear; A8 cannot
enable AUX, F5 returns FA, D4/F4 cannot enable keyboard scanning or create OBF,
and EE still returns EE. All five original success markers remain.

Migration introduced repeated immutable transaction/board/plan captures; capture
each once after successful construction instead. The inherited accumulated
failure flags also allowed later keyboard producers to run after a failed
observation/input/reply. Replace the unchecked post-run port fixture calls with
existing checked Core bus operations, then short-circuit the whole ordered
sequence. No new helper/API, production path, timing grade or expected byte.
The unused Core bus-fixture include is removed.

Actual-source real-object probe rejects all 35 monitored producer/fixture result
positions and a successful-null Model40 constructor, with zero later monitored
calls and zero live objects. The failed successful-construction probe deliberately
returns an allocated owner so teardown is exercised. Remove the temporary probe.
Normal registered tests pass x64 in 0.81s (total 1.58s) and x86 in 0.08s (total
0.10s). Qualify this App test, leaving 50 prefix-pending inventory rows; remaining
source/final verification dispositions and S94 closure stay open. MyNES, INIs,
asset masters and product executable inputs are unchanged in this batch.

### Model40 HDC Memory-Media Reconciliation

Read complete original/current HDC test and its full semantic diff, original
service helper and current board/controller/HDC fixtures. Retain the in-code
925-cylinder/five-head/17-sector/512-byte image, 5AA5h first word, empty slave,
zero explicit service delays and both 20h/A0h drive-head encodings. Each read
still checks alternate-status DRQ with IRQ retained, primary status IRQ clear,
all 256 data words, completion IRQ, rejected IDENTIFY/ERR and SRST diagnostic
success. The migrated service fixture calls the original next-due/advance path;
this owner-local protocol unit does not claim whole-machine pacing coverage.

Replace the exiting bus fixture with existing checked Core bus reads/writes.
Programming and every data-word read stop on error, return to main and release
session/image; no second helper, production API or controller change. Existing
short-circuit media construction/replacement/geometry checks remain. The complete
HEAD-to-current migration adds 32/removes 36 lines, net minus four; the companion
private-composition migration adds 78/removes 72, net plus six.

The actual-source real-object probe injects failures or opposite IRQ results at
all 553 monitored positions across both sector paths. Each rejects with zero
later monitored calls and zero live session/image objects. Constructor errors
return allocated owners; a successful-null owner and initial image allocation
failure also reject/clean up. Remove the temporary probe source and executable.
Normal tests pass x64 in 0.70s (CTest total 1.39s), x86 in 0.11s (total 0.14s).
Qualify this source; inventory retains 626 paths with 49 prefix-pending rows.
Full S94 source reconciliation and final suites remain open; no partial P,
MyNES, owner INI, external asset or product executable input change.

### Build Launch Cost Versus Audit Cost

On the renewed owner performance request, inspect the retained verifier
repairs and both local caches rather than repeat unchanged full benchmarks.
Strict-matrix command batching and CPU-manifest indexing retain their recorded
36.14s to 4.91s and 4.388s to 2.012s measurements. Both caches still select
eight unit jobs; the earlier sixteen-job experiment timed out and is not a
speedup. Unchanged-input x64 FDC target construction reports no work in 0.211s.

The changed x86 FDC target launch inside the sandbox produced no output and
left only its Ninja process, with no compiler child. Verify that exact process
command line before terminating it. The same approved target outside the
sandbox completes its four compile/link steps, and its registered test passes
once in 0.26s (CTest total 0.29s). This distinguishes this launch stall from
compiler throughput; it does not establish a general sandbox root cause.
No process remains from the diagnostic run.

Continue the current FDC source review using changed-target builds and
temporary focused checks through the verified native execution route. Its
normal x86 pass does not qualify the still-pending failure-path review or
refresh its ledger hash. Full dual-width closure suites remain mandatory;
do not repeatedly run them after source-only reading. No new cache layer,
timeout relaxation, test removal, MyNES rebuild or partial commit is made.

### Model40 FDC Protocol And FDD Geometry Reconciliation

Read the complete original/current sources, complete actual diffs and the
owner-local controller/composition/media fixtures. FDC retains all four drive
types, four media types, three rates and eight physical positions (384 contexts,
768 channel samples), including disabled recording and double-step translation.
Keep IRQ6/DMA2, READY/installed/track-zero wiring, the 8MHz conversion input,
KBC B4h and all six CMOS values. Keep both four-entry reset SENSE queues,
ST3=38h, every PIO sector byte, DMA's 512 transfers and destination byte,
terminal result observations before/after reset, out-of-range sector failure,
empty-media reset/recovery, all ten commands at three wrong rates and valid
READ ID. The in-code fixture bytes and all five success markers are unchanged.

Command, result and DMA-programming helpers now return checked Core bus/tick
failures to main's sole teardown. Check reset before consuming retained state.
Stop after an earlier assertion instead of accumulating failure while changing
the device further. CMOS values and reset results become tables/loops, without
removing contexts. The existing void DMA-transfer fixture remains owner-local;
this review does not claim new status propagation inside that fixture.

The temporary probe includes the actual source and links real compiled owners.
All 3,501 monitored failure/opposite-result positions, including 768 channel
samples, reject with no later monitored operation and zero live session owners;
a successful-null constructor also rejects before media access. Errors leave
bus/memory outputs unwritten. Remove both temporary probe files. Normal
registered FDC tests pass x64 in 0.81s (total 1.47s, preceding batch) and x86
in 0.26s (total 0.29s). HEAD-to-current test diff is +135/-141, net minus six.

FDD retains native 80/2/15/512 geometry, truncated-image rejection, media-registry
publication, reset/media retention, a 40-cylinder 360KB compatibility image
in the 1.2MB drive, and Default/5170 sector-count comparisons. Capture its
immutable FDC config once; check reset's result and the two previously unchecked
successful-null comparison constructors. Keep the existing four-owner cleanup.
The actual-source probe rejects all eleven monitored failed/opposite results
and each of the four successful-null constructors, with zero live owners and
no later monitored call. Remove its temporary source/executable. Normal cases
pass x64 in 0.85s (total 1.62s), x86 in 0.10s (total 0.12s). This complete
migration diff is +13/-21, net minus eight.

Qualify these two App paths and refresh their exact hashes. Inventory retains
626 paths/166 deletions; prefix-pending falls from 49 to 47. This does not
complete S94 or its required final dual-width suites. No production, public
API, chip timing, Shared fixture, owner INI, external master or MyNES change;
no partial P is delivered.

### Default And 5170 Profile Contract Reconciliation

Read both complete original/current bodies and actual diffs. Default's sole
change is its board interface include: every ROM reset/map, CPU/FPU, clock
ratio, controller classification, typematic/CGA, CMOS, route/service count,
descriptor validity, contract equality, invalid CPU/FPU rejection and CMOS/FDC/
memory-control/AUX port/route assertion remains unchanged. Keep pure read-only
lookups in the existing form; a temporary actual-source probe flips or nulls
each of thirteen descriptor/contract/lookup/materialization results, and every
case rejects. This does not claim a side-effectful transaction or allocation.

5170 retains the Rev3 firmware slot, native-drive/no-upgrade and CMOS 20h
facts, IRQ6/DMA2 route, DOR write/MSR read/data write endpoints and frozen board
binding. Stop failed/null construction before route lookup and capture the
immutable composition once instead of twice. Seven failed/opposite-result
producer/port/snapshot positions plus one successful-null owner reject, with
zero later monitored calls and zero live sessions; constructor errors return
allocated real owners so the teardown proof is not vacuous. Remove both
temporary probe sources/executables.

Registered cases pass together x64 2/2 in 1.04s and x86 2/2 in 0.17s. Complete
HEAD-to-current diffs are Default +1/-1 and 5170 +14/-11 (net plus three, from
explicit teardown sequencing and one local copied observation, not new API or
state). Qualify both rows with exact hashes: 626 paths/166 deletions remain,
prefix-pending falls from 47 to 45. No production, Shared fixture, profile
semantics, INI, external master or MyNES change. Full S94 reconciliation and
closure suites remain open; no partial P.

### Model40 D4 Mapping, Parity And BYOB Reconciliation

Read all three complete original/current bodies and their complete migration
diffs, Model40 ROM fixture/plan declarations and the Core composition/time
helpers. D4 map retains the even/odd byte pairs at every low/compatibility/high/
reset alias, open F40000h/F80000h reads, relocated FA/FB/FC/FD/FE/FF RAM,
unpopulated B0000h versus the selected aperture, FBFFFFh word seam, immutable
ROM writes, low/1MB A20 isolation, spare DMA page latches and all five D4
register/reset facts. D4 parity retains real Core parity storage and bit-flip
injection, port61 source enable/CMOS masking, IOCHK latch, unmask-to-NMI,
clear-by-other-memory-write, D4 acknowledgement and reset. Model40's D4 owner
replaces the former whole-board observer; the parity fixture performs the same
Core-owned injection without exposing private memory storage to App tests.

Both CHECK macros previously continued changing devices after a failed step.
Return immediately to their existing single teardown, retaining diagnostics,
step identities and every successful-path assertion. Map also replaces the
literal A20 true value with LIB_TRUE. Temporary actual-source probes inject
failure at all 73 map and 20 parity monitored positions, plus a successful-null
constructor for each. They reject with zero later monitored calls and zero
live sessions; errors leave memory/bus/observation outputs unwritten.

BYOB retains the two copied BIOS chips, optional video header/body aliases,
2MB/386/no-FPU plan and CR-MOV behavior, deterministic retirement/bounded
compatibility policy, macro 16MHz pacing with physical time unavailable,
FDD geometry, reset vector, immutable memory reconfiguration, one instruction
retirement, reset/one-tick refresh count 18 and caller-byte copy isolation.
Keep explicit-memory construction rejection and all five success markers.
Capture the immutable plan and ROM contract once; reject invalid/null results
before consumers. Replace exiting refresh-port wrappers with short-circuit
checked Core bus operations. Failed construction can no longer fall through
to session dereferences; any unexpected owner returned by the final rejected
construction now reaches cleanup.

The BYOB actual-source probe rejects all twenty monitored failed/null/opposite
results and a successful-null initial owner, with zero later monitored calls
and zero live sessions. A separate rejected-construction result deliberately
returns a real allocated owner and still cleans it up. The temporary probe's
first BYOB link omitted its existing time-fixture object; adding that actual
target object corrects the probe, without repository or production changes.
Remove the probe source and all three executables.

Normal registered D4 map/parity/BYOB cases pass x64 3/3 in 3.15s and x86 3/3
in 0.23s. After BYOB's last guard-order change, rebuild and rerun that changed
case: x64 passes in 1.70s total, x86 in 0.10s. Complete HEAD-to-current diffs
are map +5/-4, parity +11/-12 and BYOB +51/-51, combined net zero; no new
production API, fixture abstraction or timing classification. Qualify all
three exact source hashes. Inventory retains 626 paths/166 deletions,
prefix-pending falls from 45 to 42. No MyNES, external master, INI or product
artifact change; full S94 reconciliation/closure suites remain open, no partial P.

### Initialization Materialization And Honest Coverage

Read the complete original and current initialization test, its complete diff,
the composition fixture declarations and the in-code ROM constructor. Default
and 32MB/386/387 overrides retain memory/CPU/FPU, instruction/transaction/clock/
time-axis, all four KBC timing values and complete controller timing checks.
Invalid second-media slot, recovery, null constructor output, option-ROM
length/capacity/header/checksum rejection remain unchanged.

Capture each immutable plan once rather than three times. Check construction
and null plans before capture; all failed materialization reaches the existing
two-owner cleanup. Missing-firmware rejection now also destroys any unexpected
returned owner. The original reset-named pair contained no reset operation:
one called the other, which only tested missing firmware during construction.
Remove that duplicate invocation and use the truthful firmware-rejection marker.
This retains every unique assertion; it does not establish running reset coverage.

The temporary strict-C11 probe includes the actual test source. Thirteen
constructor/timing/getter failed results, including allocated owners returned
with failure, and four successful-null constructors all reject with no later
monitored calls and no live session. Initial probe path and shell-link-tail
mistakes were corrected in the ignored build directory, not product code.
Remove the temporary source and executable after verification. Normal units
pass on x64 and x86; the final marker-only change is rebuilt before recording
the exact source hash. HEAD-to-current test diff is +47/-57, net minus ten.
No production API, fixture, timing, MyNES, owner INI or artifact changes.
Initialization review is qualified; FDC-port and AUX failure-path verification
remain pending, and complete S94 acceptance remains open.

### Default FDC Port And Guest AUX Failure Boundaries

Read both complete original/current bodies, complete diffs and controller
fixture declarations. FDC retains no-media abnormal result, disk-change/STEP,
seek/SENSE, non-DMA FORMAT fill and IRQ, invalid SENSE, write protection,
reserved-rate rejection and all 512 PIO reads with the final normal result.
Its two separate one-tick command advances remain separate. Replace private
port calls/exiting test wrappers with checked public bus operations, reject
failed/null/inactive/no-Core construction and use one cleanup. The result
reader now uses the canonical boolean type. Void FDD creation/refresh remain
existing fixture operations, not newly invented status-bearing contracts.

AUX retains byte-identical guest program, PIC setup, IRQ12 handler, F4 ACK,
pre-execution count and exact FA/29/05/FD packet. A failed count memory read
now exits the bounded run immediately instead of spending its remaining
500000-instruction budget. Reject null Core before applying the entry plan.

Temporary strict-C11 probes include the actual sources and target fixture
objects. FDC rejects all 702 constructor/bus/tick/IRQ failed results plus
successful-null, inactive and null-Core owners. AUX rejects all 155
constructor/entry/run/memory/input failed results plus successful-null and
null-Core owners. Every case has zero later monitored calls and zero live
sessions. Error outputs are left unwritten. Probe-only null Core substitution
is restored before real destruction; no production state or API is changed.
The first AUX probe compile suppressed diagnostic arguments and therefore
triggered an unused-variable warning; retaining argument evaluation fixes
that temporary probe, not product source. Remove both probes and binaries.

Final normal tests pass x64 2/2 (3.55s total) and x86 2/2 (0.19s total).
HEAD-to-current diffs are FDC +72/-68 and AUX +4/-2. Qualify both exact source
hashes; prefix-pending falls from 41 to 39, not S94 acceptance. No MyNES,
owner INI, external master or product artifact change. Full closure suites
and the remaining original-to-receiver coverage review remain open.

### FDC Read-Track Corpus Reconciliation

Read the complete original/current corpus test and complete actual diff.
Retain DOR selected-motor rejection for both 0Ch and 2Ch, untouched DMA RAM,
the eight exact DMA2 writes, 18-sector generated payload, bounded real
deadline advancement, all terminal ST0/ST1/ST2/C/H/R/N checks, SENSE IRQ
clearing and non-MFM no-data result. Two command advances remain distinct
one-tick steps. The existing inactive-IRQ MSR branch remains; normal execution
does not prove that branch was reached.

Replace exiting bus wrappers and unchecked command advances with checked
public Core operations. Stop failed assertions immediately through the sole
owner cleanup. Consolidate four identical seven-byte reads and four two-byte
SENSE drains into one owner-local checked reader. Initialize expected data
before construction; remove diagnostic-only post-failure machine queries and
the never-populated CPU run result. Failure output retains stage and cached
data/result bytes, without invoking additional device operations.

The temporary strict-C11 actual-source probe rejects 130 selected failed or
unadvanced results plus null/null-Core owners, with zero later monitored
calls and zero live sessions. Small operation groups are checked at every
normal-path occurrence. The 9216 media writes and 27685 deadline advances
are sampled at first/middle/last positions rather than rerunning quadratic
prefixes; this is not an exhaustive per-iteration claim. Deadline OK with
advanced=false is separately verified at those three positions. The temporary
probe initially shared its null-construction mode with that latter case;
separating the modes and rerunning ensures each deadline location is actually
reached. Probe source/executable are removed; no production/API change.

Normal x64 and x86 cases pass once each, totals 0.46s and 0.16s. Actual test
diff is +83/-105, net minus 22. Qualify its exact hash; inventory remains 626
paths, prefix-pending falls from 39 to 38. Model40 integration review was
started but its combined original/diff read was truncated, so it is not
qualified. No MyNES, INI, external master or artifact modification; S94
closure remains unproven pending remaining coverage and full verification.

### Model40 Board Integration And Reset Status

Complete the previously truncated read with the entire original body, final
current body and complete diff. Read Core/board composition, CMOS, KBC state
and controller declarations, the in-code Model40 ROM/CMOS constructor and the
controller receiver. Original unique assertions remain: rejected missing
firmware; 386/2MB; D4 flags/registers, port61 and reset ROM byte; disabled AUX
and command-byte policy; host mouse cannot create OBF; A8 cannot enable absent
AUX; two 80-cylinder double-sided drives and CMOS22; F5 ACK, disabled keyboard
after D4/F4, EE echo; reset auxiliary PIT/FDC IRQ6 DMA2/HDC IRQ14 Compaq
personality/RTC IRQ8 and shared port routes; all four reset SENSE status/PCN
entries followed by invalid80; HDC diagnostic error1/IRQ/status-read clearing
and rejected IDENTIFY ERR/ABORT. The original duplicate AUX predicate has one
retained equivalent check, not a lost unique observation.

Replace exiting bus fixtures with checked existing Core bus operations and
check reset's status before examining the rebuilt board. Failed steps stop
through one cleanup. Capture composition once before reset and once after;
validate the drive observation before reading CMOS. Failure reporting uses
initialized cached observations, without additional post-failure device calls.
The checked one-tick FDC helper preserves the original single-step advance.
Remove the unused AT KBC production include; no public getter or test facade
is added and no Shared/production implementation is modified.

The temporary strict-C11 actual-source probe rejects all 66 monitored failed
or opposite results, plus successful-null/no-Core construction and a real
allocated owner returned by the rejected missing-firmware constructor. Every
case has zero later monitored calls and zero live sessions. Failed status
outputs are left unwritten. Void HDC service remains the existing fixture
operation; this does not invent a new failure contract. Delete probe source
and executable. Final registered units pass x64 once (1.62s total) and x86
once (0.10s total); exact final diff is +106/-118, net minus twelve.

Qualify the source hash; prefix-pending falls from 38 to 37, with full S94
source reconciliation and closure verification still open. No MyNES, owner
INI, external master or product artifact changes, and no partial P delivery.

### Current Performance Check And XT Profile Qualification

The renewed inspection confirms the existing local eight-job settings and
retained strict-matrix batching/CPU-manifest indexing, not a new benchmark
speedup. Unchanged XT target Ninja runs take 0.243s x64 and 0.169s x86.
CTest cost history identifies firmware-floppy and the 8086 decoder ledger
as longer units; those historical averages are not fresh individual timings.
The floppy receiver checks an exact PC after each instruction. Arbitrary
batching could miss that checkpoint, so this inspection does not replace it,
raise timeouts or add a cache/framework. Continue affected-target incremental
builds and transient checks; full dual-width suites remain S94 closure exits.

Read the complete original/current XT source and actual diff. Preserve the
entire fixed descriptor, 8088 identity and existing timing classification;
DMA2/FDC IRQ6/8253/Xebec type2 IRQ5/DMA3, absent memory, CGA and no-AT-alias
routes, copied text cells, one/two-ROM construction and rejected CPU override.
Check successful-null plan, registry, Core, board and session outputs before
their consumers. Capture immutable composition once, use checked public bus,
memory/reset/display operations, and release unexpected rejected-constructor
outputs through the sole cleanup. No production, Shared API or state mirror
is added. The exact tracked source diff is +69/-70, net minus one.

The temporary strict-C11 actual-source probe covers all 22 monitored failed
status positions and seven successful-null/rejected-owner modes. Every case
rejects with zero later monitored calls and zero tracked live allocations.
It does not claim exhaustive opposite-value coverage for every descriptor or
port boolean. The probe's first rejected-owner recipe mistakenly supplied an
explicit CPU override that this fixed profile rejects; correct it to DEFAULT
before the passing run so a real owner is returned. This was probe setup,
not a production failure. Remove the probe source and executable.

Final registered XT units pass once per width (0.06s CTest total each).
Qualify the exact source hash; prefix-pending falls from 37 to 36. S94 remains
active for the remaining source reconciliation and full closure verification.
MyNES, owner INIs, external masters and artifacts remain untouched in this
inspection; no partial P is delivered.

### Project BIOS Floppy Receiver

Read the complete original/current source, public plan/media declarations and
complete actual diff. Retain all eleven synthetic-media cases, guest bytes,
1.44MB geometry, cross-track two-sector read/write, ES:BX boundary, preserved
BX/CX/DX/SI/DI/BP/DS/ES and flags, recalibration/reset, zero-count/invalid CHS,
invalid drive/function, masked IRQ6 timeout and subsequent recovery,
write-protected and absent media. Existing result/error-status and byte-for-byte
payload observations remain. The exact-PC single-instruction checks and both
execution budgets are unchanged; no external ROM or media is introduced.

The migrated public board/materialization call is correct. Add successful-null
profile, Core plan, media registry, display slot, Core and board checks before
consumers, all using the existing cleanup. Normal registered units pass x64
(14.56s test, 15.26s CTest total) and x86 (12.55s test, 12.57s total).
The temporary actual-source strict-C11 probe covers 17 failed preparation/first
run status positions, all six null construction modes and two failed synthetic
disk allocations. Each rejects with zero later monitored producers/getters and
zero live tracked owners. This is preparation proof, not exhaustive injection
into every later guest retirement. Remove the probe source and executable.

Final diff is +7/-2, net plus five for the explicit null-output boundary.
Qualify the source hash; prefix-pending falls from 36 to 35. No production,
Shared API/manifest, MyNES, owner INI, external master or artifact change.
S94 still requires the remaining source review and complete closure validation.

### Machine-Time And Model40 Refresh Ordering

Read complete original/current bodies, the full actual diff, Core time and
Model40 refresh fixtures, and the D4 public lifetime contract. Preserve all
four invalid time-axis/retirement combinations; reset elapsed zero; configured
8MHz copied time-axis observations; two 286 NOP retirements at six ticks,
one further retirement at cumulative nine ticks under the original budget;
reset-to-zero; D4 refresh at tick one before the unrelated counter at tick
four. The synthetic axis configuration is not evidence that a shipped profile
has a complete physical timebase. D4 remains App-owned, using the existing
auxiliary-PIT attachment; fixture access remains at each state owner's module.

Replace accumulation after failed producers with checked short-circuit chains
and one teardown. Guard null Core/board/D4 before fixture consumers. The old
rejected-output sentinel was non-owned yet reached unconditional destruction
when a producer failed to clear it; each rejection now starts with its own
sentinel, never destroys it, and cleans up only a distinct unexpected owner.
This helper consolidates the four identical rejection/cleanup responsibilities,
not a new facade. Remove the pure status wrapper and duplicate local timeline
callback in favor of the existing owner-local fixture. No original unique
assertion, guest byte, time value or timing classification is removed.

The actual-source strict-C11 temporary probe rejects all 29 monitored failed
statuses and six null-Core/null-board/null-D4, uncleared sentinel, unexpected
real rejected owner and successful-but-unadvanced cases. Every case has zero
later monitored calls, zero live Core owners and zero non-owned destruction.
Delete the temporary probe source/executable. Registered normal cases pass
x64 (1.04s test, 2.13s total) and x86 (0.08s test, 0.10s total).
Exact final tracked source diff is +73/-79, net minus six.

Qualify the source hash; prefix-pending falls from 35 to 34. Shared production,
manifests, MyNES, owner INIs, external assets and product artifacts are unchanged
by this batch. S94 and its complete source/closure verification remain open;
no partial P is made.

### Paging And Protected Privilege Producer Review

Read complete original/current sources and actual diffs. Retain all four
paging/task-transfer contexts and seven privilege/exception/stack-atomicity
contexts, original guest bytes, error codes and register assertions. Check
successful-null constructors and failed setup/run/diagnostic producers before
consumers. Privilege continuation previously swallowed diagnostic failure;
return it instead. Preserve the intentional INTERNAL_ERROR/STOP_FAULT result
for stack atomicity. Consolidate equivalent GP/NP delivery helpers without
removing cases; use cached diagnostics and one cleanup.

Actual-source probes reject 119 paging failed-status positions plus two null
outputs, and 143 privilege failed-status positions plus one null Core. Each
has zero later monitored calls and zero live Core owners. These probes do not
claim exhaustive opposite-value assertion coverage. Both registered normal
tests pass per width: x64 CTest total 2.59s, x86 0.19s. Source diffs are
paging +22/-22 and privilege +87/-117. Remove temporary diagnostic sources and
executables. Qualify both hashes; prefix-pending becomes 32. S94 stays open.

### Renewed Incremental Build Cost Check

Measure unchanged affected-target Ninja construction once per width: x64
0.175s and x86 0.160s, both reporting no work. Both local caches retain eight
unit jobs. Existing batching/indexing speedups remain applicable; no further
compiler-cache layer or concurrency increase is justified by this check.
Current remaining work is actual-source/failure-path review, not repeated
whole-tree compilation. Use affected-target incremental builds and transient
relevant checks for code deltas, no repeated checks for unchanged source-only
review, and complete dual-width suites at S acceptance. Do not remove tests,
relax timeouts or rebuild MyNES for these NXVM test-only changes.

### Independent CPU Cross-Width State Receiver

Read the complete former App cross-width source, the complete CPU receiver and
its instruction fixture. All eight cases remain: each direction has JMP,
CALL, task gate and nested IRET. Retain exact TSS16/TSS32 saved-state spans,
register/segment values, outgoing LDTR, descriptor busy bits and NT/TS checks.
The setup recipe compares equal after replacing the old machine argument with
the CPU fixture and normalizing whitespace; the entire main case list and
success-marker list likewise compare equal. Observation adapters change from
Core run/memory/capture to CPU-owned execution and bounded in-code storage.
The fixture initializes CPU/execution/diagnostics without heap or PC wiring;
run still contains at most 128 instruction refreshes and rejects stop_requested
or failure to halt. This is chip-state evidence, not Core integration proof.

Registered x64 and x86 cases each pass once in 0.03s (CTest totals 0.16s and
0.10s). The initial guessed Ninja target was unknown; use the inspected
x86-test-cpu_task_switch_cross_width_state target. Both builds report no work.
Qualify only this unchanged receiver hash: prefix-pending becomes 31. The
Core cross-width receiver and deletion disposition remain pending; inspection
finds successful-null construction and post-failed-run observation paths to
reconcile there before qualification. No Shared source/test/manifest change,
new API, product artifact or MyNES modification is made in this batch.

### Neutral Core Cross-Width Receiver Failure Boundary

Complete the Core receiver read and reconcile it with the full former source
and independent CPU receiver. Preserve eight cases, guest byte arrays, saved
TSS/register/segment/busy/NT/TS observations and markers. Core now reads copied
public CPU snapshots rather than private CPU layout. The old LDTR flagValid
assertion remains specifically in the independent CPU receiver; the public
Core receiver checks its zero selector and saved outgoing LDTR. Do not claim
selector equality alone proves private cache validity.

Combine construction failure/null rejection with the existing prepare cleanup,
and stop before capture/semantic consumers after failed installation, run or
capture. Keep both case-local destruction paths. The repair adds eight lines
and removes two relative to the pre-review receiver, without production/API,
instruction, timing, guest byte or assertion changes. Update only this file's
test/x86 manifest entry. Normal registered cases pass x64 (0.89s test/1.76s
total) and x86 (0.09s/0.11s).

Strict-C11 actual-source probes cover every monitored failed status in each
of the eight cases: 138 positions, plus eight successful-null constructors.
Every injected case rejects with zero later monitored operations and zero
live Core owners. Remove temporary probe source/executable. Qualify the Core
receiver and its original deletion; prefix-pending becomes 29. Full S94
acceptance remains open; no partial P, MyNES rebuild or product artifact change.

### Instruction Timing Receiver And Result Failure Boundary

Read complete original/current sources and the former private IDTR helper.
Retain the nine main instruction cases and exact 3/2/3/5/3/5/9/5/19 tick
expectations; the whole main compares equal after whitespace normalization.
Retain 11-tick split/single quantum equality, reset, requested-stop zero work,
missing/empty qualification rejection, copied eligibility descriptors,
qualified NOP/Jcc 3/9 ticks, unqualified rejection and equivalent prefix.
The configured 8MHz physical metadata is a test contract, not qualification
of any shipped profile.

The migrated fault setup executes guest LIDT with limit 17h instead of
mutating private IDTR cache. Preparation now legitimately consumes time;
retain zero fault-instruction work/ticks and unchanged elapsed time relative
to that preparation, and confirm the actual UD diagnostic. No private setter
or public getter is introduced.

Repair successful-null construction in all three positive creation paths,
and stop before reading run results/accumulating ticks or reset continuation
after failure. Release unexpected outputs in invalid qualification checks.
Flatten the redundant fault guard after the new early exit. Original timing
values and positive case list remain unchanged. Strict-C11 actual-source
probes exercise the shared NOP timing helper plus quantum, fault, stop,
invalid-qualification and physical-contract paths: 74 failed-status positions
and nine null-constructor positions, all rejected with no later monitored
calls and no live Core owners. This is not failure injection of every opcode.
Delete the temporary probe source/executable.

Normal x64 test passes in 1.05s (CTest 1.96s), x86 in 0.16s (0.18s).
Update the receiver's test/x86 manifest hash and qualify its original deletion
and receiver; prefix-pending becomes 27. Full S94 closure verification remains
open. No production/API, MyNES, external asset or product artifact change.

### T359 S2 Arithmetic/Data Timing Receiver

Read complete original/current sources and the existing Board/Core fixtures.
All four profile rows and main markers compare equal after whitespace
normalization. Preserve arithmetic/data rows, SETcc, three-profile odd-word
penalties, dynamic multiply, Group-3 cases and insufficient/sufficient
105/106-budget admission with actual 46 ticks, ten 386 width/prefix cases and
legacy segment override. Guest byte arrays, timing expectations and provider
advance assertions are unchanged.

Replace private register reads/writes with checked public operations that
retain the low-word update and upper-word preservation. A small local word
helper differs from the existing exit-on-error fixture because it returns
failure to the case's cleanup; no new public interface or duplicate production
mechanism is added. Reject successful-null construction, and stop Group-3
subsequent profile construction after a failed preceding profile. The complete
main's normal cases pass x64 (1.12s test/2.15s total) and x86 (0.14s/0.17s).

Strict-C11 actual-source probes run the full main and inject 456 failed
operations plus 15 successful-null constructors. Every injected run rejects
with zero later monitored calls and zero live Core owners. Fixture mapping
and bind/freeze/reset are monitored at their call boundary, not each nested
fixture operation. Delete the temporary probe source/executable. Qualify the
receiver and original deletion, update its manifest hash; prefix-pending
becomes 25. S94 stays open; no partial P, MyNES or product artifact change.

### T359 S3 Control And Stack Timing Receiver

Read the complete former App source and Shared receiver. Preserve all 96
timing contexts: four profiles' seven baseline control/stack rows, three
profiles' four PUSHA/POPA/ENTER/LEAVE rows, four profiles' seven indirect
transfer/stack shapes, four profiles' two direct far transfers, and four
profiles' five real-interrupt/IRET rows. The entire main, instruction bytes,
expected tick arguments, branch/register values and provider elapsed-tick
assertions remain unchanged. Low-word seeds preserve the register's upper
half, matching the original private AX/CX/SP writes. No CPU timing model or
production interface changes.

Reject successful-null construction before mapping. Replace process-exiting
register helpers at this owner with checked low-word writes and checked
expected-word observations, and check overflow-flag reads before writes.
Failures now return through the existing machine cleanup. These helpers have
a distinct cleanup boundary from the inherited process-exiting fixture;
they introduce no public fixture API. Repair adds 25 lines to the prior
receiver. Both registered tests pass once: x64 1.35s (1.38s CTest total), x86
0.19s (0.21s total); each build recompiles only this object and relinks it.

Strict-C11 actual-source probes exercise the full main, injecting 792 failed
operations and 19 successful-null constructors. Every injected execution
rejects, with zero later monitored calls and zero live Core owners. Mapping
and bind/freeze/reset are monitored at their fixture-call boundary, not each
nested operation; this is not exhaustive opposite-value assertion coverage.
Remove temporary probe source and executable. Manifest verification and diff
checks pass. Qualify receiver and original deletion: 23 prefix-pending paths
remain. S94 stays active; full dual-width closure and later independent,
artifact and boot acceptance remain open. MyNES and product EXEs are unchanged
by this test-only batch.

### Retired Protected Timing Fixture And General Instruction Ledger

Read the complete retired protected-16 timing fixture, its two former
80286/80386 manifest inclusions, and the existing public bootstrap receiver.
The retired renamed mains were not invoked by either manifest runner. Their
used gate preparation/write/install operations now use the existing guest
LGDT/LIDT/LMSW/far-transfer bootstrap and copied register operations rather
than direct CPU caches. This qualifies the deletion and its source ownership,
not the still-pending complete manifest-runner recipes. Earlier qualified
protected gate receivers retain the behavioral tests; their evidence above
does not substitute for the remaining timing-runner review.

Read all 685 former general-ledger lines and the complete neutral receiver.
Retain the entire eight-group main. All 57 guest byte arrays compare identical;
the only two removed arrays are bootstrap HLT terminators. Guest protected
entry now runs its ten setup instructions under a bounded budget, starting
at 0200h, rather than executing HLT and privately clearing its halted flag.
Measured MOV-Sreg and far-transfer observations still start at the resulting
protected entry. Setup time is preserved in the cumulative elapsed axis.
The #UD rollback fixture uses its existing failing IVT memory provider rather
than a private IDTR edit. Exact normal/fault tick expectations are retained.
Neutral Core owns the direct elapsed-overflow and retirement-contract fixture
edits; they are not exposed as new public setters or App imports. The 8MHz
physical classifier setup is a unit contract, not a shipped-profile claim.

Repair successful-null constructor rejection in all five construction paths.
Propagate register read/write failure through the existing cleanup instead of
terminating the process; preserve upper halves for word writes, preserve other
flag bits for OR operations, and retain exact read-back predicates. Do not
construct a physical machine after qualification capture fails, or execute
the overflow run after loading its program fails. Owner-local checked helpers
add no public fixture API; the repair adds 25 lines to the prior receiver.

Both registered normal tests pass once: x64 1.19s (2.18s CTest total), x86
0.27s (0.29s total). Each build recompiles only its affected object and relinks.
Strict-C11 actual-source full-main probes reject all 412 injected operation
failures and 46 successful-null constructors, with zero later monitored calls
and zero live Core owners. Fixture mapping, vector blocking and
bind/freeze/reset are monitored at call boundaries, not nested operations.
This does not claim exhaustive opposite-value assertion coverage. Remove the
temporary probe files. Manifest and diff checks pass; qualify the ledger pair
and retired fixture deletion, leaving 20 prefix-pending paths. Full S94 and
later independent/artifact/boot acceptance remain open; no partial P,
production, MyNES, INI or product artifact change is delivered here.

### 80186 Instruction Ledger And Retired CPU Borrow Fixture

Read all 727 original ledger lines and the entire neutral Core receiver.
All 61 static arrays/tables compare identical after the established flag-name
mapping, and the complete main is identical. Preserve seven-by-six ALU and
Group-2 matrices, CMP/TEST, adjustment, unary, flag, L2 arithmetic and DIV
recipes, LDS/LES/BOUND, stack/memory/REP/port contexts, reset/stop/#UD/budget
and overflow assertions, and both full-byte ENTER nesting levels. This is
not a CPU timing upgrade. Neutral Core owns the retained private elapsed
overflow fixture; no App-private import or new public setter is introduced.

Reject a successful-null constructor before mapping. Replace all 29 low-word
fixture seeds with checked read/patch operations preserving the upper half;
the two ZF OR operations preserve every other flag. Checked register matches
retain the original word/selector observations. Flag controls use one checked
copied read and both original set/clear predicates. Remove the process-exiting
debug fixture include; failures return through the existing local destroy.
Two owner-local helpers implement this distinct cleanup boundary without a
public fixture API. Repair and wrapping add 68 lines to the prior receiver.

Each root width rebuilds only the affected object and link, then runs the
registered test once: x64 1.67s (2.65s CTest total), x86 0.80s (0.82s total).
An ignored strict-C11 actual-source probe passes the full normal main, then
injects failures separately into its 17 helper groups to avoid replaying
unaffected preceding groups. It rejects all 2,156 monitored failed operations
and 150 successful-null constructors, with zero subsequent monitored calls
and zero live Core owners. The common baseline-case helper uses NOP as its
cleanup representative; all its other programs are covered by the normal
main, not separately injected. Vector blocking is monitored at the fixture
call boundary, not nested provider registration. This is not exhaustive
opposite-value assertion coverage. Remove the terminal probe source and EXE.

Read the complete retired machine CPU fixture. Its nine private CPU-borrow,
state-preparation/capture/linear-read helpers and delivered-fault redirection
have no remaining executable test callers. The only remaining old include is
an intentional forbidden-boundary negative injection. Public Core debug,
memory and guest-bootstrap receivers replace private state access; the
still-pending manifest-runner recipes retain their own separate review rows.
This qualifies the retired fixture's source/ownership deletion, not those
remaining timing recipes. Qualify the ledger pair and fixture deletion,
leaving 17 prefix-pending paths. Manifest verification and diff checks pass;
S94 complete dual-width closure and S95-S97 acceptance remain open. No partial
P, production, MyNES, owner INI or executable input change occurs in this batch.

### 8086 Instruction Ledger And Verification Cost

Read the complete original 737-line ledger and complete neutral Core receiver.
All 83 static arrays, all 21 Group-3 recipe names/ticks and the complete main
compare identical. Retain the ten original groups, ALU/shift matrices, memory,
segment, WAIT/FPU, REP/branch/port, reset/stop/#UD/budget and overflow assertions.
Synthetic compatibility clock inputs are unit contracts, not physical-profile
claims. Neutral Core owns the retained private overflow fixture.

Reject successful-null construction before mapping. Replace 59 low-word seeds
with checked register patches preserving their upper halves, and two ZF OR
seeds with checked patches preserving other flags. Keep the table/macro style:
Group-3 seed tuples short-circuit on failure instead of discarding status with
the comma operator. Checked reads retain their original predicates. Failed
program loading cannot fall through into the overflow run. Remove the
process-exiting debug fixture include; existing owner-local cleanup handles
failure. Two private helpers add no public API. The receiver grows 36 lines.

Each affected target rebuilds only its object/link. Registered normal tests
pass once per width: x64 1.52s (2.55s CTest total), x86 0.47s (0.49s total).
The strict-C11 actual-source probe runs the full normal main, then separately
injects failures into all ten actual groups, without replaying preceding
unaffected groups. All 1,510 failed operations and 95 successful-null
constructors terminate with zero subsequent monitored calls and zero live
Core owners. Vector blocking is monitored at the fixture call boundary;
void FPU begin/advance operations are not failure-injected. This does not
claim exhaustive opposite-value assertion coverage. Remove the probe source
and EXE after its successful terminal result.

The renewed performance inspection confirms both local caches retain eight
unit jobs and an unchanged x64 ledger build reports no work (1.005s tool wall
time). Retain the previously measured strict-matrix batching (36.14s to 4.91s)
and manifest indexing (4.388s to 2.012s), not a new measurement or cache claim.
Remaining latency is actual-source review and failure-path repair. Use
changed-target builds and transient focused checks during repair; complete
units remain required at S closure. Do not repeat normal cases for additional
confidence, increase timeout budgets, or rebuild MyNES.

Qualify the ledger pair, leaving 15 prefix-pending paths. The updated test
manifest verifies. S94 complete closure and S95-S97 acceptance remain open;
no partial P, product behavior, owner INI or MyNES change occurs here.

### Shared Test Registration And 80286 Remaining Cleanup

Complete the original/current test/x86 CMake registration review: historical
chip/debug tests and manifest/corpus/negative gates remain. Moved Core/Board
fixtures resolve to their owner targets; optional debug/xasm targets retain
their tools/Win32 guards. The existing test/register.cmake helper handles
registration only, with no App source or second production executor. Fresh
root T344 registration integrity passes with its 335 marker. This qualifies
the registration source; complete independent execution remains S95 evidence.

Read the entire original 80286 ledger and 1,249-line Core receiver in bounded
chunks, including the setup-instruction/table/system-selector routines and
all real/protected/port/boundary groups. The receiver currently terminates
the process on copied snapshot, register or architectural setup failure,
bypassing its existing Core destroy path. Comma-expression seed/resume groups
discard setup outcomes. These findings remain open; normal green execution
does not qualify their failure handling.

First repair successful-null construction before mapping and zero-initialize
the run result used in failure diagnostics. These two safety edits change no
program bytes, expected ticks or assertions. Both targeted builds compile
only the affected object and link. Registered normal tests pass once each:
x64 1.43s (1.46s total), x86 0.23s (0.25s total). Refresh the manifest and
review hash without marking this ledger qualified. Remaining register/snapshot
and architectural-setup status propagation needs repair and actual-source
failure probes before accepting the original/receiver pair. There are 14
prefix-pending paths, not a completed S94; no P or product artifact delivery.

### 80286 Ledger Complete Failure Propagation

Finish the previously recorded 80286 cleanup. Preserve all 110 original static
program/table arrays and the complete main (identical after trailing-newline
normalization). All original row programs, expected ticks, operands and
assertion predicates remain; only failure checks/observation plumbing change.
The earlier migration executes real descriptor/table setup instead of borrowing
CPU layouts: seven-instruction protected bootstrap stops on its budget before
HLT, subsequent resume patches EIP, LLDT/LTR use architectural descriptors,
and row elapsed/observer assertions measure deltas rather than rewinding the
Core clock. Real segment memory observations use their actual physical bases.
The synthetic clock inputs are unit configuration, not physical-profile claims.

Remove the process-exiting debug fixture. Capture now returns checked status
with a copied snapshot; adjacent selector/base checks consume one successful
capture. Checked register patches retain full-width seeds, 36 low-word seeds,
one low-byte seed and two ZF OR seeds, preserving untouched bits. Checked
matches retain 56 original register predicates. The 104 comma seed/resume
groups become short-circuit failure expressions, keeping their original order.
Architectural setup, memory save/restore and register restoration all return
failure to the owning case. LDT/TR form iteration stops after a failed row
and destroys its owner before returning. No new production or public fixture
API is introduced. Final repair/wrapping adds 93 lines to the prior receiver.

Normal registered tests pass once for this complete repair: x64 1.19s (2.33s
CTest total), x86 0.30s (0.32s total). Strict-C11 actual-source failure probes
run the full normal main, then each existing case group independently. They
reject 1,125 monitored failures and 25 successful-null constructors with zero
subsequent monitored operations and zero live Core owners. The baseline case
uses NOP as its cleanup representative; its other programs remain in the full
normal main, not separately injected. Vector-block setup is monitored at its
fixture boundary, not every nested route call. This is not exhaustive opposite
assertion coverage. Remove the terminal probe source and EXE. Final indentation
and wrapping preserve every compiled source token from the tested repair.

Qualify the original/receiver pair, leaving 12 prefix-pending paths. Refresh
the complete test manifest and review hashes; complete S94 closure and S95-S97
independent/artifact/boot delivery still remain. No partial P, product artifact,
owner INI or MyNES change is delivered in this batch.

## 80386 paging receiver: neutral owner and failure cleanup

Read the complete original 1,229-line paging test and current receiver. All
27 constant byte arrays and the complete main remain identical. The receiver
tests CPU/RAM/page tables, not PC peripheral wiring: construct neutral Core
with the same RAM size and CPU/FPU choices, and link only Core observable.
Remove its Board constructor dependency and redundant extra Core link.

Initialize run/diagnostic values before possible failure reports. Guard later
producer calls after failed setup/run/capture; initialize permission fixture
state before rejecting oversized input. Cross-page preparation failure now
destroys its partially constructed Core, and failed permission/cross-page rows
stop before constructing another owner. Valid-path stage checks stop on their
first failed producer while retaining the original report fields and predicates.
Successful-null construction and prepared-entry outputs are rejected.

Normal focused paging passes x64 (0.18s test, 0.38s CTest total) and x86
(0.09s test, 0.27s total). The x64 run precedes the final null-entry guard;
x86 includes it. Both builds reconfigure in under four seconds and compile
only the changed receiver. No full-suite or independent-corpus claim follows.
Keep the original/receiver pair pending until actual failure injection and
final guard review are complete. Refresh the manifest and inventory hashes;
no production, MyNES, owner INI, artifact or partial P is changed/delivered.

The actual included-source probe completes all seven groups and the normal
main (931 monitored operations, 25 Core owners). Injection initially exposes
a missed reset-callback result in valid-path reset verification: Core reset
can return OK while the fixture's patch callback reports failure. Check the
existing reset_status before capturing the CPU. After repair, all 931 single
operation failures and 25 successful-null constructor cases reject failure
with zero subsequent monitored calls and zero live Core owners. The probe
monitors neutral construction, bind/freeze/reset, nested reset register patch,
RAM write/read/inspect/physical write, entry preparation, run, diagnostics,
snapshot/register/linear debug access and trace setup. It does not inject
every internal CPU algorithm or assertion predicate. Strict C11 compilation
passes; terminal probe source/EXE are removed from the ignored build tree.

Final normal receiver passes x64 (0.18s test, 0.20s total) and x86 (0.10s,
0.13s). Qualify the original/receiver pair: 12 pending paths become 10. Full
S94 units/gates and S95-S97 independent/artifact/boot delivery remain pending.

## CPU timing preview receiver: publication and cleanup review

Read the complete 1,833-line receiver in bounded portions and reconcile the
original through the complete line diff; unchanged sections are identical.
All 181 constant byte/profile arrays and the complete main are unchanged.
The historical opcode/profile/size-prefix/ModRM/SIB/group legality matrices,
taken branch targets and nonpublishing preview assertions remain. The limited
fetch fixture loads a real GDT/code descriptor at the last four RAM bytes;
the default high reset alias case is genuinely Board-owned, so retaining this
receiver under ibmpc-common is intentional. No new preview API is added.

Core composition fixture owns the private preview/counter access: copied
committed/cancelled/trace counts retain the original predicates. Add the six
successful-null constructor guards and do not capture the second publication
value after preview/observation failure. This repair adds ten lines to the
receiver, not a new fixture or execution path. Focused test passes x64
(1.26s, CTest total 2.40s) and x86 (0.10s, total 0.12s).

An actual included-source probe runs the complete normal main and all six
machine-bearing helper groups. It rejects 57 monitored operation failures and
six successful-null constructors with zero subsequent monitored calls and
zero live owners. Monitor create/freeze/reset, RAM write, debug register
read/write/patch, run, CPU state/observation and machine preview; publication
captures are counted only as post-failure operations, not injectable errors
because their value-returning API cannot fail. A failed preview supplies an
available lexeme so the expected-unavailable case cannot accept probe failure.
Pure lexeme scanning is covered by the unchanged normal matrices, not this
machine-operation injection. Strict C11 compilation passes. Terminal probe
source/EXE are removed. Qualify both paths: ten pending become eight; S94 full
closure and independent/artifact/boot delivery remain open. No MyNES, product
source, owner INI, artifact or partial P is changed/delivered.

## Profile floppy boot matrix: complete source/diff and input-path review

Read the complete 467-line pre-repair matrix and complete original Git diff,
including removed diagnostic bodies. Read its INI helper's open/restart/start/
pause/input handling. Terminal predicates still accept DOS prompt, date input,
installer-ready or installer-running; prior 301/303 detection still rejects
even a later terminal. Timeout remains 180,000ms and polling 10ms. Standard/
Turbo argument selection, copied display capture, seed comparison and external
INI-driven discard-only media overlays remain. Public CMOS port writes/reads
replace direct private port access with checked statuses.

Removed Board-private FDC/PIC/PIT/KBC/CMOS fields were optional timeout trace
details, not success assertions. The existing copied Model40 last-FDC result,
Core time observation/A20, CPU state, BDA/IVT/boot bytes, bus traces and screen
remain; no replacement private getters or expanded diagnostic API is invented.
The whole-board fields no longer define App integration visibility. Historical
expanded diagnostic text is intentionally not claimed byte-identical.

Repair two ignored results: observation failure cannot print uninitialized
elapsed/lifecycle, so publish that diagnostic only on successful capture;
trace setup failure reports TRACE-SETUP-FAILED and closes the session instead
of silently claiming a traced run. Both widths compile/link the changed target.
Six malformed argument cases per width reject with exit 1 without opening an
INI or starting a guest. Qualify this source path, leaving seven pending. This
is source/build/input validation, not a fresh successful boot claim: the eight
unchanged-INI checkpoints remain S96 and run once each. No MyNES, external
asset master, owner INI, product artifact or partial P changes are delivered.

## Current verification cost and 80186 continuation

On the owner's renewed cost request, the 80186 manifest target performs three
incremental steps in 5.029s; its selected x64 test passes in 3.23s (CTest total
4.24s). The immediate unchanged build reports no work in 0.376s. These are
single observations, not a cold-build benchmark or a new compiler speedup.
Both retained caches use eight unit jobs. Reuse those trees, build only changed
targets during repair, and defer complete unchanged suites to S94 acceptance;
keep complete closure coverage and S96's once-per-boot-case requirement.

Continue the actual-source review by reading the remaining 80186 runner body,
including all recipe tables, main dispatch, coverage counts and result writer.
The path remains pending: process-exit debug-fixture setup does not return
failure through the recipe's cleanup boundary, and the result writer can skip
close when its final write fails. No test coverage, timeout, public API,
product artifact, MyNES input or existing performance repair is changed here.

## 80186 manifest runner: complete source and cleanup qualification

Read the complete runner and accepted-source diff. Its entire main, including
all recipe tables, dispatch, expected ticks, coverage counts and result call,
is identical to HEAD after only VCPU_EFLAGS to Core-debug constant renaming.
Neutral construction retains the original reset and stack mappings and
synthetic timing configuration; no board or physical-time claim is added.

Remove the process-exiting register fixture from this owner. Two local checked
operations preserve historical low-word masking and DS-to-ES copying through
existing public debug calls. Preparation clears its output, rejects a
successful-null constructor, propagates every seed failure and destroys the
unpublished instance. Repeat/string preparation stops before memory writes or
execution after failure, retains earlier failure through later phases, and
uses existing destruction. The result writer closes even when its final write
fails. No recipe, assertion, success predicate or production API changes.

Final recipe tests pass on x64 (3.38s) and x86 (2.19s). The ignored strict-C11
probe first runs the actual complete main normally: 9,416 intercepted calls,
577 constructed/destroyed instances and all 616 result rows retained. Thirteen
representative actual helper groups cover base, return, interrupt, BOUND,
flags/input, repeat phases, segment and odd-word paths: 319 injected operation
failures and 16 successful-null constructors reject, make zero subsequent
intercepted operations and leave zero live instances. A final-result-write
failure rejects and closes exactly once. This checks helper cleanup, not
exhaustive fault injection into every recipe or internal CPU algorithm.

Against HEAD this test adds 164/removes 102 lines (net +62), consisting of
checked failure propagation and wrapped calls, not new test recipes. Qualify
its hash in the frozen inventory: six paths remain pending. Remove ignored
probe source, EXE and result afterward. Full S94 closure verification remains
open; no partial P, product artifact, MyNES or owner-INI change is delivered.

## Remaining manifest runners: bounded early-cleanup repair

80286 source intake finds four uninitialized fixture declarations. Two outer
return recipes and task IRET can reject missing metadata before preparation,
then destroy an undefined pointer. Initialize all four declarations, including
the adjacent task-transfer variant whose prepare already clears its fixture.
The outer return/IRET paths also performed process-exiting register writes
after failed descriptor/table setup. Guard both seeds and check the existing
public writes; keep their exact full-width ESP and FLAGS values.

Strict-C11 ignored probes run the actual full 80286 main normally (771 observed
foundation records), then reject three missing-key paths and one failed
constructor without destroying a non-null unconstructed instance. Inject the
outer descriptor write failure in both return paths: zero later register seeds.
The actual writer also rejects an injected final write failure and closes once.
The probe copy changes only the outer main name to avoid the source's own
included-main macros; all recipe bodies and tables are the current source.

Sweep the matching writer expression: 80286, 80386 and 8086 also short-circuited
close after a failed final fprintf. Separate final-write status from unconditional
close, preserving count/authorization/completeness checks and every result row.
All three normal runner tests pass per width in one concurrent selected run
(6.45s x64, 6.08s x86). After the final outer-seed correction, 80286 passes again
on x64 (2.98s) and x86 (2.78s). The two other writer error paths have matching
inspected control flow and passing normal regressions, not independent injected
whole-runner fault coverage.

These are bounded repairs, not complete file qualification. The first combined
diff read was truncated; complete 80286/80386/8086 source, diff and fixture
reconciliation remains pending. Refresh their inventory hashes without advancing
those dispositions. Six pending paths remain. Remove this round's ignored probe
files; do not deliver a partial P, alter production/API or rebuild MyNES.

### 80286 Real And Protected Preparation Failure Ownership

Read the complete two preparation bodies and their public bootstrap setup.
Both used process-exiting register helpers while holding an unpublished Core.
Replace their 22 writes with checked public operations, preserving low-word
seeds through one local read/mask/write helper. Propagate each failure before
the next memory, register, execution or observation operation. Clear the output
before validation and reject a successful-null constructor. Both candidates
still destroy through their original cleanup path; no public API is added.

The complete main and all source after the recipe dispatcher boundary are
byte-identical to this round's intake. Synthetic instruction timing, GDT and
bootstrap bytes, operands, reset alias and target recipes are unchanged.
Normal registered 80286 tests pass once per width after the final repair:
x64 2.95s (2.98s CTest total), x86 2.63s (2.66s total). Each incremental target
rebuild changes only its object and link; MyNES and product EXEs are not built.

The strict-C11 ignored actual-source probe covers 18 preparation contexts,
including interrupt, stack, bounds, flags, odd operands, DX ports, XLAT,
protected LLDT/LTR, far call/return and IRET. All 322 injected operation failures
and 18 successful-null constructors reject, leave zero live candidates and
make zero later intercepted operations. Reset-alias registration internals
and CPU algorithm internals are not independently failure-injected here.
An intermediate probe compile exposed a local wrapper-name collision and
missing generated include/result macro; correct those in the ignored probe,
not the product. An intermediate mechanical line-wrap edit was invalid;
replace it with the verified direct status-chain edit before final builds.

This qualifies the bounded preparation repair, not the whole runner. The
remaining snapshot and recipe seed helpers still require actual review and
reconciliation. Keep the inventory disposition pending and refresh its hash;
six paths remain pending. Remove the ignored probe source/copy/EXE after its
successful result. S94 full closure remains open and no partial P is delivered.

### 80286 Remaining Recipe Failure Chains And Cost Recheck

Replace the remaining process-exiting recipe snapshot/register helpers with
checked public operations, preserving low-word register seeds and routing DS
to ES through one checked owner-local helper. Guard subsequent writes, repeat
phases, execution and observations after failure. The four protected bootstrap
loops capture checked copied state before and after successful steps, retaining
their original 32/48-step budgets and CS/TR/EIP predicates. Failure diagnostics
use the last copied snapshot rather than re-reading a failed producer.

Read the remaining actual diff and recipe dispatch. The complete main equals
HEAD after only public FLAGS symbol substitution and trailing-newline
normalization: no recipe, guest byte, timing value, table row or dispatcher
coverage is removed. Compare the retired protected fixture's seed/descriptor
path with the public guest-bootstrap receiver; final whole-file fixture
reconciliation remains pending rather than being inferred from normal passes.

The strict-C11 ignored actual-source probe covers 22 representative execution
groups: odd strings and REP phases, near/far/indirect calls and returns, IRET,
protected transfers, INTO/control, outer returns, task/gate and protected
interrupt paths. All 637 injected operation failures return with zero live
candidates and zero later intercepted operations. Reset-alias registration
internals and CPU algorithm internals are not independently failure-injected.
Probe wrapper-name/enum and hand-entered recipe-key/tick errors were corrected
only in the ignored probe; they did not alter the actual recipes.

Final affected-target builds compile only this object's two width variants and
link their runners (2.26s/2.42s command wall time). Registered normal cases pass
once per width: x64 2.94s (2.96s CTest total), x86 2.69s (2.71s total).
Fresh no-op builds report no work in 0.418s/0.404s. Both caches still select
eight unit jobs. These are selected-target measurements, not cold-build or
full-suite speedup claims. Retain previously measured strict-matrix batching
and manifest indexing; do not add an unmeasured compiler cache or repeat full
suites per reading batch. Full dual-width units/gates remain required at S94
closure. No MyNES, product EXE, owner INI, external master or public API changes.
Remove the three ignored probe files after recording their result. Refresh the
source hash while retaining its pending disposition; S94/T540 remain open.

### 80286 Whole-File Reconciliation

Finish the remaining diff through its final FLAGS-table hunk and read the main
recipe-dispatch flow; its full normalized equality confirms all retained table
rows and dispatches, not merely a sample. Re-read the load-table/gate helper and
both outer-return recipes against the retired protected fixture and current
public bootstrap. The descriptor constants retain GDT 0300h, IDT 0400h, code
2000h, stack 8000h, vector 30h and handler 100h. All original GPR/FLAGS seeds
remain; guest LGDT/LIDT/LMSW and far transfer replace private table/segment
cache fabrication. Outer-return recipes extend the GDT to the same 39-byte
limit through actual LGDT and retain the 12348000h ESP upper-word case.
Additional public-bootstrap GDT slots are setup capacity, not substituted
target instructions or removed expectations. Observer installation remains
after bootstrap; setup time is not rewound or charged as a target retirement.

The remaining task/call-gate receivers were separately source-qualified; this
runner reuses their actual guest setup and copied snapshot contracts, not an
extra CPU state owner. Together with the earlier full metadata/preparation/
recipe-body reads, unconditional writer-close proof, 771-record normal-main
proof and the bounded failed-producer probes, this completes this logical
file's source/coverage reconciliation. Qualify its unchanged current hash;
five of 626 paths remain prefix-pending. This is not S94 acceptance: remaining
files and final whole-suite/gate verification are still required.

### 80386 Real-Recipe Seed Failure Ownership

Read the current partition/capture/count/writer code and both complete real
preparation paths. Their 35 fixture register writes could exit before cleanup;
subsequent memory writes could also overwrite a failed seed's status. Revise
the packet, replace the writes with checked public operations and guard later
producers through the existing status path. Reject successful-null ownership
in the actual base runner. The table-load helper now returns failed snapshot
status instead of exiting, without changing its guest instruction or pointer.

Argument-sequence comparison proves all register and memory call arguments
unchanged in each repaired path; the complete main is byte-identical to this
round's intake. Retain the distinction that the continuation preparation path
seeds ECX=2 for the continuation suffix, whereas the base runner does not.
No helper merger is justified merely by their superficially repeated setup.

The strict-C11 ignored probe includes the actual source and existing FPU
composition fixture object. Fourteen real cases exercise XLAT, multiply
zero/high, division, scan, DX input, REP base/zero/SCAS, branches/LOOPE and HLT;
fourteen corresponding probe-owned instances exercise the actual preparation
helper. All 250 intercepted failures return without later intercepted calls
or live instances. All fourteen actual base-runner successful-null cases
reject after the constructor; fourteen probe-owned caller null cases likewise
reject, but do not prove other original callers' null handling. Mapping
registration internals and CPU algorithms are not independently injected.
An initial probe link omitted the existing FPU fixture object; add that object
to the ignored link command, not to the product or public API.

Normal registered cases pass once on x64 5.56s (5.59s total) and x86 5.32s
(5.34s total); builds change only the runner object and link. Refresh the
pending source hash and remove the ignored probe source/copy/EXE. Five logical
paths remain pending. Protected/system/task/VM86 recipes still need complete
review; these bounded results do not accept the whole runner or S94. MyNES,
product EXEs, owner INIs and external masters remain unchanged.

### 80386 S4 Continuation And ESC Constructor Review

Review the three direct board-construction owners. Base recipes already reject
successful-null construction; apply that rejection to continuation and ESC.
Continuation also validates program presence/length before construction and
checks its final ESP seed without allowing the following memory write to
overwrite a failure. Retain the first unrecorded retirement and second recorded
continuation, original ECX=2 seed and all phase/form predicates.

The strict actual-source probe covers every nine-by-three continuation
primitive/width combination plus ESC. It initially exposes NULL key reaching
strcmp inside the runner lookup. Reject NULL in that owner-local lookup before
comparison, protecting all callers rather than changing Lib string semantics.
The debug probe locates the failure after the first group's injected failures;
it is not a production failure attribution. After repair, all 402 intercepted
failures, 28 successful-null constructions and 27 invalid keys reject with no
later intercepted calls or live owners. Mapping internals and FPU fixture
internals are not independently injected; normal ESC still uses the retained
FPU composition object and its 1..19 remaining-tick predicate.

Final affected builds change only the runner object/link, taking 2.403s x64
and 2.249s x86. Registered normal cases pass once: x64 5.42s (5.45s total),
x86 6.53s (6.56s total). No fixture register wrapper or fatal snapshot helper
remains in this runner. Whole-file baseline reconciliation is still required;
these results do not accept S94. Remove the ignored source/probe/EXE after
recording results. No MyNES, product EXE, INI, firmware or external-master change.

### 80386 Remaining S6 Task, VM86 And Outer Return Owners

Read and repair all six remaining S6 recipe owners: task transfer, nested-task
IRET, task interrupt, VM86 interrupt, IRET-to-VM86 and outer return. Initialize
each existing fixture before key rejection. Replace process-exiting reads and
unchecked writes with checked public operations and copied snapshots; preserve
the NT check before nested return, final TR selectors, VM86 segment base and
FLAGS predicates. A failed table load must stop the VM86 FLAGS seed rather than
have its status overwritten. Diagnostics use the existing copied state only.
Delete the now-unreferenced process-exiting snapshot helper. No production,
Shared fixture or public API changes are needed.

Actual-hunk review removes two unnecessary NULL substitutions in already
guarded suffix tests; NULL-safe formatting is limited to failure diagnostics.
The full S6 dispatcher suffix is byte-identical to this batch's intake,
including every guest array, recipe key, parameter and expected tick value.
The strict ignored actual-source probe invokes all 39 affected dispatcher
calls; all 992 failed operations, 39 successful-null constructors and 39
NULL-key cases reject with zero subsequent intercepted calls and live owners.
The existing neutral task fixture already rejects successful-null instances;
the probe covers both neutral and board constructor paths. This does not
independently inject CPU algorithms or mapping internals.

Final changed-target builds compile/link only the runner, in 2.134s x64 and
2.262s x86. Registered normal recipes pass once on the final source: x64
5.34s (5.37s total), x86 5.27s (5.30s total). Remove the ignored probe artifacts
after recording their results. S4 continuation, remaining constructor review
and whole-file reconciliation remain pending; these results do not accept S94
or T540. MyNES, product EXEs, owner INIs and external masters are unchanged.

### 80386 Outer Gate And Inner Interrupt Failure Ownership

Read the complete outer-call-gate immediate/memory and inner-interrupt owners,
including their common user-entry setup. Initial argument rejection previously
could destroy an uninitialized owner. Register seeds and diagnostic rereads
could also terminate before cleanup. Initialize the existing owner, check the
public FLAGS write, check both user-entry snapshots, and use one checked final
snapshot for assertions and diagnostics. Preserve each CS/EIP/SS predicate and
all bootstrap bytes. Do not add a fixture or public API.

The complete S6 dispatcher suffix is unchanged against this batch's intake:
all fifteen affected 16/32-bit, zero/two-parameter and interrupt variants retain
their original input bytes, pointers and expected ticks. The ignored strict-C11
probe takes its calls directly from that actual dispatcher. All 774 intercepted
operation failures, fifteen successful-null constructors and fifteen NULL-key
cases reject with no later intercepted operation or live candidate. CPU and
mapping internals are not independently failure-injected. Correct an initially
misnamed copied-snapshot call to the existing capture API; the probe also needs
the actual lib_i32 main renamed, not an assumed int main signature.

Registered normal cases pass once per width: x64 5.49s, x86 5.30s. Only the
changed runner object and link rebuild (2.235s/2.225s). Remaining task/VM86 and
outer-return bodies still require review; this batch does not qualify the
whole file or accept S94. Remove the ignored probe files after recording proof.
MyNES, product binaries, INIs, firmware and external masters remain unchanged.

### 80386 Protected And Initial S6 Failure Ownership

Continue the admitted recipe review without changing guest bytes or timing
assertions. S7 protected and page-granular LSL seeds use checked public writes,
stop subsequent producers on failure and converge on their owner cleanup.
The existing protected bootstrap rejects a successful-null constructor before
freeze/reset/setup. Its strict actual-source probe passes 3,379 intercepted
failures and fifteen successful-null cases, with no later intercepted operation
or live candidate. This does not independently inject CPU algorithms or reset
mapping internals. Both affected 80286/80386 registered cases pass once per
width: x64 3.23s/5.88s; x86 2.87s/5.62s.

The first four S6 direct/memory/return/interrupt recipe owners are initialized
before argument rejection. FLAGS and DS-from-SS seeding are checked; final
CS/EIP checks and failure diagnostics consume one checked copied snapshot.
No additional fallible diagnostic reread occurs. The actual-source probe
passes 447 failures, fourteen successful-null constructors and fourteen invalid
NULL-key cases, with zero later intercepted operations or live candidates.
Normal 80386 cases pass once: x64 6.35s, x86 5.42s; incremental builds take
2.67s/2.69s. Remaining task/VM86 paths and whole-file qualification are open.

### 8086 Control-Transfer And Memory-Stack Failure Ownership

Read the complete indirect CALL/JMP, RET/IRET, software-interrupt and memory
PUSH/POP owners, dispatchers and recipe tables. Check register seeds and stop
pointer/vector setup after a failed seed. RET frame preparation previously
continued writing later words after failure via an accumulated bitwise flag;
guard the existing loop with !failed and stop on its first failed write.
All final EIP/CS/SP predicates and diagnostics consume one checked copied
snapshot. Make the indirect NULL-recipe diagnostic safe. Keep original cleanup,
programs, target/frame words, prefixes, tick/formula predicates and table style;
do not add a public API or production mechanism.

The ignored actual-source probe invokes all four dispatchers, including 32
indirect base/LOCK contexts, ten returns, eight software interrupts and sixteen
memory-stack contexts. All 1,036 intercepted failures, 66 successful-null
constructor positions and one invalid NULL indirect recipe reject without
later intercepted operations or live owners. This does not independently
inject CPU algorithms. Remove ignored probe C/EXE after recording results.

Both profile targets build in 2.38s x64/2.27s x86. Normal 8086/8088 recipes
pass once on final source: x64 6.23s/6.30s (6.48s total), x86 6.19s/6.23s
(6.33s total). Remaining multiply/divide and pointer owners plus whole-file
reconciliation still block runner qualification. No production/API, Shared
manifest, MyNES, INI, external master or product EXE change; S94 stays open
without a partial P, commit or acceptance.

### 8086 Semantic-Family Failure Ownership

Read the complete XLAT, POP-CS, ALU, adjustment, data/stack, Group-3,
branch, FLAGS, compare, unary and LAHF/SAHF semantic owners. Their migrated
register fixture exits could bypass cleanup, and branch diagnostics repeated
fallible reads after an earlier failure. Replace preparation seeds with checked
public operations and the existing low-word helper. Check one copied final
snapshot per successful run for the retained register/FLAGS predicates; use
that same value for branch diagnostics. XLAT retains its initial high 24 bits;
POP-CS retains its pre-run SS-base capture and post-run CS-base check. Remove
an intermediate duplicate post-run capture and redundant guard during actual
diff review. No original recipe table, initial value, mask or expected result
changes. The complete source suffix from general LOCK through main is unchanged
against this batch's intake.

The ignored actual-source probe executes all eleven functions, not synthetic
replacement recipes. All 721 intercepted failures and all 50 successful-null
constructor positions reject without later intercepted calls or live owners.
This does not independently inject CPU algorithms. Correct the probe's initial
snapshot-type guess to the actual snapshot-point type before its passing
build/run. Remove ignored probe C/EXE after recording its result.

Both profile targets build in 2.43s x64/2.27s x86. Registered normal recipes
pass once on final source: x64 8086/8088 6.35s/6.38s (6.60s total), x86
5.91s/5.94s (6.03s total). Keep the whole runner pending for the remaining
control/other owners and baseline reconciliation. No production/API, Shared
manifest, MyNES, INI, external master or product EXE change. S94 remains open,
with no partial P, commit or acceptance.

### 8086 Memory, String And REP Failure Ownership

Read the complete memory/string/REP recipe bodies, LOCK/context derivation,
repeat-step predicate and first/continuation/zero-count ownership. Replace
their process-exiting register fixtures with checked public operations. The
existing low-word helper retains upper-register bits; one local ES-from-DS
helper removes three identical checked seed sequences. Stop memory writes,
execution and later observations after failure. Cached checked register values
serve both assertions and diagnostics; diagnostics make no further reads.
Guard memory-address derivation and NULL-recipe diagnostics without changing
actual recipes. REP retains its two separate owners and destroys each once.

The ignored actual-source probe copies the original fifteen primary-memory,
ten string and fourteen base-REP table entries. Its 39 calls include each
normal owner's recursive LOCK companion where present. All 1,532 intercepted
operation failures, 39 initial successful-null constructors and three NULL
recipe cases reject without a later intercepted operation or live owner.
This is public-call failure proof, not independent internal CPU fault injection
or exhaustive successful-null testing of every later constructor. Remove its
ignored C/EXE after recording the result.

The complete source suffix from the decoder probe through all subsequent
recipe tables and main is byte-identical to this batch's intake. Programs,
ticks, prefixes, first/continuation/zero phases and assertions remain. Both
targets build in 2.24s x64/2.23s x86. Normal 8086/8088 cases pass once on
the final source: x64 6.11s/6.06s (6.30s total), x86 6.14s/6.15s (6.25s
total). Remaining semantic/control recipe owners still need review; keep the
whole file pending. No production/API, Shared manifest, MyNES, INI, external
master or product EXE change; complete S94 exits and delivery remain open.

### 8086 Shared Preparation And Exact/LOCK Ownership

Inspect the shared I86 preparation, ordinary exact recipe and LOCK companion.
The migrated register fixture exits the process on failure, bypassing these
owners' cleanup. The ordinary recipe also prints an uninitialized run result
when preparation fails. Replace only these owners' fatal seed calls with
checked public writes and one local checked low-word seed preserving the upper
half. Separate successful preparation/seeding from run admission. Initialize
the result, reject successful-null construction and empty programs, and make
the ordinary NULL-recipe diagnostic safe. Retain every program, initial value,
expected tick, formula input, control outcome and existing cleanup.

The ignored actual-source probe intercepts the public preparation, register
and run operations; its CLC recipe also executes the actual LOCK companion.
All 54 operation failures, three successful-null constructors and one invalid
NULL recipe reject without a later intercepted call or live owner. This does
not inject internal CPU or memory algorithms. Correct an initial probe-only
enum guess to the existing CONTROL_NONE value before its passing build/run.
Remove its ignored C/EXE after recording the result.

Both profile targets compile/link on each width in 2.43s x64/2.28s x86.
Registered 8086/8088 normal cases pass once per width: x64 6.09s/6.07s,
x86 6.15s/6.17s (6.26s wall time per paired run). These targets share this
runner; both are therefore affected. The remaining recipe owners still contain
fatal fixture calls and require actual review and repair. Keep this path
pending; all four remaining inventory rows are unaccepted. No production/API,
Shared corpus, MyNES, owner INI, external master or product binary changes.
S94 remains open without a partial P or commit.

### 8086/8088 Whole-File Reconciliation

Finish reading every remaining dispatcher/table, decoder writer, HLT check,
result writer and main, together with the complete 1,362-line HEAD/current
diff. The final path adds 430/removes 231 lines. All 107 multiline and nine
single-line static arrays match HEAD exactly after the unchanged-value FLAGS
symbol substitution; the whole main suffix is identical after newline
normalization. No existing function is removed; the two added local helpers
own checked word seeds and ES-from-DS seeds. Existing 4,906-row metadata and
the 1,053-result requirement for each 8086/8088 profile remain.

Private CPU layout writes become checked public stopped-debug operations;
word seeds preserve upper register halves, XLAT preserves the upper 24 bits,
and copied snapshots retain the original segment/base/register predicates.
The real execution/retirement captures, LOCK companions, REP phases,
source/form/input checks and L2 Group3 model identity are unchanged. The
8088-specific transfer timing still belongs to its existing result contract,
not duplicated beside the Core timing owner.

Whole-file inspection additionally finds the decoder writer's inherited
final-fprintf/close short circuit. Revise the active packet before repair;
always perform close after the last write, as already done by the result
writer. An ignored actual-source probe passes normal output, failure of its
467th/final write, and close failure, with exactly one close in each case.
Remove its owned source/executable/output after proof.

Latest changed-target builds pass in 2.88s x64/3.13s x86. Both widths run
the two normal cases concurrently: x64 10.53s/10.51s (10.75s wall), x86
10.35s/10.40s (10.49s wall), all passing. This simultaneous-width run is not
a speedup claim; the previous per-width pair was about 6.4-6.7s wall.
Each generated profile result retains 1,053 passing rows.
Qualify the exact source hash: three of 626 paths remain pending. Complete
current S94 unit/gate closure, independent verification and artifact/boot
delivery remain open. MyNES, owner INIs and external masters are unchanged.

### 80386 Whole-File Reconciliation

Complete the source review with all 1,627 lines of the HEAD/current diff,
including every recipe owner, preparation helper, dispatcher and main table.
The final diff adds 589/removes 393 lines. The complete main suffix is equal
after the public FLAGS-symbol substitution and trailing-newline normalization:
program bytes, keys, dispatch, expected ticks and coverage counts are retained.
Both retained normal-run logs report 1,413 canonical rows (1,412 CPU and one
MCP), including 89 protected-control and 118 protected-system observations.

Reconcile the old private protected fixture with actual guest setup: GDT/IDT
bases, limits, selectors, stack and GPR/FLAGS seeds are retained. Available
TSS descriptors replace fabricated busy/TR cache state so guest LTR establishes
the real busy state. Guest LGDT/LIDT/LMSW/LTR/IRET setup precedes observer
installation; it is not counted as the target instruction's timing. Copied
snapshots retain the CS/EIP/SS, CPL, TR and VM86 predicates. The existing FPU
handoff fixture retains the opcode/modrm/kind and completion-tick checks.
The earlier bounded owner-failure probes qualify cleanup, not CPU algorithms
or internal reset/mapping failure injection.

Qualify the exact source hash: four of 626 paths remain prefix-pending. No
production, API, MyNES, INI, external master or product artifact change is
introduced by this reconciliation. Complete S94 units/gates remain required.

### Model40 Retirement Capture Whole-File Reconciliation

Read all 1,822 lines of the diagnostic, the complete seven-added/eight-removed
baseline diff and the receiving CMOS/Core fixture operations. Exact normalized
source comparison confirms only the three old private includes become two
owner-local fixture declarations, two CMOS reads call the same RTC owner,
two A20 reads call the same Core-owned field reader and the software-INT
physical read delegates to the same physical-memory implementation. Preserve
execution-time physical access and its route effects: a paused debug read
would not be an equivalent substitution in the retirement callback.

All copied form/key tables, C0/C1 transitions, FDC terminal sequence/drive/
success predicates, D4 timer and RAM checkpoints, reset history, software-INT/
IRET observations, diagnostic text and terminal acceptance expressions remain
identical. Session creation still uses external INI support, finite quantum
execution uses the same Core owner and waiting advance, and session cleanup
remains before final acceptance selection. The five synthetic diagnostic
helpers remain in the original file; no new execution or coverage is claimed.

Both diagnostic targets compile/link with the owner-local fixture sources:
x64 2.41s, x86 2.95s. This is source/migration and build qualification, not an
external-ROM boot result; S96 owns fresh runtime qualification once per group.
No source repair is needed here. Qualify the exact hash: two of 626 paths
remain pending (product CMake and DOS boot probe); inventory has zero stale
hashes. No MyNES, product artifact, INI or external master change. Complete
S94 units/gates and later acceptance remain open.

### Latest Execution-Cost Recheck

The next owner-requested check confirms both local caches retain eight jobs.
Latest affected-target rebuilds finish in 2.34s x64 and 2.47s x86; unchanged
objects are reused. CTest accumulated cost data identifies decoder and real
boot cases as long cases, but does not establish a fresh full-suite benchmark.
Keep changed-target builds and transient affected checks during review; full
dual-width units/gates remain required at S94 closure. No new cache,
concurrency increase, timeout relaxation or coverage reduction is justified.

### 8086 Remaining Multiply/Divide And Pointer Owners

Read both complete Group3 L2 loops, all memory/LOCK contexts and the complete
LEA/LDS/LES owner. Chain checked word seeds, execution and copied snapshots;
reject failed preparation before further operations and use initialized copied
values for diagnostics. Preserve the original programs, operands, AX/DX seeds,
LOCK generation, L2 source identity, tick values and result assertions.
Replace the unused fatal-helper fixture include with its actual public debug
interface dependency; deleting it without that direct dependency first exposes
missing declarations, and the corrected direct include passes both strict builds.

The ignored actual-source probe passes three complete dispatchers: 1,311
injected public-operation failures and 82 successful-null constructor positions,
with zero later intercepted calls and zero live owners. This does not inject
internal algorithms. Remove the owned probe source and executable after proof.
Registered normal cases pass once per width: x64 8086/8088 6.43s/6.48s
(6.69s wall), x86 6.24s/6.27s (6.36s wall). Whole-file/table/diff reconciliation
still remains; refresh this pending row's hash without claiming qualification.
MyNES scoped status is empty. No product API, artifact, INI, firmware or media
change; S94 and T540 remain open without partial delivery.

Both caches retain eight unit jobs. Current unchanged-target builds report
no work, in 0.917s x64 and 0.856s x86 command wall time. These are no-op
measurements, not cold builds or a new speedup claim. Previously measured
strict-matrix batching (36.14s to 4.91s) and manifest indexing (4.388s to
2.012s) remain the implemented optimizations. Repeated full suites per reading
batch would not improve source-review coverage: use changed-target builds and
transient relevant tests, then complete dual-width units/gates at S94 closure.
The next owner-requested check again reports no work: 0.563s x64 and 0.407s
x86 measured around the build command. No GCC/Ninja/CTest process is active.
Both caches still use eight unit jobs. These are no-op checks, not a new
speedup; source reconciliation remains the dominant unfinished work.
No further cache, framework, timeout relaxation or coverage reduction is
justified by this check. No MyNES, EXE, INI or external asset changes.
