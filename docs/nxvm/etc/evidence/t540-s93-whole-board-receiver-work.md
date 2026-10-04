# T540 S93 Receiver Work Record

This is intermediate evidence, not acceptance or a P delivery. Baseline is
accepted S92, d76d2b15e. S93 and T540 remain open.
S93's execution was superseded without acceptance by S94-S97 on 2026-10-03.
All progress wording below is historical; Current owns today's status.

## Implemented Working Boundaries

- FDC/HDC implementations and their configuration/observation declarations
  move to ibmpc-common. Production composition holds opaque, stable candidates
  for early DMA binding and subsequent configuration rollback.
- AT KBC moves to ibmpc-at. The adapter owns keyboard/AUX/controller state;
  composition owns PIC leases and supplies IRQ line, A20 and reset sinks.
- XT PPI moves to ibmpc-xt with its DIP/fault configuration. Composition owns
  the PIC lease; the adapter emits an IRQ line instead of depending on common
  PIC. The original Mode-0 byte, line, speaker and NMI behavior is retained.
- Video PC routes/configuration/snapshot adaptation moves to ibmpc-common.
  Production composition uses opaque video state, reset/advance and copied
  snapshots rather than accessing the video chip.
- AT Port B/parity latch/NMI has one opaque ibmpc-at owner. Composition keeps
  the physical speaker wire, without duplicate XT gate/data fields. The public
  adapter matrix also builds independently without App.
- D4 RAM decoding source/header moves to Model40. Its memory providers, parity
  producer and write observer now use the D4 RAM state as their sole owner,
  not the containing board. One IOCHK signal connects it to platform state;
  reset/address decoding/atomic publication remain unchanged. Composition
  revokes these routes before releasing the containing allocation. The D4
  final integration with the common opaque lifetime is still unfinished.
- D4 Port B, NMI/failsafe and refresh state has one opaque Model40 platform
  object. Its port route and PIT callbacks belong to that object; composition
  destroys it before the borrowed PITs. RAM IOCHK binds directly to this
  Profile owner, with no intermediate board callback. Reset latch, post-PIT
  wiring and final refresh reset preserve their distinct original phases.
  App production no longer stores those latches or refresh bytes. Temporary
  private mechanism fixtures still require final owner reconciliation.
- One frozen board Profile binding now dispatches the original reset phases,
  NMI refresh, refresh arbitration, absolute deadline and finalization. D4 RAM
  state is contained by its opaque platform allocation, with one teardown
  before borrowed PIT destruction. The board no longer embeds any D4 RAM
  registers or hardcodes the refresh deadline. Construction/diagnostic helpers
  and frozen-plan D4 choices still reside in App and must be separated before
  the whole board can move; this is not a finished Shared receiver.

The root still selects normal or observable neutral Core. Family targets do
not link back to ibmpc-common or import App headers. No chip algorithm or
firmware/media selection is intentionally changed.

## Observed Verification

Current source builds core-machine and vm-profile in
build/t540-s4-nxvm-x64 and build/t540-s4-nxvm-x86-winlibs.
Sixteen rebuilt original unit targets pass in each tree:

- port-assembly, input-display-s5;
- kbc-controller, kbc-aux-port, kbc-serial-cadence, xt-ppi-keyboard;
- fdc, hdc;
- display-authority, ega-registration-transaction;
- cga-graphics-port, cga-640-port;
- ega-external-port, ega-sequencer-port, ega-controller-port, ega-planar-port.

Each name above uses the core-machine prefix and smoke suffix. CTest labels
are unit. The final x64 run reports 16/16 in 11.03 seconds; x86 reports 16/16
in 4.04 seconds. This is the temporary selected set, not the full unit suite.
The source x86 corpus dependency verifier passes; git diff --check passes.
Git reports no asset or MyNES changes. No products have been deployed or
booted for this intermediate source state.

An intermediate video test failed because moving board-declaration validation
into low-level configure broadened its rejection semantics. The fix retains
the original split: composition validates the full board declaration, while
the adapter transaction preserves its original configuration behavior. The
original EGA transaction assertions then pass; no expected result was relaxed.

After AT parity extraction, its independent public test plus the original
parity/D4-platform/port-assembly tests pass 4/4 per width (x64 4.81 seconds,
x86 0.47 seconds); standalone App-free x86.parity passes 1/1. After the D4 RAM
owner move, the original D4 memory transaction, platform and port-assembly
tests pass 3/3 per width (x64 2.76 seconds, x86 0.46 seconds). The D4 memory
route ownership gate passes with the actual Profile source path. These are
successive intermediate results, not a full-suite result for final source.
The transaction fixture additionally drives the installed D4 parity observer
and a physical RAM write: IOCHK asserts and clears through the new signal,
while failed construction publishes neither a signal binding nor an event.
Its rebuilt test passes per width (x64 1.34 seconds, x86 0.12 seconds).
After moving the platform/refresh state, six rebuilt original tests pass in
each tree: port-assembly, time, prefetch-locality, d4-refresh-hold,
d4-platform-s4 and d4-memory-transaction (x64 6.22 seconds, x86 0.67 seconds).
The D4 route gate also passes. Two tests previously injected refresh state
without constructing any D4 board; they now construct actual D4 topology
before freezing Core, retaining the same grant/deadline assertions. The final
direct IOCHK binding and removal of the redundant configured flag are rebuilt
and pass those six tests per width (x64 6.25 seconds, x86 0.56 seconds), plus
the D4 route gate. Published handle existence is the configuration authority;
failed construction leaves no handle, routes or PIT callbacks.
With Profile lifecycle binding and copied D4 memory/video bus observations,
the six rebuilt regressions pass x64 6/6 (1.82 seconds) and x86 6/6
(0.67 seconds); the D4 gate passes. The port transaction fixture also rejects
an incomplete Profile binding without publication. Both integration boot
probe executables compile again after replacing the stale Core-to-board
lookup with explicit borrowed composition, and adapting opaque-handle calls.
No boot run is claimed. FDC/KBC/XT diagnostic private fixtures are explicitly
temporary; they must not survive complete receiver acceptance.

## Still Required Before Acceptance

- Accept the now physically received common construction/frozen-plan/reset/
  advance/deadline/teardown batch after completing its full test-owner audit.
- Complete Profile-local D4 integration into the one common attachment
  lifetime without private board getters or cycles. Lifecycle dispatch is now
  bound, and Profile-specific constructor/plan/query helpers have left the
  common receiving source. AT parity, D4 RAM and opaque platform/refresh
  receivers are working but not yet accepted.
- Reconcile every remaining App devices file and private test-layout reader.
  Mechanism fixtures still importing relocated private headers are working
  migration state, not an accepted cross-component contract.
- Consolidate old private construction helpers and declarations where the
  actual owner permits, keeping failure/retry assertions and one production
  path. Regenerate manifests only after the complete source batch settles.
- Run complete dual-width units, independent Shared tools-on/off suites,
  specialized/negative gates, all manifests and documentation verification;
  build/deploy eight fresh products and run all eight unchanged-INI checkpoints
  once. Deliver full target-separated commits, push and actual-diff review.

No partial compile or selected-test success satisfies the S93 exit criteria.

## Constructor Cut And Physical Whole-Board Move

The old common D4 configure/clear/report/observe API and implementations are
deleted. Direct fixtures construct a Model40-owned opaque object and use its
public operations; session diagnostics borrow the selected factory's actual
object. The common plan retains only a scoped construction factory/context,
never D4 config or vendor state. Failure cleanup releases only PIT output
bindings the candidate actually installed; a failed unpublished candidate
cannot revoke another owner's existing callbacks.

Seven files physically move to ibmpc-common: machine_board source/interface/
state, machine_plan, machine_display, board_advance and board_deadline. NXVM's
former board archive now owns only its two concrete Model40 sources. Shared
owns one board source list with production and explicit trace-test variants;
composition selects a single Core and board variant. Shared source contains
no App include and the corpus dependency gate passes. The D4 route gate passes.

The independent x86.board test links without App and covers keyboard family
times PIT personality (four rows), construction, reset and port installation.
It and the existing exact pit_ports test pass 2/2 in 0.76 seconds. Its initial
immediate count assertion incorrectly treated CR as already loaded CE; that
new assertion was corrected at the test boundary, not by changing the PIT
algorithm or any original expectation. Exact next-input-clock loading remains
covered by pit_ports.

The physically moved batch builds in both root widths, including the boot
probe. Five rebuilt original regressions pass x64 5/5 in 5.08 seconds and x86
5/5 in 0.40 seconds: platform, port assembly, time, refresh hold and prefetch
locality. These do not establish full-unit, boot or artifact acceptance.

The full dual-width unit build was attempted and rejected stale test coupling:
opaque adapter handles still treated as embedded objects, private controller
configuration reads, and direct keyboard-chip accesses in Profile fixtures.
The handle/configuration callers are being reconciled against their real
owners. build/s93-build-x64.log and build/s93-build-x86.log retain the actual
diagnostics. No full-unit success is claimed. MyNES, all assets and owner INIs
remain unchanged; no S93 commit, product deployment or boot is claimed.

After the mechanical opaque-handle and frozen-topology fixture reconciliation,
the second x64 full-unit build reduces to six source failures: pcat topology,
pcat composition, pcat ownership, Model339 clock contract, Model40 FDD and XT
Profile. They still read the KBC/keyboard or FDC private implementation, or
refer to the old board FDC field. Other originally failing
Profile/FDC/HDC fixtures now compile and link. Those six require behavioral
contract reconciliation or true owner-local test placement, not added private
includes merely to make the build green. All build handles from this run have
terminated; no live process or unreported wait is claimed.

The Model40 FDD and XT Profile failures above were stale config-layout reads.
They now inspect the existing frozen board topology without another API or
copied config. Their rebuilt original x64 tests pass 2/2 in 1.53 seconds.
The four remaining KBC/keyboard/FDC ownership consumers still require the
complete test-boundary reconciliation; the full unit build remains unaccepted.

## Public Behavioral Fixture Reconciliation

The four remaining compile failures are resolved without production getters.
The KBC reply fixture now uses the public Core bus and opaque AT controller.
Default AUX selection is checked by command-byte bit 5 through port 64h/60h;
Model40 absent AUX and F5 scanning-disable assertions use the same production
command/input path rather than controller/keyboard layouts. Model339 preserves
the exact 4,000,000 initial and 800,000 repeat intervals, checks no output one
tick before the edge and receives translated repeat bytes from the actual KBC.
FDC ownership is checked with two real Core compositions: a command result is
local, and destroying one composition does not remove the other's routes.
Immediate FDC command phases settle through bounded Core instruction execution,
not a fake deadline or an immediate compatibility branch.

Seven rebuilt original x64 regressions pass 7/7 in 2.43 seconds. Both complete
unit target builds succeed. The first complete x86 run reports 474/476 in
65.44 seconds: controller-authority crashes and FDC boundary-negative fails.
GDB locates the crash at board refresh_nmi(NULL): after splitting parity/D4,
the outer function dereferences the null owner before the former lower-level
guards. The original null/no-op behavior is restored at the board entry.
The FDC gate omitted the new ibmpc-common source root; its scan now includes
that root, and the unchanged seven negative injections all pass. The original
controller-authority plus two Model40 fixtures pass x86 3/3 in 0.22 seconds.
The first full logs are retained; complete post-fix reruns are still required.

Private board/controller reads elsewhere in App fixtures and boot diagnostics
remain explicitly unaccepted. This reconciliation is working progress, not
S93 closure, a product deployment, or permission for a partial commit.

The two remaining App devices headers are mechanically relocated to Machine:
guest_input_interface.h and guest_display_interface.h. Their copied-value
definitions and existing symbols are unchanged. All six source/test/gate
includes are updated; no compatibility header remains at the former path.
The former devices directory now has no files. This changes the current source
after the diagnostic full-unit runs; final source verification must rebuild it.

The post-repair complete x86 run passes 476/476 in 69.80 seconds, retained in
build/s93-unit-x86-after-reconciliation.log. This is diagnostic proof before
the copied-value header relocation, not final-source/artifact acceptance.
The ownership gate now rejects any file or build reference under the retired
App devices root instead of checking a short subset of former providers. Its
callback and keyboard checks use the actual Shared paths and opaque pointer
ownership; the Core/VM scan includes all four neutral/common/family roots.
These three static gates and the x86 corpus dependency gate pass. Remaining
App private fixture and boot-diagnostic imports are still not accepted.

The tools-enabled standalone x86 unit suite is rebuilt after the shared NMI
fix and passes 136/136 in 32.62 seconds without an App target. This is the
unit-labelled runtime suite, not the complete tools-on/off static/manifest
acceptance. NXVM documentation governance passes. The x64 post-repair full
unit run terminates successfully: 476/476 in 242.51 seconds, retained in
build/s93-unit-x64-after-reconciliation.log. Both full diagnostic runs are
green, not final-source/artifact acceptance after the remaining reader cleanup.
After the copied-value header moves, both widths' Core/Machine component
targets rebuild successfully. All build/test handles from this run terminate.

The final source sweep finds no non-interface x86 header include in NXVM
production source. App tests still contain 124 files importing private
ibmpc-common/at/xt headers (exact include-regex inventory retained as
build/s93-app-private-board-readers.txt). These are migration readers, not
124 proven hardware defects. Each must be rehomed with its actual mechanism
or converted to public behavior at its product-composition boundary before
acceptance; compiling them does not satisfy test-owner closure.

The next receiver batch relocates the original DMA binding-token and Compaq
HDC/FDC shared-port tests into test/x86/ibmpc-common. Their target/test names,
cross-machine token rejection, valid construction/reset assertions and drive
address mask/value are retained. These are Board-owned construction tests:
they retain their owner's internal topology interface but no longer import
Core's private machine/port layout. Compaq port access uses the existing public
Core bus operations and checks their results. Both targets now link only
x86-ibmpc-common and x86-core, not an App or Model40 target; the former source
locations and duplicate product target declarations are deleted.

An initial build correctly rejected the attempted removal of the Board's own
internal topology declarations. Restoring the owner-local include, rather than
exporting those declarations or adding a getter, resolves it. The unrelated
App construction helper retains its prior include pending its whole-owner
migration. Standalone build and both original tests pass 2/2. The T344 gate
tracks the two actual receiver paths and still proves all 119 classified
constructors; the x86 source dependency gate passes. Complete root units pass
476/476 on x64 (66.06 seconds) and x86 (64.62 seconds). Each build tree's
Testing/Temporary/LastTest.log retains the actual test records. Both test
processes are terminal. This is test-owner migration proof, not final S93
acceptance, artifact deployment or boot proof; no partial P is submitted.

The auxiliary-PIT isolation regression also moves into the Shared Board suite,
retaining both counters, the 1:4 source ratio, exact 1/3/4-tick advances,
latched byte reads and reset cancellation assertions. Port operations now use
the existing public Core bus, including status checks. Unlike the former
private route-presence queries, those operations require a reset machine;
their route checks therefore run after freeze/reset. An initial runtime
failure exposed that lifecycle requirement and the corrected fixture passes.
No production behavior or contract is changed. The test still imports Core's
internal arbitrary-time operation to retain exact timing proof; that remaining
peer-private test dependency is explicitly not accepted as a clean boundary.
It must receive an owner-correct disposition in the full test reconciliation,
not be hidden by exporting a test-only production API or weakening timing.
Standalone auxiliary-PIT passes 1/1; the three migrated Board tests pass 3/3
in each product width. T344 retains its 119-constructor classification. The
previous 476/476 runs precede this last receiver edit, so they are not proof
of final-source full acceptance. No test process remains live.

The FDC topology and media-change regressions and their one existing fixture
now belong to test/x86/ibmpc-common. All eight former fixture callers follow
its actual receiver; no old helper copy remains. Both migrated tests retain
their target names and Board-local FDC/PIC/DMA assertions. Their port owner is
an opaque Core handle using the existing public bus; private Core machine and
port headers and executor_port accesses are removed. The Shared fixture owns
the common FDC adapter assay and retains the original chip-public capture,
seek bounds and exact device-time helpers. This does not yet accept remaining
App fixtures which use those internal assays indirectly.

Against the committed originals, topology retains 17 failure-check sites and
media-change retains 31. All unit selections, sector-byte counts, READY-change,
IRQ, DREQ, reset and media-generation assertions remain. Both independent
builds/tests pass 2/2 without an App target. The constructor inventory retains
all 119 entries at the actual receiver paths and the x86 source dependency
gate passes. The current direct-private App include sweep finds 118 files,
down from the earlier 124, not 118 hardware defects or a completed boundary.
All affected root unit targets rebuild per width without a MyNES target.
Complete units run sequentially and pass 476/476 on x64 (107.62 seconds) and
x86 (57.61 seconds), including the original six remaining App fixture callers.
Logs are build/s93-fdc-receiver-unit-x64.log and
build/s93-fdc-receiver-unit-x86.log. Build and test handles are terminal;
diff whitespace, constructor inventory and source dependency checks pass.
These runtime results do not accept the remaining direct/indirect private
dependencies or satisfy final manifests/artifacts/boot proof. No product
binary, owner INI, MyNES source/test or external asset is changed.

The complete original FDC command/DMA matrix also moves to the same Shared
Board receiver. Its 1,171-line committed source becomes 1,169 lines; all 174
failure-check sites and eight DMA transfer calls remain. All transfer calls
retain the original two-controller assay through the existing Shared DMA
fixture, removing the App port-owner fixture dependency rather than creating
another transfer helper. The two RAM writes retain their physical address,
source bytes and lengths through public Core memory_write. Port operations
use the public bus with checked status. No Core-private header, executor RAM
or executor port access remains in this matrix; FDC private mechanics remain
with their own common-adapter test owner and chip capture remains public.
The product target declaration/old source are removed and the original target
name is independently registered in test/x86. Both FDC gates, their baseline
and seven unchanged negative controls follow the real receiver and pass.
The 119-constructor inventory passes; standalone complete FDC matrix passes
1/1 in 4.69 seconds. The direct-private App include sweep is now 117 files.
Shared test targets rebuild in both product trees. Root whole units run
sequentially and pass 476/476 per width: x64 57.81 seconds and x86 56.77
seconds, retained in build/s93-fdc-matrix-unit-x64.log and
build/s93-fdc-matrix-unit-x86.log. All handles terminate. The actual standalone
Ninja link uses x86-ibmpc-common, one x86-core and the required chips/families,
with no App or Model40 archive. Remaining App fixture, Profile and
boot-diagnostic dependencies are not accepted by this relocation; final
manifests, artifacts and boots remain outstanding. MyNES and deployed assets
are unchanged and no partial P is delivered.

## HDC Receiver Reconciliation

The complete HDC regression and its unique fixture now reside in
test/x86/ibmpc-common. Former App unit and integration callers reference that
same receiver; no old fixture copy remains. The HDC matrix retains all 13
original failure-check sites and removes an unused private Core include.
The Compaq HDC regression also moves to this receiver. Its manually initialized
stack Core and raw port registrations are replaced by opaque Core/HDC creation,
one atomic route batch, freeze/reset and checked public bus access. The original
15 failure-check sites remain, with one additional construction-failure check.
Dual media data, IRQ acknowledgement, alternate-status behavior, 3F7 wired-OR,
verify/reset and no-media ready checks remain; no production getter is added.

HDC connect/initialize are now file-local functions, and duplicated public API
declarations are removed from its private layout header. The public configure
path and its rollback are unchanged. Independent HDC/Compaq tests pass 2/2 in
0.45 seconds without an App target. All root unit targets rebuild on both
widths, and all six affected HDD/DOS/Windows integration executables compile
on both widths. These are compilation results, not boot acceptance.

The full specialized-gate run exposes remaining S93 failures: the floppy boot
matrix still accesses FDC private state, the dependency allowlist still names
old App devices edges, the raw-borrow scanner treats opaque handle names as
complete layouts, and board/family boundary checks still encode old ownership.
HDC and VADP ownership checks are corrected to require Board-owned opaque
pointers and reject either pointers or embedded device objects in neutral
Core; both pass. The stale devices edges are removed from the DAG allowlist;
its three real Machine-to-Profile edges remain and the check passes. The
raw-borrow scanner now distinguishes opaque declarations/pointer parameters
from complete layouts/by-value use and rejects private x86 board headers.
Its real-tree check passes; isolated probes accept an opaque handle and reject
a layout definition, by-value field and private include (one positive and
three negative controls). Port B, video routes, peripheral advance, refresh,
PIC locality, event-deadline checks and the floppy-matrix diagnostic compilation
remain open, not waived. T344's retained
119-constructor inventory, ATA feature boundary, x86 source boundary and
diff whitespace checks pass. Full units run sequentially and pass 476/476
on x64 (248.55 seconds) and x86 (64.85 seconds), retained in
build/s93-hdc-receiver-unit-x64.log and its x86 counterpart. Both handles are
terminal. The higher x64 runtime is recorded without attributing a cause;
test success is not complete S93 component, gate or boot acceptance.

## Board Gates And Boot Diagnostic Boundary

The six remaining ownership checks now follow the actual common-board,
AT-parity and Model40 D4 receivers. They retain publication/rollback order,
Core-only time advancement and refresh-notification assertions. All six pass;
each also rejects an isolated mutation of its required mechanism in
build/s93-board-gate-negatives.log. The full x86 specialized target finishes
successfully in build/s93-board-receiver-gates-x86.log.

The floppy boot matrix no longer imports Core/board layouts or replaces a
frozen FDC observer. It reads the existing Machine-owned copied last-terminal
observation and passive Core trace. Private live chip diagnostics and counters
which were never populated are removed; DOS/installer checkpoints, 301/303
rejection and all CMOS seed comparisons remain. Existing transaction owner/kind
enums move unchanged to trace_interface.h so public trace consumers can decode
the already-published detail bits. This adds no operation or transaction owner.
Both width unit-target rebuilds and the boot-probe build complete successfully.
The independent tools-enabled suite passes 144/144 before the trace-header
move; that earlier result is not final proof for the changed header.

The App board fixture now installs reset-memory aliases through the existing
public alias operation and obtains CPU identity through its public query.
The 15-byte prefetch window and narrower-bus aliases remain unchanged. All
unit targets rebuild successfully per width in build/s93-alias-build-x64.log
and its x86 counterpart. Full x64 units are running, so runtime acceptance is
still pending; other fixture private accesses still
require reconciliation. No S93 commit, product replacement or closure is claimed.

## Xebec Receiver And Reset Fixture Verification

The full x64 unit run after the public reset-alias fixture change passes
476/476 in 250.09 seconds (build/s93-alias-unit-x64.log). That run precedes
the Xebec receiver relocation, so it is not final verification of this batch.

Xebec wiring now resides in test/x86/ibmpc-common under its original target
name. All 25 original failure-check sites remain. Port and RAM execution use
existing Core public operations; the fixture no longer imports private Core
layout. The ten route-presence assertions use temporary public registration
conflicts before freeze: successful absent-route probes are removed immediately.
This preserves the distinction between an installed read callback and a
device-returned UNSUPPORTED status without exporting a route getter or relying
on optional trace. No probe callback is executed or retained after construction.

The existing Shared DMA transfer fixture accepts the actual controller count.
Its full DMA and FDC callers still select two controllers; Xebec selects one.
There is no second transfer loop. Original sector data, DMA request/TC, IRQ,
DCB deadline, initialize/reset and no-ATA-alias checks remain. Independent
Xebec/FDC/DMA tests pass 3/3 in 4.73 seconds in
build/s93-xebec-shared-unit.log; both width unit targets rebuild successfully.
The historical constructor gate still classifies all 119 sources at their
current receivers. The latest complete x86 suite passes 476/476 in 77.85
seconds (build/s93-xebec-unit-x86.log). Latest x64 units and its specialized
target are still being verified; S93 remains active and no product/MyNES/INI
change is made.

## Public Fixture Cut And Remaining Owner Tests

The latest Xebec batch passes complete x64 units 476/476 in 61.16 seconds
(build/s93-xebec-unit-x64.log); the x64 specialized target also finishes
successfully in build/s93-xebec-gates-x64.log. These results precede the next
fixture cut and are not current full-tree acceptance.

Memory-device fixture registration now uses the public batch contract. MOVX
publishes its two ranges in one transaction under its original provider owner.
The timing-checkpoint read uses checked public bus access. MOVX, checked-memory,
transaction-lifecycle and timing-checkpoint pass 4/4 on x86 (0.39 seconds).
PIT divider moves to its common-board owner and independently passes its
original edge/reset-phase assertions. Firmware capability/rollback moves to
test/x86/core and independently passes; its construction-phase private route
queries are now owner-local, while its stopped query uses the public contract.
The App fixture's private PIT setup and configuration-query helpers are removed.

The App board fixture no longer transitively includes CPU, Core or Board
private headers. Full builds expose remaining consumers of those private
mechanisms rather than hiding them with the old helper. Logs
build/s93-public-fixture-full-build-x86.log and its x64 counterpart retain the
compile failures; they must be reconciled at each actual owner before closure.
Public FLAGS constants replace their same-value private spellings. Raw RAM
reads are moving to public operations without changing addresses or assertions.
Seven fault-result tests need observational memory_inspect, not operational
memory_read restricted to stopped/paused. Their original RAM/stack invariants
pass with the existing observation contract: nine instruction regressions
pass 9/9 in 0.55 seconds (build/s93-public-memory-unit-x86.log).

The current whole tree is not build-green. CPU-state/prefetch/timing fixtures,
remaining board-layout readers and the auxiliary PIT private time advance are
still unresolved; no production getter or reintroduced transitive private
include is accepted as a remedy. No S93 P or product replacement is delivered.

## Reset Entry And Core Mechanism Receivers

The 80286 protected-mode and final real-exception fixtures configure their
original segment/EIP entry through existing public debug patches after reset
returns STOPPED, rather than borrowing the private execution context inside a
reset callback. No production API or hardware algorithm changes. Their guest
programs, register masks, vectors and failure assertions remain unchanged.

The FPU and 80386 paging tests now reside at test/x86/core and have independent
Shared registrations under their original target names. Core-local completion
and prepared-entry mechanisms remain with their actual owner, not hidden by an
App fixture. Paging retains its reset callback and prepared-entry semantics;
faulted RAM results use existing memory_inspect rather than operational reads.
Its observable variant links one x86-core-observable implementation, not both
Core variants. The constructor gate classifies the same 119 original sources
and checks these receivers for absence of App fixture dependencies.

Independent FPU/paging tests pass 2/2 (5.89 seconds). The four affected tests
pass 4/4 per width (x64 0.47 seconds; x86 0.33 seconds). The latest complete x86
unit-target build is still red: build/s93-build-x86.log records 38 failed
targets, compared with 42 in the preceding owner inventory. Full units were
not run on this red tree. Manifests, complete gates, final products and boot
checkpoints remain outstanding. MyNES, deployed assets and owner INIs have no
working changes; no S93 P is delivered.

## Port, Retirement And Debug Board Receivers

Port read/write conflict checks and debug register/exception/PIC checks now
reside with test/x86/ibmpc-common. The port test borrows only its own Board
layout and public Core operations; the debug test replaces private reset CPU
mutation with a stopped public register patch. Retirement/FWAIT metadata
checks live at test/x86/core, retain the exact FPU preparation, source ticks
and origin/formula assertions, and install reset aliases through public Core
operations. None of these receivers depends on App fixtures or product code.

FPU ESC remains a public-boundary test: copied CPU snapshots and a stopped
CS/DS/ES/SS/EIP/CR0 patch replace the private fixture. Original NM delivery,
stack frame, general register and profile assertions remain. No new API,
hardware behavior, guest code or timing expectation is introduced.

The three moved tests pass independently (port/retirement 2/2 in 2.05 seconds;
debug/PIC 1/1 in 3.51 seconds). All four regressions pass per width (x64 5.02
seconds; x86 0.75 seconds). T317 verifies source identity after resolving
relative target sources against SOURCE_DIR and existing inventory paths against
their declared root/target base; it does not exempt Shared receivers. T332
checks the migrated debug fixture's public lifecycle and rejects private CPU
setup, retaining 44 owners. T344 still classifies 119 constructors.

The latest full x86 unit-target build (build/s93-build-x86.log) has 34 failing
targets, down from 38 before this batch. Full units are not acceptance-green;
remaining fixture mechanisms, final gates/negatives/manifests, products and
unchanged-INI checkpoints remain. No P, MyNES rebuild or asset edit is made.

## Public Timing And Descriptor Receivers

T359 S2/S3 and legacy timing normalization replace only their raw word-register
setup/readback with existing public debug operations, retaining guest bytes,
expected source ticks and provider advance assertions. Their sources and the
unique public Board fixture now live in test/x86. All 64 fixture consumers
include its actual location; no App/private include shim remains. The timing
targets pass independently 3/3 and per width. The complete x86 build drops
from 34 to 31 failed targets after the public-register cut.

The old descriptor source contained two live cases. C7 ES-override bus/RAM
behavior now constructs neutral Core and links only x86-core. Protected-mode
exit and every original hidden segment-cache assertion move to the existing
CPU control-state receiver; no CPU algorithm or interface changes. This is a
real owner split, not a Core fixture lending CPU state to a peer. Both receivers
pass independently 2/2 and fresh root builds/tests pass 2/2 per width. CTest
results from the preceding failed root configuration are stale and excluded.
The unused old fault branch had incorrectly put this source in T337's #UD
inventory; that row is removed, while actual CPU #UD receivers remain.
T344 retains all 119 constructor identities, including neutral_create.
The complete x86 build then records 30 failed targets.

## Neutral Instruction Timing Receiver

machine_instruction_timing_smoke moves from App to test/x86/core and links
only x86-core. Core's copied executor config replaces Board construction;
explicit reset aliases preserve both 286/386 reset windows. Quantum/reset,
all existing opcode ticks, stop and physical-qualification assertions remain.
Its private Core advance rejection stays owner-local. No production API or
new public test seam is added.

The terminal #UD case executes guest LIDT with limit 17h, then patches only
the stopped EIP through the existing debug interface. The failed instruction
must retire zero instructions and add zero ticks; cumulative elapsed time
must remain at the measured guest-setup time rather than being secretly
rewound. The diagnostic must identify #UD. This removes the private IDTR
mutation without weakening failed-instruction time rollback.

The receiver passes independently and per width. The latest complete x86
unit-target build, build/s93-build-x86.log, records 29 failures; full units
are not run on this red tree. T344 still passes with 119 identities. Remaining
private CPU/timing/Board fixtures, full verification, gates, six manifests,
eight fresh products and unchanged-INI checkpoints remain open. No partial P
or product asset is delivered, and MyNES and owner INIs remain unchanged.

## CPU Profile And Secondary Timing Owner Split

machine_cpu_profile_gate_smoke now lives at test/x86/core and constructs
neutral Core. Its live NOP/PUSHA/shift/ARPL/prefix/Jcc, POP CS and C6 bus cases
retain their original programs and fault/opcode/PC/memory assertions. Stopped
public register patches replace the private reset callback. Negative cases
use an unreadable vector-6 route rather than a private IDTR-limit mutation;
the same terminal #UD diagnostic and original first-opcode assertion pass.
The existing T337 IVT-reject disposition applies; no inventory exemption is
added. The former 8088 prefetch capacity/reservation/invalidation and stale
self-modified-byte cases move into CPU's execution signal/prefetch receiver.
All original private queue assertions remain with that owner, using the
existing CPU bus fixture instead of a borrowed Core execution context.

T359 S5 secondary integer timing also lives at test/x86/core and links only
x86-core. The existing public patch writes EFLAGS/EAX/ECX/ESI and a real-mode
FS selector yielding the same 1000h base; no privately inconsistent selector
zero/base-1000h cache is fabricated. Programs, addresses, expected source
ticks, provider advancement and budget preflight remain unchanged. Illegal
LOCK still retires no instruction and advances no time; explicit #UD/fault
and elapsed-zero checks additionally verify the intended terminal cause.
The two Core receivers use one exception_fixture.h unreadable-IVT route,
with no new production capability or mirrored machine state. T337's exact
inventory now includes S5 and the prior guest-LIDT instruction timing receiver.
T344 retains 119 original constructor identities and real source paths.

After final rebuilding, all four related receivers pass independently 4/4
(5.84 seconds), root x64 4/4 (4.02 seconds) and x86 4/4 (0.55 seconds).
The complete x86 unit-target build records 27 failed targets, down from 29
before this batch. Full unit acceptance, remaining private consumers, complete
gates/manifests and product/boot proofs remain outstanding. No S93 P, product
replacement or MyNES build is delivered; deployed assets and owner INIs are
unchanged.

## Legacy Instruction Ledger Receivers

The 8086 and 80186 ledgers now reside in test/x86/core and link x86-core
without an App or Board constructor. Every original static program/recipe
array is byte-identical: 83 arrays for 8086 and 61 for 80186. Expected ticks,
provider advancement, register/memory outcomes, budget refusal, reset, stop,
and overflow assertions remain. Core-local elapsed-time overflow injection
and the 8086 opaque FPU completion/WAIT check remain Core-owned. CPU state
seeds and observations use the existing public debug register operations;
low-word seeds still preserve the upper word. Terminal #UD uses the existing
unreadable-IVT fixture, not borrowed IDTR state or a new production API.

One debug_fixture.h owns the public checked register helpers for both Core
and Board tests. Keeping it separate from executor construction avoids the
old App/Shared executor fixtures' identical include guards; the affected
checked-memory consumer is rebuilt and tested, not hidden by an exemption.
T344 retains all 119 constructor identities with actual receiver paths.

Final independent timing receivers pass 4/4 (11.61 seconds); five affected
root tests pass x64 5/5 (6.65 seconds) and x86 5/5 (1.81 seconds). The latest
complete x86 unit-target rebuild records 25 failures, down from 27 before
this batch. Full unit acceptance is still red; manifests, final products and
eight unchanged-INI boot proofs remain outstanding. No partial P or deployed
asset is delivered, and MyNES and owner INIs remain unchanged.
The final specialized-target attempt is also red: its existing 80286 timing
manifest runner still imports the private CPU fixture. The result is recorded
in build/s93-ledger-gates.log and is not replaced by an earlier green gate run.

## Privileged Timing Through Guest Setup

T359 S6 moves to test/x86/core with a sole x86-core link. Its original thirteen
static arrays, including all ten instruction programs and timing rows, are
byte-identical. Direct CR0/GDTR/CS/DS/SS cache writes are removed. Seven guest
instructions execute LGDT, LMSW, DS/SS loads and a far jump; a copied public
snapshot verifies PE, selectors, bases, limits, ring 0 and GDTR before the
row is executed. The original fixture wrote the GDT before a later reset
cleared RAM and bypassed the absent descriptors with fabricated caches.
The receiver instead reloads the same GDT bytes after every reset.

No time is reset after preparation. The setup result establishes elapsed
time and must match actual provider advancement; each row still retires one
instruction with exactly its original cost. Cumulative Core and provider
time must include both setup and that cost. The original rejected LOCK case
still retires zero instructions and advances no time, using unreadable IVT
delivery instead of the former private IDTR mutation.

Final independent receiver passes 1/1 (3.90 seconds), root x64 1/1 (2.12
seconds) and x86 1/1 (0.15 seconds). T344 preserves all 119 constructor
identities. The complete x86 unit-target rebuild records 24 failed targets;
full-unit, gate, manifest, product and boot acceptance remain incomplete.
The earlier failed setup and stale executable result are diagnostics only,
not acceptance proof. No S93 P or deployed product is delivered; MyNES,
assets and owner INIs remain unchanged.

## Public Port Timing And Manifest Receivers

The T359 S4 string/port and 80386 protected I/O timing tests now live in
test/x86/core and link only x86-core. Their original 26 and two static arrays
are unchanged. One test-local public port-mode fixture serves both receivers:
guest LGDT/LMSW, segment loads, far jump, LTR and IRETD establish actual ring-0,
ring-3 and VM86 state. Copied snapshots verify it; actual setup time remains
part of cumulative Core/provider time. Original instruction costs, port counts,
bitmap rejection and budget assertions are retained without a time rewind.
Both receivers pass independently (2/2, 6.86 seconds), root x64 (2/2, 3.70
seconds) and root x86 (2/2, 0.64 seconds).

The 8086/8088 and 80186 manifest runners now create neutral Core and use public
register/snapshot operations. Their 116 and eight original static arrays are
unchanged. They remain NXVM-owned because they consume product ledger metadata
and produce its result files; their executable links only x86-core, not Board.
The 80186 runner retains all 616 rows and passes x64/x86. Both 8086/8088 runners
and both result-contract checks pass per width (4/4, 26.65/23.53 seconds).
This complete x86 unit-target rebuild reduced compile failures from 24 to 19;
it did not qualify the whole unit suite.

## Control/PIC Composition Receiver

The complete control-state-board test moves to test/x86/ibmpc-common. Its 15
original static arrays, profile/LOCK matrix, IRQ frame and ISR assertions, and
early-80386 MOV-CR cases remain unchanged. Public paused register patching
replaces raw CPU setup. A public Core overlay makes vector 6 unreadable before
freeze, retaining the explicit terminal #UD contract without private IDTR
mutation. Board-owned PIC handles remain at their actual test owner; no private
Core or CPU layout is imported, and no production API or algorithm changes.

The removed reset provider only performed the same initial register setup;
the receiver now performs it after the sole Core reset. It uses one explicit
constructor to publish the negative route before freeze, so T344 records 120
actual direct constructors instead of 119. All 101 historical identities and
the exact constructor classification remain checked; the gate passes.

The final independent receiver passes 1/1 (3.56 seconds), root x64 1/1 (1.56 seconds)
and root x86 1/1 (0.26 seconds). The subsequent complete x86 unit-target build
records 18 failed targets in build/s93-build-x86.log. Full units, specialized
gates, final manifests, products and boot acceptance are still incomplete.
No S93 commit, deployed EXE, MyNES, owner INI or asset change is delivered.

## CLI/STI And HLT Receiver Batch

The base CLI/STI, S48 and HLT tests now build at test/x86/ibmpc-common with
only x86-ibmpc-common and x86-core. Public register patches, copied snapshots
and the public halted-state observation replace borrowed CPU execution state.
S48 retains all six original static arrays and HLT all three; opcode, prefix,
LOCK, IRQ masking/shadow, stack return IP, ISR/IRR, FLAGS and GPR assertions
remain. Negative real-mode cases install an unreadable vector-6 route before
freeze, only for that negative fixture. Ordinary programs can still access
the IVT RAM; applying this overlay indiscriminately overlapped a positive
PIC-mask program and was rejected during development.

Protected/VM86 preparation now executes LGDT/LIDT/LMSW, segment loads, far
jump, LTR and IRETD without running setup to HLT or clearing private state.
Copied snapshots verify the resulting GDT/IDT/TR and privilege state. The
old ring-3 fixture fabricated CPL=3 while retaining selector 8 and a ring-0
descriptor. The real entry instead uses selector 1bh; its exception-frame
assertion is corrected accordingly. Real IRETD also supplies FLAGS bit 1.
These are fixture corrections, not unchanged-byte claims or timing upgrades.
HLT's terminal protected #DF case explicitly removes its #GP gate and retains
the original fault/rollback contract; VM86 uses the actual loaded TSS and
delivers the original #GP handler. No production algorithm or API changes.

CPU control-state's owner-local matrix retains exact complete segment storage
and GPR preservation, and full t_cpu equality on rejected forms. It covers
CLI/STI/HLT across four profiles and four attribute forms, plus the original
386 LOCK domain: 60 cases. A further protected HLT fault case checks full
private CPU rollback. Board tests compare only the existing public copied
values; no hidden storage assertion is silently weakened or made public.

The old App S48 and HLT files are removed. The old base CLI/STI file remains
temporarily because the not-yet-received software-INT, IRET and combined
interrupt-return tests still include it. It is not linked as an independent
target and must be removed with those remaining consumers before S93 closes.
T332 retains 44 identities and now checks the Shared inherited construction;
T344 passes with 121 classified direct constructors and all 101 historical
identities. The new base replaces a formerly indirect fixture constructor.

Four final affected tests pass per width: root x64 4/4 (1.86 seconds), root
x86 4/4 (0.46 seconds). Independent final tests pass 4/4 (3.46 seconds),
recorded by CTest in build/t540-s4-shared-x86. The complete x86 unit-target build now records 15
failures, down from 18 before this batch; that red build is not full unit
acceptance. Full gates/manifests, products and eight boot checkpoints remain
outstanding. No S93 P, deployed asset, MyNES or owner-INI change is delivered.

## Interrupt Entry Receiver Batch

The interrupt-entry target now builds at test/x86/ibmpc-common, retaining all
24 original top-level cases and handler/frame/fault/PIC assertions. Public
Core creation, reset, memory and debug operations replace the reset provider
and private CPU caches. Guest LGDT/LIDT/LMSW, segment loads and a far jump
establish the protected state; ring-3 entry uses real IRETD. Gate kinds are
local encoded test bytes, not a new public CPU layout contract. Faulted-state
memory verification uses the existing observational memory-inspect API.
The existing two-round delivery fixture preserves handler-visible results.

The old limited-stack seed cannot be loaded unchanged through MOV SS: the
current executor checks ESP after every instruction, so it faults in the
setup rather than at the tested interrupt. The public fixture loads SS with
in-range ESP and then patches the original out-of-range stopped ESP. The
original fault/rollback assertions remain; no CPU algorithm is changed.
The code-limit negative now also clears the descriptor's upper limit byte,
matching the original zero cached limit. The original fabricated ring-3 CS
and stack become actual selector 0bh/13h caches through IRETD.

CPU-owned cpu_idt_privilege_entry_smoke retains the hidden assertions: two
NMI cases check consumed-on-success/no-replay and pending-on-rejected-entry,
while four delivery failures check complete private CS/SS cache equality,
register/FLAGS rollback, descriptor access byte and untouched stack bytes.
The Board observes copied public snapshots; no private getter is added.

The old App source remains only for not-yet-received including tests; its
independent target is removed and the Shared target is the sole linked source.
T332 retains 44 strict identities; T344 retains 101 historical identities
and passes with 122 explicitly classified direct constructors.
Independent and both root-width affected tests pass 2/2 each. The complete
x86 unit-target build records 14 failures in build/s93-build-x86.log, down
from 15. This is not complete-unit, manifest, product or boot acceptance.
No S93 P, deployed asset, MyNES or owner-INI change is delivered.

## Software INT Receiver Batch

S50 now builds at test/x86/ibmpc-common and the old App source is removed.
Public stopped register patches and copied snapshots replace raw CPU access;
the already-received CLI/STI and interrupt-entry preparation is reused.
All eight original static matrices are retained verbatim across Board and
CPU receivers. Board still checks real-mode interrupt frames and FLAGS across
five CPU profiles and 386 attributes, protected INT/INT3/INTO success and
rejection, terminal target/table faults, VM86 faults and PIC IRR/ISR behavior.
Its VM86 case executes real LTR/IRETD and supplies FLAGS bit 1 rather than
fabricating CPL=3 on a protected cache.

The original terminal #UD matrix limited private IDTR to 17h before frame
writes. A public unreadable-IVT route is not equivalent: delivery can write
the frame before that read fails. It was rejected as a replacement because
the original untouched-stack assertion failed. The 36 legacy attribute and
12 386 LOCK negative forms therefore retain their exact IDTR seed at the CPU
owner, checking full t_cpu rollback and unchanged stack bytes. Board does not
borrow IDTR or weaken that assertion. CPU-local positive coverage retains
the original GPR seeds and complete six-segment storage for 24 real transfers
and eight INTO-clear cases. Six CPU-local protected/VM86 cases retain the
full CPU equality asserted by the old S50 receiver.

Independent final affected tests pass 3/3 (2.05 seconds); root x64 passes 3/3
(0.91 seconds) and root x86 3/3 (0.27 seconds), measured CTest test time.
T332 keeps all 44 strict identities, and T337's terminal #UD owner now names
the actual CPU receiver. T344 still passes with 122 classified constructors.
The complete x86 unit-target build records 13 failures in
build/s93-build-x86.log. Full verification, manifests, artifacts and boot
checkpoints remain outstanding; S93 is not accepted or partially committed.
MyNES, owner INIs and deployed artifacts remain unchanged.

## VM86 And Hardware Priority Receiver Batch

VM86 delivery, hardware-delivery S3 and VM86 LGDT/LIDT S5 now build at
test/x86/ibmpc-common. The four obsolete App sources, including the earlier
interrupt-entry source used by S3, are removed. Shared source registration is
the sole linked implementation; no private CPU fixture remains in these
receivers. Actual LGDT/LIDT/LMSW, segment loads, far jump, LTR and IRETD build
VM86 state with the original six selectors. LTR loads an available descriptor
and produces the busy TR with its real 67h limit, rather than fabricating a
busy cache whose old 0bh limit disagreed with the descriptor.

IRETD supplies reserved FLAGS bit 1; the existing stopped debug operation then
sets the original test FLAGS. This preserves the original frame values, not a
new expectation mask or guest-time rewind. Paging uses existing stopped CR0/
CR3 operations; public observations retain exception frames, source memory,
register/table values, IRQ ISR/IRR and IRET round-trip assertions.

The CPU-local cpu_vm86_delivery_state receiver preserves all eleven original
prepublication failure cases with complete t_cpu equality and adds untouched
stack checks. Five exception/debug cases retain segment invalidation and the
original private DR0/DR6/DR7 breakpoint proof. Its two NMI cases retain pending/
masked versus consumed latch checks; an IRQ input through the CPU bus contract
also checks all four invalidated data segments and the nine-word VM86 frame.
PIC priority remains verified by the four original Board S3 scenarios.
No public layout getter, new production API or CPU algorithm is introduced.

The four affected tests pass independently (2.11 seconds test time) and on
both root widths. T332 retains 44 strict owners; T344 retains the 101
historical identities and passes with 122 classified constructors. The
complete x86 unit-target build records 10 failures, down from 13, in
build/s93-build-x86.log. This is progress, not full unit/product acceptance.
S93 remains open with no partial P, deployed artifact or out-of-scope change.

## IRET Receiver Batch

VM86 IRET and protected same-CPL IRET now build independently at test/x86/core
and link only x86-core. Public neutral construction, reset, guest LGDT/LMSW,
segment loads and far jumps establish their initial state. VM86 IRET retains
both original opcode forms, high-word selector truncation, all six copied
segment properties, the stack-boundary rollback and the existing paging case.
The CPU-local receiver repeats those four original cases, retaining valid/
kind cache assertions and full t_cpu equality rather than deleting fields
that do not belong in the copied public snapshot.

Protected IRET retains its eleven top-level success/failure cases, including
small-stack operand width, conforming code, user FLAGS filtering, nonpresent/
limit/type/DPL failures and SS-limit rejection. Preparation enters through
valid descriptors first; target descriptor failures are applied only after
the current code/stack caches have been loaded. The one-byte limited SS loads
with in-range ESP before the original stopped out-of-range ESP is applied.
Ring-3 setup executes actual IRETD with selectors 0bh/13h. Faulted memory reads
use existing observational memory-inspect. The CPU-owned receiver retains the
original hidden CS/SS cache equality and unchanged descriptor access byte.
No CPU algorithm, public production API or guest-time rewind is added.

The old App VM86 IRET source is deleted. The old protected IRET source remains
only for the pending S51 including receiver; its independent root definition
is removed and the Shared target is the sole linked implementation. T344
retains all 101 historical identities; the new direct neutral protected
constructor is explicitly classified, making 123 direct constructors. T332
still verifies 44 strict owners.

Final affected tests pass 4/4 independently and on both root widths.
The complete x86 unit-target build records 8 failures, down from 10, in
build/s93-build-x86.log. Full units, manifests, artifacts and boot checkpoints
remain outstanding. No partial S93 P, product deployment, MyNES or owner-INI
change is delivered.

## S51 And Interrupt-Return Composition Receiver Batch

S51 Board real-mode/PIC IRET and S4 software-INT/IRET/hardware-IRQ composition
now build at test/x86/ibmpc-common. Their register seeds and entry state use
existing public patches; observations are copied snapshots, memory and PIC
IRR/ISR. CPU-local S51 retains twelve positive real-mode forms and seventeen
negative cases, including full CPU/stack rollback and terminal-invalid
preflight. Protected IRET continues through its neutral Core receiver.
The four old App S51/S4/CLI-STI/protected-IRET sources are deleted, not retained
as parallel fixtures. No production algorithm or API changes in this batch.

Three affected tests pass independently and on x64/x86 after fresh builds.
The complete x86 unit-target rebuild in build/s93-iret-composition-build.log
records six failed targets; it is not a full-unit pass. T332 verifies all 44
strict owners with the neutral protected constructor explicitly checked;
T344 retains 101 historical identities and 123 classified constructors.
Full units, manifests, products and boot checkpoints remain outstanding.
No partial P, product replacement, MyNES build or owner-INI change is delivered.

## Call-Gate, I/O Authorization And Cross-Width Task Receiver Batch

Call-gate and TSS I/O authorization now reside at test/x86/core and link only
x86-core. Existing guest LGDT/LIDT/LMSW/LTR/IRET programs still establish
protected and user state; public stopped register patches replace the private
real-mode reset provider. All seven original I/O authorization rows remain.
Denied operations retain zero provider reads/writes and the executed #GP
handler marker. The existing public delivery/handler-round fixture moves from
Board to Core's debug fixture, with one implementation; no Core scheduler or
exception behavior is changed.

Cross-width task switching retains all eight JMP/CALL/task-gate/nested-return
cases in its public Core receiver. The CPU-owned receiver executes those same
programs and retains private TR busy type and LDTR validity assertions. Public
Core observes copied registers, Intel descriptor type, saved TSS memory and
descriptor busy bytes. No private getter or production API is added.
The three superseded App sources are deleted. The 286/386 manifest includers
reference the Shared recipes; the 386 manifest builds and passes per width.
The 286 manifest still has its earlier private-fixture/time-rewind dependency.

The call-gate, I/O authorization and interrupt-entry tests pass 3/3
independently and on both widths. Cross-width Core/CPU tests pass independently
and 2/2 per width. Static-array comparison retains 6/6 call-gate, 9/9 I/O and
10/10 cross-width arrays byte-identically. T332 and T344 remain green with 44
strict owners, 101 historical identities and 123 classified constructors.
The latest full x86 unit-target rebuild in build/s93-task-gate-iomap-build.log
records three failures: prefetch locality, 80286 instruction timing ledger and
80286 timing manifest. Full unit acceptance, final manifests, product artifacts
and boot checkpoints remain outstanding. No partial P or excluded change is
delivered; S93 remains active.

## 80286 Instruction Timing Ledger Receiver

The original ledger now resides at test/x86/core and links only x86-core.
All 109 original static byte arrays are retained byte-identically. Ordinary
register preparation uses existing stopped debug operations; table and system
selector preparation executes LGDT/LIDT/LLDT/LTR. Setup consumes actual guest
time; each original measured instruction cost is checked against the subsequent
Core/observer delta. Core-local overflow injection retains its true owner.
No CPU-private header or state getter is introduced.

Two migration setup errors were resolved without production changes: the
real-mode bootstrap moved from address zero to 0200h to avoid the frozen vector
6 rejection route; segment selector writes now use their real linear code/data
addresses instead of the original private seed's stale cached bases. Original
selectors, result values and measured instruction costs remain asserted.
The old App source and its root target definition are deleted, not duplicated.

Fresh independent and x64/x86 ledger builds/tests pass. T332 retains all 44
strict owners; T344 retains 101 historical identities and 123 constructors.
The complete x86 unit-target rebuild in build/s93-286-ledger-build.log records
two failed targets: prefetch locality and the 80286 timing manifest. It is not
a complete unit pass. No product artifacts, owner INIs or MyNES paths change;
S93 has no partial P and remains active.

## Prefetch/Locality Owner Split

The retained Core bus tests now build independently at test/x86/core. Neutral
construction replaces PC-board construction. Core-local external-cycle
begin/overlap/commit/cancel, DMA HOLD invalidation, physical-publication failure,
retirement wait, prefetch grants and configured memory/port windows retain
their exact assertions. The refresh-pulse invalidation case directly exercises
Core's existing public pulse operation, with the same 2-tick miss and reset
checks. A bounded test attachment supplies the refresh request/completion
input for the Core arbitration case; it does not model a second device or
introduce a production API.

The Model40 test now belongs to test/app-nxvm/unit/core/profiles/model40.
It keeps Core opaque and owns only the actual D4 latch. It verifies the
original prefetch grant/suppression/resumption/reset sequence and actual
PIT-driven refresh request/acknowledgement/commit/release over the retained
20-tick interval. Those ticks come from real one-tick FNINIT retirements;
the test does not inject arbitrary guest time or invent a deadline. D4 reset
clears the pending/pulse/address state. Core cache invalidation and D4 wiring
therefore have distinct real-owner receivers, not a cross-layer getter.

The real scheduler is compiled for tests with only its outgoing CPU grant
redirected to a spy, as before. The one shared test object serves both receivers;
the original CPU grant still executes. No production source/API changes.
The superseded mixed App test is removed, recoverable in the Git baseline.

Both receiver tests pass after fresh x86/x64 builds; the Core test also passes
independently. T344 retains 101 historical identities and explicitly adds the
new Model40 constructor, giving 124 classified constructors. The complete
x86 unit-target rebuild in build/s93-prefetch-receiver-build.log records one
failed target: the 80286 timing manifest. No full-unit pass, final manifest,
product build or boot claim is made; S93 remains active without partial P.

## 80286 Manifest And Full-Unit Reconciliation

The 80286 manifest no longer imports CPU private headers, the private CPU
borrow fixture or the inherited private protected-state implementation.
Fifty-two GPR/word seeds use existing stopped debug operations; thirty PC,
CS/TR observations use public reads or copied snapshots. The three protected
gate recipes use the existing public protected bootstrap, retaining their
original GPR/FLAGS seeds. Table-register changes execute actual LGDT/LIDT.
LIDT for the more-privileged interrupt is established before entering CPL 3.
The protected-system bootstrap stops after eight actual instructions rather
than executing HLT and mutating its latch; guest time is never reset to zero.
Setup temporarily clears TF/IF for the table-load instruction and restores
the original FLAGS before the measured retirement. No production API changes.

Thirty-nine of the original forty-one static byte arrays are byte-identical;
one retains identical bytes with the vector constant's renamed spelling;
the obsolete setup HLT is removed. All original manifest keys, costs and
classification/observer assertions remain. Fresh x86/x64 manifest tests pass.
The complete unit-target builds now have no failed targets.

The first complete executions passed 478/483 on both widths. All five
failures used paused/stopped memory_read in configuration, faulted or local
transaction-fixture state. Existing memory_inspect now observes the same
ROM/source/stack bytes without imposing that operational-read lifecycle or
device side effects. This repairs the tests' boundary, not production routing.
ROM rollback/capacity/reset-alias, LDS/LES fault rollback, segment-stack
rollback and DMA publication assertions remain. Focused tests pass per width.
Fresh complete unit executions then pass 483/483 on both widths (66.05/68.24
seconds), in build/s93-unit-x86.log and build/s93-unit-x64.log.

Four T388 gates still named the removed App timing-test paths. Their sole
change is to select the actual Shared receiver; every original condition is
retained. All four direct checks pass. Full specialized targets pass on both
widths in build/s93-manifest-gates-x86.log and
build/s93-manifest-gates-x64.log. T332 retains 44 owners and T344 124 classified constructors;
documentation governance and diff whitespace checks pass.

This does not complete S93: remaining private-fixture ownership, independent
tools-on/off proof, final manifests, fresh products and eight unchanged-INI
checkpoints remain. The 386 manifest and three other live sources still name
the legacy private CPU fixture; their green results do not prove that boundary.
No MyNES, asset, owner INI or production algorithm changes occur in this batch.
No partial P is delivered.

## Generic Timing Ledger Receiver

The generic instruction timing ledger now resides in test/x86/core and links
only x86-core. Its six constructors use the neutral executor. Public stopped
register operations replace every CPU-private GPR, FLAGS and CR0 access;
word seeds preserve the previous high half. The terminal-invalid producer case
blocks vector 6 through the existing memory-provider fixture before freeze,
instead of mutating the CPU's private IDTR. Core-local overflow and physical
classifier fault injection stay with their actual Core owner.

Both protected preparation recipes execute their original ten instructions
and stop at the far-jump boundary. They no longer execute HLT and then clear
the CPU latch by hand. Preparation time remains on the sole timeline; only
the observer's accumulated delta is reset before the measured instruction.
All 57 retained static byte arrays are unchanged; the two obsolete setup
HLT arrays are removed. Original timing, eligibility, budget, reset and
overflow assertions remain. No production algorithm or API changes.

Fresh independent and x86/x64 receiver builds and executions pass. T332 still
classifies 44 owners; T344 retains 124 constructors after selecting the new
source path. The old App source and executable definition are removed. The
remaining live private CPU fixture source consumers are the 386 manifest,
S65 FPU interface test and its inherited protected-state fixture. Earlier
483/483 full-unit results precede this receiver change; final full verification
remains required. No products, MyNES or owner INIs change; S93 remains active
without a partial P.

## 386 Manifest Public Register And Ring-Zero Table Preparation

The subsequent final receiver removes the inherited private protected fixture
and CPU-borrow helper. It reuses protected_16_bootstrap_fixture.h, retains the
original register seeds, and executes actual LTR/IRET for outer call gates and
inner interrupts. VM86 setup executes LTR before loading VM86 segments through
the existing stopped debug contract. The task-gate recipe executes LIDT after
its original ten-instruction bootstrap. No setup rewinds guest time or changes
the measured instruction/expected timing. The kernel-only prepare helper has
no unused user-code selector.

Fresh final manifest executions pass on x64/x86 (5.52/5.34 seconds). All 96
original static lib_u8 arrays remain value-identical; normalization accounts
only for replacing private VCPU_EFLAGS_IOPL with the same 0x3000u value.
The obsolete helper's only remaining test-source mention is the intentional
cpu_bus_boundary_negative injection, which must remain rejected. T332 keeps
44 owners. T344 retains 125 classified constructors and 101 historical
identities, mapping the retired private fixture to the real existing public
bootstrap header and scanning that header for its constructor. The generated
type-vocabulary support inventory now names the live shared debug fixture.
Complete per-width unit/gate results are recorded below; the targeted results
alone do not accept S93, artifacts or boot behavior.

The App-owned manifest retains its original corpus metadata and assertions.
GPR/FLAGS reads and writes use existing stopped debug operations; segment
base/TR observations use existing copied snapshots. S7 DS/ES preparation and
same-CPL far transfer DS loads use public segment writes rather than hidden
CPU cache mutation. Seven GDTR limit updates now execute real LGDT before
the measured retirement. Its CS override deliberately avoids assuming a valid
DS in outer-return preparation; the initial DS-relative version failed that
case and is not accepted evidence. Preparation restores code, EIP and FLAGS
without rewinding Core time. The post-LTR manual halt clear is removed because
the one-instruction setup budget never executes HLT.

Fresh final x64 and x86 manifest target builds and CTest executions pass
(5.49 and 5.23 seconds respectively). A regex comparison against HEAD checks
all 96 original static lib_u8 instruction arrays: none is missing or changed.
git diff --check passes. A transient process-initialization failure prevented
one earlier x86 launch; the successful fresh retry, not that failed launch,
is the recorded execution result. No production API/algorithm, owner INI,
MyNES or deployed artifact changes are introduced by this batch.

Privilege-transition/TSS/VM86 preparation and the inherited protected-state
fixture remain private consumers. They and final whole-batch verification
still block S93 acceptance; no partial P is delivered.

## FPU Interface Ownership Receivers

S65's mixed App test is removed. The Core receiver constructs the neutral
executor and uses existing stopped register operations and copied snapshots.
Its original CPU/FPU matrices, exception frames, extension transaction,
completion and next-deadline assertions remain. Its protected bootstrap executes
the original nine instructions from 0200h and stops before the handler, rather
than executing HLT and mutating the CPU halt latch. It never rewinds guest time.
The vector-6 rejection uses the existing memory-provider fixture before freeze.
VM86 inputs use existing CR0/FLAGS writes followed by segment loads through the
public stopped debug contract; no cached descriptor is borrowed from CPU.

The two WAIT/FNINIT IRQ cases now belong to ibmpc-common. They retain the
original PIC pulse, vector-20 delivery, saved instruction IP, GPR preservation,
ISR and IRR assertions. Board-private PIC access stays with that Board owner;
CPU observations are copied. Neither Core nor CPU includes Board private state.

The CPU-local receiver retains full t_cpu rollback for every original legacy
attribute, LOCK and incompatible-FPU failure. It also retains full cached
segment comparisons for the original no-FPU success matrix, prefixed 386
forms, 8087 FNINIT and original VM86 seed. These are not replaced by smaller
Core debug snapshots. Its small extension bus sink observes no PC policy;
the Core receiver separately proves the real extension transaction handoff.
Additional test code is the cost of retaining these distinct private-state
and integration assertions at their actual owners, not a production wrapper.

All three receivers pass independently and on x86/x64 after fresh builds.
Both complete specialized gate targets then pass in
build/s93-fpu-gates-x64.log and build/s93-fpu-gates-x86.log. Their initial
registration failure exposed the new Board IRQ target missing from NXVM's
unit inventory; that real target is now included, with the count/uniqueness
checks unchanged. The expected rejection diagnostics inside negative
self-tests are not verification failures; both final build exit codes are zero.
T332 keeps 44 strict owners. T344 now classifies 125 direct constructors:
its previous 124 plus the explicit neutral FPU receiver; all 101 historical
identities remain. No production source/API, MyNES, owner INI or deployed
artifact changes are made. The two remaining live legacy CPU-fixture consumers
are the 386 timing manifest and its inherited protected-state fixture.
Final full units, independent corpus, manifests, products and eight boot
checkpoints remain required before S93 acceptance; no partial P is delivered.

## Final Full Units And Independent Corpus Verification

After the final manifest receiver, complete units pass 485/485 on x86
(68.01s) and x64 (70.98s). Both complete specialized gate targets pass all
81 commands, including T332's 44 owners, T344's 125 constructors/101
historical identities and the 403-row strict-compilation matrix. The current
logs are build/s93-unit-{x86,x64}.log and build/s93-gates-{x86,x64}.log.

An earlier x86 whole-unit run failed the runner error-propagation test; its
isolated replay passed. Inspection found that its reset fault callback was
replaced while running. The test now waits for acknowledged pause before
installing that same callback, resumes, then requests reset and checks the
original error outcome. No production behavior or timeout changes. The
earlier failure is retained in s93-unit-x86-before-paused-reset-injection.log;
the later complete passes, not the isolated replay, are acceptance evidence.

Independent tools-on verification exposed two obsolete negative expectations:
PS/2 mouse and keyboard imports of x86/ibmpc-at/kbc.h now violate the
chip-to-board dependency rule, rather than include-path syntax. Both cases
still require rejection; only their expected reason changes to Forbidden
x86 edge. The verifier itself is unchanged. Tools-on builds pass on both widths:
195/196 first-pass cases succeeded, then the corrected negative and manifest
cases pass 2/2 per width. No successful hardware case was rerun. The earlier
failed run is not full-suite acceptance evidence.

Fresh standalone test/x86 trees build with X86_BUILD_TOOLS=OFF on both widths,
then pass all 190 cases (x86 109.97s, x64 86.00s). The trees are
build/t540-s93-shared-no-tools-{x86,x64}; tools-on trees are
build/t540-s4-shared-{x86,x64}. Their Testing/Temporary/LastTest.log files
record actual executions. No NXVM App or Common Debug dependency is needed
by the tools-off build. Source corpus verification also passes. All six
manifest checks, NXVM documentation governance and git diff --check pass.
Eight isolated NXVM Release product builds are in progress; product/boot
acceptance and actual-change review remain pending. No partial P is delivered.

## Current Product Verification

The fresh default and XT Release products and boot probes build on both widths.
Unchanged-INI one-shot checkpoints pass for default x64/x86 (`dos-prompt`)
and XT x64/x86 (`installer-running`), recorded in
build/s93-{default,xt}-{x64,x86}-boot.log. AT product links and boot-probe
builds also finish on both widths; its fresh checkpoints are in progress.
Model40 builds remain in progress. No earlier S92 boot result substitutes
for these S93 executions.

Both AT checkpoints subsequently pass with `installer-running`. All eight
product/build-probe jobs finish with exit code zero; Model40 one-shot
checkpoints are now running. PE width, 0.5.0540 banner, absence of `.debug`
sections and deployed-versus-linked SHA256 equality pass for all eight EXEs.
Every production link response contains exactly one libx86-core.a and no
observable Core; each EXE defines one core_machine_neutral_create symbol.
The checks are in build/s93-artifacts.ps1. Product SHA256 identities are:

| Product | SHA256 |
| --- | --- |
| default x64 | 6E8CF57F9F4DAD60B11529DC0302FCEE99FD1F8BF5930ADC5AADA583245CBDC5 |
| default x86 | 583729CFAFDD77E037396C7ADBC211E1099444ED170EC856C8DCF7C9826C7EF1 |
| XT x64 | 0B11BC1395B73432B52B29F6E157EC3A431CD6DB901939DB0103ACFD80793EA0 |
| XT x86 | E6BB282FCEA3EB4562EAC26E25ABF82842E2356A2C9090AF2DE5A81448E98D19 |
| AT x64 | FF4D39C9CC8E1E162ED89F755264A1DDFB08AB10AFB7424906E3DB6002977F9A |
| AT x86 | D2BB494E2FF55722AB2D1026FE129BD7778BD2650E28F44DED715CD54545A2E0 |
| Model40 x64 | 922D1888864FFDF038ED70B1DD68497EC79C2FF47E498DF0300375303CE04292 |
| Model40 x86 | 47BB6D2D7144BB67BB2B39E5AD4C84BE3F1C66C115BA10081B0839BF566548E6 |

The complete default-profile integration inventory contains 20 cases.
Its already-passed boot-matrix case is excluded from repeat execution;
the other 19 executables are rebuilt and run serially on both widths by
build/s93-integration.ps1. This verification remains in progress, not green.
MyNES source/test/documentation/artifact and NXVM INI Git diffs are empty.
Final hashes, sole-Core link inspection, remaining checkpoints and full
actual-change review still precede any S93 P delivery or acceptance.

All eight fresh unchanged-INI checkpoints subsequently pass, each once:
default x64/x86 at `dos-prompt`, XT/AT/Model40 x64/x86 at
`installer-running`. All boot processes exit zero. The x64 default-profile
integration remainder passes 19/19 in 56.72s; together with its already-passed
boot-matrix case this covers all 20 registered cases without rerunning it.
The x86 integration rebuild/run is still in progress. No S93 P is delivered.

## Final Private-Board Reader Audit

The final cross-boundary search finds 99 App test/diagnostic files importing
private Board/FDC/HDC/video/family headers. This is not a production reverse
dependency, but it contradicts a complete fixture-boundary claim. Removing two
apparently unused Board-layout includes exposes actual frozen-plan private
access in initialization-atomicity and Model40 BYOB tests; those includes are
restored pending the complete owner receiver, not claimed unused. The
construction-failure fixture likewise uses owner-private construction
operations. All 99 files require explicit owner/public-bus
reconciliation within S93, not new production getters or deletion of hardware
assertions. Existing full-suite/boot passes remain behavior evidence but do
not close this newly measured acceptance gap. No partial P is authorized.

Both default-profile integration remainders now pass: x64 19/19 in 56.72s,
x86 19/19 in 61.69s. Each width's separately passed boot-matrix case completes
the 20-case inventory, without a repeated successful test. All product,
integration and boot jobs have terminal zero exit codes. The final reader
audit, not runtime verification, now prevents S93 acceptance.

The [finite reader intake](t540-s93-final-private-board-readers.md) names all
99 open files: 56 initially Shared candidates, 35 App composition cases and
eight external integration/diagnostic scenarios. Those candidate classes are
not final dispositions. Every row retains its original assertion receiver;
external inputs stay App-owned. This ledger prevents another unnamed
per-field migration or false completion based solely on green tests.
## Final private-Board reader batch: instruction/PIC composition

The frozen 99-file intake now has 15 received rows and 84 unresolved rows.
The received GPR MOV/PUSH/POP, LEA, MOFFS, sign extension, XCHG, immediate
PUSH, PUSHA/POPA, ENTER/LEAVE, legacy segment stack, LES/LDS, LSS/LFS/LGS,
segment MOV and prefix-attribute tests belong to the common Board because
they compose public Core execution/debug operations with Board-owned PIC
endpoints. They contain no App, Profile, external firmware or media inputs.

All 15 source bodies compare exactly with their pre-move versions after
normalizing only the two same-owner fixture include paths. Programs, case
loops, register/memory rollback checks, IRQ/IRR/ISR assertions and markers
are unchanged. The old App source and executable definitions are removed;
the Shared test project alone defines and links each receiver to the sole
Core and Board implementations. Independent builds and executions pass
15/15 on x86 and x64.

The first full-unit replay exposes the CPU boundary negative test's copied
old App paths, not a hardware failure. Its original 96 rejection assertions
are preserved, with the same Shared/App source resolution as the production
boundary gate. Fresh serial full units pass 485/485 on x64 (62.29s) and
x86 (63.89s). The first x64 specialized replay detects T344 scanning only
the old directory, omitting the 15 moved constructors. Explicitly adding
their actual receiver paths restores 125 constructors/101 historical
identities; the denominator and predicates are not relaxed. Both complete
81-command specialized targets then pass, with 44 T332 owners and all 403
strict-matrix rows (382 strict, 21 deferred). The original failed x64 gate
log is superseded by the complete target replay after the T344 correction;
do not treat that failed log as proof. The owned serial verification command
ends with exit zero after completing the x64 corrected target and x86 full
unit/target run. All six manifests, documentation and diff checks pass.

The remaining 84 intake rows remain unresolved. No production input,
product binary, MyNES input, owner INI, external asset or public API changes
in this test batch. S93 remains active; no partial P is delivered.

## Final private-Board reader batches: descriptor/FLAGS/string and port delivery

Seventeen tests and five unique fixtures now live at test/x86/ibmpc-common:
bit scan/test, double shift, IMUL, rotate, SETcc, direct FLAGS, LAHF/SAHF,
PUSHF/POPF, three INC/DEC groups and LODS/MOVS/SCAS/STOS. Each fixture's
actual users move together. Descriptor preparation executes guest LGDT/LMSW,
segment loads and far transfer; fault, vector and IRQ assertions retain their
original public Core observations and owner-local PIC endpoints. No App,
external ROM or media is required. Removing unnecessary CPU/Core private
includes and substituting the original IOPL 00003000h and reserved-flag
FFFC802Ah masks does not change their values. Exact normalized-body comparison
passes for all 22 files; no assertion, program byte, case loop or return
condition is removed. Independent 17/17 tests pass per width. The fresh serial
full replay passes x64 485/485 in 75.39s and x86 485/485 in 69.37s, with both
81-command targets, 44 T332 owners, 125 T344 constructors/101 historical
identities and 403 strict-matrix rows (382 strict, 21 deferred). Its process
has terminal exit zero. These changes receive six frozen-intake rows.

The next six tests, CMPS, ARPL, BOUND, scalar/string port I/O and table-register
delivery, also belong to the common Board owner. All six bodies compare exactly
after the two fixture path substitutions and removal of the table test's
private CPU include; its sole private descriptor constant remains 0Bh (busy
32-bit TSS). IRQ/IRR/ISR, real GDT/IDT/CPL bootstrap, fault frame, provider-error,
guest timestamp, SGDT DOS discriminator and register/memory rollback checks
are preserved. Original App sources and executable definitions are removed;
the existing Shared registration loop owns each target with strict warnings.
Independent builds and six executions pass per width. The first x86 build
command used a nonexistent directory, before any x86 build started; the
correct existing shared-x86 tree then builds and passes. The serial complete
replay passes x64 485/485 in 60.48s and x86 485/485 in 74.15s; both complete
81-command targets pass and the process has terminal exit zero.

The frozen 99-file intake has 27 received and 72 unresolved rows. All six
manifests, documentation and diff checks pass. Production/API/hardware
algorithms, MyNES, INIs, assets and product inputs are unchanged. No partial P
or S93 acceptance is delivered.

## Final video-behavior receiver batch

Ten CGA, EGA, VGA, generic/Compaq CECG and display-authority tests now live
at test/x86/ibmpc-common with one video_fixture.h. The actual source review
separates them from the still-open mixed registration/fault-allocation test.
The ten receivers no longer read Core machine, RAM or port layouts. Their
adapter/chip access is local to the Board owner. The fixture constructs real
opaque neutral Core, configures the real adapter, then freezes/resets before
the first operational bus access. Directional route assertions now exercise
exclusive public registration before freeze and remove only a successful
probe's own route; they neither write a device register nor publish a getter.
The S11 route checks move before its first guest bus writes for that reason.

All original port, aperture, pixel/palette, generation, planar-mode, DAC,
chain-4, generic/Compaq decode, reset and failed-config retry assertions stay.
The display-authority body is unchanged. Low-level one-MiB scratch allocations
in the Compaq fixtures become the neutral executor's valid default RAM extent;
these tests exercise the same video addresses, not RAM-capacity policy.
The shared fixture owns real lifecycle preparation, not another state flag.
No production source, algorithm, ABI, profile configuration or media changes.
Counting physical lines in the ten before/after C sources plus the new fixture:
1262 removed, 1249 added, net -13 (excluding CMake/docs/manifests/artifacts).

Independent strict builds and 10/10 executions pass per width: x64 6.75s,
x86 4.54s. The first sandboxed Ninja had no compilation children or output;
its exact CMake/Ninja processes were inspected and terminated. The same build
then succeeds outside the sandbox; the terminated run is not passing proof.
T344 now classifies 126 real direct constructor sources, adding the single
neutral video fixture, while retaining all 101 historical identities. It
does not relax the count to hide an unclassified constructor.

The complete root replay passes 485/485 per width: x64 268.68s, x86 70.87s.
Both complete 81-command gate targets pass and the process exits zero.
The frozen intake at this batch has 37 received and 62 open rows.
The video registration/fault matrix remains explicitly open; private Core
allocation and provider counters cannot simply move into a peer Board test.
S93 remains active without a partial P; MyNES and owner INIs are unchanged.

## KBC protocol receiver batch

The AUX packet/command/IRQ test and serial-cadence test move to
test/x86/ibmpc-at. Their same-owner KBC layout access stays local; both stop
importing neutral Core machine/port layouts. A single kbc_fixture.h constructs
real opaque neutral Core, installs the actual KBC route batch and freezes/resets
before the first guest bus cycle. Unlike the retired port-only owner shim,
this does not copy a private port table into a fabricated Core allocation.

One kbc_irq_fixture.h moves to the common composition test owner. It uses
public PIC source leases only; the still-open mixed KBC test consumes that
same fixture rather than keeping a second IRQ callback/binder implementation.
The unused AT header is removed from this common fixture. The mixed
construction/allocation rollback matrix is not counted as received.

The counted C/header before/after surface is 341 removed, 379 added, net +38,
excluding CMake/docs/manifests/artifacts. The increase is the bounded real
executor/bus fixture replacing private fabricated-port ownership; it adds no
production layer or public API. IRQ wiring has one receiver, not two copies.

All 60 original AUX failed-predicate groups and all 12 serial groups match
after bus-call substitution. AUX adds one real PIC construction status check.
Command/reply byte order, IRQ1/IRQ12 vectors, EOI, response timing, packet
saturation, keyboard admission and cleanup-deassert assertions remain.
Independent strict builds and 2/2 tests pass per width: x64 0.83s, x86 0.39s.
T344 names the new real fixture, classifies 127 constructor sources and keeps
all 101 historical identities. The frozen intake has 39 received and 60 open
rows. Complete root units pass 485/485 per width (x64 60.00s, x86 58.94s);
both complete 81-command gate targets pass and the process exits zero.
No production/API/profile/input,
artifact, MyNES or owner INI change, and no partial P or S93 acceptance.

## RTC IRQ, CMOS and time-axis receiver batch

Three tests move to test/x86/ibmpc-common. IRQ8 wiring and CMOS event delivery
now construct a real opaque neutral executor and PIC, freeze/reset it, then
program the same guest ports. No private Core machine/port object remains.
The CMOS fixture uses its one finalization routine for failed construction and
normal completion. Register alarm/event masking, pending IRQ enabling, read-C
deassertion, destruction deassertion and seed/NMI behavior remain unchanged.
The time-axis test body changes only its local public fixture path and removal
of the unneeded Core-private include: same-owner Board RTC inspection and the
trace-enabled Core/Board variant retain every timestamp/order/reset assertion.

Actual review preserves the original hardware checks, including all eight
CMOS and 17 time-axis failed-predicate groups. Counted physical C source:
364 removed, 377 added, net +13, excluding CMake/docs/manifests/artifacts.
The added preparation is real executor ownership and checked construction,
not a public test API or new production layer. Independent strict builds and
3/3 tests pass per width: x64 2.63s, x86 1.53s.
T344 names the additional neutral constructor and the two historical receivers:
128 real constructor sources, all 101 historical identities retained.
The frozen Board intake has 40 received and 59 open rows. This count is not
the complete Core-private test universe; its other private-header readers
also need the admitted ownership reconciliation. No production/API/profile,
artifact, MyNES or owner INI change. Latest root x64 units pass 485/485 in
59.36s; x86 passes 485/485 in 58.79s. Both complete 81-command targets
pass and the serial verification process exits zero.

## DMA/RTC authority receiver

The original test moves to test/x86/ibmpc-common. Eight private port writes
become public bus writes; two DMA-status reads use the existing public bus
fixture. Same-owner Board inspection stays at Board, and the Core-private
header is removed. Independent strict compilation exposed a further private
time-injection call. Its refresh case now executes four real one-tick FNINIT
retirements through the existing debug/run interfaces, checking executed
instruction and tick counts at the unchanged 3+1 tick boundaries. The neutral
default CPU/FPU configuration remains unchanged; only the test program entry
is prepared through the public paused-debug register patch.

All original PIT1/DMA0 TC, binding-conflict, CMOS/NMI, IRQ8, five-second RTC
and reset-seed assertions remain. No production API, algorithm, product input,
MyNES or artifact change is made. Dual-width independent tests pass: x64 case
1.13s, x86 case 0.89s. T344 retains 128 constructor sources and all 101
historical identities. The frozen Board intake is 41 received/58 open.
Full root validation after this move passes 485/485 per width (x64 61.30s,
x86 60.19s). Both complete 81-command targets pass; serial process exits zero.

## Core mechanism test receivers

Five tests now live at test/x86/core: memory inspection, immutable ROM-route
transaction rollback, timeline, explicit time and transaction lifecycle.
ROM rollback's body is byte-for-byte unchanged. Timeline and transaction
lifecycle differ only in the relative public fixture include. Memory
inspection replaces full-PC construction with the existing neutral Core
configuration/create operations; all parity, observation side-effect, paging
and lexeme assertions remain. The initial neutral ROM experiment was rejected:
the old test also covers the real Board reset-alias callback. Its public Board
construction is retained, rather than dropping capacity/alias checks.

Explicit-time private lifecycle, arbitrary-time and overflow probes belong
to Core. RTC seconds are observed through its actual public 70h/71h route;
the old private Board and direct RTC-chip includes are removed. HLT, zero
advance rejection, five-tick progression, reset and overflow assertions remain.
No test getter, production API, forwarding layer or implementation is added.
The private Core access is local to the Core test owner; its Board interactions
use only public headers. The trace-dependent transaction test retains the
observable Core/Board variant, not a second production executor.

Exact prepared-source comparison passes for all five receivers. All 98
failed-predicate groups remain: 22 inspection, 7 ROM, 28 timeline, 37 lifecycle
and 4 explicit-time. Counted physical C source is 907 removed, 911 added,
net +4. Independent dual-width runs pass: three-test inspection/ROM/timeline
batch 3/3 (x64 1.26s, x86 1.18s); explicit-time case 1.05s/0.69s;
transaction-lifecycle case 1.31s/1.42s. T344 keeps 128 constructors and all
101 historical identities at their actual receivers. The frozen Board intake
is 42 received/57 open; the other four Core cases were outside that inventory.
Full root verification after this batch passes 485/485 per width (x64 60.91s,
x86 59.66s), with both complete 81-command targets passing and serial process
exit zero. No MyNES, owner INI, production, API or artifact change is part of
this batch.

## Scheduler owner reconciliation

The old mixed App scheduler test is removed. Its Core receiver retains real
Board composition through public interfaces, but forwards each instrumented
callback through the original copied attachment instead of calling private
Board implementation functions. Core budget, retirement/provider advancement,
media/device dispatch, deadline copy and progress-disposition assertions remain.
The original Board timing qualification procedure has a separate Board-owned
receiver. Its private PIT clock calculations and false/true qualification
observations are unchanged; the opaque Board construction result replaces
reading Core's private attachment context. Neither receiver imports its peer's
private layout. No production API, callback or implementation is added.

The original 340 physical C lines become 303 Core and 43 Board lines: net +6,
including separate test entry/includes. Independent strict builds and tests
pass 2/2 per width (x64 1.71s, x86 0.73s). The original historical identity
remains counted; the genuine separate Board constructor makes the measured
T344 inventory 129, not a waived missing constructor. Frozen Board intake is
43 received/56 open. Full root units after this split pass 486/486 per width
(x64 58.59s, x86 58.05s). Initial x64 verification correctly rejected the new
Board case missing from the root canonical unit target list. The real target
was added; no verifier or expected count was relaxed. Both complete specialized
targets then pass, including registration of 327 routes and strict compilation
of 404 rows (383 retained strict, 21 deferred). Serial verification exits zero.
Six manifests, documentation and diff whitespace checks pass. Prepared-source
comparison matches both receivers exactly; no original assertions are removed.

## Core construction and CPU/PIC lifecycle owners

Memory-device registration, immutable ROM mapping, rational provider-clock and
bounded-executor tests move from App to test/x86/core. Their existing public
neutral constructors replace whole-board construction; CPU/FPU, memory and
provider-clock values and all original assertions remain. They require no
Board implementation. The existing reset-alias helper moves unchanged to one
Core test fixture, which the Board fixture also includes; there is no copied
preparation path or production API addition.

The former mixed CPU/PIC lifecycle test splits at actual ownership. Core tests
retain the same opaque CPU execution instance across both resets. The Board
test retains the complete IRQ0-15 exercise (excluding cascade IRQ2), both
reset/execution passes and zero master/slave IRR observations. Both consume one
public guest-program fixture with the original CPU-state, port and HLT checks.
Neither imports its peer's private layout. The Core identity case deliberately
uses real public Board composition because it proves that integration, rather
than fabricating another board.

Prepared-source comparisons match all six receivers and both new fixtures.
Counted physical C/header lines for the five old tests plus the old Board
fixture are 586 removed and 625 added, net +39. The increase is separate owner
entry points/guards/includes, not duplicated guest code or new runtime logic.
Independent strict builds and tests pass 6/6 per width (x64 5.47s, x86 3.41s).
T344 measures 130 constructors/101 historical identities; its extra constructor
is the genuine separate Core identity case, not a relaxed expectation. The
frozen Board intake is 44 received/55 open; the four pure Core cases are outside
that inventory. The test/x86 manifest is regenerated. Fresh full root units
pass 487/487 per width (x64 132.82s, x86 62.01s), and both complete 81-command
specialized targets pass. Registration has 328 routes; strict compilation has
405 rows (384 retained strict, 21 deferred). The serial process exits zero;
all six manifests, documentation and whitespace checks pass. No
production/API/profile/INI/artifact or MyNES change belongs to this batch;
S93 remains active without a partial P.

## Pure real-mode execution and provider configuration

Six additional App tests move to test/x86/core: 386 address-size execution,
REP CMPS/SCAS, the real-mode corpus, exact real-mode ticks, INT/IVT delivery
and configuration/freeze lifecycle. They now link only x86-core and use the
existing neutral constructor. Guest bytes, CPU/FPU and timing values, failure
predicates and historical markers are retained. The configuration case makes
the formerly implicit reset alias explicit through the existing Core fixture;
no production API, alternate execution path or board substitute is added.
Unneeded private Core includes are removed. Prepared-source comparison checks
the complete six files, not only passing outputs.

Independent strict builds and tests pass 6/6 per width (x64 5.21s, x86 4.51s).
T344 retains 130 constructors and 101 historical identities. These six are
outside the frozen Board intake, which remains 44 received/55 open. The
test/x86 manifest is regenerated. Fresh full-root units pass 487/487 per width
(x64 64.21s, x86 62.22s); both complete 81-command specialized targets pass.
The serial verification process exits zero. Registration remains 328 routes;
the strict matrix retains 405 rows (384 strict, 21 deferred). All six manifests,
documentation and whitespace checks pass. The six source files change from
758 to 760 physical lines, net +2. No product artifact, profile, owner INI or
MyNES change belongs to this batch. S93 is still active without a partial P.

## Core INTA transaction and real opaque PIC

The PIC phase test now belongs to test/x86/core. Core-private transaction and
CPU-bus assertions remain at that owner; public PIC construction, source leases
and register observations replace Board-private endpoint imports. One local
attachment connects the actual PIC pair to Core, resetting and finalizing it
through the existing contract. The pending/acknowledge callbacks retain the
production adapter's scan/get semantics, without extra refresh behavior.
Unrelated whole-PC controllers are not constructed. This adds no production
API or controller substitute.

All eight original failure expressions compare exactly after substituting
the public PIC handles. The original IRQ0 and IRQ14, vectors, rejected DMA-
occupied INTA, unchanged ISR on rejection, two acknowledgement trace events,
stack-write ordering, CPU handler and reset counter assertions remain.
The full prepared-source comparison matches. Physical C lines are 212 before
and 265 after, net +53 for independent attachment composition/teardown.
Independent dual-width strict builds and tests pass. Fresh full-root units
pass 487/487 per width (x64 62.87s, x86 61.23s), and both complete 81-command
specialized targets pass. Serial verification exits zero, retaining 328
registered routes and 405 strict-matrix rows (384 strict, 21 deferred).
Six manifests and documentation/whitespace checks pass. This batch changes no
production, API, product artifact, owner INI or MyNES input. S93 remains open
without a partial P; frozen Board intake is 45 received/54 open. The KBC
controller test remains open: its port-allocation injection, AT protocol and
whole-Board CPU/IRQ1 checks still require separate actual owners.

## Attachment phases and controller authority ownership split

The mixed controller-authority test now has separate Core attachment/reset/NMI/
finalization and Board FDC/HDC/IRQ/DMA/media receivers. Core's private lifecycle
predicates stay at test/x86/core; Board uses public Core bus operations rather
than importing port/Core layouts. Invalid PIT-ratio construction still checks
rollback of the unpublished Core. Original device failure predicates remain.
The App source retains the exact original joint context/back-pointer/callback
identity predicate, explicitly unresolved; neither a getter nor a weaker
behavior assertion substitutes for it. This partial split does not close the
frozen intake row: counts remain 45 received/54 open.

Independent x64 and x86 strict builds/tests pass 2/2. The retained App binding-
identity test passes on both widths. T344 classifies 132 actual constructors,
retaining all 101 historical identities. The test/x86 manifest is regenerated.
No production API, implementation, product artifact, owner INI or MyNES change
belongs to this split. Full-root units pass 489/489 per width (x64 61.85s,
x86 61.95s); both complete 81-command specialized gate targets pass and the
serial verification process exits zero. Registration has 330 routes; strict
compilation has 407 rows (386 strict, 21 deferred). All six manifests and
documentation/whitespace checks pass. Prepared-source comparisons match all
three complete receiver/residual files. S93 remains active without a partial P.

## Core DMA competition with Board-owned construction

The 80386 DMA competition case moves to test/x86/core, retaining the original
Core HOLD/transaction checks, CPU retirement, exact deterministic eight-tick
advance, DMA byte at 11234h, CPU/DMA/PIT/PIC ordering, absent FDC/HDC events and
reset HOLD-release predicates. Its separately compiled ibmpc-common fixture
constructs the actual Board and binds the original channel-2 provider. That
source owns the private Board import; it publishes only existing opaque Core/
DMA handles and the copied nonce. Core never imports the Board layout, and
the fixture never imports the Core layout. No production API or getter is added.

An attempted public-deadline adaptation failed because the original default
clock is deliberately unqualified and has compatibility progress disabled.
That adaptation is removed, not repaired by inventing clock qualification or
changing the eight-tick expectation. The final test uses the original private
deterministic advance at its legitimate Core owner. All other original failure
predicates remain, with create/bind status delivered by the construction fixture.
Independent strict builds/tests pass per width (x64 1.11s, x86 0.63s).
T344 retains 132 constructors/101 historical identities; the test/x86 manifest
is regenerated. Frozen intake is now 46 received/53 open. Full-root replay of
this latest move is pending; S93 remains open without a partial P.
Root target builds and the original focused case also pass per width (x64
1.05s, x86 0.09s). Complete prepared-source comparison matches the 227-line
Core receiver; the old source has 229 lines. Its 35 failure sites become 34
plus the same bind failure returned from the Board fixture. Six manifests,
documentation and whitespace checks pass. MyNES and owner INI diffs remain empty.

## Exact Core/Board binding identity without peer layouts

The remaining controller-authority identity source now resides in test/x86/core.
Its separately compiled Board fixture checks the original Board-to-Core back-
pointer and null NMI callback. It returns expected construction values for the
existing public copied attachment type, independently of Core's saved binding.
Core retains the exact context inequality/identity and reset-devices, reset-
clocks, refresh-NMI and finalization callback comparisons. No opaque context is
dereferenced by Core's test. The fixture never imports Core's private layout.
No production getter/API or weaker behavior surrogate is introduced.

Independent strict builds/tests pass per width (x64 0.31s, x86 0.16s). The
original App residual source is deleted, closing the controller-authority row.
Frozen intake is now 47 received/52 open; T344 retains 132 constructors and all
101 historical identities at their actual constructor source owners. The
test/x86 manifest is regenerated. Fresh full-root units pass 489/489 per width
(x64 63.11s, x86 60.80s), covering both this identity receiver and the preceding
DMA competition move. Both complete 81-command specialized targets pass; the
serial verification process exits zero. Registration retains 330 routes; the
strict matrix has 409 rows (388 strict, 21 deferred). All six manifests and
documentation/whitespace checks pass. Complete prepared-source comparison
matches the Core identity receiver. MyNES and owner INI diffs remain empty.
This is not S93 acceptance or a partial P.

## RAM allocation and construction fixture source ownership

RAM creation now belongs to test/x86/core. Its allocation callback owns Core
memory/port fault injection. The separately compiled Board construction fixture
owns the original production projection, attachment and back-pointer predicate;
only copied executor configuration and opaque handles cross this test boundary.
The constructor callback is used synchronously and is not retained. The former
App fixture importing both private layouts is deleted. Plan and port-assembly
tests reuse the one new fixture path; their bodies otherwise remain unchanged.

Default/minimum RAM allocation success and failure still assert one allocation,
null failed outputs and the original installed sizes. All seven invalid clock
ratios still fail before any allocation. Original standalone RAM read/write,
null arguments, freeze/reset and default/auxiliary-PIT/XT publication checks
remain; Board back-pointer checks run in the actual Board source, and Core's
attachment context comparison stays Core-owned. No production API is added.
Independent strict RAM builds/tests pass on x64 and x86 (1.17s and 1.15s).
The frozen intake now has 49 received/50 open rows. T344 retains 132
constructors and all 101 historical identities. The test/x86 manifest is
regenerated. Fresh complete units pass 489/489 per width (x64 64.45s,
x86 59.03s), both complete specialized targets pass and the serial process
exits zero. The strict matrix has 413 rows (392 strict, 21 deferred).
S93 stays active.

## Core port construction and rollback source ownership

Nine original functions now live once in test/x86/core/port_assembly_fixture.c:
fresh construction, range/batch atomicity, supplied read time, AT Port B time,
DMA byte lanes, complete constructor allocation failure, PIT route rollback and
PIC pair rollback. The original App driver calls those same functions in its
original order; an independent strict Shared driver replays the Core-owned
subset. The Core source imports only public Board/PIC/PIT contracts, never
private Board layouts. Board construction retains the actual back-pointer
predicate; Core checks its own attachment context. Fault allocation objects
still have their original lifetime and indices. No production API changes.

Prepared old/new function comparison matches all nine bodies after removing
static linkage and relocating the already owner-checked Board back-pointer
predicate. The probe callbacks have one implementation, not copied bodies.
Original PIT failures 1--7, PIC failures 1--8, default/auxiliary-PIT/XT constructor
failure-through-success loops, 64-bit supplied timestamps, byte lanes and
reset/CPU timestamp observations remain. App RTC/FDC/HDC/D4 assertions are not
removed; that mixed part remains open and the intake stays 49 received/50 open.

Independent strict tests pass on x64/x86 (0.86s/1.77s). The original App test
and independent receiver pass on both root widths. Complete units pass 490/490
(x64 61.29s, x86 62.40s). An omitted canonical target-list entry is fixed without
weakening the registration gate; it now proves 331 unique routes. Both full
specialized targets pass; the guarded serial verification process exits zero.
The strict matrix has 417 rows (396 strict, 21 deferred). T344 classifies 133
constructor sources while preserving its 101 historical identities. The prior
x64 gate log records the superseded missing-registration failure; the fresh
guarded x64 gate command and subsequent x86 replay are the current proof.

Physical line accounting against this turn's saved input: old App source 745;
new App 411, Core receiver 331, declarations 25, independent driver 13: 780
total, net +35. Added lines are the separate declaration/independent-build seam,
not a second production path or new hardware behavior. This is an internal
working batch, not S93 acceptance or a partial P; no product artifacts are
rebuilt and MyNES/owner INI changes remain excluded.

## Complete port-assembly private ownership

The same Core receiver now owns the remaining FDC/RTC/Port-B/HDC transaction
assertions. A separately compiled Board fixture constructs original FDC/HDC
topologies, injects chip creation failure and checks unpublished controller,
topology, DMA and parity/profile binding state. It returns transient copied
test predicates and borrowed public DMA handles, never private layout. Core
retains its own route/allocation/RAM predicates. App contains only the original
ordered driver and genuine D4 attachment. No production interface changed.

Original factory failure, each route allocation failure, rollback, retry,
duplicate provider, busy DMA, ATA port collision and Compaq shared 3F7 read
checks remain. The independent driver runs all common cases; the App driver
also runs D4. Neither source imports the peer's private layout. Physical test
lines are App 51, Core receiver 598, declarations 35, independent driver 33,
Board source 174 and Board declarations 31: 922, +142 from the previous batch
and +177 from the original mixed source. The increase buys separately compiled
owner-local assertions and an independent driver, not duplicated case bodies.

Independent focused tests pass x64/x86 (0.86s/2.01s); both root original and
independent tests pass. Fresh complete units pass 490/490 per width (x64 61.95s,
x86 61.33s), and both complete 81-command specialized targets pass. Strict
compilation covers 419 rows (398 strict, 21 deferred); T344 retains 132 actual
constructor sources and all 101 historical identities. The guarded verification
process exits zero. Six manifests and whitespace checks pass. Frozen intake
advances to 50 received/49 open. No production input, EXE, MyNES or owner INI
changes in this batch; whole S93 acceptance and its P remain outstanding.

## Frozen-plan regression at its actual owners

The original 492-line App plan test is removed. Its Board receiver retains
plan topology, declarations and timing-rule validation; a separately compiled
Core test source owns attachment-context identity, allocation failures 1--3,
the thirteen pre-reset/three post-reset XT route predicates, raw port setup
and exact PIT/DMA deadline progress. Board still checks its own back-pointer
and frozen controller rule. DMA is borrowed through its existing opaque bus
contract; no peer's private layout is imported and no production API is added.

All ten original case groups, their order and fourteen historical markers are
retained. The original PIT 1-tick and source-DMA 3-tick tests remain, with no
timing classification change or substituted physical qualification. The
original allocation loop and route/deadline predicates move unchanged apart
from fixture parameters; invalid-plan null-output, topology preflight, copy,
every declaration rejection and controller-rule matrices remain Board-owned.
The prepared Board source matches the actual file. Core contains no Board
layout access and Board contains no Core layout access. The original root
target is replaced by the same independently registered Shared target, not a
second execution of a copied test.

Physical lines: original 492; Board 436, Core 99, declarations 14 = 549,
net +57 test lines. Source-list changes remove six/add five lines, for +56
across the six changed code/build paths, excluding manifests/documentation.
Added lines express the separately compiled assertion ownership rather than
a production facade or duplicated case bodies.

Independent tests pass x64/x86 (0.86s/0.50s). Fresh complete units pass 490/490
per width (x64 62.72s, x86 61.70s), both 81-command specialized targets pass
and guarded serial verification exits zero. Strict compilation covers 420 rows
(399 strict, 21 deferred); T344 retains 132 constructor sources and all 101
historical identities. Intake advances to 51 received/48 open. No production,
artifact, MyNES or owner INI change; whole S93 proof and acceptance remain open.

Post-run six-corpus verification caught a regenerated manifest header using
`revision` instead of the required `corpus-revision`; only that header is
corrected. All six manifests then pass, as do documentation governance and
whitespace checks. The excluded MyNES and owner-INI diff remains empty.

## Video registration and planar parity owner reconciliation

The original 365-line mixed video transaction source is replaced by a 360-line
Core receiver and a separately compiled 38-line Board fixture with 13 lines of
declarations (+46 test lines). Core retains its actual stack executor and owns
raw route capacity, failure injection, priority and rollback assertions. Board
retains the actual stack adapter and owns chip publication, configuration,
aperture and unchanged chip identity after rejected configuration. Neither
source imports the other owner's private layout. No production API, allocation
or persistent state is added.

All CGA allocation failures 1--8, EGA/VGA/Compaq route failures, sentinel
preservation, route counts, duplicate/removal/retry, overlay priority, freeze
rejection and the final Compaq FC6 collision remain. The historical
ROUTE-REGISTRY-SCALABILITY marker is retained. The chip-identity capture moves
immediately before configure; intervening Core route registrations never change
the adapter, so the predicate remains the same.

The original 164-line planar parity test becomes a 140-line Board receiver and
44-line Core fixture with 11 declaration lines (+31 test lines). Core owns the
real parity-bit flip at 1234h, original RAM value, memory-parity publication and
unbound reconfiguration case. Board retains Port-B masks, PIT advance, NMI
mask/latch/signal/clear/reset and failed-port publication/retry predicates. The
expected opaque owner identity is borrowed only for equality comparison; it
exposes no layout or new production getter. Test registration replaces the old
targets with their same named Shared receivers rather than adding copies.

Independent video tests pass x64/x86 (1.27s/1.91s); independent parity tests pass
(1.21s/0.64s). Fresh complete root units pass 490/490 on x64 (61.18s) and x86
(60.41s). Both complete 81-command specialized targets pass; serial verification
exits zero. Strict compilation covers 422 rows (401 strict, 21 deferred).
T344 classifies 133 actual constructors and preserves all 101 historical
identities. The frozen intake advances to 53 received/46 open. These are
test/build-only changes; no production, artifact, MyNES or owner INI change.
Whole S93 acceptance and its complete P remain outstanding.

## Instance, checked-memory and timing-checkpoint receivers

Three former App tests now build independently: Core owns instance isolation
and checked-memory access, while Board owns the display timing checkpoint.
Their sole common construction helper moves to the Board test corpus and is
named test_board_create_executor to distinguish it from neutral Core's executor
fixture. It still invokes the exact same real Board constructor and failure
cleanup; no second factory or production wrapper is introduced. Only include
paths and that test-helper name change in the four moved files.

Original instance CPU reset identity, distinct RAM/providers, A20/92h/KBC
interaction during Core RUNNING, rejected direct mutation and reset clearing
remain. Core-private lifecycle and raw-port checks are now compiled at Core,
without any private Board or KBC header. Checked-memory preserves all route,
query/read/write counters, values, eligibility and overflow rejection checks.
Timing checkpoint retains sixteen NOPs, one retirement/three ticks per call,
all cumulative tick assertions and equality of both reset runs' display status.
Board and Core construction remain the original shared implementation.

The sources retain 158, 115 and 72 lines; the moved helper retains 22 lines.
Test code therefore has zero net line growth. Shared source lists add six
lines, old root definitions remove thirteen, and the constructor scanner adds
one receiving path: net -6 code/build lines, excluding manifests/documents.
The registered names are unchanged and each occurs only in Shared CTest,
not also in the root. T344 still has 133 actual constructors/101 historical
identities. These three sources were outside the frozen private-Board intake,
which remains 53 received/46 open rather than falsely crediting extra rows.

Independent strict builds and 3/3 tests pass x64/x86 (2.70s/1.02s). Fresh full
units pass 490/490 per width (x64 60.43s, x86 58.96s); both full 81-command
specialized targets pass and the guarded serial process exits zero. Strict
compilation remains 422 rows (401 strict, 21 deferred). All six manifests and
whitespace checks pass. No production/API, EXE, MyNES or owner INI changes.
Whole S93 acceptance remains open without a partial P.

## KBC protocol, construction and CPU IRQ1 receivers

The original 927-line mixed KBC source and unique 19-line port-copy helper are
removed. The AT protocol receiver has 787 lines, Core construction/reset fixture
67, Board CPU/IRQ1 fixture 88 and test declarations 16. The existing AT setup
fixture grows seven lines to share its original neutral executor construction.
Test code grows 19 lines; target registration replaces five root lines with
eight Shared lines (+22 test/build lines, excluding documents/manifests).

Core retains the real stack executor, four allocation failures, 64h conflict,
sentinel, route publication, retry and original CPU reset-entry operation. AT
retains all keyboard/AUX/FIFO/translation/typematic/BAT/command tests. Board
retains the actual 286 FF/FA/AA sequence, PIC mask and vector, ISR and reset/IRQ1
lease assertions. Private observations compile only at their state owner; no
source imports a peer's private header and no production getter is introduced.
The old helper no longer copies a private route registry into a heap executor.

Actual source comparison maps all 172 original failed-check statements; the
receiver has 173, adding only a status check for the existing PIC-mask bus write.
All ten static byte arrays/scalars retain their original values. All AT port
operations use existing stopped Core bus operations; KBC ignores the copied
bus tick, so this does not change protocol time. The original success marker
and diagnostic case names remain.

Independent strict builds/tests pass x64/x86 (1.44s/1.48s). Complete units pass
490/490 per width (x64 133.20s, x86 60.05s); both full 81-command specialized
targets pass and the serial process exits zero. Strict compilation has 424
rows (403 strict, 21 deferred). T344 retains 133 actual constructors and all
101 historical identities. Frozen intake advances to 54 received/45 open.
No production/API, artifact, MyNES or owner INI changes; S93 remains open.

## XT PPI wiring, private state and Core route rollback

The 390-line App XT regression becomes a 352-line Board test, a 37-line Core
route fixture, a 48-line XT state/construction fixture, 18 declaration lines
and a 14-line Core time fixture (+79 source/test lines). Shared registration
and the auxiliary-PIT link add ten nonblank lines, replacing three root lines;
the constructor scanner adds one receiving path. These eight net nonblank
build lines are counted separately from documents and manifests.

XT owns its original per-iteration zeroed stack PPI and byte-ready, IRQ1,
NMI-signal and unpublished-chip predicates. Core owns the actual stack executor,
failures 1--8, 80h sentinel, all four read/write route absences, retry and double
finalization. Board owns DIP, speaker, parity/NMI, serial/BAT/FIFO wiring and
the six refused-completion variants. Every fixture imports only its own private
layout and public peer contracts. No production allocation, state or API changes.

All 148 original failed-check statements map to the actual receivers; the new
149th statement aggregates the separately compiled construction result. The
16-byte FIFO input and five success markers remain unchanged. Exact 12499/12500,
300000, 300, 60, 260 and 25-tick boundaries, overflow FF and no duplicate delivery
remain. The deadline comparison uses the existing copied elapsed_ticks from the
same observation call; inspection of Core confirms it copies the sole clock
without advancing it.

The original arbitrary-time operation is private Core test setup, not a durable
runtime capability. One separately compiled Core test fixture preserves it for
both XT and auxiliary PIT; neither Board test includes Core's layout. Auxiliary
PIT retains exact 1/3/4-tick ratios, counter reads and reset/deadline assertions.
The T344 scanner now names the relocated XT constructor explicitly, retaining
133 actual constructors and all 101 historical identities rather than reducing
the count when its old App path disappears.

Independent strict builds and 2/2 tests pass x64/x86 (3.07s/2.19s). Complete
units pass 490/490 per width (x64 61.14s, x86 61.64s); both complete 81-command
specialized targets pass and guarded serial verification exits zero. Strict
compilation covers 428 rows (407 strict, 21 deferred). All six manifests pass.
Frozen intake advances to 55 received/44 open. This is not S93 acceptance or
a partial P; artifacts, MyNES and owner INIs remain untouched.

## App video and Profile composition boundary

Six Model40 CECG cases, default display composition/sequencer and the
5170-versus-default topology case retain their actual App fixtures. Their
658 source lines become 618; all 60 original failed-check statements remain.
Eight files compare identically after the explicit public-operation, constant
and owner-fixture substitutions. The topology case retains all CGA failure
bits 1--8 and EGA bits 1--4 in Core's separately compiled route fixture, and
the original aperture bits 9/5 in Board's separately compiled fixture.
Snapshot, pixel, palette, generation, reset and firmware expectations remain.

The Core fixture preserves the exact lightpen write-route and physical-memory
route predicates. Board owns the actual video aperture predicate. Neither
imports a peer's layout. App uses the existing stopped Core bus and memory
operations; its reads retain the original 32-bit result rather than narrowing
it to a byte. The original Board byte-read helper delegates to that same
neutral test helper. Video ignores the bus tick; S28's memory operation reaches
the same physical write through the existing stopped interface, additionally
performing its normal prefetch invalidation and trace bookkeeping. No guest
execution or timeline advance is added by those operations.

All nine focused tests pass per width. The subsequent complete rebuild covers
the final 32-bit bus-read refinement: full units pass 490/490 per width
(x64 123.94s, x86 62.29s), both complete 81-command specialized targets pass,
and guarded serial verification exits zero. Strict compilation has 432 rows
(411 strict, 21 deferred); T344 retains 133 constructors/101 historical
identities. All six manifests and diff checks pass; MyNES, artifacts and owner
INIs are unchanged. Frozen intake is 64 reconciled/35 open. Reconciled App
composition is not misrepresented as a Shared scenario. S93 remains active
without a partial P or final artifact/boot acceptance.

## Reset-ROM alias and App CMOS/RTC composition

The reset-ROM regression moves to Core, retaining its entire 182-line body
apart from include changes and two Board-local configured predicates. It
keeps the 286/386 high reset aliases, exact jump/HLT bytes, four policy modes,
mapping counts 1/2/2/4, 14/15/16-byte boundaries, explicit high-ROM precedence,
overlap coverage and one-attempt allocation failure with unchanged retry and
ROM-before-absent-memory behavior. Board alone reads its absent-memory flag;
Core alone retains allocation injection, provider count and ROM internals.
No test source reads both private layouts.

App CMOS/RTC composition remains App-owned. Its 174 lines become 156, with
all 21 failure-check sites intact after mechanical public-bus and owner-fixture
substitutions. Board's 35-line fixture retains the exact RTC advance then PIC
refresh sequence, scan/get operations, RTC-only reset and failure diagnostic.
The 50000/50/100000-tick cases, IRQ8 vector/EOI, UIE/PIE/AIE/SET behavior and
0Eh--3Fh seed/checksum test remain. RTC and PIC port reads ignore the supplied
bus tick, so stopped public access does not advance their original timeline.
The four new fixture source/header files total 58 lines; net source/test change
is +40 lines, with no new production state or API.

Independent reset-ROM builds/tests pass x64/x86 (1.37s/0.64s); both root
focused tests pass per width (3.40s/0.26s). The initial independent link exposed
the new test's missing explicit x86-core selection; it now selects the same
neutral Core as the other Board consumers, without changing production targets
or borrowing an App aggregate. Complete rebuilds/units pass 490/490 per width
(x64 64.68s, x86 66.07s); both full 81-command specialized targets pass and
guarded serial verification exits zero. Strict compilation has 434 rows
(413 strict, 21 deferred); T344 retains 133 constructors/101 historical
identities. Six manifests, documentation and diff checks pass. Frozen intake
is 66 reconciled/33 open. MyNES, owner INIs, production and artifacts are
unchanged. S93 remains active without a partial P.

## PC/AT and 5170 App composition boundaries

Four App scenarios retain actual default/5170 Profile composition and all
original assertions. Core alone compiles registry-presence, A20 and the seven
transaction-timing predicates. Board alone compiles the original PIC source
exercises, KBC command/reply handling and immutable FDC/HDC/RTC configuration
observations. The KBC protocol fixture moves unchanged from App to AT tests;
the two remaining Model40 consumers change only its include path.

The reset regression's raw port write after a CPU budget stop remains a
Core-local test operation: substituting the stopped public bus would reject
its original running lifecycle. Default synthetic-time advancement reuses
the existing Core fixture. CMOS register observations stay direct at Board,
without changing RTC index/NMI state. The 5170 diagnostic port uses the
public 32-bit Core bus at its original zero guest time. No production API,
state, clock, algorithm or compatibility branch changes.

Mechanical substitution review matches all four complete source bodies;
all static instruction arrays are unchanged. The original 74 failure-check
sites retain their predicates; two PIC checks compile at Board, with one
additional App aggregation. Seven transaction conditions, the 120-tick
unmask delay, every default port leaf and all five IRQ/DMA routes remain.
The four App sources shrink from 835 to 776 lines. New Core/Board fixtures
total 127 lines and CMOS helpers add six: net source/test +74 lines,
excluding CMake/manifest/documentation. This increase separates private
owner checks rather than creating a production facade or mirrored state.

The four focused tests pass per width (x64 4.12s, x86 0.52s). Complete
rebuilds/units pass 490/490 per width (x64 65.08s, x86 64.77s); both full
81-command specialized targets pass and guarded serial verification exits
zero. Strict compilation has 443 rows (422 strict, 21 deferred), retaining
133 constructors/101 historical identities. Six manifests, documentation
and diff checks pass. Frozen intake is 70 reconciled/29 open. Production,
MyNES, owner INIs and artifacts are unchanged by this batch. S93 remains
active without a partial P or final artifact/boot acceptance.

## Model40 FDD and 5170 firmware/FDC composition

Both scenarios retain their App-owned synthetic firmware, media, geometry,
reset and Profile checks. The 5170 case uses the existing separately compiled
Core port-presence fixture and Board copied topology fixture. Model40 uses
a Board-local copied actual FDC connection config. Baseline diff review
found that its intermediate WIP had substituted planned topology for actual
connection; this batch restores the original fact, not merely the same value.
There is no production getter, mutable pointer exposure or new state.

Exact normalized diffs against HEAD match only header and owner-operation
substitutions. All seven failure-check sites and 42 comparisons remain.
The two App sources lose two net lines; eight test-only fixture lines and
five CMake lines are added. No production algorithm, API, MyNES, owner INI,
external asset or artifact changes belong to this batch.

An initial include-depth compile failure was corrected. Full verification
was rerun after restoring the actual connection assertion, rather than
using the earlier planned-topology results. Final complete units pass
490/490 per width (x64 65.39s, x86 63.84s); both full 81-command specialized
targets pass. Guarded serial process 15516 exits zero. Strict compilation
has 450 rows (429 strict, 21 deferred), retaining 133 constructors/101
historical identities. Six manifests and diff checks pass. Frozen intake
is 76 reconciled/23 open; S93 is not accepted and no partial P is delivered.

## App FDC/HDC media composition boundaries

The default FDC binding, FDC port, T242 read-track and ATA port cases remain
App-owned with their original in-memory media and actual Profile assembly.
They now use the existing public Core bus. Board-local test operations retain
the exact synthetic FDC advances, media refresh, chip-plus-PIC interrupt
checks, copied FDC/HDC observations, HDC due-service and IRQ sampling. All
eight FDC binding predicates and three HDC binding predicates stay at Board;
no private pointer or new production getter crosses the boundary.

All 59 original failure-check sites and every static command-byte array
match after the explicit owner substitutions. Coverage includes absent media,
disk change cleared by real STEP, non-DMA format/read/write-protect/rate
handling, 18-sector DMA and untouched RAM on motor failure, non-MFM no-data,
ATA CHS/LBA multi-sector progression, zero-as-256 count, reset, NIEN and
error states. The 8272A wire constants move unchanged into one value-only
test header, consumed by the old Board fixture and App fixture alike.
FDC port reads ignore the tick argument; these App fixtures have no trace
provider, so public bus trace calls do not publish events or advance time.

The four App sources shrink from 682 to 656 lines. The Board fixture adds
85 lines; separating the existing FDC values adds four net lines: net
source/test +63, excluding build/manifest/documentation changes. Production
algorithms, interfaces, media policy and deadlines are unchanged. An initial
compile found three multiline HDC IRQ calls missed by substitution; all now
use the same Board operation, without weakening the assertions.

The four focused tests pass per width (x64 4.96s, x86 0.46s). Complete
rebuilds/units pass 490/490 per width (x64 66.24s, x86 64.40s); both full
81-command specialized targets pass and guarded serial verification exits
zero. Strict compilation has 447 rows (426 strict, 21 deferred), retaining
133 constructors/101 historical identities. Six manifests, documentation
and diff checks pass. Frozen intake is 74 reconciled/25 open. Production,
MyNES, owner INIs and artifacts are unchanged by this batch. S93 remains
active without a partial P or final artifact/boot acceptance.

## Model40 DMA, HDC and D4 composition boundaries

Three original App scenarios keep their Profile assembly, synthetic ROM/media
and product predicates. Core-local checks preserve all three DMA wait/ready
conditions, including the original nonzero truth semantics. Board-local test
operations retain copied DMA wiring/configured state, duplicate channel bind
with the original App provider/request storage and owner identity, pending
request sampling and exact shared-PIT advancement. Duplicate-bind failure
must not shorten provider lifetime; the fixture borrows the original values.

HDC uses public Core bus reads/writes and the existing Board-local exact due
service and IRQ operation. New copied actual connection config and slave
media ID checks restore the accepted baseline; intermediate WIP incorrectly
used planned topology for the service config. The two drive-head selections,
255 remaining word reads, status-read IRQ clearing, unsupported IDENTIFY,
SRST diagnostic and geometry/media predicates remain unchanged. D4 retains
the original 19-tick PIT call and sole immutable ROM/reset/Port B scenario.

All three pre-edit bodies match explicit owner substitutions after the
documented actual-config correction. Ten failure-check sites and all static
byte declarations remain. App test sources shrink from 224 to 214 lines.
Test-only fixtures compile private checks at the real owner; no production
getter, state mirror, algorithm, API, MyNES, owner INI, external asset or
artifact changes belong to this batch.

Focused x64 tests pass 3/3 (3.16s); final full rebuilds/units pass 490/490 per
width (x64 77.56s, x86 63.95s), and both full 81-command specialized targets
pass. Guarded serial process 63549 exits zero. Strict compilation has 454
rows (433 strict, 21 deferred), retaining 133 constructors/101 historical
identities. Six manifests and diff checks pass. Frozen intake is 79 reconciled/
20 open; S93 remains active without a partial P or final product acceptance.

## Model40 frozen plan and S7/S8 hardware composition

BYOB keeps its App firmware/assets, immutable owned bytes, memory policy,
pacing qualification, one-instruction retirement and reset scenario. Board
owns its frozen plan; the test copies only the six asserted scalar fields,
not configuration pointer members. Core owns the exact ROM mapping-start
predicate and synthetic one-tick advancement. The alias assertion is not
weakened to an address-inside-range coverage query.

S7 retains every original transaction field, all eight Board clock-ratio
comparisons, CMOS/D4/speaker state and three-tick retirement result. Its
post-budget raw test I/O stays in a separately compiled Core fixture instead
of adding a pause transition or weakening the public debug bus lifecycle.
S8 retains copied drive/topology flags, reset ready-change results, FDC IRQ
source assertion and exact due-service, HDC diagnostic/IRQ/status order,
CMOS values and product input routing. Existing owner fixtures are reused.

Accepted-baseline review found two intermediate semantic substitutions in
both S7 and S8: AUX-enabled was replaced by command-byte-only observation,
and keyboard scanning by a rejected 1Ch native-byte injection. Original
hardware predicates are restored through separate AT/Board test fixtures;
neither owner imports the other's private header. The injection is removed,
so observing scanning no longer modifies the device. An unused new test
submit-byte helper was removed before final verification.

Mapped body audits retain all 42 failure-check sites. Hex constants match
apart from removal of the intermediate 1Ch injection; ROM bytes, geometry,
clocks, timing, ports and command values stay unchanged. App sources shrink
from 445 to 442 lines. All added operations are test-only, with no production
API, persistent mirror, MyNES, owner INI, external asset or artifact change.

Focused x64 cases pass 3/3 (3.00s). After final cleanup, complete rebuilds/
units pass 490/490 per width (x64 73.82s, x86 63.55s); both full 81-command
specialized targets pass. Guarded serial process 19232 exits zero. Strict
compilation has 468 rows (447 strict, 21 deferred), retaining 133 constructors/
101 historical identities. Six manifests and diff checks pass. Frozen intake
is 82 reconciled/17 open; S93 remains active without a partial P or acceptance.

## Session identity and four-Profile DMA deadline qualification

The App two-session case retains its two product constructors, distinct DMA
tokens, RAM bytes 11h/22h, EAX values 11111111h/22222222h and read-watchpoint
at 1234h. Core-local code checks distinct CPU, memory and port owners; Board
code checks distinct RTC, FDC and HDC owners. S92 compared addresses of inline
FDC/HDC state. After the opaque allocation migration, comparing addresses of
pointer slots no longer tested controller ownership. The receiver compares
actual opaque controller objects, restoring the original isolation purpose.
No pointers or private layouts are returned to App.

The timing test retains all four App Profile constructors and the original
DMA configuration, provider freeze, reset and deadline-disposition checks.
The 000Ah/02h write uses the existing public Core bus after reset, and a
separately compiled Board fixture asserts the same original request binding.
Neither operation adds production API or new persistent state.

Both complete App bodies match the S92 baseline after only the documented
include/owner substitutions. Every remaining assertion, value and error
message is unchanged. The existing test fixtures and CMake receivers are
extended; no MyNES, production, firmware, external asset, owner INI or product
artifact change belongs to this batch.

Complete rebuilds/units pass 490/490 per width (x64 72.00s, x86 62.67s), and
both full 81-command specialized targets pass. Guarded serial process 7955
exits zero. Strict compilation has 471 rows (450 strict, 21 deferred), retaining
133 constructors/101 historical identities. All six manifests and diff checks
pass. Frozen intake is 84 reconciled/15 open; S93 remains active without a
partial P or final acceptance.

## XT route and initialization plan boundaries

XT's entire original App Profile/topology/BYOB scenario remains App-owned.
Existing Core fixtures test every original read/write route; public Core bus
performs the same post-reset 3D8h/0Dh write before the X/1Fh CGA snapshot.
Board controller fixtures copy actual FDC/HDC connection configuration.
Accepted-baseline review restores these actual-config predicates from the
intermediate WIP's planned topology substitution. Planned Profile constants
remain separate from actual constructed controller assertions.

Initialization retains constructor output, missing assets, option ROM
arguments, recovery, invalid media slot and retained-config checks. Its
default and overridden sessions still compare RAM/CPU/FPU plan scalars and
separately use public Core getters to prove applied values. The nine original
timing predicates and full controller-rule comparison compile with the Board
owner. App copies expected neutral timing values from its own descriptor;
four byte copies preserve the original full-structure comparisons, not a
reduced field subset. Only pure comparison order changes: controller rules
are checked with the other timing predicates instead of immediately before
them. No assertion effect or initialization behavior is changed.

The XT complete body matches explicit accepted-baseline substitutions;
initialization's nine timing predicates match after the neutral expectation
rename, and its remaining complete body matches the documented owner moves.
All original constants, scenarios, errors and markers remain. No production
API, product state mirror, MyNES, owner INI, external asset or artifact change
belongs to this test/CMake batch.

Complete rebuilds/units pass 490/490 per width (x64 71.39s, x86 64.08s), and
both full 81-command specialized targets pass. Guarded serial process 27107
exits zero. Strict compilation has 475 rows (454 strict, 21 deferred), retaining
133 constructors/101 historical identities. Six manifests and diff checks
pass. Frozen intake is 86 reconciled/13 open; S93 remains active without a
partial P or final acceptance.

## 5170 clocks, original keyboard repeat and FDC DOS headers

The App 5170 case retains its selected/generic Profile comparisons and public
timing dispositions. Existing Board fixtures copy actual DMA/PIT/RTC/VADP
ratios, RTC rate/provenance, controller rules and KBC configured timing;
plan observation adds time-axis kind and controller rules. No planned clock
is substituted for an actual Board domain. The pacing/physical availability
checks retain their original values and do not assert new qualification.

Accepted-baseline review found that intermediate WIP had replaced the original
keyboard-chip repeat callback test with KBC serial output. Those are different
observations. The seven original admit/deadline/advance/repeat-byte/break checks
and original callback now compile in the existing AT owner fixture, reached
through a Board-local binding. The exact 4000000/3999999/1/800000 tick sequence
and native 1Ch/F0h bytes remain. All seven statements match the baseline after
only parameterizing the two expected intervals. The serial KBC complement is
retained through owner-local operations and public Core port reads; no raw
keyboard or KBC pointer escapes to App.

The remaining complete 5170 body matches the explicit baseline field/owner
substitutions. The FDC Read Track DOS complete body matches the baseline after
only header changes: unused private FDC/Core headers are removed in favor of
the public Core contract. Its guest program, IRQ/results, FAT overlay transform,
one/128-instruction runs, DMA bytes and all failure reports stay unchanged.
No production API/state, MyNES, external master, owner INI or artifact change
belongs to this test/CMake batch.

Focused 5170 x64 passes. The existing external INI/overlay FDC integration
passes once per width (x64 15.39s, x86 14.29s). Complete rebuilds/units pass
490/490 per width (x64 75.56s, x86 61.21s), with both full 81-command specialized
targets passing. Guarded serial process 59591 exits zero. Strict compilation
has 478 rows (457 strict, 21 deferred), retaining 133 constructors/101 historical
identities. Six manifests and diff checks pass. Frozen intake is 88 reconciled/
11 open; S93 remains active without a partial P or final acceptance.

## HDD Integration Diagnostic Boundaries

Five App diagnostics no longer import private Core/Board/HDC layouts. Four
compile the existing Board controller fixture separately and consume its copied
HDC observation. ATA PIO DOS removes the unused private fixture and uses the
public chip constants. No production API, fixture API, state or library source
is added. Duplicate public includes are removed; the setup diagnostic locally
undefines Win32's exception_code macro instead of renaming the Core field.

Actual baseline review compares every complete executable body against S92
d76d2b15e. Before the directory-budget correction, all five match after only
the original embedded-HDC observation becomes the existing opaque Board
fixture operation. All guest bytes, CHS/MBR/VBR comparisons, overlay writes,
IRQ/NIEN/DRQ polling, fault output and success markers remain. The six changed
source/build paths add 35/remove 40 lines, net -5, measured against pre-batch
snapshots with whole-file line LCS plus the literal CMake hunks. Documentation
and ignored diagnostics are excluded.

Explicit builds pass for all five executables per width. HDD firmware handoff
and ATA PIO DOS runtime cases pass once per width: x64 0.25s/10.25s and x86
0.11s/10.04s. Setup and HDD-admission probes are not registered runtime cases;
this batch claims their build coverage only, not a Windows Setup pass.

The registered directory checkpoint initially fails at c-dir on both widths
(x64 16.33s, x86 15.36s), retaining its original five-second command window.
Failure reporting pauses/shuts down the executor, so its printed running=0
does not prove spontaneous guest stop. One ignored bounded diagnostic copy
changes only the directory window to thirty seconds and observes the same
103-file directory's completion at 6420ms, with C drive/BDA/ATA assertions
passing. This is diagnostic evidence, not a substitute registered pass.
The coordinator therefore corrects only full-directory containment to a
separate sixty-second window; short command waits, original success predicates,
owner INI, media master, production timing and outer 130-second CTest limit
remain unchanged. Required corrected runtime results are recorded below.

Corrected registered directory checkpoints pass once per width: x64 18.65s
and x86 19.70s. Guarded process 11540 exits zero. Frozen intake is now 93
reconciled/six open. The ignored diagnostic source/binary is removed after
retaining these bounded results; it is not another retained test or runtime
path. Final whole-S verification and actual-change review remain required.

Complete unchanged unit suites pass 490/490 per width (x64 64.46s, x86 66.81s),
with both full 81-command specialized targets passing. Strict compilation has
480 rows (459 strict, 21 deferred), retaining 133 constructors/101 historical
identities. Serial process 28110 exits eight solely to propagate its recorded
x86 directory integration failure; the complete unit/gate runs themselves
succeed. Six unchanged manifests and diff checks pass. No production, MyNES,
owner INI, external master or product artifact belongs to this batch. S93
remains active without a partial P.

## D4 Refresh And DMA Competition Test Boundaries

The two App D4 composition cases no longer import private Core or Board
layouts. Actual DMA binding lives in the separately compiled Board fixture;
wait remaining, refresh callback and HOLD exclusion live in the Core fixture.
A Model40-owned fixture reads the real pending/address/pulse fields. It
does not add mirrored state or a production API. Existing Core-owned time
fixtures preserve the original exact synthetic intervals rather than replacing
them with a different public execution workload.

Actual baseline-diff review retains the 19/11-tick refresh boundary, unchanged
output-byte side effects of the refresh callback, 2/1/9-tick DMA-ready checks,
three-tick CPU retirement and eight-tick advance, all CPU/DMA/PIT/PIC ordering
predicates, and reset/HOLD release. HOLD request and acknowledgement still both
execute even if the first operation fails; release still executes after the
CPU-rejection check. The product scenario remains App-owned.

Nine source/test/build paths add 141/remove 78 lines, net +63, measured by
whole-file line LCS against pre-batch snapshots for eight test paths and the
two literal CMake target hunks (+9/-2). Documentation and manifest hashes are
excluded. The positive delta is separately compiled same-owner test inspection,
not runtime forwarding or new guest state. The full CMake snapshot was truncated
by tool output, so it is not used as a whole-file counting baseline.

Both affected tests pass once on each width. Complete unit suites pass 490/490
on x64 (75.14s) and x86 (65.99s); both complete 81-command specialized targets
pass with terminal exit zero (76228 and 7348). T344 has 487 compilation rows,
466 strict and 21 deferred, retaining 133 constructors/101 historical identities.
All six manifests and documentation/diff checks pass. Frozen intake is now
95 reconciled/four open; indirect readers still require final reconciliation.
No production, MyNES, owner INI, external master or executable input changed
in this batch. S93 remains active without a partial P.

## Model40 FDC Composition Test Boundary

The original S24 scenario remains App-owned. It borrows opaque Core and Board
handles; separately compiled Board controller fixtures copy the actual FDC
connection and drive bindings, not the planned topology. They perform the same
one-tick/128-tick/deadline advance, reset, refresh and IRQ lease checks. DMA
transfer uses the existing Shared fixture's real address/count-register and
phase advancement loop, preserving its byte-flip-flop read effects. KBC C0
still advances one tick before the original B4h reply assertion.

Actual diff review preserves every original format/rate/position matrix input,
all command byte arrays, PIO and DMA 512-byte loops, terminal observation,
media-removal READY behavior, four reset notifications, invalid-rate command
matrix, result bytes and five success markers. Public Core bus access replaces
raw private port access. No production getter, fake second drive or new timing
algorithm is introduced. The original case builds/runs once per width and
passes: x64 0.96s, x86 0.23s. Frozen intake is 96 reconciled/three open.

Four source/test/build paths add 138/remove 110 lines, net +28: whole-file
line LCS for three saved test sources, plus the literal CMake hunk (+3/-1).
Documentation and manifest hashes are excluded. The delta is owner-local test
inspection; original source assertions are not replaced by planned values.
Six manifests and documentation/diff checks pass after the fixture hashes are
updated. The preceding complete-unit/gate proof predates this last edit and
does not establish final whole-S verification. Production, MyNES, owner INIs,
external media masters and executable inputs remain unchanged in this batch.

The old App DMA fixture also has a live transaction-test consumer. It is not
deleted until that actual indirect-private dependency is reconciled; the final
sweep must cover it beyond the frozen Board-reader intake. No partial P or S93
acceptance is claimed.

## Model40 Retirement Diagnostic And Mixed FDC Gate

The complete Model40 retirement source was reviewed. Two RTC register 0Fh
reads now use the existing Board-owned CMOS fixture; two A20 observations
use the existing Core predicate. The execution-time physical read uses a
separately compiled Core fixture calling the exact original memory operation,
including route side effects. It is not replaced by paused-debug inspection.
All addresses, casts, diagnostic histories, command-line modes, checkpoint
predicates and original execution containment remain unchanged. No production
API or synthetic smoke invocation is added. Both diagnostic targets build
successfully; this unregistered target has no new runtime-pass claim.

Four source/test/build paths add 21/remove 9 lines, net +12: whole-file line
LCS against saved pre-batch sources for three test files and the literal
CMake target hunk (+3/-1). The extra code establishes actual owner-local test
access rather than a production wrapper. The two Core fixture hashes are
updated to their measured current values.

The mixed FDC negative source and both invoked gates were reviewed in full.
The gates inspect App machine assembly and FDD transfer-cursor ownership in
addition to Shared chip, board, Core and DMA routes. Keeping this cross-boundary
integration test at App avoids an independent Shared test depending on App
source. It imports only isolated source-text copies and injects seven original
violations; it does not compile an App private-layout reader. A concise
ownership comment records that disposition. The original baseline and all
seven negatives pass once per width: x64 2.49s, x86 2.44s. The comment adds two
lines in one additional test path, with no assertion or injection change.

Frozen intake is 98 reconciled/one open, the BYOB DOS boot probe. The indirect
Core/fixture sweep is still required beyond those 99 rows. Earlier full-suite
results predate these fixture edits and are not final verification. No
production input, MyNES, owner INI, asset master or executable changes belong
to this batch; S93 remains active without a partial P.

## Additional CMOS And Fault Reader Cleanup

Full source and actual-diff review of the INI CMOS integration test and
Model40 CMOS repository-only test preserves the exact 70h/71h accesses,
seed/checksum bytes, writable-state/reset checks, and separate-session
assertions. Existing separately compiled Core raw-port fixture operations
replace App access to the executor port layout. These operations call the
same original port implementation without adding paused-debug admission or
timing effects. No new fixture or production API is introduced. The fault
outcome runner needs only the existing public memory header; its whole body,
guest LIDT/UD recipe and fault/reset expectations remain unchanged.

Four source/test/build paths add 14/remove 18 lines, net -4, measured by
whole-file line LCS for three saved pre-batch tests plus the two literal CMake
target hunks (+4/-2). All three targets build on both widths. Model40 CMOS and
fault outcome tests pass once each per width: x64 0.85s/1.20s, x86 0.12s/0.10s.
The INI CMOS target has build proof only in these root test trees; no external
scenario pass is inferred. A missing test header search path was corrected
with the existing relative test include convention before successful rebuild.

The initial sandbox builds stalled after CMake regeneration with only two
cmake/ninja pairs and no compiler descendants. Their exact commands/PIDs were
verified, the four owned processes were stopped, both handles became terminal,
and native out-of-sandbox builds completed. This is not a source-code failure
or a reason to weaken the assertions.

The direct Core include sweep still has 19 C/header paths, listed in the
final-reader record. That is a separate outstanding class from the 98/99 Board
dispositions; relative and indirect consumers still need review. Source App
hits from this query are absent, but the query does not prove all production
dependencies closed. No production, MyNES, INI, asset master or EXE input
changed in this batch. Whole-S verification and delivery remain open.

## Core RAM/Port, Input Guards And App Error Injection

The original RAM/port context test has no App or Board behavior. It now lives
in test/x86/core and links only x86-core. Only four blank lines are removed;
the two allocations, 16MiB/2MiB bounds, A20 alias behavior, rejected 17-byte
mapping's unchanged-count assertion, frozen mapping rejection, independent
port scratch values, reset readback and both success markers remain intact.

The original input/display case belongs to the common Board receiver. Its
complete source and all predicates were reviewed before relocation. Core
fixture operations set the same lifecycle/firmware-active fields at their
owner, without adding production transitions or mirror state. Both XT and AT
topologies still exercise all five original lifecycle values, individual and
batch keyboard input, scan set, mouse, display observation/capture, null
arguments and firmware-operation exclusion. The three-tick trace order and
reset timeline remain exact. The test explicitly links the observable Core
and Board variants; the first link failure exposed that missing test link
dependency and was corrected before the passing runs.

The App runner error test keeps its paused Common exclusion before injecting
the same impossible lifecycle and reset provider. Separately compiled Core
fixture operations replace App layout writes. Original ERROR-versus-STOPPED
and runner_failed checks, waits, cleanup and provider result are unchanged.
The default-profile integration source drops an unused private memory include;
its whole scenario body and original media/reset criteria are unchanged.

Root x64: RAM/port 0.20s, input/display 0.75s, final runner-error 1.30s.
Root x86: RAM/port 0.08s, input/display 0.09s, final runner-error 0.11s.
Independent Shared x64: RAM/port 0.22s, input/display 0.83s; x86: 0.11s/0.72s.
Each case is run once per applicable receiver after successful build. Default
integration has dual-width build proof here, not a new external runtime claim.
No production, MyNES, INI, media master or executable input changed.

Eight logical source/test/build paths add 41/remove 26 lines, net +15.
Whole-file line LCS compares seven saved pre-batch files to their receivers,
counting each move once; the literal root CMake hunks add 2/remove 10.
Documentation and manifests are excluded. Positive delta is owner-local test
fault/guard setup and independent registration, not runtime API or guest state.
The old input/display source deletion was denied by the sandbox after the
receiver was created; its exact old path was verified and deleted natively.
There is no retained duplicate; the saved pre-batch source and Git retain it.

Four of the 19 direct Core-reader paths are reconciled; 15 remain open, plus
the separately tracked indirect fixture review. The Board inventory retains
98 reconciled/one open. This is not fresh complete-unit/gate/artifact acceptance;
S93 stays active without a partial P.

## D4 Core Access And Mixed Time Reconciliation

The D4 platform, parity and SKEY tests retain their original App scenarios.
Core composition fixture operations own actual elapsed/halt/shutdown state,
raw port effects, parity storage and the real bit flip; the existing Core
time fixture advances the original exact intervals. App no longer imports
the Core layout. No new production getter, copied guest state or relaxed
condition is introduced. Original D1 bytes 03h/01h/00h, ROM byte A5h,
parity address 12345h and the original PIT/NMI/speaker checks remain.

Final focused x64 results: parity 0.79s, SKEY 0.70s, platform 0.89s.
x86: parity 0.11s, SKEY 0.09s, platform 0.09s. The platform's initial build
exposed its private arbitrary-time operation; linking the existing Core time
fixture resolves it without making time advancement a new production API.
Six logical source/test/build paths add 59/remove 23 lines, net +36, using
whole-file LCS for the five saved pre-batch sources and literal CMake hunks.

Debug mapping removes one unused private include only: +0/-1, one source.
All original command/lease/watchpoint/plan checks remain; x64 passes in 1.34s,
x86 in 0.10s. The first CTest regex omitted the registered `unit.` prefix and
matched zero cases; only the corrected actual executions count as proof.

The mixed time test preserves Core/Model40 interaction. A Core-owned fixture
schedules the original increment callback at tick 4; Model40's fixture sets
and observes its actual pending latch. The test still proves D4 consumes tick
1 first, the counter stays zero there, and tick 4 increments it exactly once.
All four original invalid-axis clauses, 3-tick instruction budgets and reset
observations remain. The non-dereferenced failed-construction output sentinel
uses an existing local object address instead of allocating a private Core
layout; only clearing that opaque output is tested.

Self-review caught a CRLF-sensitive pre-edit block extraction that had omitted
the invalid-axis block. It was restored before final proof, and a literal
comparison confirms all four original failure clauses are preserved. Earlier
time-test passes do not qualify this corrected source. Fresh final x64/x86
builds and executions pass in 1.17s/0.08s. Six source/test/build paths add
46/remove 21 lines, net +25, using whole-file LCS for five saved files plus
the four-line target-source hunk. The added code is owner-local test setup,
not runtime state or public API.

The direct sweep has ten open paths after these five reconciliations; the
99-row Board intake still has one open BYOB probe. All six manifests,
documentation governance and diff checks pass after the fixture hash updates;
final full verification remains separate. No production, MyNES,
owner INI, external asset or executable input changed in these batches.
S93 remains active without a partial P.

The specialized replay exposed one stale constructor inventory path from the
earlier input/display relocation. Its historical identity now names the real
Shared receiver, and the direct-source scan includes that receiver explicitly.
The expected 133 constructors and 101 historical identities are unchanged;
the standalone T344 shape verifier passes after the two-line receiver update
(+2/-1 across one CMake gate source). The original failed aggregate replay
is not reported as a passing target. Its retained strict matrix still passes
499 rows, 478 strict and 21 declared deferred, on each width. Both original
aggregate targets exit nonzero for the obsolete shape path; corrected shape
target replays pass per width. The aggregate final acceptance remains open.

## D4 Memory, CPU/PIC And Preview Test Owners

D4 memory transaction setup and registration failure assertions now compile
at the Core test owner. The Model40 receiver retains D4 configuration,
mapping, IOCHK and clear/reset checks. Four original exhaustion modes and
successful retry remain. Focused dual-width execution passed before the final
filename-only relocation, whose target builds on both widths. This batch
adds 186/removes 122 lines across six logical paths including the move.

Five CPU/PIC cases no longer hand-build private Core port state in App
fixtures. Separately compiled Core test code owns bare port initialization
and teardown; original CPU setup, PIC operations and assertions remain.
All five pass per width (0.94s x64, 0.50s x86 total). Seven logical paths
add 57/remove 34 lines. Neither batch adds a production API or CPU executor.

CPU timing preview now builds at test/x86/ibmpc-common. Original scanner
matrices, six execution scenarios and markers remain. Core test code captures
actual committed/cancelled transaction and trace counters and invokes the CPU
preview through the Core-owned execution context. No production getter or
mirrored guest state is added. The old App source/target is removed; the root
test identity remains. Independent x64/x86 builds and executions pass
(1.18s/2.24s). An initial independent link failed for missing Core linkage;
explicit x86-core-observable linkage resolves it without App dependencies.
T344/T388 references select the actual receiver. Direct private App imports
now occupy four paths. Whole-S verification remains open. No MyNES, INI,
external master or executable input changed in these test batches.

## Transaction S2 Core Receiver

The complete transaction test now compiles at test/x86/core. Private
transaction reset/cancel, DMA memory admission and actual Core memory/port
mechanics are owned there; chip provenance uses the public CPU contract.
The fixture's real DMA Core instance owns its port table and remains initialized
for route registration. DMA no longer registers against a temporary allocated
Core whose port table is copied in and out. The sole App consumers of those
two helper headers are gone, so both obsolete headers are deleted rather than
moved or wrapped. No production API, state mirror or transfer algorithm changes.

Original instruction bytes, trace pair/provenance checks, five controller
phases, out-of-range rejection, before/after callback order and failed device
write commit/cancel counts remain. Independent Shared x64/x86 build and test
pass (1.38s/0.71s); final root receivers pass (1.37s/0.09s). T344 retains the
same classified identity at the actual receiver. The remaining direct App
private imports are the 80386 timing manifest and BYOB probe. Indirect review
and whole-S verification remain open; no partial P is delivered.

The 80386 manifest's remaining direct Core layout access is removed. A
separately compiled Core test operation checks actual FPU completion status
and transaction address/opcode, value/ModRM and CPU-FPU kind. The original
nonzero/at-most-19 completion bound stays in the runner. Root x64/x86 builds
and executions pass (5.92s/5.73s). The embedded task-switch recipe source
uses public Core/CPU contracts, not private layout; its textual source sharing
still requires the final indirect recipe-ownership review. The App manifest
has no remaining direct private Core include or member access. The BYOB probe
is the last direct-private reader, not proof that the entire S is complete.

## BYOB Diagnostic Owner Boundaries

BYOB remains an App integration scenario with unchanged INI/external media
loading. Its 24 physical diagnostic reads use the actual Core test operation,
not paused-debug access. Core test code also owns the write-time CPU capture,
raw fault diagnostic, reset fetch, memory query, ROM metadata copies and
diagnostic write-observer attachment after construction. It returns no RAM,
CPU or ROM image pointer. A considered public fault-diagnostic replacement
was rejected during review: its running-state restriction would suppress
the original timeout diagnostic. The original raw capture is retained at Core.

Board diagnostics reuse existing controller/composition fixtures for FDC, HDC,
IRQ and KBC deadlines. Additional boot fixture code owns real PIT, DMA, video,
FDC terminal-observer binding and refresh-request observations. Private AT
keyboard BAT/repeat and XT PPI pin/byte observations compile separately at
their family owners; the Board fixture sees only their test contracts.
No App diagnostic imports a private Core/Board/family header or borrowed
mutable layout. No production API, state mirror, timing or guest input changes.

All 231 original printf format strings are identical and ordered against the
saved pre-edit probe. Both final probe builds pass. With the unchanged default
INI, one Turbo/short diagnostic execution per width exits zero and reports
DOS prompt (7.39s x64/6.89s x86 command wall time); these are probe checks,
not fresh delivered-product or other-profile acceptance. All 99 initial Board
rows have direct boundary dispositions and the direct import query is empty.
Indirect recipe/fixture review and final independent/full/gate/product/boot
verification remain mandatory. S93 remains active without partial delivery.

The first indirect pass finds 18 App test C/header paths importing CPU-private
headers or CPU construction fixtures. These are not production regressions,
nor disposed by the zero Core/Board import result. They comprise three decoder
inventory runners, legacy ALU, five protected-16 board cases, privileged call
gate, CPU execution-context, two CPU/FPU profile tests, three CPU/PIC cases,
protected-privilege and the protected-PIC support header. Actual symbol/layout
use must decide obsolete include removal versus an owner-local test receiver.
Two App timing runners textually include Shared task-switch recipes; the 80286
runner also includes a call-gate source. Their recipe sharing requires an
explicit ownership/duplicate-state review, not an automatic passing disposition.

## CPU-Private App Test Reconciliation

All 18 identified App C/header paths now have actual dispositions. Three
decoder inventory and two metadata/profile cases need only the existing public
CPU interface. Legacy ALU and protected privilege also use public diagnostics;
the legacy FLAGS expectation retains the same explicit 0xfffc802a reserved-bit
input. Six protected/call-gate cases retain their descriptor and IOPL values
as named guest-encoding data in their existing bootstrap fixture, independent
of CPU-private implementation names. No production API or algorithm changes.

The execution-context test moves intact to test/x86/chips/cpu and uses its
existing CPU-local bus fixture. Five CPU/PIC cases and protected-PIC support
also move intact there: hidden CPU preparation and assertions are CPU-local,
PIC observations use public contracts, and the actual Core port table setup
stays in separately compiled core/composition_fixture.c. Old App sources and
target definitions are removed, not left as forwarders. Source changes in
these six receivers are include paths only; their original markers remain.

Root x64/x86 builds pass for nineteen affected cases. The six initially
checked cases pass 6/6 per width; the following thirteen pass 13/13 per width.
Independent Shared execution-context tests pass per width (0.20s/0.14s total),
and all five independent CPU/PIC cases pass per width (0.78s/0.63s total).
T344 still classifies 133 direct constructors and 101 historical identities.
App CPU/chip-private include query is now empty. This proves this inventory
batch, not all indirect dependencies or S93 acceptance. Timing-recipe ownership,
final independent/full-suite/gates and eight fresh product checkpoints remain.

Actual-diff fidelity review of eight public-encoding cases and six relocated
CPU/PIC files finds no body differences beyond the declared include/name/value
substitutions; the guest values are unchanged. A recursive quoted-include walk
from App source/tests traverses 254 non-public files, stopping at declared
interfaces, and reaches zero Shared production-private headers. Shared public
headers also have no private x86 include. These are dependency checks, not
substitutes for behavioral verification.

The two timing runners' textual recipe imports have a valid retained reason:
they reuse the same original task-switch/call-gate preparation bytes as the
standalone Core regressions. Both recipes and their debug fixture use existing
public neutral Core/CPU operations only; their sole instance is an opaque Core
handle, with no hidden CPU/RAM state, clock rewrite or embedded production
implementation. The renamed test main is not invoked as a second executor.
Keeping the one shared recipe source avoids copying its setup. The remaining
App .c import is its own command.c whitebox test, not a Shared peer layout.
This completes the identified indirect recipe/CPU-fixture batch.

Fresh complete root units pass 491/491 on x64 (104.21s) and x86 (75.37s).
The extra case is the retained execution-context executable's now-declared
unit route. Initial x86 specialized verification detected its missing root
target-ledger declaration; that declaration is corrected without weakening
route-count or duplicate-registration checks. Complete gate results and final
independent builds/products/checkpoints remain pending, not accepted here.


## Legacy Current Snapshot

On 2026-10-04 S97 removes accumulated T539/T540 history and superseded packets
from Current, preserving their text below with relative links rebased. This is
historical context only: apparent active/acceptance headings retain their
original time/task context, not current admission. Current is the sole status
authority; S94-S97 supersede S93's unaccepted execution packet. No test/source
requirement is retired by moving this material. The original removed block has
480638 characters after LF normalization.

S93 working migration has opaque common FDC/HDC/video and independent AT KBC
and XT PPI receivers. Sixteen rebuilt hardware/rollback regressions pass per
width; this is not full-unit or boot acceptance. Common whole-board ownership,
Profile integration/lifetime and fixture reconciliation remain. AT parity
has a working independent owner; D4 RAM decoding and opaque platform/refresh
state now live in Model40 without board-layout imports. The board dispatches
Profile reset/NMI/refresh/deadline/finalization through one frozen binding;
D4 RAM is no longer embedded in board state. Product-specific construction
and diagnostics have left the common receiver. Seven common board files now
live in x86; copied guest input/display values now belong to the NXVM Machine
adapter, and the former App devices directory has no remaining files.
The real board target builds without App, and its four-row public
construction/reset matrix passes. Selected regressions pass per width, not the full
acceptance suite. The
[work record](../../etc/evidence/t540-s93-whole-board-receiver-work.md) distinguishes
current proof from the complete acceptance still required. No partial P is
delivered and no current product artifact is replaced.

The four residual unit compile failures are resolved through public KBC/Core
bus behavior, without private getters. The post-repair x86 diagnostic unit
run passes 476/476; the x64 post-repair run also passes 476/476. They cover the
restored null NMI callback and updated FDC negative scan. After the copied-value
header relocations both Core/Machine target builds pass, but final complete
verification is still required. The tools-enabled independent unit suite passes
136/136. Private fixture/diagnostic owner reconciliation is not complete.

The original DMA token and Compaq shared-port construction tests now belong
to test/x86/ibmpc-common and build without an App. Their coverage is retained;
private Core port access is replaced by the existing public bus contract.
After this receiver change complete units pass 476/476 per width; the
constructor inventory and source dependency gates pass. This does not close
the remaining fixture/diagnostic reconciliation or S93.

The auxiliary-PIT regression is also Shared-owned and passes independently
and per width with its original exact timing/reset assertions. Its bus access
is public, but the retained Core-private arbitrary-time test operation still
needs owner reconciliation; this is not a completed private-boundary cleanup.

FDC topology and media-change tests and their unique fixture now reside with
the Shared Board owner. The two tests use opaque Core bus operations rather
than Core layouts; original IRQ/DREQ/READY/reset/sector assertions remain.
They pass independently and complete root units pass 476/476 per width after
all affected targets rebuild. App's direct-private include inventory is now
118 files; indirect internal-fixture and diagnostic dependencies remain, so
neither this count nor green units qualifies S93 for acceptance.

The complete FDC command/DMA matrix now also builds independently at the
Shared receiver. Its 174 failure-check sites and eight transfer calls remain;
Core port/RAM access is public and the existing Shared DMA fixture is reused.
Full root units again pass 476/476 per width, with the original seven FDC
boundary negatives passing. Direct-private App include readers now number
117; remaining test/diagnostic boundaries and final acceptance remain open.

## S93 Active Packet

The final 386 manifest now uses the existing public protected bootstrap.
LTR establishes the actual busy TSS; IRET publishes user CS/SS/CPL, and VM86
segment loads use existing stopped debug operations. Task-gate IDTR setup
executes LIDT after the original bootstrap. The obsolete protected timing
fixture and its private CPU-borrow helper are deleted; only an intentional
negative-test injection still names the forbidden helper. Fresh final manifest
tests pass per width; all 96 original static byte arrays retain their values,
with the private IOPL macro replaced by its unchanged 3000h value. T332 keeps
44 owners and T344 keeps 125 constructors/101 historical identities by mapping
the retired constructor to its existing public-bootstrap receiver. Complete
485-case units pass per width (x86 68.01s, x64 70.98s); both complete
81-command specialized gate targets pass. The runner reset-error test now
installs its fault callback only after acknowledged pause, then resumes before
requesting reset. Independent tools-off tests pass 190/190 per width; tools-on
passes all 196 cases per width after correcting two obsolete negative-test
reasons. All six manifests, documentation and diff checks pass. Eight isolated
NXVM Release builds finish successfully on both widths for all four products.
All eight PE/version/debug-section/hash and sole-production-Core checks pass.
Fresh default, XT and AT unchanged-INI checkpoints pass on both widths;
the last two Model40 checkpoints are running.
All eight fresh unchanged-INI checkpoints pass. The complete default-profile
integration remainder passes 19/19 per width, without repeating its
already-passed boot case. Final actual-change review also finds 99 App
test/diagnostic private Board/peripheral/family header readers, including
frozen-plan and construction-failure access. They require explicit owner/public-bus
reconciliation. This test boundary, final verification and actual-change
review remain required. S93 is not accepted.

The final reader reconciliation receives 38 instruction/PIC, FLAGS, fault,
string and port/table-register tests plus five unique fixtures at
test/x86/ibmpc-common. The 15-, 17- and six-test independent batches pass
on both widths. Original programs and assertions remain; changes are local
fixture paths, unnecessary private includes and three exact architectural
constants instead of private CPU macros. The frozen 99-file intake now has
27 received rows and 72 unresolved rows. After the 17-test batch full units
pass 485/485 per width (x64 75.39s, x86 69.37s), with both complete
81-command gates. T332 retains 44 owners, T344 125 constructors/101 historical
identities, and the strict matrix 403 rows/382 strict/21 deferred. The six-test
batch's root replay passes 485/485 on x64 (60.48s) and x86 (74.15s), with both
complete 81-command targets and terminal exit zero.
All six manifests and documentation/diff checks pass. These are test-only
changes; no production input, artifact, MyNES or owner INI changes. S93 stays
active without a partial P.

The next ten video-behavior receivers and their shared fixture now build
independently at test/x86/ibmpc-common without Core-private headers/layouts.
Independent strict builds and 10/10 tests pass per width. Port ownership uses
the public exclusive registration contract before freeze; operational accesses
use the existing stopped Core bus/memory contracts. Pixel/palette, planar,
DAC/chain-4, Compaq/generic routing, reset and failed-config retry checks remain.
The frozen intake has 37 received and 62 open rows. T344 classifies 126 real
constructors, including the single neutral video fixture, with all 101
historical identities retained. Its complete root replay passes 485/485 per
width (x64 268.68s, x86 70.87s), with both complete 81-command gate targets
and terminal exit zero. No
production/API/input/artifact or MyNES change; S93 remains unaccepted.

Two KBC AUX/serial protocol tests now build independently at test/x86/ibmpc-at.
One real neutral Core fixture replaces fake stack Core/port ownership; public
bus accesses freeze/reset the executor before use. IRQ wiring uses one
common-owned public PIC fixture, also consumed by the remaining mixed test.
All original command/reply, IRQ, FIFO and serial deadline assertions remain.
Independent strict builds and 2/2 tests pass per width (x64 0.83s, x86 0.39s).
The frozen intake has 39 received and 60 open rows; the mixed KBC construction
test stays open. T344 classifies 127 real constructors/101 historical identities.
Its complete root replay passes 485/485 per width (x64 60.00s, x86 58.94s),
both complete 81-command targets pass, and the process exits zero.

Three RTC IRQ/CMOS/time-axis tests now live at test/x86/ibmpc-common.
The first two replace private fabricated Core/port instances with real opaque
neutral executors. The third keeps same-owner Board RTC inspection and the
original observable Core/Board variant, with no Core-private header.
Independent strict builds and 3/3 tests pass per width (x64 2.63s, x86 1.53s).
All old hardware predicates remain. T344 classifies 128 constructors with
101 historical identities. The frozen Board intake has 40 received/59 open;
Core-private test dependencies outside that 99-file inventory still require
the same ownership reconciliation before acceptance. Latest root x64 units
pass 485/485 in 59.36s; x86 passes 485/485 in 58.79s. Both complete
81-command targets pass and the serial verification process exits zero.
S93 remains active without a partial P.

The DMA/RTC authority test now has a Shared Board receiver with public Core
bus operations. Its refresh case replaces private arbitrary-time injection
with four actual one-tick FNINIT retirements through the existing debug/run
interfaces, explicitly checking the original 3+1 tick boundaries. No original
hardware assertion is removed. Independent strict dual-width builds/tests
pass (x64 case 1.13s, x86 case 0.89s). The frozen Board intake is now
41 received/58 open at that batch. Full root validation then passes 485/485
per width (x64 61.30s, x86 60.19s), with both 81-command targets passing;
the serial process exits zero.

Five further Core mechanism tests have real test/x86/core receivers:
memory inspection, ROM-route rollback, timeline, explicit time and transaction
lifecycle. Their private Core checks remain Core-owned. Memory inspection
uses the existing neutral executor; ROM rollback and timeline retain public
Board composition because they actually test reset aliases and device order.
Explicit time reads RTC through public guest ports instead of Board layout.
All original assertions remain; no production API or implementation changes.
Independent dual-width tests pass. The frozen Board intake is 42 received/57
open; the four other Core receivers are outside that inventory. Full root
verification after this five-test batch passes 485/485 per width (x64 60.91s,
x86 59.66s), with both 81-command targets passing and serial process exit zero.

The scheduler test now has separate Core and Board timing receivers. Core
instruments its copied attachment and forwards through the existing callback
contract, without importing Board private state or functions. The Board test
retains the original PIT/clock/deadline qualification checks at their real owner.
Independent strict dual-width tests pass 2/2 (x64 1.71s, x86 0.73s).
The frozen Board intake is 43 received/56 open. Full root units after this
ownership split pass 486/486 per width (x64 58.59s, x86 58.05s). Both complete
specialized targets pass after the new Board test is added to the canonical
unit target list; serial verification exits zero. No production/API/profile/
artifact change is part of this batch.

Four pure Core tests now use neutral executors at test/x86/core: memory-device
registration, immutable ROM mapping, rational provider clocks and bounded
execution. CPU/PIC lifecycle has separate Core identity and Board IRQ/reset
receivers; one public guest-program fixture retains the original execution
checks. Reset-memory alias preparation also has one Core-owned implementation,
reused by the Board fixture. Independent strict builds and six tests pass per
width (x64 5.47s, x86 3.41s). The frozen Board intake is 44 received/55 open;
the four pure Core moves are outside that inventory. T344 classifies 130 real
constructors with all 101 historical identities retained. Fresh full-root
verification passes 487/487 per width (x64 132.82s, x86 62.01s), and both
complete 81-command specialized targets pass; the serial process exits zero.
All six manifests and documentation/diff checks pass. This is not S93
acceptance or a partial P.

Six more pure execution/configuration tests now live in test/x86/core and
link only the neutral Core: 386 address size, REP CMPS/SCAS, the real-mode
corpus and exact ticks, INT/IVT and provider freeze lifecycle. Original guest
programs and failure/timing assertions remain. No production API is added;
the formerly implicit configuration reset alias uses the existing fixture.
Independent strict tests pass 6/6 per width. Fresh full-root units pass
487/487 per width (x64 64.21s, x86 62.22s), with both complete 81-command
specialized targets passing and the serial process exiting zero. T344 retains
130 constructors/101 historical identities. These moves are outside the
frozen Board intake, which remains 44 received/55 open. Six manifests and
documentation/whitespace checks pass. S93 is not accepted or partially committed.

PIC phase/INTA now resides at test/x86/core. Its real opaque PIC pair uses
the existing attachment contract, while Core-private transaction checks stay
at their owner; Board-private imports are removed. All eight original failure
expressions, IRQ0/14 cascade, vector/ISR/trace/frame/reset checks remain.
Independent dual-width strict tests pass. Fresh full-root units pass 487/487
per width (x64 62.87s, x86 61.23s), and both complete 81-command specialized
targets pass; serial verification exits zero. The frozen Board intake is now
45 received/54 open. KBC's mixed protocol/CPU/port-allocation test remains open.
No production/API/artifact/owner-INI/MyNES change belongs to this batch, and
S93 remains active without a partial P.

Controller authority now has separate Core attachment-phase and Board FDC/HDC
receivers, passing independent strict tests 2/2 per width. The original joint
callback/context identity test remains in App and passes per width; its private
boundary is explicitly unresolved. T344 retains 101 historical identities and
classifies 132 actual constructors. Frozen intake stays 45 received/54 open.
Fresh full-root units pass 489/489 per width (x64 61.85s, x86 61.95s),
and both complete 81-command specialized targets pass. Serial verification
exits zero, with 330 registered routes and 407 strict-matrix rows (386 strict,
21 deferred). Six manifests and documentation/whitespace checks pass.
This is not S93 acceptance.

The 80386 DMA competition test now belongs to Core, with separately compiled
Board-owned construction retaining the real DMA channel provider and opaque
handles. Original transaction/HOLD, exact eight-tick and CPU/DMA/PIT/PIC order
assertions remain. Independent strict builds/tests pass per width. A failed
public-deadline adaptation was removed rather than inventing clock qualification.
Frozen intake is 46 received/53 open; T344 retains 132 constructors and 101
historical identities. This move is covered by the full-root proof below.
Root dual-width focused builds/tests also pass; six manifests and documentation/
whitespace checks pass. S93 remains active without a partial P.

The remaining controller binding-identity test now belongs to Core. A separately
compiled Board fixture checks its own back-pointer and provides independently
expected attachment callback/context values; Core retains the exact saved-
binding comparisons without Board layout imports. Independent strict tests
pass per width. Original App residual source is deleted. Frozen intake is
47 received/52 open, with 132 constructors/101 historical identities retained.
Fresh full-root units pass 489/489 per width (x64 63.11s, x86 60.80s), and
both complete 81-command specialized targets pass. The serial verification
process exits zero: 330 registered routes and 409 strict-matrix rows (388
strict, 21 deferred). Six manifests and documentation/whitespace checks pass.
S93 remains active without a partial P.

RAM creation and its allocation fixture now belong to Core; a separately
compiled Board fixture retains production projection, attachment and Board
back-pointer validation. No source imports both private layouts. The original
allocation counts, seven pre-allocation clock-ratio failures, null outputs and
three topology publication cases remain. Independent dual-width RAM tests and
root RAM/plan/port-assembly focused tests pass. Frozen intake is 49 received/
50 open; 132 constructors and all 101 historical identities remain classified.
Complete units pass 489/489 per width after this RAM change (x64 64.45s,
x86 59.03s); both complete 81-command gate targets pass, with 413 strict-matrix
rows (392 strict, 21 deferred).

Nine construction/port mechanism functions now have one Core-owned test source,
used by the original App regression and a new independent strict Shared test.
Original ranges, allocation indices, null outputs, retries, byte lanes, supplied
64-bit ticks and PIT/PIC rollback checks remain. The Core source uses only opaque
Board/PIC/PIT contracts, not private Board layouts; Board construction retains
its back-pointer check. Independent tests and original root regression pass
per width. Complete units pass 490/490 (x64 61.29s, x86 62.40s). The missing
canonical registration-list entry is corrected; both full specialized targets
pass, with 331 unique registered routes and 417 strict-matrix rows (396 strict,
21 deferred). The serial verification process exits zero.
The FDC/HDC/RTC rollback matrix now uses the same Core receiver, with private
controller/topology assertions at a separately compiled Board fixture. App
retains only D4 attachment and the original driver. All original failure,
retry, busy DMA/port and shared-3F7 checks remain. Independent and root focused
tests pass per width; complete units pass 490/490 (x64 61.95s, x86 61.33s).
Both 81-command specialized targets pass, including 419 strict-matrix rows
(398 strict, 21 deferred). Intake is 50 received/49 open; T344 retains 132
constructors and all 101 historical identities. This batch changes no
production/API/artifact, MyNES or owner INI. S93 remains unaccepted.

The frozen-plan regression now builds at the Shared Board receiver, with
Core attachment, allocation, XT route and exact deadline assertions compiled
at their actual Core owner. All ten case groups and fourteen historical
markers remain; no source reads its peer's private layout. Independent tests
pass per width; complete units pass 490/490 (x64 62.72s, x86 61.70s), and
both complete 81-command specialized targets pass. The strict matrix contains
420 rows (399 strict, 21 deferred). Frozen intake is 51 received/48 open;
T344 retains 132 constructors and all 101 historical identities. This test-only
batch does not change production, EXEs, MyNES or owner INIs. S93 remains active.

Video registration rollback now has a Core-private receiver and separately
compiled Board video assertions; planar parity likewise separates actual Core
RAM fault/publication from Board Port-B/PIT/NMI behavior. Original hardware,
allocation, rollback, retry and identity checks remain, with no new production
API or peer-layout import. Independent tests pass per width. Latest complete
units pass 490/490 (x64 61.18s, x86 60.41s), both complete 81-command targets
pass and verification exits zero. Strict compilation covers 422 rows (401
strict, 21 deferred); T344 retains 133 constructors/101 historical identities.
Frozen intake is 53 received/46 open. No production/artifact, MyNES or owner
INI change in these test-only batches; whole S93 acceptance remains open.

Instance isolation and checked-memory tests now build at Core, and the 286
display timing checkpoint at Board, with their original assertions unchanged.
Their unique Board construction fixture is moved, not duplicated. Independent
3/3 tests pass per width. Latest complete units pass 490/490 (x64 60.43s,
x86 58.96s); both complete 81-command targets pass and verification exits zero.
T344 retains 133 constructors/101 historical identities and 422 strict-matrix
rows (401 strict, 21 deferred). These additional Core-private/public tests are
outside the frozen Board intake; its 53 received/46 open count is unchanged.
No production, artifact, MyNES or owner INI change; S93 remains active.

The mixed KBC regression now has AT protocol, Core route/reset and Board IRQ1
receivers, with no peer-private header imports or copied route registry. All
172 original failure checks and ten static data arrays remain; a public bus
write additionally checks its status. Independent tests pass per width, as do
fresh full units (490/490: x64 133.20s, x86 60.05s) and both complete 81-command
specialized targets. Strict compilation covers 424 rows (403 strict, 21
deferred); T344 retains 133 constructors/101 historical identities. Frozen
intake is now 54 received/45 open. This test-only batch changes no production,
API, artifact, MyNES or owner INI; S93 remains active without a partial P.

XT PPI now has a Board wiring/time receiver, XT-local state assertions and
Core-local route-allocation rollback. All 148 original failure checks and FIFO
bytes remain; no source reads a peer's private layout. The auxiliary-PIT test
also removes its private Core include: exact synthetic time advances compile
at the Core test owner, not through a new production API. Independent 2/2
tests pass per width; complete units pass 490/490 (x64 61.14s, x86 61.64s), and
both full 81-command specialized targets pass with serial exit zero. Strict
compilation has 428 rows (407 strict, 21 deferred); T344 retains 133 actual
constructors/101 historical identities. Frozen intake is 55 received/44 open.
No production/API/artifact, MyNES or owner INI changes; S93 remains active.

Nine App video/Profile regressions now use public Core/Board operations while
their original private route and aperture predicates compile separately at
Core/Board owners. Product composition stays App-owned; all 60 original failed
checks and original topology failure bits remain. After the final 32-bit bus
read refinement, complete rebuilt units pass 490/490 per width (x64 123.94s,
x86 62.29s), both full 81-command specialized targets pass and serial
verification exits zero. Strict compilation covers 432 rows (411 strict,
21 deferred), retaining 133 constructors/101 historical identities. Six
manifests and diff checks pass. Frozen intake is 64 reconciled/35 open;
production/API, MyNES, artifacts and owner INIs remain unchanged. This is
working-boundary progress, not S93 acceptance or a partial P.

Reset-ROM alias now builds independently at Core, with its two original
absent-memory flags checked in a separately compiled Board fixture. App
CMOS/RTC keeps Profile/seed composition and uses public Core bus operations;
the original synthetic RTC/PIC advance, interrupt, reset and diagnostics
compile at Board. Both original test bodies retain their conditions and
values after the explicit owner substitutions. Independent/focused tests
pass per width, followed by complete units 490/490 (x64 64.68s, x86 66.07s),
both full 81-command specialized targets and serial exit zero. Strict
compilation has 434 rows (413 strict, 21 deferred), retaining 133 constructors
and 101 historical identities. Frozen intake is 66 reconciled/33 open.
Six manifests, documentation and diff checks pass; production/API, MyNES,
owner INIs and artifacts remain unchanged. S93 is not accepted.

Four default/5170 App composition cases now use separately compiled Core
registry/timing/A20 and Board config/IRQ/KBC fixtures. All original Profile,
CMOS, floppy-format, reset and refresh assertions remain; no production API
changes. Complete units pass 490/490 per width (x64 65.08s, x86 64.77s), both
full 81-command specialized targets pass and guarded serial verification
exits zero. Strict compilation has 443 rows (422 strict, 21 deferred),
retaining 133 constructors/101 historical identities. Frozen intake is
70 reconciled/29 open. Six manifests, documentation and diff checks pass;
MyNES, owner INIs and artifacts are unchanged by this batch. S93 remains
active, without a partial P.

Four App FDC/HDC binding, port and T242 media cases now use public Core
bus operations and separately compiled Board controller fixtures. All 59
original failure-check sites, command bytes and 18-sector/ATA progress
assertions remain. Complete units pass 490/490 per width (x64 66.24s,
x86 64.40s), both full 81-command specialized targets pass and guarded
serial verification exits zero. Strict compilation has 447 rows (426 strict,
21 deferred), retaining 133 constructors/101 historical identities. Frozen
intake is 74 reconciled/25 open. Six manifests, documentation and diff checks
pass; production/API, MyNES, owner INIs and artifacts are unchanged by this
batch. S93 remains active without a partial P.

Model40 FDD geometry and 5170 firmware/FDC topology cases now retain App
composition while their private checks compile at Core/Board owners. The
Model40 case checks actual FDC connection config, restoring the accepted
baseline instead of substituting planned topology. All seven failure-check
sites and 42 comparisons remain, with exact owner-substitution body audits.
Complete units pass 490/490 per width (x64 65.39s, x86 63.84s), both complete
81-command specialized targets pass and guarded serial verification exits
zero. Strict compilation has 450 rows (429 strict, 21 deferred), retaining
133 constructors/101 historical identities. Frozen intake is 76 reconciled/
23 open. Six manifests and diff checks pass. No production API, MyNES,
owner INI or artifact change belongs to this batch; S93 remains active.

Model40 DMA, HDC and D4 composition checks now use separately compiled
Core/Board fixtures and public Core bus operations. Actual HDC connection
config replaces an intermediate planned-topology substitution; the original
duplicate-bind provider/request lifetime, DMA wait/ready predicates, 19 PIT
ticks and media/IRQ/reset operations remain. Exact body audits preserve all
ten failure-check sites and static bytes. Complete units pass 490/490 per
width (x64 77.56s, x86 63.95s); both complete 81-command specialized targets
pass. Strict compilation has 454 rows (433 strict, 21 deferred), retaining
133 constructors/101 historical identities. Frozen intake is 79 reconciled/
20 open; manifests and diff checks pass. No production API, MyNES, owner INI
or artifact change belongs to this batch. S93 remains active without a P.

Model40 BYOB and S7/S8 composition cases now read copied Board plan/clock/
topology values and owner-local Core ROM/transaction state. Original exact
ROM-start lookup, one-tick reset interval and post-budget raw test I/O remain.
Baseline review restores AT AUX/scanning predicates that intermediate WIP
had replaced with command-only checks or native-byte rejection; the 1Ch
injection is removed. All 42 failure-check sites remain. Complete units pass
490/490 per width (x64 73.82s, x86 63.55s), both full 81-command specialized
targets pass and guarded serial verification exits zero. Strict compilation
has 468 rows (447 strict, 21 deferred), retaining 133 constructors/101
historical identities. Frozen intake is 82 reconciled/17 open; all manifests
and diff checks pass. No production API, MyNES, owner INI or artifact change
belongs to this batch. S93 remains active without a partial P.

Two-session isolation and four-Profile DMA timing qualification now retain
App scenarios without private Core/Board imports. Separately compiled owner
fixtures keep CPU/memory/port and RTC/controller identity checks. FDC/HDC
compare actual opaque objects, not addresses of the newly introduced pointer
slots; RAM/register/watchpoint checks and all four deadline cases remain.
Exact baseline-mapped body audits pass. Full units pass 490/490 per width
(x64 72.00s, x86 62.67s), both full 81-command specialized targets pass,
and process 7955 exits zero. Strict compilation has 471 rows (450 strict,
21 deferred), retaining 133 constructors/101 historical identities. Frozen
intake is 84 reconciled/15 open. No production API, MyNES, owner INI or
artifact change belongs to this batch; S93 remains active without a P.

XT Profile and session initialization tests now use owner-local Board plan/
controller and Core route fixtures instead of private layout imports. XT
checks actual FDC/HDC connection values, restoring the accepted-baseline
checks that intermediate WIP had replaced with planned topology. All XT
ports, geometry, BYOB and display assertions remain. Initialization retains
nine timing predicates, full byte comparisons and controller rules; separate
public Core getters still check applied RAM/CPU/FPU. Baseline-mapped body
audits pass. Full units pass 490/490 per width (x64 71.39s, x86 64.08s), both
full 81-command specialized targets pass and process 27107 exits zero.
Strict compilation has 475 rows (454 strict, 21 deferred), retaining 133
constructors/101 historical identities. Frozen intake is 86 reconciled/13
open. Six manifests and diff checks pass. No production API, MyNES, owner
INI or artifact change belongs to this batch; S93 remains active without a P.

5170 clock selection now reads copied actual Board clocks/controller rules/
RTC/KBC timing and opaque plan observations. Baseline review restores all
seven keyboard-chip repeat callback checks at the AT fixture owner; the
working KBC serial test remains complementary, not a replacement. FDC Read
Track DOS removes unused private headers with its entire scenario unchanged;
external INI/overlay integration passes once per width (15.39s/14.29s).
Full units pass 490/490 per width (x64 75.56s, x86 61.21s), both full
81-command specialized targets pass and process 59591 exits zero. Strict
compilation has 478 rows (457 strict, 21 deferred), retaining 133 constructors/
101 historical identities. Six manifests and diff checks pass. Frozen intake
is 88 reconciled/11 open. No production API, MyNES, owner INI or artifact
change belongs to this batch; S93 remains active without a partial P.

Five HDD integration diagnostics now use the existing separately compiled
Board controller fixture or public chip constants; private Core/Board/HDC
imports are removed. Complete bodies match S92 after observation substitutions,
except a diagnosed test-containment correction: full DIR takes 6.42s rather
than the old five-second command window. Its separate sixty-second window
preserves success predicates, production behavior, owner INI and outer CTest
limit. Corrected directory checks pass once per width (18.65s/19.70s); HDD
handoff and ATA PIO DOS also pass once per width. The two unregistered probes
have explicit dual-width build proof only. Full unchanged units pass 490/490
per width (64.46s/66.81s), with both 81-command gate targets passing and strict
matrix 480/459/21. Six manifests and documentation/diff checks pass. Frozen
intake at that batch is 93 reconciled/six open; S93 remains active without a partial P.

The two D4 refresh/DMA competition cases now keep their Profile scenario in
App while separately compiled Model40, Core and Board fixtures own the actual
private assertions. Original 19/11-, 2/1/9- and eight-tick boundaries, HOLD
exclusion, refresh callback, trace ordering and reset checks remain. Full units
pass 490/490 per width (x64 75.14s, x86 65.99s); both full 81-command targets
pass with process exit zero. The frozen intake is 95 reconciled/four open.
Six manifests and documentation/diff checks pass; production, MyNES, owner
INIs and product artifacts are unchanged in this test-only batch. S93 stays
active; final indirect-reader sweep and whole-packet acceptance remain required.

The Model40 FDC S24 case also retains its App scenario while Board-owned
fixtures inspect actual controller/drive bindings, advance/reset the FDC and
reuse the existing DMA transfer fixture. Core ports use the public bus.
The original channel matrix, PIO/DMA sector transfer, IRQ/READY/reset and
terminal-callback assertions pass on x64 (0.96s) and x86 (0.23s). Frozen intake
is 96 reconciled/three open. The preceding 490/490 full-suite results predate
this controller-fixture edit; final full verification remains required.
The old App DMA fixture still has a transaction-test caller and is not deleted
by implication. Its indirect boundary belongs to the final whole-source sweep.

The Model40 retirement diagnostic now uses separately compiled Core/Board
fixtures for its five actual private observations. Both diagnostic targets
build; no registered runtime pass is claimed. The FDC negative test remains
App-owned because it checks App media/assembly as well as Shared controllers;
its baseline and all seven negatives pass once on x64 (2.49s) and x86 (2.44s).
Frozen intake is 98 reconciled/one open: the BYOB DOS boot probe. The indirect
Core/fixture sweep and fresh complete verification remain required; S93 is
active with no partial P.

The final Core-reader sweep also identifies 19 direct-private C/header test
paths beyond Board-only dispositions, recorded in the final-reader evidence.
Two CMOS tests now preserve their actual port operations through the existing
Core fixture; the fault outcome test uses the public memory header. All three
targets build per width; the two repository-only cases pass once per width.
INI CMOS runtime proof is not claimed here. Indirect fixtures and the 19
remaining direct-reader dispositions must be exhausted before acceptance.

Four direct Core-reader paths are subsequently reconciled: RAM/port and
input/display tests have independent Shared receivers, App error injection
uses its Core fixture, and default integration drops an unused private include.
Original assertions pass on both widths; the two Shared tests also pass in
independent builds. The direct sweep now has 15 open paths, separate from the
Board intake's one open BYOB probe and indirect-fixture review. No production,
MyNES, owner INI or artifact input changed. Final full verification remains open.

Three D4 Core-access tests, the mixed D4/time test and the Debug mapping test
are subsequently reconciled. Their original real parity, A20, shutdown,
PIT/port side effects, tick-1/tick-4 order, four invalid-axis checks and Debug
lease assertions pass on x64/x86. Core operations compile at the Core test
owner, D4 latch operations at Model40; Debug drops only an unused include.
That sweep initially left ten unresolved C/header paths. D4 memory
transactions and five CPU/PIC cases now use separately compiled Core test
owners; their focused dual-width checks pass. CPU timing preview is received
at Shared Board, retaining its scanner matrices, execution cases and actual
publication counters; independent x64/x86 execution passes. Transaction S2 is
also received at Shared Core, with direct DMA registration on its actual port
table instead of a temporary copied owner. Both obsolete App DMA/port helper
headers are deleted. Independent and root dual-width regression passes.
The 80386 manifest's FPU completion and transaction identity checks now
compile at Core's test owner; root dual-width regression passes. BYOB now
uses separately compiled Core/Board/AT/XT diagnostic owners. Its 231 printf
format strings remain identical and ordered; both default INI probes reach
DOS prompt. App direct private imports/member accesses are empty; all 99
original Board rows have direct boundary dispositions. The mandatory indirect
fixture sweep, timing-recipe ownership review and complete acceptance remain.
T344 retains 133
constructors/101 historical identities, T388 Jcc and six manifests pass.
This is test/build/evidence-only progress, not final full-unit/S93 acceptance.

The 18-path CPU-private App test sweep is now reconciled: thirteen cases use
existing public CPU contracts and explicit guest encoding inputs; the CPU
execution-context case and five PIC composition cases, including their shared
support fixture, now compile at test/x86/chips/cpu. Original bodies/assertions
remain; separately compiled Core port setup is not copied into CPU tests.
All nineteen affected root cases pass per width; six received cases also
build/run independently per width. App chip-private include query is empty.
The identified indirect timing recipes retain one public-operation setup
source, not a hidden layout or second executor; the recursive App include
sweep reaches no Shared private header. Fresh full units pass 491/491 per
width. Final specialized/independent verification, products and checkpoints
remain open; no partial P is delivered.

The 386 manifest's GPR/FLAGS and copied segment observations now use existing
public Core debug operations. S7 data-segment preparation and same-CPL far
transfer recipes no longer write hidden DS/ES caches. Seven ring-zero GDTR
updates execute real CS-relative LGDT, preserving code/EIP/FLAGS and never
rewinding preparation time. The obsolete post-LTR halt-latch write is removed.
Fresh x86/x64 manifest builds and executions pass; all 96 original static
instruction arrays remain unchanged against HEAD. Privilege-transition,
TSS/VM86 and inherited protected-bootstrap private setup still remains, so
this is not complete fixture reconciliation or S93 acceptance.

FPU S65 now has Core, Board IRQ and CPU-local state receivers. All three pass
independently and per x86/x64; original full CPU rollback/cache assertions
remain at the CPU owner. The old mixed App source is removed. T332 retains
44 owners, and both complete specialized gate targets pass after adding the
new Board IRQ test to the actual registration inventory. T344 classifies
125 constructors including the explicit neutral
FPU constructor. Only the 386 manifest and its inherited protected-state
fixture still consume the private CPU helper. Final whole-batch verification
and product/boot acceptance remain open; no partial P is delivered.

The latest generic timing ledger receiver also builds independently and passes
on x86/x64. Its 57 retained program arrays and original cost/failure assertions
remain; public registers and ten-instruction guest bootstraps remove its CPU
private fixture dependency. T332/T344 retain 44 owners and 124 constructors.
Three live sources still use that private fixture: the 386 manifest, S65 FPU
interface and inherited protected-state fixture. The earlier 483/483 full-unit
passes precede this latest move; final full verification remains required.

HDC and Compaq HDC regressions and their unique fixture now build at the
Shared receiver. Opaque construction and public bus operations replace the
Compaq test's raw Core/port setup; original hardware checks remain. Complete
units pass 476/476 per width after rebuilding all affected unit targets;
six affected integration executables compile per width, without boot claims.
Independent HDC tests pass 2/2. DAG/raw-borrow and HDC/video ownership checks
are reconciled. Six remaining family gates now pass with isolated negative
controls, and the x86 full specialized target passes. The floppy boot diagnostic
uses copied observations/public trace rather than private device layouts; its
build passes. Private fixture reconciliation and final full verification,
manifests, artifacts and eight boot checkpoints still block S93.

The App reset-alias fixture uses public Core operations; complete x64 units
pass 476/476 after that change. Xebec wiring now builds independently with its
Shared owner, retaining all 25 failure-check sites and ten port-route checks.
One parameterized Shared DMA fixture serves its one-controller path and the
existing two-controller matrices. Xebec/FDC/DMA pass 3/3 independently;
both width target builds pass. Latest full units/gates are still being verified,
and other private fixture/diagnostic consumers remain unresolved.

Latest Xebec-batch x64 units pass 476/476 and specialized gates pass. A subsequent
public-fixture cut removes its transitive CPU/Core/Board private headers;
the full tree currently has compile failures in remaining private-mechanism
tests and is not acceptance-green. Public provider/bus regressions pass 4/4,
fault-memory observations pass 9/9, and migrated PIT-divider/firmware rollback
tests pass independently. No old private helper is restored to hide the
remaining owner reconciliation, and no S93 P or artifact is delivered.

The next reconciliation removes private reset-time CPU access from the 80286
protected-mode and real-exception receivers through existing stopped debug
operations. FPU and paging mechanism tests now build at test/x86/core without
App fixtures; original completion/page-fault/reset assertions remain. All four
receivers pass per width; FPU/paging also pass independently. The latest x86
whole-unit build has 38 failing targets, down from 42 in the preceding inventory.
This is measured compilation progress, not full-unit acceptance. No product,
MyNES or owner-INI change is made; S93 remains active.

The latest batch also moves port ownership and debug/PIC tests to the Shared
Board and retirement/FWAIT observations to Core. FPU ESC uses existing copied
debug snapshots and public register patches, without a private CPU fixture.
These four regressions pass per width; the three moved tests pass independently.
T317 source identity now resolves each target's actual source directory; T332
retains all 44 strict owners and T344 retains all 119 constructors. The latest
x86 whole build records 34 failing targets. Remaining private CPU/timing and
Board consumers, complete verification and products still block acceptance.

The subsequent public-register timing batch moves three unchanged timing
matrices and the unique public Board fixture to Shared. Descriptor coverage
is split between a neutral Core C7/ES bus case and CPU-owned protected-mode
exit/cache assertions. All affected receivers pass independently and per
width. The instruction-timing/quantum/qualification receiver now constructs
only neutral Core; its terminal #UD setup executes guest LIDT rather than
borrowing CPU state. It also passes independently and per width. The latest
complete x86 unit-target build records 27 failures, down from 34 before these
batches. This remains a red build, not full-unit acceptance. See the work
record for the individual proofs and the stale-result exclusion. No S93 P,
product replacement, MyNES build or owner-INI change is delivered.

The latest CPU profile receiver separates 8088 queue/cache assertions into
the CPU execution test and keeps public fault/opcode diagnostics in neutral
Core. T359 S5 also constructs only neutral Core, with public register patches
and the original timing matrix. One shared test fixture explicitly blocks
IVT delivery for both negative receivers, rather than lending private IDTR
state. Four related receivers pass independently and per width; the complete
build remains red and S93 is not accepted.

The 8086 and 80186 instruction timing ledgers now construct neutral Core and
live under test/x86/core. Their 83 and 61 static program/recipe arrays are
unchanged. Public debug register operations replace borrowed CPU state;
Core-local overflow and opaque FPU completion checks retain their owner.
Four timing receivers pass independently and five affected tests pass per
width. The latest full x86 unit-target rebuild records 25 failures, down from
27. Full units, final manifests/products and boot acceptance remain open;
no S93 P, deployed binary, MyNES or owner-INI change is delivered.

T359 S6 privileged timing now constructs neutral Core at test/x86/core.
Guest LGDT/LMSW, segment loads and far jump replace direct CR0/segment-cache
mutation; reset-cleared GDT bytes are reloaded before each setup. All thirteen
original arrays remain byte-identical. Preparation time stays on the same
timeline and the original ten row costs are measured after it, without a
rewind. The receiver passes independently and per width. Subsequent string/
protected-port timing and 8086/8088/80186 manifest receivers retain their
original arrays and assertions through public Core operations. Control/PIC
composition now builds independently at test/x86/ibmpc-common and passes on
both widths. CLI/STI and HLT Board receivers now also pass independently and
per width; full private CPU storage assertions stay with the CPU owner.
The interrupt-entry receiver now also passes independently and per width,
retaining its 24 original top-level cases. NMI pending and full hidden CS/SS
rollback checks have CPU-local receivers, not public getters. Limited-stack
preparation loads real SS with a valid ESP before applying the original
stopped-state negative ESP. The latest complete x86 unit-target rebuild
recorded 14 failures after that batch. The subsequent software-INT receiver
retains all eight static matrices across public Board and CPU-local tests;
its 48 terminal-invalid forms, 32 real-mode state cases and six full protected/
VM86 rollback cases remain checked without peer-private storage. Three
affected tests pass independently and per width; the latest complete x86
unit-target rebuild records 13 failures. The old App S50 source is removed.
The subsequent VM86 delivery, hardware-priority S3 and privileged-table S5
receivers pass independently and per width. Their Board setup executes actual
LGDT/LIDT/LMSW/LTR/IRETD; the existing stopped FLAGS patch retains the original
frame bytes. Eleven private failure cases retain full CPU and stack rollback
at the CPU owner, which also checks DR breakpoint delivery, segment
invalidation and masked/consumed NMI latches. Four obsolete App test sources
are removed. The complete x86 unit-target rebuild now records 10 failures.
VM86 IRET and protected same-CPL IRET now also pass independently and per
width through neutral Core public setup. CPU-local receivers retain all six
VM86 cache checks, full stack-fault CPU rollback and the eleven original
protected success/failure cases. The old VM86 IRET App source is removed;
the protected source remains only as a not-yet-received S51 include, with no
independent linked target. The complete x86 unit-target rebuild now records
8 failures. T344 retains 101 historical identities and classifies 123 direct
constructors, including the new public protected-IRET owner.
S51 real/PIC IRET and S4 interrupt-return composition now pass independently
and on both widths. CPU-local S51 retains its real-mode and terminal-invalid
state/rollback cases. Four obsolete App test sources are removed; the latest
complete x86 unit-target rebuild records 6 failures, down from 8. T332 still
verifies 44 strict owners and T344 still classifies 123 constructors.
Call-gate and TSS I/O authorization now build independently with neutral Core.
Cross-width task switching has public Core and CPU-local receivers retaining
its eight original cases and private TR/LDTR assertions. All pass per width;
the 80386 manifest includer also passes per width. Three obsolete App sources
are deleted. The latest complete x86 unit-target rebuild records 3 failures:
prefetch locality, 80286 instruction timing ledger and 80286 timing manifest.
Full units and final artifacts remain unverified; S93 stays open, with no
partial P, product replacement or out-of-scope change delivered.

The 80286 instruction timing ledger now builds independently at test/x86/core
and passes independently and on both widths. Its 109 original byte arrays
remain unchanged. Public register patches and actual LGDT/LIDT/LLDT/LTR
preparation replace private CPU mutations; measured rows retain their original
costs while setup time remains on the Core timeline. Real-mode segment seeds
now use the corresponding architectural linear addresses rather than a stale
hidden base. The obsolete App source/target is removed. The latest complete
x86 unit-target rebuild records two failures: prefetch locality and the 80286
timing manifest. T332/T344 retain 44 strict owners and 123 constructors.
Full-unit acceptance and final manifests/products/boot checkpoints remain open.

The subsequent manifest reconciliation clears the last compile failure without
private CPU access or guest-clock rewind. Fresh full units pass 483/483 on
both widths after five memory-observation lifecycle errors are corrected using
existing memory_inspect; original rollback and byte assertions remain. Four
T388 gates now select the real Shared timing-test receivers, with unchanged
conditions. Both full specialized targets pass.
Remaining private-fixture ownership, final independent/corpus verification,
products and boot checkpoints still block acceptance. The
[work record](../../etc/evidence/t540-s93-whole-board-receiver-work.md) records the
exact scope; no partial P, MyNES or owner-INI change is delivered.

Prefetch/locality now has a neutral Core receiver and a separate Model40
refresh/preload receiver. Both pass per width; Core also passes independently.
Core-owned cancellation, HOLD, overflow/wait and locality assertions remain
private to Core, while Model40 observes copied refresh transactions and owns
its pending latch. The real board test advances through actual one-tick FNINIT
retirements rather than a private arbitrary-time operation. The obsolete mixed
App test is deleted. T344 retains 101 historical identities and classifies
124 constructors after this ownership split. The latest full x86 unit-target
rebuild records one failure, the 80286 timing manifest; no complete acceptance
or artifact delivery is claimed.

## Remaining Acceptance Plan

The owner requests bounded linear continuations on 2026-10-03 because S93
has run too long. S93 is superseded without acceptance or a P delivery; its
working source, test, build, artifact and evidence changes remain intact.
The accepted reference stays d76d2b15e. No component or artifact is accepted
by this re-plan, and no original whole-component completion predicate is waived.

T540 S94-S97 are the planned continuations, admitted one at a time with
their own complete packet. S94 is now admitted below. The
[proposal](../../history/M5-T540-shared-ibmpc-integration-proposal.md#remaining-acceptance-s94-s97)
defines each bounded scope, exit and transfer. S94-S96 produce bounded review
or verification results, not partial acceptance of the pending implementation.
S97 owns the complete implementation delivery and final acceptance. Existing
uncommitted code is agent-owned carryover, not unrelated user work; it must
neither be discarded nor reported as committed.

Fresh root units pass 491/491 per width and both specialized gate targets
pass. Independent tools-on x64 passes 291/291; the other three independent
suite results and final eight-product checkpoints remain pending. Prior
chronological S93 entries above retain their original evidence dates and
limitations; this section and Current Work govern the revised task state.

## S94 Review Record

T359 S2 receiver review retains all four profile timing rows, odd-word penalties,
dynamic multiply, Group-3 budget edges and width/segment/LOCK prefix cases.
Reject null construction, propagate public register-operation failure into
case cleanup, and stop subsequent Group-3 profile construction after failure.
Verify dual-width normal cases and actual failed/null producers without new
Shared APIs, private CPU readers or timing/guest-program changes.

Instruction-timing receiver review preserves nine guest timing cases,
split-quantum/reset equality, terminal fault/no-charge, requested stop and
physical qualification/copy/rejection checks. Guard successful-null owners
and failed run results before tick accumulation or reset continuation; release
unexpected owners in invalid-qualification checks. Verify both widths and
actual failed/null producers; no production/API or timing-value changes.

Core cross-width review preserves all eight 16/32-bit task transfer cases,
guest bytes and saved-state assertions. Guard null construction and stop
failed install/run/snapshot producers before consumers through each case's
existing cleanup. Retain independent CPU and Core registrations; verify both
widths and failed/null producers. No production/API or timing changes.

Task-paging/privilege review retains four paging/IRQ task-transfer contexts and
seven protected interrupt/exception/atomicity contexts, all original guest
bytes and error/frame/register assertions. Guard null construction and failed
install/run/diagnostic producers before consumers, propagate a failed delivery
diagnostic rather than masking it as success, and preserve the expected Core
fault in the stack-atomicity case. Consolidate equivalent GP/NP delivery checks,
use cached failure reports and one cleanup, and verify both widths plus actual
failed/null producers; no production/API or instruction/timing change.

Machine-time review retains all four invalid-axis rejections, reset-zero time,
exact 286 retirement budgets and D4 refresh-before-counter deadline ordering.
Chain failed producers before observation consumers, guard null Core/board/D4,
and never destroy the non-owned rejection sentinel. Use the existing single
cleanup and retain original tick values; verify dual-width normal and failed
producer/null-output paths without production, Shared API or timing changes.

Project-firmware floppy review retains all eleven in-code BIOS/media cases,
cross-track read/write, buffer and register preservation, recalibration/reset,
CHS rejection, masked-IRQ timeout/recovery and readonly/absent errors. Reject
successful-null profile/plan/registry/display/Core/board construction before
consumers through the existing cleanup; verify normal dual-width behavior and
failed/null preparation. Preserve exact single-instruction PC checkpoints and
budgets; no external input, production/API or timing change.

XT profile review retains the complete fixed descriptor, DMA/FDC/Xebec/CGA
routes, open-bus window, copied text frame and one/two-ROM BYOB construction.
Stop failed/null plan, registry and machine creation before bus access; capture
board configuration once and use checked public operations. Release rejected
BYOB outputs through the same cleanup. Verify original assertions and both
widths; do not introduce Shared API, board state mirrors or timing changes.

Model40 integration review retains missing-firmware rejection, CPU/RAM/D4/ROM,
disabled AUX/keyboard commands, two-drive CMOS/topology, reset device routing,
four FDC reset SENSE entries and HDC diagnostic/IRQ/IDENTIFY behavior. Use
checked public bus/reset calls and one cleanup, capture immutable composition
once per pre/post-reset phase, and report cached observations without new
post-failure device reads. Verify original unique assertions and dual-width
normal/failure paths; no production or Shared fixture/API change.

FDC read-track corpus review retains selected-drive motor rejection, untouched
RAM, DMA2 setup, all eighteen sector bytes, bounded real deadline advancement,
terminal CHRN/status, SENSE/IRQ clearing and non-MFM rejection. Replace exiting
bus wrappers and unchecked command advances with checked existing operations;
stop on first failed step and destroy the sole owner. Remove diagnostic-only
post-failure machine queries and initialize reported data before construction.
Consolidate identical result-byte reads, without adding public API or changing
guest timing. Verify original assertions and dual-width behavior.

Initialization review preserves default/override materialization, timing,
constructor output, invalid media, recovery and option-ROM rejection. Read
original and migrated bodies; consolidate repeated immutable captures and
check null plans before use, with one cleanup on failed construction. The
two old reset-named helpers only repeated missing-firmware construction;
remove that duplicate and correct the marker, without claiming reset proof.
Verify dual-width normal and actual-source failed/null results; no production
API, new test framework or timing change.

Default FDC-port/AUX review retains command/result phases, no-media and
disk-change handling, format/IRQ, write protection/rate rejection, full PIO
sector and real guest AUX IRQ12 packet delivery. Replace exiting bus/tick
test helpers with checked existing interfaces and release failed/null/inactive
construction through one cleanup. Failed AUX count reads must stop the run
loop rather than consume the remaining budget. Verify original coverage,
dual-width normal and actual-source failure/null paths; no production/API,
chip timing or external media change.

Model40 D4/BYOB review retains every alias, RAM/open-bus seam, A20, page
latch, reset, parity/IOCHK/NMI and copied-ROM/plan/pacing observation. Failed
CHECK steps must stop and release the existing owner; BYOB construction,
refresh-port reads and final rejected construction must use checked cleanup.
Capture immutable plan/ROM once without weakening assertions. Verify both
widths and actual-source failed/null producers; no production/API or timing
classification change.

Default/5170 profile review preserves the complete immutable descriptor/CPU
contract and FDC route/port assertions. Keep the Default test's interface-only
migration; in 5170, stop failed construction before route lookup and capture
the board composition once. Verify dual-width normal and failed/null lookup,
construction and port checks without production, profile or API changes.

Model40 FDD review retains native/compatible media geometry, rejected truncated
media, registry publication, reset retention and Default/5170 comparisons.
Reject failed reset and successful-null constructors before consumers, capture
the immutable FDC binding once and use the existing four-owner teardown.
Verify both widths and actual-source error/null cleanup; no production/API,
media master or profile topology change.

Model40 FDC protocol review retains all drive/media/rate/pitch matrix rows,
CMOS/KBC/topology facts, reset IRQ queue, PIO/DMA sector completion, terminal
observation, absent-media recovery and wrong-rate command results. Return
checked command/result/DMA-programming failures to one teardown and reject
failed reset/tick producers before later consumers. Consolidate repeated CMOS
and reset-result assertions as value tables/loops without deleting cases.
Verify dual-width normal and actual-source failure/null paths; no production,
public API, chip timing or external media change.

Model40 HDC review retains memory-backed 925/5/17 media, both drive-head
encodings, DRQ/IRQ status acknowledgement, full-sector consumption, rejected
IDENTIFY and SRST diagnostic checks. Replace exiting bus fixtures with checked
Core bus operations so failed programming/read reaches the existing cleanup;
retain owner-local service semantics and verify dual-width plus failed-producer
paths without production/API, media-master or timing changes.

Model40 private-composition review retains ROM rejection, transaction/clock
windows, CMOS/D4/speaker, prefetch retirement and disabled-AUX keyboard checks.
Capture each immutable owner-local contract once after successful construction;
chain checked bus/input producers before reply consumers instead of ignoring
port failure or continuing after an earlier assertion. Verify dual-width normal
and actual-source failure/null cleanup without production/API or timing changes.

Four-profile DMA-deadline review retains Default/Model339/Model40/XT clock
selection, DMA wiring/token and deadline disposition checks. Reject null or
failed plan/Core construction through cleanup, check the port write before
asserting DREQ and never print failed observation outputs. Verify dual-width
normal cases plus failed/null producer cleanup; no production/API or timing
classification change, and keep profile assembly coverage in App.

Instance-isolation review retains CPU/memory/port/RTC/FDC/HDC/watchpoint and
DMA-token independence plus PCAT NMI mask checks. Preserve the migrated real
FDC route-locality check before/after destroying the other composition. Reject
null or failed constructors, short-circuit later producers and clean up both
owners on every exit; verify dual-width normal/error paths without production
changes, duplicate media ownership or a new public state getter.

Model40 CMOS seed review preserves default/custom seed, checksum, session
isolation and reset-persistence assertions using the original in-code assets.
Short-circuit later construction after failure and reject a successful null
custom-seed owner; retain teardown of all returned owners. Validate dual-width
normal cases and constructor/reset failure/null-output paths; no firmware,
production or external asset changes.

Model339 clock review retains all descriptor/plan/board clock ratios, timing
dispositions and pacing classification, including the original owner-local
keyboard repeat check plus the migrated real KBC serial path. Short-circuit
failed producers and guard later KBC advances in the App test and its AT-owned
cadence fixture; sweep callers and verify dual-width normal/error paths. Keep
this profile-specific test in App; no production/API or timing-grade changes.

No-media video review restores the dormant legacy BIOS checkpoint as an INI
integration case, not a synthetic-HLT unit. Move its source to integration/dos,
register the Default-only target and remove media through existing per-session
FDD/HDD owners before reset. Preserve its instruction budget, INT10/no-F2,
BIOS text and cursor assertions. Guard construction/reset/observation failures
and clean up the INI session. Do not alter user INIs, firmware or asset masters;
verify actual dual-width execution before qualifying the retained coverage.
The registration sweep also restores the omitted mantle/entry-plan/arbitration
targets to the root aggregate inventory; Shared retains their sole CTest routes.
Validate both roots' exact registration counts without relaxing the verifier.

DMA competition review admits short-circuiting failed setup/run/observation
and guarding later programming/request assertions in both contexts. Preserve
CPU/DMA ordering, BUSRDY and reset-HOLD checks; keep the D4-attaching test in
NXVM. The Core-owned HOLD fixture must stop acknowledgement after failed
request and skip CPU exclusion after failed HOLD admission while retaining
release cleanup. Sweep its callers and verify normal dual-width cases plus
actual-source failure paths; no production/API change.

D4 refresh receiver review admits short-circuiting failed construction, attach,
DMA binding, reset, timing and observation producers, and guarding subsequent
port programming/request assertion. Keep this Model 40 test in NXVM; preserve
19/11-tick BUSRDY boundaries, refresh-before-DMA/HOLD ordering, A20/reset and
the non-D4 contrast. Verify normal dual-width and failure cleanup before its
qualification; use only existing owner-local fixtures, no production/API change.

Arbitration receiver review admits guarding subsequent setup/run, timeline and
trace observations after failed producers. Preserve the 3-tick CPU retirement,
DMA/PIT/PIC sequence, final run boundary and reset observations. Move the
App-independent test to test/x86/ibmpc-common, using the existing observable
Core/Board libraries and one registration. Verify dual-width normal execution,
failed-producer cleanup and independent compilation; no production/API change.

Entry-plan review admits short-circuiting later execution, reset and state
observations after failed preparation/validation. Preserve immutable-ROM entry,
running-state rejection, preload rollback, overlap rejection, RAM entry and
reset checks. Move this App-independent board-composition test to test/x86/
ibmpc-common with one Shared registration, retaining its target and marker.
Add its receiver to the inventory and verify dual-width normal and failure
paths; no production/API change.

Default PC/AT apply review admits separating failed construction from CMOS
observations, reading the observed floppy type once, and consolidating the
format helper's cleanup. Reject failed 80186 refresh reads before guest setup;
destroy returned owners on construction failure. Preserve all four formats,
CMOS checksum, refresh edge and guest polling assertions. Verify dual-width
normal cases and failed-producer paths before qualification; no production/API.

Fault-outcome and provider-composition review admits rejecting failed producers
before subsequent execution/reset/media observations, checking NXVM reset and
initializing the composition failure report. Preserve terminal #UD, faulted
run result, reset clearing, provider/media freeze and HLT predicates. Move the
App-independent composition test to test/x86/ibmpc-common with one Shared
registration and remove its App registration. Add the receiver to inventory;
verify dual-width normal cases and failed producer cleanup. No production/API.

Retained CGA-system coverage reconciliation admits registering the original
320-mode guest-instruction test as a repository-only unit and moving the
BIOS-dependent 640-mode test to integration. Preserve the original guest bytes,
pixels/palette, BDA 06h/03h and return-to-text assertions; use the existing INI
and discard-overlay integration path, never a unit BIOS service or asset write.
Add that integration receiver to the frozen inventory and verify both widths.
Its boot wait uses the existing Core deadline path. The 320-mode receiver also
rejects a failed snapshot capture rather than silently retrying it until the
budget expires; retain the successful non-graphics waiting case. No production/API changes.

NXVM video-system receiver review admits chaining the EGA sequencer's reset,
port and aperture producers before later consumers, routing failed construction checks
through its existing destruction, and removing the unused bus fixture include.
Preserve both CGA system programs, pixel/palette/BDA checks and EGA reset,
register-mask and aperture checks. Verify normal dual-width cases and failed
EGA producers/cleanup; no production/API or timing change.

8086 real-mode corpus review admits chaining setup/execution/diagnostic
producers and publishing the delivered-fault output only after complete success.
Guard all four consumers after failure, retaining segment override, REP direction,
INT/IRET and ordered port transactions. Verify dual-width normal cases and
actual-source failed-producer cleanup/output publication; no production/API change.

Xebec Board receiver review admits routing failed media binding, command-phase
checks, DMA admission and source-memory writes to the existing main cleanup.
Retain port ownership/no-ATA-alias, DMA3/RAM/media, IRQ5, terminal-count, mask,
initialize and reset coverage. Verify dual-width normal execution and actual-
source failed setup/read/write cleanup; no production/API/timing change.

Segment MOV Board receiver review admits guarding IRQ admission after failed
register patch and chaining execution/snapshot/frame reads. Preserve six
protected-fault and three SS/DS/FS IRQ-shadow contexts. Verify dual-width cases
and actual-source failed producers/cleanup; no production/API/timing change.

Legacy segment-stack Board receiver review admits chaining setup, snapshot,
expected-fault execution, diagnosis and memory producers before consumers.
Guard IRQ setup after failed register patch/capture. Preserve all sixteen
null-selector, rejected-selector, stack-limit and IRQ/frame contexts; verify
dual-width normal cases and failed-producer cleanup without production/API change.

IMUL immediate Board receiver review admits guarding its memory-source write
after preceding program/vector/handler writes fail. Retain protected-limit
rollback and both register/memory IRQ-no-shadow forms. Verify dual-width normal
cases and each failed memory-write position; no production/API/timing change.

T359 S6 timing receiver review admits removing its redundant unchecked snapshot
query from the setup-failure report. Report the existing initialized copied
values, retaining all ten system-instruction rows and the illegal-LOCK rollback.
Verify dual-width normal cases and all setup-producer failures; no timing, API
or production change.

Hardware-delivery S3 review admits guarding PIC preparation and execution after
failed NMI admission or NMI-mask setup in the real, protected and VM86 priority
cases. Preserve all four priority/frame/PIC contexts; reconcile retired private
NMI-pending assertions with CPU-owned coverage rather than adding a public
getter. Verify dual-width cases and failed-signal/mask paths before qualification.
The same review also admits destroying an already-created Core when the real
priority fixture's bind/freeze/reset preparation fails; retain its existing
failure result and verify lifecycle rollback rather than leaking that owner.

TSS I/O-map receiver review admits chaining preparation, execution, memory and
diagnostic producers before their consumers and initializing copied values used
by its existing failure report. Retain all seven 286/386 permission, denial,
truncated-map and IOPL contexts, guest tables/programs and successful predicates.
Verify both widths and actual-source failed-producer cleanup before qualification;
no production, API, timing or coverage change.

FPU receiver review admits guarding separate diagnostic/result predicates in
the 8087 arithmetic interface, S65 handoff/#NM delivery/protected #NM, and
the retained App ESC receiver. Initialize the 8087 helper's run result before
its failure report. Preserve every command, profile, exception/frame/register,
transaction and deadline assertion; verify normal dual-width cases and
actual-source failed-run/diagnostic/state probes. Reconcile the complete
mixed S65 original with its Core/CPU/Board receivers before accepting its
deletion; no production/API, guest timing or coverage change.

VM86 receiver review admits guarding the existing LGDT/LIDT post-run assertions
behind successful setup, execution and diagnostic capture. A failed run can
short-circuit diagnostic capture; its unwritten output must not be read by a
later assertion. Preserve both instruction contexts and all successful-path
checks, sweep the VM86 receivers for equivalent reads, and verify normal runs
on both widths plus an injected failed-run check. No production/API change.

Continue that failure-output sweep through IRET and stack receivers. The
protected IRET user-FLAGS case must guard its separate post-run diagnostic
predicate after a failed run/capture. Retain successful register/flags/stack
checks and verify its direct receiver and the S51 includer on both widths.
The S51 real/PIC cases also guard separate run-result/diagnostic predicates
when their producing call failed. Preserve every successful-path assertion.
Other stack sources with separate unchecked output readers remain unqualified
until their complete source/baseline review and matching guard repair finish.
No production/API or reduced assertion scope is admitted.

The four PUSH-immediate, PUSHA/POPA, ENTER/LEAVE and GPR PUSH/POP receivers
retain their original protected-fault and IRQ contexts. Guard ten separate
diagnostic/frame predicates and the following variant-specific IRQ assertions
behind successful producers. Compare complete S92 bodies with their receivers,
run normal dual-width cases and actual-source failed-run probes before qualifying
their source/deletion inventory rows. Preserve bootstrap bytes, partial-stack
effects, register/segment rollback, IRQ frame and PIC checks; no new API.

Neutral memory/reset/time receiver review includes the real-mode tick test's
failure diagnostic. Initialize its run result, copied observation and profile
before failed producers can reach the existing error report; retain all six
instruction rows and their exact ticks/status/retirement assertions. Verify
normal dual-width cases and actual-source failed producer reporting. The same
batch reconciles immutable ROM, overlay device routes, RAM/port isolation,
386 address-size execution and explicit RTC/time/reset/overflow checks against
their complete originals. No production, API or timing reclassification.

CPU/FPU metadata and decoder inventory review admits repairing the three
80186/80286/80386 test runners' existing file-output cleanup. Final fprintf
failure must not skip fclose; the 386 intermediate write failures must also
close the runner-owned stream. Keep lexical loops, accepted counts, JSON
schema/masks and success markers unchanged. Verify normal dual-width outputs
and actual-source write-failure/close-failure probes, plus profile/diagnostic
and paired execution-context regressions. No production or Shared API change.

LEA/MOV/MOFFS/XCHG receiver review admits guarding nine separate post-run
diagnostic, register and frame predicates behind their successful producers.
Retain all fourteen original fault/IRQ execution contexts and assertions.
XCHG's accumulator failure report must not dereference an absent Board after
failed construction; preserve its initialized result report and read PIC values
only when that owner exists. Verify dual-width normal cases and actual-source
failed-run, failed-snapshot and failed-construction probes; no production/API.

Video/interrupt/FPU receiver review admits correcting the CECG S9 negative
case so unexpected acceptance of its invalid configuration cannot pass. Retain
the generic retry on the same owner and the complete directional port matrix.
Guard the interrupt-return composition's separate result/diagnostic predicate;
initialize the call-gate reporter's result/diagnostic before failed producers.
Initialize the FPU IRQ fixture before its oversized-input short circuit and
check size without overflow. Preserve original programs, frames, registers,
video paging/routes and exception checks; verify actual-source failure probes
and dual-width normal cases. No production/API or coverage reduction.

Core/PIC receiver review admits two existing test failure-boundary repairs:
the INT/IVT receiver must return failure when neutral Core construction fails;
the protected far/data PIC fixture must use the existing checked IRQ-binding
helper before asserting a source. Preserve CPU profiles, programs, frame/register
and PIC assertions, and verify normal dual-width runs plus failed setup probes.
These sources already belong to the frozen inventory; no production/API change.

Windows HDD admission-probe review admits its diagnostic lifetime repair:
retain the overlay read buffer until the final diagnostic has consumed its
partition entry, and initialize the run-result diagnostic for pre-run failures.
The existing source is already in the frozen universe. Sweep HDD/Windows
diagnostic readers; preserve all guest bytes, timing budgets and success checks.
No production/API, Shared, media or profile change is admitted.
The same pre-result diagnostic sweep includes HDD firmware handoff: initialize
its run-result before a failed Core call can reach its existing error report.
Keep failure reporting and success criteria unchanged; this source is already
in the frozen universe.
The existing BYOB boot probe has the same failed-run diagnostic reader; include
its run-result initialization in this sweep. Its whole-source review remains
pending and cannot be inferred from this bounded diagnostic check.

Integration-source review admits checking all five ignored presentation capture
results in the keyboard, prompt and MEM-fault DOS tests. A missing frame cannot
qualify prompt/text success or supply diagnostic characters. Keep all guest
programs, timeout/input/assertion paths; no production or Shared API change.
Add the two previously unchanged prompt/MEM-fault sources to the frozen universe
(620 paths). Validate absent-frame rejection and retain runtime qualification
in S96; do not fabricate a frame or weaken the success checkpoints.

Gate-source review admits repairing the migrated production scan universe in
the existing controller and CPU/PIC authority verifiers: include all three
Shared IBM-PC receivers, update retired Board path predicates, and exclude only
the PIC bus's own private implementation/header from its private-import ban.
Keep every existing constraint and validate positive plus copied-input negatives
for elapsed time, lifecycle/READY, CPU layout and PIC layout in the new roots.
No production/API change or new framework is admitted.
The same sweep includes FDC's missing AT/XT consumers, FPU's migrated neutral
Core/Board consumers, display and DMA/RTC caller scans, and the product-execution
gate's formerly unreachable runner exception. Add the previously unchanged FPU
verifier to the frozen universe (618 paths, one added CMake path); retain all
original coverage. Repair these existing scans and prove their new-root rejects
using copied test inputs, without changing production or MyNES.

The owner's renewed performance request admits batching the strict matrix's
Ninja command inspection into one invocation. Keep every row and warning check;
index compiler commands by their actual target object directory, retain negative
classification probes and measure the unchanged matrix before/after. No persistent
result cache, test removal, MyNES build or product behavior change is admitted.
The current firmware-capability/legacy-normalization batch additionally admits
construction rejection before private memory queries, initialized failure-print
outputs, and null timing-case rejection before diagnostic dereference. Preserve
all firmware rollback/context/lifecycle and 25 arithmetic plus exception timing
cases; no production or API changes. Verify actual-source failed producers.

The current Board LES/LDS/LSS/LFS/LGS batch additionally admits guarding
snapshot/diagnostic consumers after failed producers, initializing diagnostic
print values, and avoiding absent-board access in failure reporting. Preserve
all protected fault, source atomicity, IRQ frame and shadow assertions; verify
the actual receivers under failed construction, run, capture and diagnosis.

The current Core RAM/ROM/transaction/timeline batch additionally admits
immediate test rejection after failed construction and guarded consumption of
run/timeline outputs. Preserve all success, rollback, transaction ordering and
standalone timeline assertions; no production/API or MyNES change. Verify the
actual receivers under unwritten-output failures before qualification.

The current retirement-observation receiver batch additionally admits guarding
its two separate post-run predicates after failed setup or execution. Preserve
every original copied observation, CPU/profile, timing, snapshot and provider
lifecycle assertion; compare the reset aliases with the original fixture and
verify normal dual-width execution plus actual-source unwritten-output probes.
No production, API, timing classification or executable input changes.

The current CPU/PIC receiver batch additionally admits checking IRQ-source
binding in task-switch16, outer-IRET and IDT-privilege receivers before CPU
execution, using their existing finalization paths. Guard outer-IRET failure
printing when PIC construction did not succeed. Preserve every original CPU
state, task/frame, IRQ and register assertion; verify both widths and actual
source construction/binding rejection. Reconcile the earlier binding-sweep
claim against these three previously unqualified paths. No production/API change.

The software-INT receiver batch additionally admits guarding separate run,
diagnostic and protected-snapshot consumers after failed producers, plus one
CPU receiver indentation correction. Preserve all original real-mode frame,
FLAGS, GPR/segment, protected-fault, VM86 and PIC boundary contexts across the
CPU and Board receivers. Verify normal dual-width cases and actual-source
construction/run/diagnostic rejection before qualifying the original deletion.
No production/API or instruction/timing behavior change.

Interrupt-entry review additionally admits guarding external-origin execution,
fault-delivery setup and delivery-rollback capture after failed preparation.
Retain all 26 original gate/frame/fault/PIC/NMI contexts and the CPU-owned
cache/latch assertions. Use existing cleanup and public fixtures; no production
or API change. Validate both widths before final receiver qualification.

MOVX and FS/GS receiver review additionally admits short-circuiting setup,
run, diagnostic and snapshot consumers instead of accumulating failure while
continuing to read unwritten outputs. Preserve their original opcode/profile
matrices, source-read exclusion and stack/segment rollback assertions, plus
the already separate CPU tests. No production/API or successful-path change.

Protected/real exception receiver review admits the same short-circuit repair
for before snapshots, execution, diagnosis and subsequent handler execution.
Remove the two test-local real-mode run wrappers by expressing their existing
operations directly in each checked sequence; initialize fault run results.
Preserve all six protected and six real #UD delivery contexts and both real
#GP contexts, metadata matrices and exact frame/register assertions. Verify
both widths and actual-source failed producers; no production/API change.

The paired segment-selector review also admits short-circuiting bootstrap,
capture/write/run/diagnostic sequences, guarding their variant assertions and
using existing destruction before setup-error returns. Initialize the bootstrap
failure report's run result. Retain all four 286 rejection, five LXS, four MOV,
two POP and final POP-rollback contexts; no guest bytes/API/production change.

The 80286 protected-mode receiver admits guarding all nine contexts' separate
producer/consumer chains and initializing its positive-case diagnostic report.
Validate helper pointers before optional fault-table preparation, preserve the
original programs/frames and public stopped entry patch, and verify actual-source
failed setup/run/capture/read/diagnosis cleanup. No production or API change.

Protected call-gate review admits removing two redundant unchecked diagnostic
queries in the 16-bit DPL rejection failure reporter. Report the already
initialized/copied values; do not perform additional Core queries after failed
setup/execution/capture. Preserve all ten 16/32-bit entry/rejection contexts,
guest frames and the shared guest-only bootstrap. Verify both widths and
actual-source failed producers; no production/API or assertion change.

The same protected-bootstrap sweep includes four external/interrupt-gate,
outer-NMI and outer-IRET/RETF receivers. Remove the external software-entry
reporter's two unchecked re-queries and two outer-IRET/RETF reporter snapshot
re-queries. Preserve all 33 original contexts, fault/NMI source, privilege,
frame/FLAGS/stack assertions and initialized reports. Verify all six receivers
and the common bootstrap against full originals plus normal dual-width and
actual-source failed-producer checks. No guest behavior or API change.

The unchanged 515-row matrix now passes on x64 in 4.91s (36.14s before) and
x86 in 5.14s. Duplicate, missing-source, strict/deferred and dependency-owner
negative probes reject correctly. This is focused verifier evidence, not S94
closure; actual-source review and current complete units remain required.

The renewed S-latency inspection keeps the existing incremental trees and
eight-way verification scheduling. A sixteen-way full-unit experiment produced
timeouts, including tests that emitted their success marker; it was stopped
and is not acceptance evidence. Do not raise timeout budgets to mask this
result. Reuse only unchanged-input passing checks; build affected targets and
run their temporary focused selection during repairs, then run complete
dual-width units/gates at S closure. Native desktop serialization stays intact.
This workflow changes no production, Shared API, MyNES or executable input.

The owner's next performance request also admits optimizing the NXVM CPU
manifest verifier's repeated array copying and generated-key scans. Use only
invocation-local appendable collections and a key index; preserve every input
validation, duplicate rejection, output record and ordering. Compare the complete
canonical output hash and all consuming timing/decoder checks before continuing
the source audit. No persistent cache, Shared change or reduced test corpus.

The unchecked IRQ-binding sweep additionally records the existing, previously
unchanged `test/x86/ibmpc-common/core_machine_pit_irq0_s2_smoke.c` receiver. Add
that implicated test to the review universe (617 paths, no original removal);
qualify its setup failure along with the fifty flagged test sites before closure.
Preserve its exact PIT edge/counter/gate/reset assertions; no production/API change.
This sweep admits one checked binding helper in the existing test PIC fixture
for the 49 positive standalone calls in the other 39 receivers. Leave intentional
invalid/assigned binding calls unchanged. Reconcile PIT-IRQ0 initialization as a
single checked construction/freeze/reset/cleanup path and use the existing checked
port-write helper. Preserve every positive argument, program and assertion;
verify termination on failed binding and both-width receiver regressions.
All 50 sites are repaired and the 40 receivers pass once on each width.
Mechanical body/hash comparison retains the original programs/assertions;
the failed-binding termination probe passes. Remaining full-source review
dispositions stay open; this focused batch does not close S94.

FLAGS receiver review also admits checking the existing PIC fixture's vector
programming and register capture status, plus IRQ binding in the existing
CPU/Board IRQ fixture. Fail through existing test termination/cleanup before
observing IRQ results; retain all flags, frames, descriptor and arithmetic
assertions. Both fixtures already belong to the frozen inventory. The remaining
raw IRQ-binding callers require explicit sweep disposition before closure.

80386 timing-runner review admits checked public register seeding in both real
recipe preparation paths and explicit successful-null construction rejection.
Stop after failed seed before memory, execution or observation operations;
retain all original operands, repeat-phase distinctions and tick expectations.
Check the table-load snapshot through its existing result rather than exiting
the process. Verify dual-width normal cases and bounded actual-source failure
probes; the remaining protected/task recipes still require separate review.
No production or public API change is admitted by this test repair.

The same 80386 review also admits its S7 protected and page-granular LSL
recipes: replace process-exiting register seeds with checked public operations,
stop subsequent table/program/observer work on failure and use one existing
owner cleanup point per recipe. Preserve all S7 programs, descriptor operands,
four LSL selectors and 21/25/22/26 tick rows; verify failed producers and both
widths without a new fixture or public API.
Its existing protected bootstrap fixture also rejects a successful-null Core
constructor before further setup; this is the same admitted CPU-test ownership
mechanism, not a change to production construction or bootstrap bytes.
The four initial S6 direct/memory/return/interrupt recipes also initialize their
owner before validation, check FLAGS and DS-from-SS seeding, and read final
CS/EIP through one checked copied snapshot. Failure diagnostics use that copied
value rather than performing more fallible reads. Retain all target bytes,
ticks and selector/IP assertions; qualify failed and invalid-input paths.
The same admitted review covers all remaining S6 task/VM86/outer-return owners
and S4/ESC constructors. Reject successful-null ownership and check the final
ESP seed; the runner's own key lookup rejects NULL before string comparison.
Preserve every recipe, target byte, phase and timing predicate; no Lib/API change.

Composition review also admits checking PIC programming and source binding in
the existing Board test `test_board_pic_unmask_delay_matches`: stop before
source assertion or deadline checks when setup fails; preserve its 5170
120-tick assertion and existing test-only contract. No production/API change.

RTC review revision also covers `core_machine_rtc_smoke.c`: check its PIC
programming and IRQ-source binding through the existing setup cleanup path.
Retain every RTC timing, interrupt and reset assertion; no production API changes.

DMA review revision admits replacing unchecked successful port writes in the
existing channel receiver with one test-only checked-write helper. Preserve
intentional rollback/error-code calls, all 126 first-service rows and subsequent
transfer assertions. Update the existing fixture/manifests; no production change.

AT KBC review also admits the existing AUX port receiver's setup/cleanup repair:
stop after failed PIC construction or IRQ binding and route its three early
assertion failures through the existing KBC-before-PIC cleanup. Preserve all
AUX commands, queue and serial timing checks; no production/API changes.

[S94 review](../../etc/evidence/t540-s94-source-review.md) freezes the original 609 source/test/
build/tool paths plus the owner-requested strict-verifier optimization and
inherited debug-adapter comment repair and PIC fixture failure repair
(625 total after the admitted integration capture, manifest-cost sweeps,
BIOS-dependent CGA, provider-composition, entry-plan and arbitration receivers), including 165 deletions.
Eight receiving files have current
source inspection, with baseline construction/reset/publication comparisons;
whole-diff and coverage qualification remain pending. Four comment-consistency
findings are recorded, not silently accepted. No source changes or P delivery
occur in this initial review batch.

## S96 Accepted Verification Result

Under the owner-approved S94-S97 split, all eight optimized stripped 0540
products pass their profile/PE/banner/sole-Core/deployment checks and each
unchanged-owner-INI boot group passes once. Default reaches the DOS prompt;
XT, AT and Model40 reach installer-running, on x64 and x86. The owner's build
performance follow-up removes deployment's redundant same-file INI write-back;
four INI hashes/timestamps remain unchanged by repeated deployment, and the
existing artifact gate rejects its reintroduction. MyNES is unchanged.
The [S96 evidence](../../etc/evidence/t540-s96-artifacts-and-performance.md)
records actual timings, eight hashes, source review and verification boundaries.
No partial implementation P is delivered. S97 still owns full pending-tree
delivery, final external integration and T540 closure review.

## S92 Accepted Review

Coordinator actual-pushed-diff review accepts Shared P1 `5a1033665` and
NXVM P2 `4aa8dc29c`: one opaque Shared DMA bus owns controllers, page/latch,
bindings, routes and cycles. App callers use its public contract; old copies
and embedded layouts are removed. The original 126-row first-service matrix
and Core failure assertions retain their receivers. Complete units pass
474/474 per width, independent tests 138/138, tools-off tests 132/132.
Both specialized targets, DMA negatives, six manifests and documentation pass.
Eight fresh stripped 0540 products pass unchanged-INI checkpoints once each;
all eight sole-Core link proofs pass. MyNES and owner INIs are unchanged.
The [S92 evidence](../../etc/evidence/t540-s92-dma-aggregation.md) records actual
source review, counted changes and product hashes. This closes S92, not T540.

## S91 Accepted Review

Coordinator actual-pushed-diff review accepts Shared P1 `98fd9460b` and NXVM
P2 `f19d84304`: PIC port/cascade/source ownership is physically Shared, all
producers borrow opaque leases and old App copies are deleted. Original chip
algorithms and hardware assertions remain. Independent tests pass 135/135;
tools-disabled tests pass 129/129; complete units pass 472/472 per width.
Both specialized gates, six manifests and documentation checks pass. Eight
current stripped 0540 products pass their unchanged-INI checkpoints once.
MyNES and owner INIs are unchanged. [S91 evidence](../../etc/evidence/t540-s91-pic-aggregation.md)
records proof, line counts and product hashes. Whole DMA aggregation is the
next receiver; this acceptance does not close T540 or imply AT/XT completion.

## S90 Accepted Review

Coordinator actual-commit review accepts Shared P1 `ddc957eaf` and NXVM P2
`f33730fea`: stateless PIT routes have one Shared implementation; every
primary/auxiliary caller is connected and old binding files/layout are deleted.
The unused display mode ABI is removed completely. Composition selects one
Core implementation; Release observation proof covers the corrected link.
Independent tests pass 130/130; full units pass 472/472 per width. Gates,
six manifests, documentation and eight final single-run unchanged-INI boot
checkpoints pass. Eight stripped 0540 products are current. MyNES, chip
algorithms and owner INIs are unchanged. The [S90 evidence](../../etc/evidence/t540-s90-pit-port-extraction.md)
records actual-diff review, complete proof and final product hashes.

## S89 Accepted Review

Coordinator actual-commit review accepts Shared P1 `4e23f14b6` and NXVM P2
`eed6b8e54`: four provider source/header files and the original registry test
have one Shared owner; every App caller is connected and old copies/archive
are deleted. Production bodies and original assertions retain their behavior.
The standalone suite passes all 123 cases; complete units pass 471/471 per
width. Specialized gates, six manifests and all eight single-run unchanged-INI
boot checkpoints pass. Eight stripped 0540 products are current; MyNES and
owner INIs are unchanged.

The [S89 evidence](../../etc/evidence/t540-s89-common-provider-extraction.md)
records actual-diff review, line counts and hashes. The next common-board
receiver must also remove the pre-existing unused display-mode binding half;
this does not defer any provider extraction or claim T540 complete.

## S88 Accepted Review

Coordinator actual-commit review accepts Shared P1 `8d3c57df3` and NXVM P2
`89f9416b2`: 35 source/header moves, eligible neutral tests and independent
build have one owner; former App copies and duplicate proof compilation are
deleted. Private-header cleanup relocates only the existing trace declaration
and Release no-op. All original algorithm and assertion behavior is preserved.

The final independent suite passes 121/121. Both complete unit suites pass
470/470; both specialized targets and 45 electrical/construction/duplicate
negatives pass. The final eight stripped 0540 EXEs pass identity/freshness/hash
inspection and all original unchanged-INI boot terminals once each. All six
manifests, documentation governance and actual pushed-diff checks pass.
Post-commit source comparison and both 81-check specialized runs pass as well.
MyNES and owner INIs are unchanged; no unaffected MyNES build was performed.

The [S88 evidence](../../etc/evidence/t540-s88-neutral-core-extraction.md) records
the complete ledger disposition, original coverage, line counts, actual-change
review and product hashes. Governance P3 closes S88 only. The next receiver
must physically deliver flat IBM-PC board components, not repeat Core
per-function preparation or claim T540 complete.

## S87 Accepted Review

Coordinator actual-commit review accepts immediately pushed P1 `c662ecd08`:
119 NXVM-only paths comprise six production files, 98 test paths, one gate,
six documents and eight artifacts. Reviewed working blobs match the committed
version; HEAD equals origin/master. Constructor, pre/post-bind failure cleanup,
neutral header and direct-fixture classes retain one owner and original
coverage. Documentation governance, selected local links and diff checks pass.
The [S87 evidence](../../etc/evidence/t540-s87-construction-private-boundary.md)
records exact checks, counts and artifacts. Governance P2 accepts S87 only;
physical component extraction and T540 remain open.

The oversized former S12 port batch was split into linear receivers. S88 now
moves Shared Core; board-family code still requires its physical receivers.
M5 T539 is closed. S1-S45 are accepted. S43 P1 `4ff59cd5c` establishes its
CPU-local descriptor receiver; the retained control-state source is assigned
only to S45. The former eleven-file, 7,000-plus-line arithmetic assignment is
split into S30-S35 under the existing automatic-S authorization. At S36 intake,
its oversized FLAGS/string/port row was divided into S36-S39. At S40 intake,
the 7,736-line descriptor/system row was divided into S40-S46 and the formerly planned
S41-S48 became S47-S54. At S48 intake, the former 6,724-line protected
transfer row was divided into S48-S55; S49 intake further divides the 1,181-line
control-transfer source into S49--S51; S68 intake divides the 20,417-line timing
corpus into S68-S73; the former oversized physical-relocation row is divided
into S76-S81 and final acceptance is S82. S78 intake found that the former
transfer/data row contains 19 files and 8,292 lines, so its unaccepted work is
split into S78-S86 before implementation; the remaining protected/system row
is then divided into S83-S100 and S100 is the final receiving audit. S48-S100
are accepted. S101 deleted the audit's discovered stale CPU-source-copy gap,
updated its static inventories and completed the final whole-ledger review.
Earlier accepted packets retain their historical prospective numbering; the
linked work plan owns the current sequence. The [CPU work packages](../../etc/architecture/t539-cpu-work-packages.md)
and [S101 evidence](../../etc/evidence/t539-s101-cpu-source-cleanup.md) record
the completed CPU extraction.

| Task | Status |
| --- | --- |
| T539 | Closed: every finite chip-ledger row is either extracted to its sole Shared owner or retained with its stated board-only reason; S101 removed the last historical CPU copy. |
| T540 S4 | Accepted: one Shared chip path, reconnected NXVM consumer, both-width complete units and four-profile 0540 artifact pairs. |
| T540 S5 | Accepted: source-inspected Core/board ownership and finite S6-S8 cut. |
| T540 S6 | Accepted: 92h has one board-owned route; both-width complete units and all eight 0540 products pass. |
| T540 S7 | Accepted: source-inspected Core/board state and API handoff before physical relocation. |
| T540 S8 | Accepted: first complete port registration cut for 92h, PIC, PIT and XT PPI; x64/x86 complete units and all eight 0540 products pass. |
| T540 S9 | Accepted: DMA uses one Core-owned typed route batch; x64/x86 complete units and all eight 0540 products pass. |
| T540 S10 | Accepted: KBC 60h/64h use one Core-owned typed route batch; x64/x86 complete units and all eight 0540 products pass. |
| T540 S11 | Accepted: FDC uses one Core-owned typed route batch; x64/x86 complete units and all eight 0540 products pass. |
| T540 S12 | Accepted: RTC/CMOS uses one Core-owned typed route batch; x64/x86 complete units and all eight 0540 products pass. |
| T540 S13 | Accepted: planar-parity and D4 Port-B routes are atomic; x64/x86 complete units and all eight 0540 products pass. |
| T540 S14 | Accepted: all HDC personality ports use a Core-owned typed batch; Compaq 3F7 wired-OR and XT rollback remain intact. |
| T540 S15 | Accepted: CGA and staged EGA/Compaq/VGA ports use Core-owned atomic batches; both-width units, focused EGA integration, gates and eight 0540 products pass. |
| T540 S16 | Accepted: VADP CGA/planar memory routes and EGA observer use one Core owner transaction; snapshots use copied Core inspection. |
| T540 S17 | Accepted: D4 replacement windows, parity and observer publish atomically; both-width complete units, Model-40 boot and eight 0540 products pass. |
| T540 S18 | Accepted: ROM images and both alias kinds use Core-owned routes and owner-scoped rollback; dual-width units, four-profile external boots, gates and eight 0540 products pass. |
| T540 S19 | Accepted: bounded Core A20 signal and absent-memory fallback route, without raw RAM in board adapters; dual-width units, gates, external boots and eight 0540 products pass. |
| T540 S20 | Accepted: one bounded Core DMA bus-cycle operation; no board-facing RAM or transaction pointers, with dual-width units and eight boot checkpoints passing. |
| T540 S21 | Accepted: source-inspected scheduler/PIC ownership and divided the oversized move into linear S22-S26 receivers. |
| T540 S22 | Accepted: copied board deadlines feed the one Core time observation, retaining immediate and L1-blocking disposition; dual-width units and eight boots pass. |
| T540 S23 | Accepted: Core time settlement calls the board-owned peripheral tail after readiness; dual-width units and eight boots pass. |
| T540 S24 | Accepted: board FDC/HDC, then Core FPU, then board RTC readiness effects retain their exact order; dual-width units and eight boots pass. |
| T540 S25 | Accepted: source-only Core/board arbitration intake and S26-S31 receiver map; no runtime change. |
| T540 S26 | Accepted: copied board D4 refresh request and success-only completion around Core HOLD/transaction; both-width units and eight boots pass. |
| T540 S27 | Accepted: board DMA clock/request/chip effects around Core wait/HOLD/grant and prefetch; both-width units and eight boots pass. |
| T540 S28 | Accepted: board PIT/PIC post-prefetch tail with earlier copied clock ticks; both-width units and eight boots pass. |
| T540 S29 | Accepted: copied PIC signals and Core CPU locality event; both-width units and eight boots pass. |
| T540 S30 | Accepted: source-only mixed plan/reset ownership audit and bounded S31-S38 receivers. |
| T540 S31 | Accepted: VM config/rules mirrors deleted; the frozen board plan owns composition and Core alone validates its transaction input before allocation. Dual-width units and eight boots pass. |
| T540 S32 | Accepted: one private neutral Core create phase retains preflight, allocation, CPU/FPU, time, transaction, bus, RAM and port failures; dual-width units and eight boots pass. |
| T540 S33 | Accepted: private board creation retains port/device order, while the sole validated create-from-plan and topology rollback live with board plan; dual-width units and eight boots pass. |
| T540 S34 | Accepted: one board-device cold-reset phase retains order; Core firmware-failure and processor-only reset remain intact; dual-width units and eight boots pass. |
| T540 S35 | Accepted: the sole destructor delegates board releases and reuses ROM route rollback for owner-only image release; dual-width units and eight boots pass. |
| T540 S36 | Accepted: entry, ROM and trace implementations use Core state only; unused board helper include removed; dual-width units and eight boots pass. |
| T540 S37 | Accepted: S31-S36 actual-diff/caller audit froze a finite neutral Core/board file ledger and identified owner-sized pre-move receivers. |
| T540 S38 | Accepted: F0000h alias derivation and sole firmware bind continuation moved to board owner with one rollback; dual-width units and eight boots pass. |
| T540 S39 | Accepted: high-reset RAM alias and parity resize veto are board-owned; Core retains one checked memory operation; dual-width units and eight boots pass. |
| T540 S40 | Accepted: D4 shutdown reset choice and native XT/8042 input dispatch are board-owned; dual-width units and eight boots pass. |
| T540 S41 | Accepted: six named board clocks moved with their advance/deadline owner; Core retains provider clock and one timeline; dual-width units and eight boots pass. |
| T540 S42 | Accepted: finite private Core/board ownership ledger assigns all mixed state groups and source stages to linear receivers. |
| T540 S43 | Accepted: remaining board constructor/callback separation, dual-width units/gates and eight boot checkpoints. |
| T540 S44-S52 | Accepted: board attachment and PIC/PIT/DMA/RTC/FDC/HDC/keyboard/VADP instance receivers; dual-width complete units, gates and eight boot checkpoints at each code receiver. |
| T540 S53 | Accepted: residual XT keyboard chip pointer moved to board; remaining electrical/callback row split into S54-S59. Dual-width 469/469 units, gates and eight boot checkpoints pass. |
| T540 S54 | Accepted: five planar-parity state fields moved into the sole board attachment; dual-width complete units, gates and eight boot checkpoints pass. |
| T540 S55 | Accepted: D4 platform/Port-B/NMI state moved into the sole board attachment; dual-width complete units, gates and eight boot checkpoints pass. |
| T540 S56 | Accepted: three D4 refresh electrical fields moved into the sole board attachment; Core HOLD stays bounded, gates and eight boots pass. |
| T540 S57 | Accepted: four XT speaker electrical fields moved into the sole board attachment; both-width units, gates and eight boots pass. |
| T540 S58 | Accepted: absent-memory fallback windows moved into the sole board attachment; both-width units, gates and eight boots pass. |
| T540 S59 | Accepted: source-audited 14 board providers and one firmware binding/rollback path; no redundant revoke added; dual-width units/gates pass. |
| T540 S60 | Accepted: measured the oversized neutral private/public header and allocated S61-S65 as bounded numeric receivers. |
| T540 S61 | Accepted: D4-specific mutable memory state moved to board; Core retains one atomic memory route; both-width units/gates and eight boots pass. |
| T540 S62 | Accepted: live Core retains only validated timing declarations, while board owns controller timing rules and DMA provenance; both-width units/gates and eight boots pass. |
| T540 S63 | Accepted: neutral private header no longer defines board plan/topology or includes concrete chips; both-width units/gates and eight boots pass. |
| T540 S64 | Accepted: measured 694-line/174-includer public interface and split its distinct owner boundaries into linear S65-S70 receivers; no source change. |
| T540 S65 | Accepted: one board composition supplies thirteen neutral constructor fields; both-width units/gates and eight single boots pass. |
| T540 S66 | Accepted: board values and dependent declarations have one owner; both-width units/gates and eight boot checkpoints pass. |
| T540 S67 | Accepted: five neutral validator definitions move verbatim to Core; both-width units/gates and eight boot checkpoints pass. |
| T540 S68 | Accepted: four private lifecycle bindings replace direct board calls; both-width full units/gates and eight single boots pass. |
| T540 S69 | Accepted: actual neutral sources link/run independently with only CPU/FPU/Lib dependencies and synthetic owned inputs. |
| T540 S70 | Accepted: all board NMI/reset signal sites use opaque Core operations; both-width units/gates and eight single boots pass. |
| T540 S71 | Accepted: RAM aliases and parity construction use Core-owned publication and rollback; dual-width units/gates and eight single boots pass. |
| T540 S72 | Accepted: sole Core route transaction and candidate destruction; every constructor port allocation failure, dual-width units/gates and eight single boots pass. |
| T540 S73 | Accepted: all refresh PIT initialization callers use the chip contract; dual-width units/gates and eight single boots pass, with no Core port borrowing. |
| T540 S74 | Accepted: Core owns firmware publication/rollback; board reset aliases use bounded neutral operations; dual-width units/gates and eight single boots pass. |
| T540 S75 | Accepted: six board lifecycle consumers use copied Core observation; both READY operations belong to the neutral scheduler; dual-width 470/470 units/gates and eight single boots pass. |
| T540 S76 | Accepted: complete copied timing table has one Core publication owner; deadline qualification is a callback value; dual-width 470/470 units/gates and eight single boots pass. |
| T540 S77 | Accepted: Core supplies copied time to all typed port reads; both Port-B routes no longer borrow the private clock; dual-width 470/470 units/gates and eight single boots pass. |
| T540 S78 | Accepted: complete attachment ownership intake and one copied-binding/opaque-board target; no runtime change. |
| T540 S79 | Accepted: all nineteen callback slots use one copied binding/publication/lifetime; dual-width 470/470 units/gates, eight independent Core executions and eight single boots pass. |
| T540 S80 | Accepted: complete callback class consumes board state; dual-width 470/470 units/gates, seven injected negatives, eight neutral executions and eight single boots pass. Public board API migration remains open. |
| T540 S81 | Accepted: frozen-plan dual-handle publication and sole failure cleanup; dual-width 470/470 units/gates, nine injected negatives, eight neutral executions and eight single boots pass. Public board operations and physical movement remain open. |
| T540 S82 | Accepted: P1 37b941c35 completes configuration/allocator handle publication; 203 existing calls migrated; dual-width 470/470 units/gates, twelve negatives, eight neutral executions and eight single boots pass. [Evidence](../../etc/evidence/t540-s82-config-board-publication.md); public board operations and physical movement remain open. |
| T540 S83 | Accepted: P1 231d0ec95 completes five input operations and all 36 calls on actual board handles; dual-width 470/470 units/gates, nineteen negatives, eight neutral executions and eight single boots pass. [Evidence](../../etc/evidence/t540-s83-board-input-handle.md); remaining board/physical receivers stay open. |
| T540 S84 | Accepted: P1 d23281d1b completes the display class and all callers on actual board handles; dual-width 470/470 units/gates, 21 negatives, eight neutral executions and eight one-shot boots pass. [Evidence](../../etc/evidence/t540-s84-board-display-handle.md); other board receivers and physical movement remain open. |
| T540 S85 | Accepted: P1 8b67d2cdb completes the controller configuration/callback class; dual-width 470/470 units/gates, 22 negatives, eight neutral executions and eight single boots pass. [Evidence](../../etc/evidence/t540-s85-board-controller-handles.md); electrical, constructor/fixture and physical component cuts remain open. |
| T540 S86 | Accepted: P1 5654912d7 completes electrical/RAM admission; dual-width 470/470 units/gates, 34 negatives, eight neutral executions and eight single boots pass. [Evidence](../../etc/evidence/t540-s86-board-electrical-boundary.md); constructor/private association and physical extraction remain open. |

## T540 S1 Acceptance

S1 inspected all four existing profile construction paths, the retained board
adapters and the present product regressions.  Its audited result is
[`t540-ibmpc-board-audit.md`](../../etc/architecture/t540-ibmpc-board-audit.md):
XT requires its own board family; 5170 and Default may share only individual,
proven AT electrical mechanisms; Model 40 remains a separate composition until
each proposed mechanism proves an identical owner, reset lifetime and rollback
boundary.  The current Model-40-from-5170 values shortcut is recorded as a
later source decision rather than hidden Shared inheritance.

No source, ABI, asset, INI or executable input changed.  `git diff --check`
and `Verify-DocumentationGovernance.ps1 -Product nxvm` passed.  Coordinator
review accepted `caaa6e337`; the required S1 governance closure follows.

## T540 S2 Acceptance

S2 reviewed the full retained adapter surface and recorded each mechanism's
state owner, current consumers, destination and required regression boundary in
[`t540-board-adapter-ledger.md`](../../etc/architecture/t540-board-adapter-ledger.md).
It established that generic x86 Core must move to neutral `x86/core` before
the future independent PC Apps can exist, while the flat `x86/ibmpc-*` layers stay limited to
actual IBM-PC wiring.  No source, ABI, asset, INI or executable input changed.

`git diff --check` and `Verify-DocumentationGovernance.ps1 -Product nxvm`
passed.  Coordinator review accepted `fd36e8412`; the required S2 governance
closure follows.

## T540 S3 Acceptance

S3 accepts one canonical future target layout:
`x86/{chips,core,ibmpc-common,ibmpc-at,ibmpc-xt,xasm32,debug}`. All live
NXVM design, proposal, roadmap and state authorities now use those names. The
current `x86/devices` source tree, its CMake targets and all historical T539
evidence remain explicitly current-source truth; S3 neither moved source nor
changed ABI, runtime behavior, assets, INI inputs or executables.

`git diff --check` and `Verify-DocumentationGovernance.ps1 -Product nxvm`
passed. Coordinator review accepts `ed5634677`; this closure records the
accepted naming boundary for later, separately admitted source moves.

## T540 S4 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S4, the next linear S after accepted S3. |
| Admission And Approval | The owner approved automatic admission of each bounded T540 S and selected `x86/chips` as the sole independent-chip destination; the prior S4 plan assigns this structural rename. Shared and NXVM are the declared targets. MyNES is a read-only receiving review because its link inputs exclude x86. |
| Objective | Relocate the complete independent-chip source/test family from `src/x86/devices` and `test/x86/devices` to `chips`, repair every live source/build/test/tool reference, and remove the old path. |
| Non-goals | Chip behavior or public-symbol changes, generic Core relocation, board-adapter extraction, profile/ROM/INI changes, new device framework, or MyNES edits. |
| Reference Baseline | `30d12eeef`, accepted S3 flat component layout and T539 closed chip corpus. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [board audit](../../etc/architecture/t540-ibmpc-board-audit.md), and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Shared `src/x86`, `test/x86`, their CMake/manifest/static gates; NXVM direct includes, build paths, product-only tests/tools/static gates and task evidence. Public symbols and runtime ABI stay intact. Each P commit changes exactly one declared target. |
| Applicable Rules | Architecture: single chip owner, Shared never imports App, and NXVM remains the sole board/host adapter. Coding: delete old include/build paths, no forwarding header or duplicate source. Execution: `git mv`, one target per P, complete units before S closure and dual-width receiving-product artifact review. Documentation: product-local packet/evidence and governance gate. Source policy is not triggered: no external source or binary is imported. |
| Verification | Search live source/build/test/tool trees for the retired `x86/devices` path; verify both manifests and x86 corpus/negative gates; build and run complete x64/x86 repository-only units, focused chip and four-profile regressions; build/review affected NXVM x64/x86 runnable products and hashes; run documentation and diff checks. |
| Expected Markers | One `src/x86/chips` implementation and `test/x86/chips` suite; no `src/x86/devices` or `test/x86/devices`, no forwarding include, and unchanged public symbols and chip behavior. |
| Asset Needs | Existing BYOB build roots only for the affected NXVM executable rebuild; no new ROM/media input. MyNES artifact hashes must remain unchanged. |
| Reporting Requirements | Record per-target P commits, moved-file and reference counts, added/removed/net tracked source/test lines, each width's unit and product build result, manifest/gate results, and artifact disposition. |
| Stop Conditions | Stop before changing any chip semantics, product firmware, MyNES file, or public contract; revise the packet if the path migration reveals a new dependency owner. |
| Exit Criteria | All chip files and references use the one `chips` path; Shared and NXVM compile and pass required tests on both widths; all affected runnable NXVM artifacts are reviewed and updated where changed; actual diff review finds no second chip owner or compatibility path. |
| Original Owner Request | Put fully decoupled Intel/x86 chips in `src/x86/chips` for reuse by later PC and arcade Apps. |
| Similar-Issue Sweep | Inspect every current source/test/CMake/static/tool include or literal of `x86/devices`, including negative probes and manifest inventory; distinguish historical documents from live paths. |

## T540 S4 Acceptance

The Shared chip tree and suite have one `chips` location; the old `devices`
path has no live source, build, test or tool reference. Shared P2
`c4d2fc29d` moved the corpus and repaired its manifests and gates. NXVM P3
`e8ea2ddc8` reconnected the product and advanced the artifact revision; NXVM
P4 `0483f1df1` deployed all eight verified 0540 EXEs and retired the
superseded 0539 pairs. No chip algorithm, public symbol, board wiring, INI or
MyNES input changed.

Shared standalone tests passed 119/119 on each width. The full repository-only
unit suite passed 467/467 on each width. Both manifests, the x86 corpus and
negative gates, both-width Release builds, PE checks, `git diff --check`, and
documentation governance passed. The actual diff review found no compatibility
header or second chip implementation. [S4 evidence](../../etc/evidence/t540-s4-chip-path-migration.md)
records file and line counts, source commits, artifact SHA-256 values and the
receiving-App disposition. T540's full external integration gate remains due
at task closure; S4 is accepted without claiming the board extraction complete.

## T540 S5 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S5, the next linear S after accepted S4. |
| Admission And Approval | The owner approved automatic admission of bounded T540 S tasks and explicitly selected a neutral `x86/core` ahead of IBM-PC board extraction. Shared and NXVM documentation are the declared review targets; this S makes no source or executable change. |
| Objective | Inspect the complete mixed `app-nxvm/devices` Core/board dependency cut, resolve the stale deferment of `x86/core`, and assign each file and coupled state group a finite S6-S8 receiver or board-retained reason. |
| Non-goals | Moving or rewriting code, adding public API or callback framework, changing chip behavior/timing, profile/ROM/INI changes, MyNES edits, or claiming a complete Core cutover. |
| Reference Baseline | S4 closure `45576e51e`, S1 board audit and S2 board-adapter ledger. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [board audit](../../etc/architecture/t540-ibmpc-board-audit.md), and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM design/roadmap/status and T540 supporting ledger/proposal only; no C source, tests, CMake, assets, ABI or executable inputs. |
| Applicable Rules | Shared execution, architecture, coding and documentation rules; NXVM system architecture and source layout. The source/research policy is not triggered: no external source or binary is used. |
| Verification | Review the actual includes, state fields and build targets for generic and board families; reconcile every changed authority, run documentation governance and `git diff --check`. No synthetic runtime pass is claimed by a design-only S. |
| Expected Markers | A finite owner map for S6-S8, an explicit one-clock/one-machine state rule, and no statement that `x86/core` must wait until after T540. |
| Asset Needs | None. Current 0540 binaries remain the product baseline. |
| Reporting Requirements | Name exact mixed files and state fields, dependency-direction risk, safe first source batch, retained board-only paths and the later verification gates. |
| Stop Conditions | Stop if the only plausible cut requires a second machine state owner, an App include in Shared, or product-visible timing behavior change; revise the implementation plan rather than inventing a temporary compatibility facade. |
| Exit Criteria | All Core/board candidate rows have one intended owner and ordered receiver; S6 can begin source extraction without guessing where the mixed scheduler, port 92h or plan transaction belongs. |
| Original Owner Request | Establish `x86/core` and flat `x86/ibmpc-*` components so later independent PC Apps reuse the same chip and board mechanisms. |
| Similar-Issue Sweep | Inspect `machine.h`, machine execution/plan/scheduler, memory/port/transaction/timeline/clock, CPU bus, firmware/debug/display, and all old App-only Core target sources for equivalent board leakage. |

## T540 S5 Acceptance

The [neutral Core cut](../../etc/architecture/t540-s5-neutral-core-cut.md)
inspected the mixed machine layout, scheduler, plan transaction, CPU bus and
fixed 92h memory port rather than moving their files wholesale. It assigns
pure clock/timeline/transaction mechanisms to S6, checked memory/port
mechanisms with board port separation to S7, and the mixed execution lifetime
and its related interfaces to S8. The IBM-PC device attachments and actual
chip states retain distinct owners. S8 must be split into later linear S
numbers at intake if its observed implementation batch cannot be reviewed as
one complete cut.

The principal Architecture, Coding and Roadmap documents now state the actual
closed T539 chip path and active T540 Core/board sequence. S2's historical
deferral is explicitly superseded without rewriting its source observations.
No source, ABI, tests, build input, asset or INI changed; all eight 0540
artifacts remain current. `git diff --check` and NXVM documentation governance
passed. Actual-diff review found no new framework, compatibility facade or
mirrored state. T540 remains open for implementation and external integration.

## T540 S6 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S6, next unused linear S after S5. |
| Admission And Approval | Automatic bounded-S authorization applies. S6 intake found that moving only timeline/transaction would expose mutable private Core layouts or add a transient wrapper; the [S5 cut amendment](../../etc/architecture/t540-s5-neutral-core-cut.md) records the narrower first board/Core seam. NXVM is the only changed target; Shared and MyNES are read-only receiving reviews. |
| Objective | Remove the fixed port 92h callbacks and registration from the generic memory mechanism; install the identical read/write route once from the existing IBM-PC board owner during the same machine construction step. |
| Non-goals | A20 electrical behavior change, changing which current profile has 92h, a new public getter/setter or adapter framework, moving private timeline/transaction state alone, ROM/INI/asset changes, or MyNES edits. |
| Reference Baseline | S5 acceptance `de382a478`, complete 0540 product artifact pairs. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S5 Core cut](../../etc/architecture/t540-s5-neutral-core-cut.md), and [board ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | `src/app-nxvm/devices/{memory.c,memory.h,machine_board.c,machine.c,machine.h}`, focused owner-local test and the active NXVM evidence; no Shared source, no public symbol addition. |
| Applicable Rules | Architecture: one route/one A20 state owner. Coding: remove old function and bit constant rather than forwarding. Execution: full both-width unit suites, focused port/board tests and all four dual-width 0540 product artifact review; documentation gate. Source policy is not triggered. |
| Verification | Compare port 92h read/write and memory A20 wrapping before/after, run relevant focused unit cases, full x64/x86 repository-only units, static/build gates, eight optimized Release product builds and hashes, `git diff --check` and documentation governance. |
| Expected Markers | `memory.c` has no fixed I/O address or port callback; `machine_board.c` owns one 92h route; machine construction calls it in the previous registration position; no old `core_machine_memory_register_ports` remains. |
| Asset Needs | Existing BYOB roots for the four product builds; no new external input. Preserve all owner INIs. |
| Reporting Requirements | Record actual source/test added, removed and net lines, owner/path diff, x64/x86 units and artifact hashes or unchanged-hash disposition. |
| Stop Conditions | Stop before altering route availability or timing, public Core A20 debug semantics, KBC A20 behavior or adding a second mutable A20 copy. |
| Exit Criteria | The one 92h board route is relocated without observable behavior change, all required gates pass, and no unrelated product or Shared corpus changed. |
| Original Owner Request | Build a neutral shared x86 Core and separate IBM-PC board wiring before splitting the four PC Apps. |
| Similar-Issue Sweep | Check other fixed I/O addresses in proposed neutral memory/port/timeline/transaction files and retain their board allocation for the next S intake. |

## T540 S6 Acceptance

The [S6 evidence](../../etc/evidence/t540-s6-a20-board-port.md) records the sole
92h route after the move, removal of the old memory-local registration path,
and a two-machine A20 regression. The five production files add 34/remove 33
lines; the owner-local test adds 21 lines. No Shared or MyNES source or binary
changed, and all adjacent NXVM INIs remain unchanged.

Focused x64/x86 tests and complete repository-only units pass 467/467 per
width. All four fixed profiles rebuilt as optimized x64/x86 0540 products and
passed PE architecture checks; their eight SHA-256 values are in the evidence.
`git diff --check` and NXVM documentation governance pass. S6 does not claim
that neutral Core extraction or the T540 external integration gate is done.
The S5 prospective S6 mechanism-only move is superseded by its recorded intake
correction; S7 must first derive the private-state cut without a temporary
wrapper, second machine owner or board logic in `x86/core`.

## T540 S7 Acceptance

The [Core/board handoff](../../etc/architecture/t540-s7-core-board-handoff.md)
records one opaque Core owner, one board attachment, the required bounded
port/memory/time/reset exchanges and prospective linear S8-S17 source batches.
The previous file-first relocation order is superseded; `machine_display.c`
remains board-facing. The inspected port and RAM searches found 23 and 19
files respectively; seven production files directly access named board
fields, with 142 matching references in the three mixed Core/board files.

The actual S7 changes are NXVM documentation only. `git diff --check` and
NXVM documentation governance pass. No C, test, CMake, asset or executable
input changed; the eight verified S6 0540 executables remain current. S7
does not claim runtime verification or T540 completion.

## T540 S8 Acceptance

NXVM implementation P1 `8f2fa4a6a` replaces the four assigned raw board-port
registrations with one typed atomic route batch. The Core port table remains
the sole owner; no Shared or MyNES code or artifact is in P1. PIC/PIT/XT
adapters and their existing KBC caller have one current Core runtime link
owner, with no circular static-library dependency. The full actual diff was
reviewed against the S8 packet and the [S8 evidence](../../etc/evidence/t540-s8-port-route-batch.md).

Focused route/rollback and board regressions pass. Complete x64 and x86 units
pass 467/467 each. Four fixed NXVM profile pairs rebuild as optimized,
stripped 0540 products with the eight verified hashes in the evidence; their
INIs remain unchanged. NXVM documentation governance, the T345 ownership
verifier and its self-test, CMake source-owner configuration and diff checks
pass. The later S7 handoff assigns DMA, KBC, FDC and remaining routes to
S9-S12 respectively. T540's neutral Core move, Shared board extraction and
external integration gate remain open; S8 makes no timing-grade claim.

## T540 S9 Acceptance

Coordinator review of NXVM implementation P1 `6691f16f4` inspected its actual
source, test, CMake, documentation and eight artifact changes against the S9
packet. DMA has one typed Core-owned route batch and no raw port-registration
path; Core alone owns route publication and rollback. The page-bank width
contract preserves consecutive eight-bit latches without changing native
controller ports. The [S9 evidence](../../etc/evidence/t540-s9-dma-port-routes.md)
records the scope, similar-issue receivers, source/test delta and artifact hashes.

Focused route/rollback tests pass 3/3 per width; complete repository-only x64
and x86 units pass 467/467 each. All four profile-specific Release pairs have
the expected PE width and no `.debug` section. The T345 ownership verifier and
self-test, NXVM documentation governance and diff checks pass. KBC, FDC and
remaining board routes stay assigned to S10-S12; no Shared or MyNES input was
changed. The neutral Core move, board extraction and external integration gate
remain open T540 work; S9 makes no new timing-grade claim.

## T540 S10 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S10, next linear S after accepted S9. |
| Admission And Approval | The owner approved automatic admission of bounded numeric T540 S work and the Core/IBM-PC board split; the accepted S7 handoff assigns the KBC port batch to S10. Target: NXVM only; Shared and MyNES are read-only receiving reviews. |
| Objective | Replace KBC's raw `t_port` callbacks and registration checkpoint with one Core-owned typed, atomic 60h/64h route batch while preserving keyboard/aux command, BAT, IRQ1/IRQ12 and reset behavior. |
| Non-goals | KBC chip protocol or timing changes, KBC A20/RAM and reset-signal decoupling (S14), FDC and remaining ports (S11-S12), physical Core/board relocation, new framework, profile/firmware/INI/media changes, or MyNES edits. |
| Reference Baseline | Accepted S9 governance commit `6c9d33f53`; full x64/x86 units 467/467 each and eight optimized 0540 NXVM artifacts. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../../etc/architecture/t540-s7-core-board-handoff.md), and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM `devices/{kbc,machine}` and necessary headers, owner-local synthetic tests/fixtures, relevant build/static gate and S10 evidence. KBC initializer receives the opaque Core owner for route installation; no raw port-table or route-entry pointer crosses the board boundary. |
| Applicable Rules | Architecture: Core solely owns port routes and atomic publication; keyboard/controller chips own protocol state, KBC board attachment owns signal wiring. Coding: delete the old callbacks/checkpoint without a compatibility path. Execution: complete implementation P, actual-diff review, both-width full repository-only units, affected four-profile 0540 artifacts and documentation governance. Source policy: no external material imported; existing approved BYOB firmware embedding is used for artifact rebuild. |
| Verification | Focused KBC controller/aux/serial-cadence, route-collision/allocation rollback, 5170/default/Model40 profile and port assembly tests; complete x64/x86 unit suites; applicable T345 source-owner/static gates; four-profile x64/x86 stripped Release pairs with PE/debug-section/hash checks; `git diff --check` and NXVM documentation governance. |
| Expected Markers | Only two KBC route descriptions (60h data and 64h status/command) are published by the Core batch; no KBC `t_port` callback, `core_machine_port_add_*` or registration checkpoint remains. Failure preserves existing routes and destroys uncommitted chips. |
| Asset Needs | Existing selected BYOB roots for product rebuild only; no new ROM/media input. Preserve adjacent NXVM.ini and all MyNES paths. |
| Reporting Requirements | Record actual changed paths and source/test added, removed and net lines; old/new owner and failure-boundary comparison; focused/full tests, static gates, eight artifact hashes, actual-diff review, and FDC/other-port successor allocation. |
| Stop Conditions | Stop before altering KBC command replies, status/IRQ timing, A20/reset semantics, guest port-width policy, or any Shared/MyNES target; revise the packet if typed value callbacks cannot preserve the KBC path. |
| Exit Criteria | KBC has one typed Core route batch with no raw port-table execution/registration dependency; collision and allocation failures leave no partial route or leaked chip; both-width tests and product artifacts pass, and no second KBC port path remains. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components before splitting four PC Apps, admitting bounded numeric S tasks automatically without patch-on-patch architecture. |
| Similar-Issue Sweep | Search every NXVM board adapter and test for raw `t_port` callbacks, `core_machine_port_add_*` and registration checkpoints. S10 consumes all KBC port hits; FDC is S11, VADP/HDC/RTC/board routes S12, and KBC memory/signal pointers S14. Record the residual owner and receiver in S10 evidence. |

## T540 S10 Acceptance

Coordinator review of NXVM implementation P1 `98a3fe04a` inspected the
actual source, test, documentation and eight artifact changes against the S10
packet. KBC publishes only the 60h/64h typed Core route batch; no raw KBC port
callback, direct registration or local checkpoint remains. Existing KBC
protocol, IRQ and signal functions remain in their original owners. The
[S10 evidence](../../etc/evidence/t540-s10-kbc-port-routes.md) records the
failure boundary, source/test delta, successor allocations and artifact hashes.

Focused KBC and port tests pass on both widths; full x64 and x86 repository-only
units pass 467/467 each. The T345 ownership verifier and negative self-test,
documentation governance and diff checks pass. All four profile-specific
Release pairs have the expected PE width and no `.debug` section. No Shared,
MyNES, INI or media input changed. FDC and remaining board ports stay assigned
to S11-S12; neutral Core, board extraction and T-level external integration
remain open. S10 makes no new timing-grade claim.

## T540 S11 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S11, the next linear S after accepted S10. |
| Admission And Approval | The owner approved automatic admission of bounded numeric T540 S work and the Core/IBM-PC board split; the accepted S7 handoff assigns FDC port routes to S11. Target: NXVM only; Shared and MyNES are read-only receiving reviews. |
| Objective | Replace FDC's raw `t_port` callbacks, retained port-table pointer and external registration checkpoint with one Core-owned typed, atomic route batch for status, data, DOR, DIR, diagnostic and control endpoints; preserve DRQ/IRQ, media-change observation and reset behavior. |
| Non-goals | 8272A chip commands or timing changes, FDC DMA/memory transaction decoupling, VADP/HDC/RTC or other board routes (S12), neutral Core/board physical relocation, new device framework, profile/firmware/INI/media changes, or MyNES edits. |
| Reference Baseline | Accepted S10 governance commit `424da67a1`; full x64/x86 units 467/467 each and eight optimized 0540 NXVM artifacts. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../../etc/architecture/t540-s7-core-board-handoff.md), and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM `devices/{fdc,machine_board}` and necessary headers, owner-local port and controller tests, S11 evidence and affected 0540 artifacts. FDC connection retains an opaque Core owner instead of a raw port table; Core alone registers/publishes routes. |
| Applicable Rules | Architecture: one Core port owner and atomic route publication, FDC board attachment owns wiring, chip owns command state. Coding: delete obsolete raw callbacks/checkpoint and avoid a forwarding path. Execution: complete implementation P, actual-diff review, both-width full repository-only units, four-profile 0540 Release pairs and documentation governance. Source policy: no new external material; existing approved BYOB firmware embedding is used for artifact rebuild. |
| Verification | Focused FDC port/topology/media-change/controller/Model40 tests, route collision/allocation rollback and port assembly; complete x64/x86 unit suites; applicable T345 source-owner/static gates; four-profile x64/x86 stripped Release pairs with PE/debug-section/hash checks; `git diff --check` and NXVM documentation governance. |
| Expected Markers | FDC route descriptions cover each configured register direction, including optional DIR/diagnostic/control and shared 3F7 direction separation; no FDC `t_port` callback, stored port-table pointer, `core_machine_port_add_*` or external checkpoint remains. A failed candidate preserves all previous routes and destroys the uncommitted FDC chip. |
| Asset Needs | Existing selected BYOB build roots for the product rebuild only; no new ROM/media input. Preserve adjacent NXVM.ini and all MyNES paths. |
| Reporting Requirements | Record actual changed paths and tracked source/test added, removed and net lines; old/new owner and failure-boundary comparison; focused/full tests, static gates, eight artifact hashes, actual-diff review and residual-route successors. |
| Stop Conditions | Stop before changing FDC command replies, status/IRQ/DRQ timing, guest port-width policy, DMA semantics, HDC shared-read semantics or any Shared/MyNES target; revise the packet if the typed value callback cannot preserve the FDC path. |
| Exit Criteria | FDC has one typed Core route batch and no raw port-table execution/registration dependency; collision/allocation failures leave no partial route or leaked chip; both-width tests and products pass; no second FDC port path remains. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components before splitting four PC Apps, admitting bounded numeric S tasks automatically without patch-on-patch architecture. |
| Similar-Issue Sweep | Search all NXVM board adapters and relevant tests for raw `t_port` callbacks, stored `connect.port`, `core_machine_port_add_*` and registration checkpoints. S11 consumes all FDC port hits; VADP/HDC/RTC/other board ports remain S12, and FDC DMA/memory signal exchange stays with the later bounded board/Core cut. Record every residual owner and receiver in S11 evidence. |

## T540 S11 Acceptance

[S11 evidence](../../etc/evidence/t540-s11-fdc-port-routes.md) records the
source-inspected change, failure/rollback contract, source and test line
counts, both-width verification and all eight artifact hashes. Implementation
P1 `acd6fd859` removes the raw FDC port callbacks and board checkpoint and
publishes one typed Core route batch. Actual-diff review found no parallel FDC
port path or changed chip command/timing rule. Both full repository-only unit
suites passed 467/467; the FDC and T345 source-boundary gates, documentation
governance, artifact PE/no-debug checks and staged diff check passed.

S11 changes only NXVM. The then-prospective S12 port batch contained
VADP/HDC/RTC and remaining board ports; S12 intake splits it into bounded
linear receivers. Later cuts own memory, signal, deadline and reset exchanges. T540
remains open for the neutral Core move, proven board extraction and full
external integration gate. No new timing grade is claimed.

## T540 S12 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S12, the next linear S after accepted S11. |
| Admission And Approval | The owner approved automatic admission of bounded numeric T540 subtasks. Source intake splits the former oversized S12 port batch by distinct owner and rollback boundary: S12 RTC/CMOS, later linear S for Port-B/D4, HDC and VADP. Target: NXVM only; Shared and MyNES are read-only receiving reviews. |
| Objective | Install the RTC/CMOS index-write and data-read/write endpoints through one typed Core-owned atomic route batch, deleting the board's separate precheck and external port-registration checkpoint while preserving register selection, NMI mask, chip defaults and IRQ behavior. |
| Non-goals | RTC register/timing semantics, Port-B/D4 parity or speaker wiring, HDC/VADP ports, memory/signal/deadline/reset cut, physical Core/board relocation, new device framework, profile/firmware/INI/media changes, Shared or MyNES edits. |
| Reference Baseline | Accepted S11 governance commit `126dc94ef`, complete 0540 artifact pairs and both-width units 467/467. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and [S11 evidence](../../etc/evidence/t540-s11-fdc-port-routes.md). |
| Files And ABI Surface | NXVM `devices/machine_board.c`, owner-local RTC/port-assembly tests, source-boundary verifiers, the S7 handoff's S12-intake numbering refinement, S12 evidence and affected 0540 products; no public ABI or Shared source. |
| Applicable Rules | Core alone owns port entries and atomic publication; RTC chip alone owns register state, board owns IRQ/NMI wiring and frozen CMOS defaults. Delete the obsolete availability/checkpoint path; no duplicate route or state. Full both-width units, all affected products, actual diff and documentation review are required. The source policy permits rebuilding the existing owner-approved embedded-ROM EXEs but no new ROM acquisition or raw-ROM commit. |
| Verification | Focused RTC/CMOS, NMI, route-collision and allocation-failure tests; both-width complete repository-only units; relevant source-owner/static gates; four-profile x64/x86 optimized 0540 artifacts and hashes; `git diff --check` and NXVM documentation governance. |
| Expected Markers | One two-route Core batch, index write only and data read/write; pre-existing routes survive every failed candidate, no partial RTC routes or leaked chip; no board-held port checkpoint. |
| Asset Needs | Existing selected BYOB build roots only; no new asset. Preserve adjacent NXVM.ini and all MyNES files. |
| Reporting Requirements | Record actual changed paths, tracked source/test added/removed/net lines, before/after rollback boundary, test/gate results, eight artifact hashes, actual diff and residual port-route receivers. |
| Stop Conditions | Stop before changing RTC chip behavior, input clock/timing, NMI semantics, memory parity, other controller ports or any Shared/MyNES target; revise the packet if the existing typed route cannot preserve behavior. |
| Exit Criteria | RTC/CMOS has exactly one Core-owned atomic route batch and no external registration checkpoint or second port path; failed route or chip creation leaves no partial state; all required tests, gates and products pass. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components, admitting bounded numeric S tasks rather than a patch-on-patch migration. |
| Similar-Issue Sweep | Search all NXVM board adapters for raw `t_port`, `core_machine_port_add_*` and external registration checkpoints. S12 consumes the RTC hits; Port-B/D4, HDC and VADP remain distinct later numeric S receivers, while the Core-private port table and CPU bus stay with the neutral-Core move. |

## T540 S12 Acceptance

[S12 evidence](../../etc/evidence/t540-s12-rtc-port-routes.md) records the
source-inspected RTC change, rollback and collision tests, tracked line counts,
both-width verification and eight product hashes. Implementation P1
`a128b4f67` deletes the external RTC port checkpoint and publishes its index
and data routes atomically. Actual-diff review found no duplicate RTC state,
changed chip timing or second route path. Both complete repository-only unit
suites passed 467/467; focused tests passed 11/11 per width; both RTC/Core
source gates, documentation governance, PE/no-debug checks and staged diff
check passed.

S12 changes only NXVM. Port-B/D4, HDC and VADP remain separately assigned
to prospective S13-S15; memory, signal, deadline, reset, neutral Core and
board extraction follow. The full external integration gate is still a T540
exit requirement. No new timing grade is claimed.

## T540 S13 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S13, next linear S after accepted S12. |
| Admission And Approval | Owner-approved automatic admission of bounded numeric T540 subtasks applies. S12 source intake assigns the two mutually exclusive AT Port-B personalities and their distinct parity rollback to S13. Target: NXVM only; Shared and MyNES are read-only receiving reviews. |
| Objective | Publish planar-parity and D4 Port-B routes through the Core-owned typed batch; make parity allocation, route publication and board-state publication failure-atomic; delete the raw board checkpoint and redundant availability path without changing speaker, refresh, failsafe or NMI semantics. |
| Non-goals | Chip PIT/parity behavior or timing changes, RTC/HDC/VADP ports, DMA/KBC memory and signal decoupling, neutral Core/board physical relocation, profile/firmware/INI/media changes, Shared or MyNES edits. |
| Reference Baseline | Accepted S12 governance commit `8c75ae95c`, dual-width full units 467/467 and eight optimized 0540 artifacts. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM `devices/{machine_board,memory}` implementation/internal headers, owner-local Port-B/parity/D4/port-assembly tests, affected static gates, S13 evidence and eight 0540 artifacts. No public Shared contract or product configuration change. |
| Applicable Rules | Core owns the one port table and atomic route publication; RAM parity storage has one owner and must be released if Port-B route publication fails. Perform all potentially failing preparation before board latches/PIT output callbacks are changed. Keep the 61h personality exclusivity and one guest-time path. Both full unit widths, artifacts, actual-diff review and documentation gate apply. The source policy permits rebuilding the existing owner-approved embedded-ROM EXEs only. |
| Verification | Focused planar parity, D4, Port-B exclusivity, NMI/refresh/failsafe and injected allocation/collision rollback tests; both-width complete repository-only units; relevant static boundary gates; eight optimized Release artifacts with hash/PE/no-debug check; `git diff --check` and NXVM documentation governance. |
| Expected Markers | Each personality has one typed route on 61h with no external port-table checkpoint. On parity allocation or route failure, no new route, parity allocation, board-configured flag, speaker/PIT mutation or leaked state remains. Successful Port-B behavior matches the original tests. |
| Asset Needs | Existing selected BYOB build roots only; preserve adjacent NXVM.ini and all MyNES files. |
| Reporting Requirements | Record source/test paths and added/removed/net lines, before/after failure order, relevant regressions, eight artifact hashes, actual-diff review and residual route receivers. |
| Stop Conditions | Stop before adding generic device/transaction framework, changing parity electrical behavior, altering guest timing, modifying HDC/VADP/RTC or Shared/MyNES, or publishing a second mutable parity owner. Revise the packet if a bounded rollback cannot be achieved. |
| Exit Criteria | Both 61h personalities use one Core route each; no raw board port checkpoint remains for them; every failure leaves prior Core and board state intact; both-width unit/product gates pass with no second path. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through correct, minimal, individually tracked chip/board cuts. |
| Similar-Issue Sweep | Search all Port-B/board adapters for direct port-table registration and parity-memory rollback; consume the planar-parity/D4 hits, assign HDC and VADP to later bounded S and Core-private storage to the neutral-Core cut. |

## T540 S13 Acceptance

Implementation P1 `24da4b5f9` replaces both raw Port-B providers and
checkpoints with one Core-owned typed route apiece. Planar parity prepares its
RAM store before route publication, releases it on failure through the same
owner-local cleanup used by final destruction, and changes board latches and
PIT callbacks only after successful publication. D4 follows the same
publish-before-state order without a parity store. Focused route-allocation
and retry cases cover both personalities and both allocation positions;
existing tests retain the electrical and signal behavior.

Complete repository-only unit suites pass 467/467 on x64 and 467/467 on x86.
The Port-B static gate, documentation governance, eight optimized 0540
profile-specific product builds, PE architecture and no-debug checks, and
staged diff check pass. The [S13 evidence](../../etc/evidence/t540-s13-board-port-b-routes.md)
records the changed-line accounting, all eight artifact hashes and receiving
HDC/VADP work. S13 changes NXVM only; Shared and MyNES remain untouched.
Neutral Core relocation, shared IBM-PC board extraction and the full external
integration gate remain open T540 work.

## T540 S14 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S14, next linear S after accepted S13. |
| Admission And Approval | The owner's automatic admission for bounded T540 S work applies to the accepted S7/S12 handoff, which assigns HDC personalities and Compaq 3F7 to S14. Target: NXVM only; Shared and MyNES are read-only receiving reviews. |
| Objective | Replace HDC's raw noncontiguous port registration/checkpoint with one Core-owned typed route batch for ATA, WD1003, Compaq WD and XT Xebec; retain the 3F7 FDC/HDC wired-OR read and make chip, route and XT DMA binding failure-atomic. |
| Non-goals | HDC protocol behavior or timing changes, media/geometry policy, VADP or unrelated ports, DMA runtime semantics, neutral Core/board physical relocation, Shared/MyNES source edits, firmware/INI/media changes. |
| Reference Baseline | Accepted S13 governance commit `edac1deea`; eight optimized 0540 profile executables and dual-width complete units 467/467. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM `devices/{machine_board,port_interface}` and owner-local HDC/port-assembly tests, a registered boundary verifier if mechanically justified, S14 evidence and the eight 0540 artifacts. The only allowed cross-module API addition is bounded Core route removal by owner, reusing Core's existing private removal mechanism; no raw `t_port` or checkpoint leaves Core. |
| Applicable Rules | One Core port table and one atomic batch per HDC personality; one HDC chip state owner; one original FDC read plus Compaq's explicit wired-OR contributor on 3F7. Prepare chip before publishing routes. A failed XT DMA bind revokes only HDC-owned routes and destroys only HDC state. Preserve all existing timing grades, guest-visible protocol and product configuration. Record actual code-size change and all retained paths. |
| Verification | Focused HDC personality, XT DMA, 3F7 wired-OR, port-collision and injected per-route allocation/rollback tests; new DMA-bind failure rollback case; complete x64/x86 repository-only units; relevant static gates; eight optimized Release artifacts with SHA-256, PE and no-debug proof; documentation governance and staged diff checks. |
| Expected Markers | No HDC adapter `t_port`, availability-probe, checkpoint or per-port raw registration remains. All HDC routes succeed or none do; Compaq retains the pre-existing FDC read and HDC wired-OR contribution; failed chip/route/DMA preparation leaves no HDC route, chip, topology, DMA request or configured flag; retry succeeds. |
| Asset Needs | Only existing BYOB selected-profile build roots for approved embedded-ROM executable rebuilds. Do not edit adjacent NXVM.ini or MyNES artifacts. |
| Reporting Requirements | Record exact source/test added, removed and net line counts, actual-diff review, four protocol dispositions and failure order, test results, eight artifact hashes and residual VADP/neutral-Core receivers. |
| Stop Conditions | Stop before adding a generic controller framework, changing any guest HDC protocol/timing, altering media assets or needing Shared/MyNES code, or exposing raw Core port/DMA layouts across the planned boundary. Revise the packet before any material scope expansion. |
| Exit Criteria | All four HDC personalities register their complete route set through the single typed Core batch; Compaq 3F7 preserves wired-OR; every injected route and DMA-bind failure leaves all non-HDC state intact; dual-width units/products and documentation gates pass with no second HDC registration path. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through complete, minimal, individually tracked board cuts, then extract them for independent PC Apps. |
| Similar-Issue Sweep | Search all NXVM board adapters, tests and static gates for raw HDC route registration, checkpoints, FDC/HDC 3F7 overlap and owner rollback. Consume every HDC personality hit in S14; assign remaining staged VADP ports to S15 and Core-private storage to the later neutral-Core move. |

## T540 S14 Acceptance

Pushed implementation P1 `962700027` was reviewed against the actual diff:
all four personalities use one Core-owned typed route batch; Compaq 3F7
retains FDC/HDC wired-OR; XT DMA failure revokes only HDC-owned routes; the
old raw HDC registration/checkpoint/availability path is deleted. Production
adds 65/removes 108 lines (net -43) without another state owner or timing
claim. Both full unit suites pass 467/467, focused HDC selections pass 11/11
per width, the registered static gate and documentation checks pass, and all
eight optimized 0540 products pass architecture/no-debug checks. The
[S14 evidence](../../etc/evidence/t540-s14-hdc-port-routes.md) records the failure
matrix and executable hashes. S14 changes NXVM only and is accepted. S15
receives staged VADP ports; neutral Core, IBM-PC board extraction and the full
external integration gate remain open T540 work.

## T540 S15 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S15, next linear S after accepted S14. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S work applies. The accepted S12 refinement assigns remaining staged VADP ports to S15. Target: NXVM only; Shared and MyNES remain read-only. |
| Objective | Publish initial CGA and staged EGA generic/Compaq/VGA port routes through the one Core-owned typed batch; preserve candidate-chip and memory-route preparation, exact original port directions, and failure-atomic candidate rollback. |
| Non-goals | Video register semantics, CGA/EGA/VGA geometry or timing, VADP memory-map extraction (S16), board signals, physical Core/board relocation, new generic device framework, Shared/MyNES edits, product profile, firmware, INI or media changes. |
| Reference Baseline | Accepted S14 governance commit `8b3e7a48c`, both-width complete units 467/467, HDC-focused 11/11 and eight optimized 0540 artifacts. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | NXVM `devices/{vadp,port_interface,machine}` and owner-local CGA/EGA/Compaq/VGA route/rollback tests, affected CMake boundary gates, S15 evidence and eight 0540 artifacts. The VADP adapter may receive only a bounded Core machine capability for port publication; no `t_port` or port-entry checkpoint remains in it. Test fixtures change to consume that production contract rather than keeping a raw alternate path. |
| Applicable Rules | One VADP video-chip owner and one Core port table. Prepare candidate chip and its existing memory routes before publishing all selected EGA/Compaq/VGA routes in one atomic batch. Initial CGA routes are published once during creation. A failed candidate leaves the original chip, original CGA routes, memory owners and display-configured state intact; successful publication then swaps the chip. Preserve all port read/write and Compaq overlap behavior. |
| Verification | Inject each initial and staged route-allocation failure and memory-registration failure; prove no partial route, memory owner or candidate state and successful retry. Run focused CGA/EGA/Compaq/VGA and display-authority tests, complete x64/x86 repository-only units, affected static gates, eight optimized Release products with hash/PE/no-debug proof, documentation governance and staged diff checks. |
| Expected Markers | No `t_port`, raw `core_machine_port_add_*` or external registration checkpoint in the VADP adapter. Each successful configuration has one selected port route set; no second registration path, duplicate chip state, changed graphics result or new timing claim. |
| Asset Needs | Existing selected BYOB build roots only for approved embedded-ROM executables; preserve adjacent INIs and all MyNES files. |
| Reporting Requirements | Record all affected test receivers, actual production/test added/removed/net lines, candidate failure order, port-direction and Compaq overlap proof, eight artifact hashes and the remaining S16 memory receiver. |
| Stop Conditions | Split an oversized unstarted remainder into the next numeric S before implementation; stop before adding a second VADP port path, changing video semantics/timing, moving memory maps prematurely or modifying Shared/MyNES. Revise the packet for any material scope expansion. |
| Exit Criteria | Initial and staged VADP ports use the one Core typed batch; failed candidate and allocation paths preserve old chip/routes and leave no new memory or port owner; old raw VADP port registration is deleted; dual-width tests/products and documentation gates pass. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through correct, minimal and individually tracked board cuts before extracting them for independent PC Apps. |
| Similar-Issue Sweep | Search all VADP board adapters and tests for raw `t_port`, port add/checkpoints, candidate memory and port rollback. S15 consumes port hits; retain bounded VADP/ROM/D4 memory attachments for S16 and Core-private table for the neutral-Core move. |

## T540 S15 Acceptance

P1 `b656b0456` is pushed. The [S15 evidence](../../etc/evidence/t540-s15-vadp-port-routes.md)
records the actual diff, all CGA/EGA/VGA/Compaq allocation positions, a
late Compaq port collision, memory-owner rollback, port-direction preservation,
both-width 467/467 complete units, focused 14/14 and external EGA 1/1 per
width, all specialized gates, and eight optimized 0540 artifact identities.
Actual-diff review confirms one VADP chip state and one Core route table, no
raw VADP port registration or new timing claim. Only NXVM-owned files changed;
Shared, MyNES and adjacent INIs remain untouched. S16 owns the remaining
VADP memory attachment; D4 and ROM have separate S17/S18 receivers. Neutral
Core and board relocation remain open.

## T540 S16 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S16, next linear S after accepted S15. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 subtasks applies. [S16 source intake](../../etc/architecture/t540-s7-core-board-handoff.md) separates VADP, D4 and ROM by their distinct memory owner/rollback boundaries. Target: NXVM only; Shared and MyNES remain read-only. |
| Objective | Give VADP a bounded Core memory route and copied inspection boundary for candidate CGA/planar mappings, observer and display snapshot, without `t_ram` in the VADP adapter. |
| Non-goals | D4 parity/windows (S17), immutable ROM/reset aliases (S18), KBC/DMA memory cycles (S19), video register semantics, frame geometry/timing, new generic device framework, profile/INI/firmware/media changes, physical Core or board relocation. |
| Reference Baseline | Accepted S15 P1 `b656b0456` and P2 `700d7f0c9`; both-width full units 467/467, focused EGA integration 1/1 per width, complete specialized gates and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S16 source refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and [S15 evidence](../../etc/evidence/t540-s15-vadp-port-routes.md). |
| Files And ABI Surface | NXVM `devices/{vadp,memory_interface,memory,machine_display,machine}` and owner-local CGA/EGA/Compaq/VGA memory/snapshot tests, affected CMake gates, S16 evidence and eight 0540 artifacts. Expose only the typed memory route/owner and copied inspection operations VADP actually needs; Core retains the sole mapping table. |
| Applicable Rules | Prepare the candidate chip first; publish its selected CGA provider and planar provider/write observer through one Core-owned failure-atomic operation. An observer-only EGA configuration is equally supported. On failure, no candidate mapping or observer remains, old chip/routes survive, and retry succeeds. Snapshot reads are observational and copied, never an operational read or raw RAM borrow. |
| Verification | Inject each provider/observer publication failure and existing owner collision; verify rollback, retry, CGA/planar aperture semantics and side-effect-free text/graphics capture. Run affected focused tests, full repository-only x64/x86 units, registered static/governance gates, eight optimized Release products with PE/hash/no-debug proof and staged diff review. |
| Expected Markers | No `t_ram`, `executor_memory`, raw memory registration or unregister in `vadp.c`/`vadp.h`; one Core mapping table, one VADP chip, one candidate publication/rollback path, unchanged video semantics and timing grade. |
| Asset Needs | Existing selected BYOB build roots only; preserve adjacent INIs and every MyNES file. |
| Reporting Requirements | Record each affected test receiver, actual source/test added/removed/net lines, candidate-memory and snapshot order, rollback proof, eight artifact hashes and remaining S17/S18 receivers. |
| Stop Conditions | Split an oversized unstarted remainder into the next numeric S before implementation; stop before adding a second video/memory owner, moving D4/ROM prematurely, introducing a generic device framework or changing Shared/MyNES. Revise the packet for any material scope expansion. |
| Exit Criteria | VADP uses only bounded Core memory operations for configuration and copied inspection; all candidate failure paths are atomic; the old raw VADP memory path is deleted; dual-width tests/products and static/documentation gates pass. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through correct, minimal and individually tracked cuts before extracting them for independent PC Apps. |
| Similar-Issue Sweep | Search VADP and tests for all raw `t_ram` mapping, write observer, unregister and snapshot inspection calls. S16 consumes VADP hits; retain D4 windows/parity for S17 and ROM/reset aliases for S18. |

## T540 S16 Acceptance

P1 `1fc8b2054` is pushed. The [S16 evidence](../../etc/evidence/t540-s16-vadp-memory-routes.md)
records the actual source and test diff, candidate provider/observer rollback,
duplicate-owner rejection, port-batch failure recovery, copied observational
snapshot path, both-width 467/467 complete units, focused EGA planar DOS
integration 1/1 per width, specialized and documentation gates, and eight
optimized 0540 executable identities. Coordinator actual-diff review confirms
one Core mapping table, one VADP chip owner, and no raw memory access in the
VADP adapter. Only NXVM-owned files changed; Shared, MyNES and adjacent INIs
remain untouched. S17 owns D4 memory/parity, S18 owns ROM/reset aliases;
neutral Core and IBM-PC board relocation remain open.

## T540 S17 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S17, next linear S after accepted S16. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 subtasks applies. Target NXVM only; Shared and MyNES remain read-only. The S16 handoff assigns Model-40 D4 memory/parity to this separate owner boundary. |
| Objective | Publish Model-40 D4's two replacement windows, parity allocation and write observer atomically through Core's sole memory owner, preserving D4 reset and fault behavior. |
| Non-goals | ROM/reset aliases (S18), KBC/DMA memory cycles (S19), D4 protocol/timing reinterpretation, generic device framework, profile/INI/firmware/media changes, Core or board physical relocation. |
| Reference Baseline | Accepted S16 P1 `1fc8b2054` and P2 `6b6f13b65`; both-width complete units 467/467, focused EGA integration 1/1 per width, specialized gates, eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md), and [S16 evidence](../../etc/evidence/t540-s16-vadp-memory-routes.md). |
| Files And ABI Surface | NXVM `devices/{d4_memory,memory_interface,memory,machine_board,vadp}` and affected D4/Model-40 tests plus owner-local transaction test, static gates, S17 evidence and eight 0540 products. Extend only Core's existing typed memory batch to express replacement routes and optional parity; delete unused separate D4 memory wrappers. |
| Applicable Rules | Core remains the sole mapping/parity table. Prepare D4 configuration before publication; on failure retain unrelated owners and no D4 provider, observer, parity or configured state. On success retain one D4 owner and unchanged reset/IOCHK semantics. Do not move D4 into shared IBM-PC code. |
| Verification | Inject first/second replacement-route, parity-conflict and observer-capacity failures; verify rollback and retry, Model-40 windows, parity fault and reset. Run focused D4/Model-40 tests, complete repository-only x64/x86 units, specialized/documentation gates, affected external checkpoint, eight Release products with PE/hash/no-debug proof, and staged actual-diff review. |
| Expected Markers | No separate stepwise D4 registration path or partial `configured`; one Core memory table, one owner identity for D4 routes/parity/observer, unchanged D4 reset/IOCHK behavior, no new timing grade. |
| Asset Needs | Existing selected BYOB build roots only; preserve adjacent INIs, all Shared and MyNES files. |
| Reporting Requirements | Record actual source/test added/removed/net lines, each failure injection and surviving owner, D4 reset/parity proof, eight artifact hashes and remaining S18 ROM receiver. |
| Stop Conditions | Split an oversized unstarted remainder into the next numeric S before implementation; stop before a second memory registry, D4-in-common move, unapproved Shared/MyNES change, or D4 hardware semantics change. |
| Exit Criteria | D4 configuration is failure-atomic with Core-bounded replacement routes/parity/observer; old piecemeal path is deleted; affected regressions and complete dual-width gates/products pass. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through correct, minimal, individually tracked cuts before extracting them for independent PC Apps. |
| Similar-Issue Sweep | Search D4 and other board adapters for stepwise memory publication with parity/observer; consume D4 here, retain ROM/reset aliases for S18 and KBC/DMA cycles for S19. |

## T540 S17 Acceptance

NXVM P1 `f58a536ce` adds replacement-route and optional parity support to
Core's existing memory batch and removes D4's three stepwise registration
wrappers. D4 state is committed only after the full batch succeeds. The
owner-local transaction test covers first and second route failure, parity
conflict, observer capacity, retry, reset and teardown. The [S17 evidence](../../etc/evidence/t540-s17-d4-memory-routes.md)
records the actual source/test diff and all eight Release artifact hashes.

The coordinator's committed-diff review found only NXVM paths: no Shared,
MyNES, INI, firmware or external-media change, no second mapping table, and
no unowned D4 callback. Final-source x64/x86 complete units each passed
468/468; Model-40 external boot passed 1/1 per width; the specialized gate
passed 72/72. Documentation governance and staged diff checks passed. S17 is
accepted while ROM/reset aliases (S18), bounded KBC/DMA cycles (S19), neutral
Core relocation and IBM-PC board extraction remain open T540 work.

## T540 S18 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S18, next linear S after accepted S17. |
| Admission And Approval | The owner's automatic admission applies to bounded numeric T540 S tasks. Target NXVM only; Shared and MyNES remain read-only. The S16 handoff assigns immutable ROM/reset-alias memory ownership to S18. |
| Objective | Make immutable ROM image, ordinary alias and pre-A20 reset-alias registration and rollback one Core-owned, owner-scoped memory path, retaining one image owner, mapping priority and firmware construction semantics. |
| Non-goals | KBC/DMA memory cycles (S19), board deadline/PIC exchange, Core/board physical relocation, firmware-byte selection, ROM acquisition, profile/INI/media changes, hardware or timing reinterpretation, generic device framework. |
| Reference Baseline | Accepted S17 P1 `f58a536ce` and P2 `48d3c083a`; dual-width complete units 468/468, Model-40 boot 1/1 per width, 72/72 specialized gates and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md), and [S17 evidence](../../etc/evidence/t540-s17-d4-memory-routes.md). |
| Files And ABI Surface | NXVM `devices/{rom_mapping_interface,memory_interface,memory,machine_firmware,machine}` and affected ROM/firmware/reset tests, owner-local failure tests, static gates, evidence and eight 0540 products. Extend the existing typed Core memory route mechanism only as needed for ordinary, overlay and pre-A20 reset priority; remove direct ROM provider-table mutation. |
| Applicable Rules | Core owns one mapping/provider table and copied immutable image. An alias borrows bytes from its source and never owns a second copy. Firmware configure plus derived reset alias either publishes completely or removes only its new owners; existing unrelated routes survive. The high reset alias must retain pre-A20 priority. No App asset path or ROM bytes enter Shared. |
| Verification | Inspect every ROM/alias registration and rollback caller; inject provider-capacity, alias-source and reset-alias failure after earlier successful registration; prove unchanged unrelated routes, image lifetime, retry and reset priority for 286/386. Run focused ROM/firmware tests, complete x64/x86 repository-only units, specialized/documentation gates, affected external boot checkpoints and eight Release products with PE/hash/no-debug proof. |
| Expected Markers | One owner-scoped Core route operation for each ROM image/alias and one firmware transaction rollback; no direct ROM-specific provider-array loop or second memory table; no duplicate image copy for aliases; unchanged reset-only decode semantics. |
| Asset Needs | Existing selected BYOB build roots only. Do not change external originals or adjacent NXVM.ini files. |
| Reporting Requirements | Record each original ROM/alias caller and disposition, failure positions and surviving owner, actual source/test added/removed/net lines, affected boot checkpoints, eight artifact hashes and the S19 KBC/DMA receiver. |
| Stop Conditions | Stop before changing reset-vector priority, image lifetime, firmware callback authority, Shared/MyNES source, protected ROM inputs or profile semantics. If one batch proves too large, split its unstarted remainder into the next numeric S before implementation. |
| Exit Criteria | ROM image and both alias kinds publish and roll back through the one Core memory owner without leaking or displacing unrelated routes; old direct ROM provider manipulation is gone; affected regressions and complete dual-width gates/products pass. |
| Original Owner Request | Build reusable neutral x86 Core and IBM-PC board components through correct, minimal, individually tracked cuts before extraction for independent PC Apps. |
| Similar-Issue Sweep | Search all production/test/build registration and rollback of immutable ROM, alias, reset alias, raw provider entries and firmware construction. Consume ROM variants here; retain the non-ROM absent-memory fallback and KBC/DMA cycles for the next bounded Core-memory receiver. |

## T540 S18 Acceptance

P1 `b987233dd` unifies immutable ROM image and ordinary/reset-alias route
publication through the Core memory batch and replaces direct provider-table
rollback with owner-scoped removal. The actual-diff review confirms one copied
image owner, alias byte borrowing, unchanged priority and firmware construction
authority, plus an explicit failure/rollback test and static gate. It found no
Shared, MyNES, INI, media or external-asset change.

The [S18 evidence](../../etc/evidence/t540-s18-rom-memory-routes.md) records x64
and x86 complete units **469/469** each, specialized gates **72/72**, each of
the four external boot checkpoints once per width, documentation governance,
eight optimized 0540 products, PE/no-debug checks and their SHA-256 identities.
S18 is accepted. S19 consumes bounded KBC/92h A20 and non-ROM absent-memory
routes; DMA bus transactions move to S20. The neutral Core and board extraction
remain T540 work.

## T540 S19 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S19, the next linear S after accepted S18. |
| Admission And Approval | The owner authorized automatic admission of bounded numeric T540 subtasks. Target NXVM only; Shared and MyNES are read-only. S19 source intake splits the former KBC/DMA memory row by distinct signal and bus-transaction boundaries before implementation. |
| Objective | Make KBC output and port 92h read/write use one bounded Core-owned A20 signal rather than a raw RAM pointer, and publish non-ROM absent-memory fallbacks through Core's typed memory route without losing open-bus priority or rollback. |
| Non-goals | DMA bus cycles/HOLD (S20), board deadline/PIC exchange (S21), physical Core/board relocation, KBC command changes, new memory/A20 electrical behavior, profile/INI/asset/media changes or a generic device framework. |
| Reference Baseline | Accepted S18 P1 `b987233dd` and P2 `d961f287a`; complete units 469/469 per width, 72/72 specialized gates, all four external boot checkpoints once per width, eight verified 0540 Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S19 refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md), and [S18 evidence](../../etc/evidence/t540-s18-rom-memory-routes.md). |
| Files And ABI Surface | NXVM `devices/{kbc,kbc.h,machine,machine_board,memory_interface,memory_interface.h}` plus affected KBC/A20/absent-memory tests, Core route gate, S19 evidence and eight 0540 products. KBC keeps a bounded A20 callback/context instead of `t_ram`; 92h uses the same Core signal. Add only the existing fallback provider's mode to the typed route and retire its direct registration call. |
| Applicable Rules | One Core A20 truth and one provider table; no board adapter reads/writes `t_ram` or owns another latch for A20. KBC remains capable of signalling A20 while the machine runs; paused-debug A20 mutation retains its restriction. Absent windows remain lower priority than live apertures, with candidate state removed on publication failure. No Shared or MyNES edit and no App-specific profile branch in Core. |
| Verification | Inspect every A20 writer/reader and absent-memory registration; focused KBC command/output, 92h and absent-window priority/rollback tests; complete repository-only x64/x86 units; specialized/documentation gates; affected external boot checkpoint once per profile/width and eight optimized 0540 EXEs with PE/hash/no-debug proof. |
| Expected Markers | No KBC or 92h raw RAM link; both A20 inputs observe the same Core flag; one Core typed fallback route and owner-scoped failure cleanup; no new callback path for CPU/debug A20 or duplicate open-bus provider. |
| Asset Needs | Existing selected BYOB build roots only. Do not alter external originals or adjacent NXVM.ini files. |
| Reporting Requirements | Record source/test added, removed and net lines; all original A20/fallback callers and disposition; KBC running-write, port 92h readback, open-bus overlap/rollback proof, boot checkpoints and eight product hashes; explicitly transfer DMA to S20. |
| Stop Conditions | Stop before changing KBC semantics, A20 wrapping policy, DMA transfer order, reset/firmware roles, Shared/MyNES source or protected assets. A newly required behavioral change needs owner review before implementation. |
| Exit Criteria | KBC and 92h have no raw RAM pointer and retain their behavior via one Core signal; absent fallback uses Core typed registration and fails atomically; focused/full dual-width verification, affected external boots and artifact checks pass. |
| Original Owner Request | Build reusable neutral x86 Core and proven IBM-PC board components for later independent PC Apps without duplicate state or a patch stack. |
| Similar-Issue Sweep | Search all tracked NXVM production/test/build paths for direct `flagA20`, `t_ram` board links and raw absent fallback registration. Consume KBC, 92h and fallback here; keep Core-internal memory ownership, CPU/debug operations and DMA transaction arguments with their distinct S20 receiver. |

## T540 S19 Acceptance

P1 `ad38fee6a` replaces the KBC RAM borrow with a bounded A20 signal,
routes port 92h through the same Core state, and registers absent-memory
fallback via Core's typed route. The actual diff has one signal owner and one
provider table: no board/KBC `flagA20` access, no second latch, no changed
KBC command behavior or fallback priority. DMA's distinct transaction
arguments remain assigned to S20. The [S19 evidence](../../etc/evidence/t540-s19-a20-fallback-routes.md)
records caller dispositions, allocation-failure/overlap tests, added and
removed source/test lines, eight executable hashes and a failed x86 relink
caused solely by an active test handle; the retry passed after that test
finished.

Complete repository-only unit suites passed x64 and x86 **469/469** each.
Specialized gates passed **74/74** steps and documentation governance passed
after the evidence was added. All four external boot checkpoints passed once
per width, and all eight optimized 0540 products have the expected PE format
and no `.debug` sections. No Shared, MyNES, INI, external media or firmware
byte changed. S19 is accepted; T540 remains open.

## T540 S20 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S20, the next linear S after accepted S19. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. The S19 source-intake refinement assigns DMA's raw RAM/transaction cut to S20. |
| Objective | Replace DMA board callbacks' `t_ram` and transaction-state access with one bounded Core-owned DMA bus-cycle operation. Preserve query, begin, device-before-memory, memory, device-after-memory, commit/cancel, and Core-owned HOLD arbitration. |
| Non-goals | Device command/register semantics, DMA chip timing, new board wait ratios, PIC/deadline exchange (S21), reset/plan split (S22), physical Core/board move (S23), profile/INI/firmware/media changes, generic bus framework or second transaction owner. |
| Reference Baseline | S19 P1 `ad38fee6a` and P2 `33d8382f9`; complete units 469/469 per width, 74/74 specialized-gate steps, all four external boot checkpoints once per width, eight verified 0540 Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S19 refinement](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md), and [S19 evidence](../../etc/evidence/t540-s19-a20-fallback-routes.md). |
| Files And ABI Surface | NXVM `devices/{dma_bus,dma_bus.h,memory_interface,memory_interface.h,machine_scheduler}` and directly affected DMA/transaction tests and fixtures, static boundary gate, S20 evidence and eight 0540 products. DMA submits physical address, width, direction, channel and copied value/device effect to a Core operation; it receives no RAM or transaction layout. Scheduler retains its one Core HOLD decision. |
| Applicable Rules | One Core RAM, transaction and guest-time owner; no board adapter borrows their mutable layout. Device callbacks remain board-owned and fire in the original order. The typed operation has bounded inputs, explicit failure and no parallel compatibility call. Structural extraction makes no L3 claim. One NXVM target per P; preserve other product surfaces. |
| Verification | Inspect all DMA cycle and HOLD callers; focused byte/word, primary-only/paired, verify, mem-to-mem, request/terminal, query-failure/transaction-cancel and bus trace tests. Run complete repository-only x64/x86 units, specialized/documentation gates, four external boot checkpoints once per width, and eight optimized 0540 EXEs with hash/PE/no-debug proof. |
| Expected Markers | No `t_ram` or `core_machine_transaction_state` in DMA board callback ABI or implementation; Core alone queries, begins, reads/writes, commits/cancels. Existing DMA HOLD, channel and board topology results remain unchanged. No chip source, INI or timing data is modified. |
| Asset Needs | Existing owner-provided profile firmware/media only for the external boot checkpoints and eight Release products. Repository-only tests use no external files. No new asset or import. |
| Reporting Requirements | Record each original raw DMA caller and disposition, exact before/after cycle order, failure rollback and trace proof, production/test added/removed/net lines, external boot checkpoints, eight artifact hashes and explicit S21 transfer. |
| Stop Conditions | Stop before a change requiring hardware/timing reinterpretation, a second transaction path, direct Shared/MyNES edit, protected asset change, or an unpreserved device-before/after-memory ordering. Report any discovered contract expansion before proceeding. |
| Exit Criteria | All raw RAM/transaction board links are retired from DMA; one Core operation owns each cycle and its failure boundary; tests, gates, external checkpoints and products pass; coordinator actual-diff review accepts the implementation P and governance P records S20 closure. |
| Original Owner Request | Build reusable `x86/core` and `x86/ibmpc-*` before splitting four PC Apps, with flat sole ownership, no patch layering, linear numeric S tasks, automatic S admission and preserved product behavior. |
| Similar-Issue Sweep | Search all tracked NXVM production/test/build paths for DMA use of `t_ram`, `executor_memory`, transaction-state pointers, raw memory query/read/write and HOLD access. Consume DMA cycle callers here; retain Core-private bus/refresh operations and transfer deadline/PIC exchange to S21 with distinct owner reason. Add a static boundary gate for the forbidden DMA shape. |

## T540 S20 Acceptance

P1 `9e4c22e2f` moves the DMA physical route query, transaction begin,
memory operation and commit/cancel into one Core-owned cycle. The actual-diff
review confirms the DMA adapter has no raw RAM or transaction-state ABI, no
parallel legacy cycle and no change to chip timing, HOLD arbitration or
primary-only/paired board topology. Device effects remain in their original
before/after positions. The [S20 evidence](../../etc/evidence/t540-s20-dma-core-cycle.md)
records caller disposition, failure cancellation, source/test line counts,
all eight boot results and the eight executable SHA-256 identities.

Final-source complete repository-only unit suites passed x64 and x86
**469/469** each. Specialized gates completed **76/76** steps, including the
new DMA boundary gate. Documentation governance and diff hygiene passed.
Each of the four external boot checkpoints passed once per width. All eight
optimized 0540 Release products have the expected PE format and no `.debug`
sections. A broad x86 unit-tree build incidentally relinked the MyNES x86
product; it was restored from HEAD and the accepted diff contains no MyNES,
Shared, INI, media or firmware-byte change. S20 is accepted; T540 remains
open. S21 receives board deadline/advance and PIC acknowledge separation.

## T540 S21 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S21, the next linear S after accepted S20. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES remain read-only. Source intake must split a too-large or unlike scheduler/PIC receiver into further linear S numbers before implementation. |
| Objective | Source-inspect every deadline, advance and PIC caller; freeze the exact Core/board owner map and regression anchors, then divide this oversized combined move into bounded linear numeric receivers before code changes. |
| Non-goals | Production source, ABI, test or binary changes; new timing formulas or L3 claims, device command/register changes, physical Core relocation, Shared board move or a second scheduler. |
| Reference Baseline | Accepted S20 P1 `9e4c22e2f`; complete units 469/469 per width, 76/76 specialized-gate steps, all four external boot checkpoints once per width and eight verified 0540 Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and [S20 evidence](../../etc/evidence/t540-s20-dma-core-cycle.md). |
| Files And ABI Surface | Read NXVM `devices/{machine_scheduler,cpu_bus,pic_bus,machine,machine_interface}` and their direct deadline, PIC-phase, IRQ, HOLD and trace tests. Change only the NXVM handoff document, source-intake evidence and state record; do not touch code. |
| Applicable Rules | Core alone advances the guest timeline and owns CPU bus transactions. The IBM-PC board owns only its installed device deadlines, signal wiring and PIC service effect. The adapter sends copied observations/results, not mutable Core or chip internals; remove old callers in the same change. |
| Verification | Source-inspect every deadline publisher, advance caller, CPU time publication and PIC acknowledge path; map focused regression anchors. As a documentation-only S, run documentation governance and `git diff --check`; retain the accepted S20 complete dual-width baseline without rebuilding products. |
| Expected Markers | One named Core timeline/CPU bus owner, distinct board deadline/advance/PIC receivers, explicit unchanged phase order and no unassigned cross-owner path. |
| Asset Needs | Existing owner-provided profile firmware/media only for external checkpoints and products; no asset import or modification. Repository-only tests use no external files. |
| Reporting Requirements | Record exact original deadline/advance/PIC callers, owner disposition, critical phase order, test anchors and the later linear receiver map. Note zero production/test/binary diff and transfer plan/reset to S25. |
| Stop Conditions | Stop before changing physical timebase, chip or PIC instruction semantics, an unresolved event-order contract, Shared/MyNES code or protected assets. Refine S21 into bounded linear receivers rather than layering a second scheduler. |
| Exit Criteria | Source owner audit and bounded linear task plan are complete; no code or binary changed; documentation governance and diff hygiene pass; coordinator actual-diff review accepts the audit P and governance P records closure. |
| Original Owner Request | Build reusable neutral `x86/core` and proven `x86/ibmpc-*` board components before splitting the PC Apps, preserving behavior with flat ownership and linear numeric S tasks. |
| Similar-Issue Sweep | Search the full scheduler, CPU bus, PIC bus and their direct tests for device state inside CPU-time decisions or CPU transaction state inside board callbacks; classify each as Core-private, board-owned or later plan/reset work, with no unowned remainder. |

## T540 S21 Acceptance

P1 `fa9aa0c27` source-inspects the 436-line scheduler, 332-line CPU bus and
PIC service. The [S21 intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md)
assigns one Core timeline/time-publication owner, copied board deadlines,
ordered board effects, PIC pending/INTA service and the DMA-HOLD locality
policy to distinct receivers. The original S21 code move would have mixed
these responsibilities; the revised linear S22-S26 plan removes that risk
without creating a second scheduler. The actual diff is documentation only:
no source, ABI, test, product, Shared, MyNES, INI or media change.

Documentation governance and `git diff --check` passed. The prior accepted
S20 full dual-width **469/469** unit and **8/8** external boot baseline stays
unchanged. S21 is accepted; T540 remains open. S22 receives copied board
deadline publication only.

## T540 S22 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S22, the next linear S after accepted S21. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. S21 source evidence narrows this receiver to copied device deadlines. |
| Objective | Move IBM-PC board device deadline discovery behind one copied observation contract while Core remains the sole guest-time/min-deadline owner. Preserve immediate-due, unsourced-DMA L1 blocking and existing min-order. |
| Non-goals | Device advance effects (S23), PIC CPU INTA handoff (S24), plan/reset lifecycle (S25), neutral Core physical move (S26), timing formula changes, profile/INI/firmware/media edits or a second scheduler. |
| Reference Baseline | Accepted S21 P1 `fa9aa0c27` and [source intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md); S20 complete units 469/469 per width, 76/76 specialized gates, four external boots once per width and eight Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and [S21 source intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md). |
| Files And ABI Surface | Inspect and change only NXVM deadline observation in `devices/{machine_scheduler,machine,machine_interface}` plus direct tests/static gate and eight products. Board reports copied due/immediate/block disposition; Core compares against its timeline and elapsed ticks. Do not expose board chip state to a neutral Core contract. |
| Applicable Rules | One Core clock and one min-deadline decision; board owns chip-state queries but not Core elapsed-time mutation. Unknown/unsourced timing remains honest L1/L2 as before. Delete old direct queries in the same cut, no parallel compatibility path. |
| Verification | Focused PIT/RTC/DMA/FDC/HDC/KBC/PIC/XT keyboard deadline and HLT tests; full repository-only x64/x86 units, specialized/documentation gates, each four-profile external boot once per width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Neutral Core deadline choice no longer sees chip private state, while board returns copied next-due/immediate/blocked facts and existing causal order and time grade are unchanged. |
| Asset Needs | Existing owner-provided firmware/media only for the external boot checkpoints and products. No new or modified asset, INI or external original. |
| Reporting Requirements | Record original device query callers and their destination, min/immediate/L1 cases, overflow/failure behavior, production/test added/removed/net lines, full dual-width verification, eight boot checkpoints and hashes; transfer device advancement to S23. |
| Stop Conditions | Stop before a new physical rate/timing inference, chip behavior reinterpretation, Shared/MyNES edit, protected asset change or a second time source. Split another genuinely distinct receiver into the next linear S before code if intake finds one. |
| Exit Criteria | One copied board deadline publication replaces Core's direct chip query path; full/focused verification, external boots and products pass; coordinator actual-diff review accepts implementation P and governance P records closure. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board layers before splitting PC Apps, with a single guest timeline and no patch-over-patch behavior. |
| Similar-Issue Sweep | Inspect every chip deadline query and the immediate/L1-blocked decisions, not just PIT; retain Core timeline and FPU completion as neutral, and classify every other board query or explicitly defer it with a unique owner reason. |

## T540 S22 Acceptance

P1 `dbef7f81e` removes every IBM-PC device-deadline query from Core's
observation path and binds one copied board provider. Core still owns
`elapsed_ticks`, timeline and FPU completion, and alone chooses the minimum
and advances time. The [S22 evidence](../../etc/evidence/t540-s22-copied-board-deadlines.md)
records the exact query move, immediate/L1/blocked cases, line counts and
eight product hashes. Actual staged and committed review found 19 scoped
paths: NXVM source/tests/gates/evidence and exactly eight optimized 0540
EXEs; no MyNES, Shared, INI, firmware or external original changed.

Full x64 and x86 repository-only units passed **469/469** each; specialized
gates passed **75/75**; all four external boots passed once in each width
(**8/8**). All eight products have the expected PE width and zero `.debug`
sections. Documentation governance and diff hygiene passed. S22 is accepted;
T540 remains open. S23 receives the still-mixed ordered board effects.

## T540 S23 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S23, the next linear S after accepted S22. |
| Admission And Approval | The owner's automatic admission of bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Move the peripheral-tail XT keyboard/KBC, PIC and VADP effects behind one bounded board call. Core retains the sole tick settlement and the arbitration/readiness/peripheral order. |
| Non-goals | Interleaved readiness and Core FPU (S24), DMA/refresh arbitration and prefetch (S25), CPU PIC INTA (S26), plan/reset (S27), neutral Core physical move (S28), timing formula or profile/INI/media/firmware changes, event framework or second loop. |
| Reference Baseline | Accepted S22 P1 `dbef7f81e`, [S22 evidence](../../etc/evidence/t540-s22-copied-board-deadlines.md) and [S21 source intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md); dual-width units 469/469, specialized gates 75/75, eight external boots and eight Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S23 refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S21/S22 evidence. |
| Files And ABI Surface | Inspect `devices/machine_scheduler.c` peripheral tail and direct chip APIs. Extract only the XT keyboard/KBC, PIC and VADP effects to a private board-owned implementation; Core passes the source-tick delta, not its timeline or CPU transaction. Update direct tests/gates and eight products. |
| Applicable Rules | One Core guest clock and settlement order, no copied owner state or parallel advance route. Keep DMA/PIT/PIC arbitration before FDC/HDC/FPU/RTC readiness before peripheral tail. S24/S25 retain the FPU/prefetch interleaving rather than moving Core work into board. |
| Verification | Focused KBC, XT keyboard, PIC and video advance/IRQ tests plus full dual-width repository-only units, specialized/documentation gates, four external boots once per width and eight optimized 0540 products with hashes/PE/no-debug proof. |
| Expected Markers | Core publication calls one board peripheral-tail seam after readiness; old direct XT keyboard/KBC/PIC/VADP advance calls in Core are deleted. Core still owns time and CPU/FPU settlement. |
| Asset Needs | Existing external owner-provided assets only for boot checkpoints and products; no new or changed ROM, media, INI or font. |
| Reporting Requirements | Record original and receiving peripheral calls, exact order, source-tick/zero-tick behavior, before/after lines, full verification, eight boots and hashes; transfer interleaved readiness/arbitration to S24/S25. |
| Stop Conditions | Stop before event-order reinterpretation, new physical timing claim, second scheduler, Shared/MyNES edits, protected asset modification or moving interleaved Core FPU/prefetch/bus transaction into the board. |
| Exit Criteria | Single board-owned peripheral-tail seam with no parallel direct Core chip-advance path; all source and external gates pass; coordinator actual-diff review accepts P1 and governance P2 records closure. |
| Original Owner Request | Make neutral `x86/core` and reusable IBM-PC board layers available before splitting PC Apps, with flat owner boundaries and linear numeric S tasks. |
| Similar-Issue Sweep | Inspect all three original phases and every direct chip effect; preserve and explicitly assign FPU/prefetch/transaction interleaving to S24/S25 rather than moving it by file proximity. |

## T540 S23 Acceptance

P1 `545086ce9` moves exactly the peripheral-tail XT keyboard/KBC, PIC and
VADP advancement to a board-owned implementation. Core still publishes the
sole time axis and calls arbitration, readiness, then the board peripheral
provider. One `board_owner` serves deadline and peripheral callbacks; no
second owner or advance loop exists. The
[S23 evidence](../../etc/evidence/t540-s23-board-peripheral-advance.md) records
the source-intake split, exact call order, net +5 production lines and eight
product hashes. Actual diff review found 19 scoped P1 paths: NXVM source,
tests, gates, evidence and eight Release EXEs only; Shared, MyNES, INIs,
firmware and external media are unchanged.

Complete x64/x86 repository-only unit suites passed **469/469** per width,
specialized gates **76/76**, and four external boots once per width (**8/8**).
All eight rebuilt products have their expected PE architecture and zero
`.debug` sections. Documentation governance and diff hygiene passed. S23 is
accepted; T540 remains open. S24 receives the interleaved readiness boundary.

## T540 S24 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S24, the next linear S after accepted S23. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES remain read-only. |
| Objective | Move board FDC/HDC readiness and RTC clock/chip effects behind explicit board calls while Core keeps FPU advancement between them, preserving the existing due-tick and trace order. |
| Non-goals | DMA/refresh/PIT/PIC arbitration and CPU prefetch (S25), CPU PIC INTA/HOLD locality (S26), mixed plan/reset (S27), physical neutral Core move (S28), chip timing/grade or profile/INI/firmware/media changes. |
| Reference Baseline | Accepted S23 P1 `545086ce9`, [S23 evidence](../../etc/evidence/t540-s23-board-peripheral-advance.md), [source intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md): dual-width units 469/469, gates 76/76, eight boots and eight Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S23 source-refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and board adapter ledger. |
| Files And ABI Surface | Inspect and change only private `devices/{machine_scheduler,board_advance,machine}` readiness calls plus direct tests/gates and eight products. Board receives source ticks and absolute due tick as values; it does not own Core FPU or elapsed-time mutation. |
| Applicable Rules | Keep exact sequence: FDC advance/trace, HDC advance/trace, Core FPU advance, RTC clock/chip advance/trace. Use direct named calls on either side of FPU rather than a generic phase framework or callback into Core. Delete old direct board calls in the same P. |
| Verification | Focused FDC/HDC, RTC, FPU, PIC ordering and scheduler tests; complete x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles per width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Core readiness contains FPU advancement between two bounded board calls; Core no longer directly queries or advances FDC/HDC/RTC chips. No second readiness path. |
| Asset Needs | Existing owner-provided external firmware/media only for boot checks and products; no new or changed binary original, INI or font. |
| Reporting Requirements | Record exact original/moved calls, due-tick and zero-tick behavior, trace order, production/test line delta, full/focused verification, eight boot results and product hashes; transfer arbitration to S25. |
| Stop Conditions | Stop before reordering FPU against FDC/HDC/RTC, inventing a physical rate, moving Core time/transaction ownership into the board, Shared/MyNES edits, protected asset changes or a second scheduler. |
| Exit Criteria | One ordered readiness seam around Core FPU, old direct Core chip calls deleted, all gates and products pass, actual-diff P1 accepted and governance P2 records closure. |
| Original Owner Request | Create neutral `x86/core` and reusable IBM-PC board layers before splitting PC Apps, with flat ownership and linear numeric S tasks. |
| Similar-Issue Sweep | Inspect every readiness advance, clock conversion and trace event; keep FPU as Core even though it sits among board effects, and identify any other interleaving before moving code. |

## T540 S24 Acceptance

P1 `8f1b7a68c` moves FDC/HDC readiness and RTC advancement into two named
board callbacks that surround, but never own, Core's FPU advancement. The
original due-tick, clock conversion, zero-tick and trace order remain. The
[S24 evidence](../../etc/evidence/t540-s24-board-readiness.md) records the
exact move, net +27 production lines, complete verification and eight
product hashes. Actual diff review found 18 NXVM-only scoped P1 paths:
source, test, gates, evidence and eight optimized EXEs; no Shared, MyNES,
INI, firmware or external-media change.

Complete x64 and x86 repository-only units passed **469/469** each; all
specialized gates passed; four external boot checkpoints passed once per
width (**8/8**). The rebuilt products have the expected PE architecture
and zero `.debug` sections. Documentation governance and diff hygiene
passed. S24 is accepted; T540 remains open for S25 arbitration.

## T540 S25 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S25, the next linear S after accepted S24. |
| Admission And Approval | The owner's automatic admission of bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES remain read-only. |
| Objective | Source-inspect the entire mixed arbitration function, freeze its exact Core transaction/CPU prefetch and board request/chip effect owners, and assign bounded S26-S31 code receivers before implementation. |
| Non-goals | Any production/API/test/asset/INI/executable change, timing or grade reinterpretation, generic bus/event framework or speculative physical relocation. |
| Reference Baseline | Accepted S24 P1 `8f1b7a68c`, [S24 evidence](../../etc/evidence/t540-s24-board-readiness.md), [S21 intake](../../etc/evidence/t540-s21-scheduler-pic-intake.md); dual-width units 469/469, all specialized gates, eight external boots and eight Release products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S25 refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md) and board adapter ledger. |
| Files And ABI Surface | Read `devices/{machine_scheduler,board_deadline,dma_bus,machine_board,transaction}` and Core prefetch contract; document copied request/completion boundaries. No source or ABI change in this S. |
| Applicable Rules | Preserve clock conversion, pending refresh HOLD, DMA wait/grant, Core prefetch, DMA trace, PIT advance/trace and PIC refresh/trace in exact order. One Core tick and transaction owner; board owns chip/request state. Split unlike responsibilities before code. |
| Verification | Source audit every arbitration branch and its failure behavior; map focused tests. As a documentation-only S, run documentation governance and diff hygiene; retain S24 complete dual-width unit, gate, boot and product baseline without unnecessary rebuild. |
| Expected Markers | Every existing arbitration effect has a unique Core or board owner, an explicit linear S26-S31 receiver and a regression anchor; no unassigned Core-prefetch or board-DMA path. |
| Asset Needs | Existing external owner-provided firmware/media only for boot checks and products; no new or changed ROM, disk, font or INI. |
| Reporting Requirements | Record exact original calls, wait/hold failure behavior, prefetch gate, trace order, zero production/test/binary diff, S26-S31 receiver plan and regression anchors. |
| Stop Conditions | Stop before moving Core transaction/prefetch state into board, new physical timing inference, changed DMA/PIT/PIC order, second scheduler/transaction owner, Shared/MyNES edits or protected asset change. |
| Exit Criteria | Complete source-only owner map and linear receiver plan, documentation/diff checks pass, actual-diff P1 review and governance P2 close S25. |
| Original Owner Request | Build reusable neutral `x86/core` and IBM-PC board layers before splitting PC Apps, without layering compatibility patches. |
| Similar-Issue Sweep | Inspect both DMA wait-quanta and no-wait paths, D4 refresh HOLD, prefetch eligibility, PIC refresh and every trace effect; avoid moving Core authority merely because it shares a function with board chips. |

## T540 S25 Acceptance

P1 `3f14bf9d2` source-inspects the original arbitration function and
separates board D4 request/address, board DMA clock/request/chip effects,
Core HOLD/transaction/wait/prefetch, and board PIT/PIC tail effects. The
[S25 intake](../../etc/evidence/t540-s25-arbitration-intake.md) records exact
call and failure order plus the linear S26-S31 receiver map. The actual
two-file P1 diff is documentation only: zero production/API, test, EXE,
Shared, MyNES, INI or external-asset changes.

Documentation governance and diff hygiene passed. The accepted S24 full
dual-width **469/469** unit, specialized-gate, **8/8** boot and eight-product
baseline is unchanged. S25 is accepted; T540 remains open. S26 receives
the D4 refresh request/completion boundary only.

## T540 S26 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S26, the next linear S after accepted S25. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Make D4 board refresh pending/address a copied request to Core; Core alone performs HOLD and refresh transaction and reports success so board clears/increments only after commit. |
| Non-goals | DMA clock/wait/grant and prefetch (S27), PIT/PIC tail (S28), CPU PIC INTA/locality (S29), mixed plan/reset (S30), physical neutral Core move (S31), new timing formula, profile/INI/firmware/media changes. |
| Reference Baseline | Accepted S25 P1 `3f14bf9d2`, [arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md) and S24 accepted dual-width 469/469 units, all specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
| Files And ABI Surface | Inspect `devices/{machine_scheduler,machine_board,machine,transaction}` and D4 tests. Board reports only copied pending/address and receives a success completion; Core transaction, HOLD owner and guest timeline stay private. Update direct gates/tests and eight products. |
| Applicable Rules | Original pending snapshot precedes service and remains available to prefetch gate. Failed HOLD/ack/begin retains board request/address. Only successful Core commit clears pending and increments address; release HOLD in original cases. No parallel refresh path. |
| Verification | Focused D4 refresh/HOLD, parity, transaction and prefetch tests; complete x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles/width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Core refresh routine reads copied request and sends success only; it no longer directly reads/writes D4 pending/address. Board never receives a transaction pointer or publishes Core time. |
| Asset Needs | Existing owner-provided external firmware/media for boots/products only; no new or changed ROM, disk, font or INI. |
| Reporting Requirements | Record original and new success/failure transitions, pending-before-service prefetch fact, line delta, focused/full checks, eight boot outcomes and product hashes; transfer DMA wait/grant to S27. |
| Stop Conditions | Stop before inventing refresh physics, moving Core transaction/HOLD into board, changing failed-HOLD retry semantics, Shared/MyNES edits, second guest clock or protected-asset changes. |
| Exit Criteria | Single D4 copied request/completion seam with no direct Core board-state mutation and no parallel path; full verification and eight products pass; actual-diff P1 and governance P2 close S26. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect all D4 refresh pending/address reads and writes, reset, PIT callback and prefetch condition; leave DMA and PIC effects explicitly for S27-S29. |

## T540 S26 Acceptance

NXVM P1 `0d5cac0f9` keeps the original Core HOLD/request/acknowledge/begin/
commit/release order and transfers only copied D4 pending/address and
success-only completion to the board owner. The actual staged diff was 17
NXVM paths, 164 lines added and 7 removed, including one board-boundary gate,
two focused unit updates and eight optimized product updates. The first x64
full run exposed a scheduler fixture that substituted the board owner without
forwarding the new callbacks; the fixture was repaired before acceptance.

The final complete unit run passed **469/469** on both x64 and x86, the
specialized gates passed, and all four external boot checkpoints passed once
per width (**8/8**). Eight Release products have the expected PE format,
zero `.debug` sections and recorded SHA-256 hashes. Documentation governance
and staged diff checks passed. The [S26 evidence](../../etc/evidence/t540-s26-board-refresh-request.md)
records the owner split and exact products. S26 is accepted; S27 receives the
remaining DMA clock/request/wait/grant/prefetch boundary. T540 remains open.

## T540 S27 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S27, the next linear S after accepted S26. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Give board code DMA clock/request/chip effects while Core alone owns cycle wait, HOLD/grant, transaction arbitration and CPU prefetch reservation, preserving their exact order. |
| Non-goals | PIT/PIC tail (S28), CPU PIC INTA/locality (S29), mixed plan/reset (S30), neutral Core move (S31), timing formula, profile/INI/firmware/media changes or a generic bus framework. |
| Reference Baseline | Accepted S26 P1 `0d5cac0f9`, [S25 arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md) and S26 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
| Files And ABI Surface | Inspect `devices/{machine_scheduler,dma_bus,machine_board,machine,transaction}` and DMA/prefetch tests. Board supplies copied pending/request facts and performs only chip/clock effects; no Core transaction or raw RAM pointer crosses out. |
| Applicable Rules | Preserve refresh-before-DMA, wait quanta before DMA chip service, Core HOLD/grant and bounded DMA memory-cycle ownership, prefetch after DMA, then PIT/PIC tail. No second DMA scheduler or independent guest clock. |
| Verification | Focused DMA grant/wait/request, refresh arbitration, prefetch and transaction tests; full x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles/width, and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Scheduler no longer directly advances board DMA chips or reads mutable DMA request fields; Core still evaluates copied requests, transaction/HOLD state and CPU prefetch. |
| Asset Needs | Existing owner-provided external firmware/media for boots/products only; no new or changed ROM, disk, font or INI. |
| Reporting Requirements | Record call/failure order, DMA clock/request/wait/grant and prefetch owner map, actual line delta, focused/full checks, eight boot outcomes and product hashes; transfer PIT/PIC tail to S28. |
| Stop Conditions | Stop before inventing DMA timing, moving Core HOLD/prefetch into board, adding a second DMA path, editing Shared/MyNES, or changing protected assets. |
| Exit Criteria | One copied board DMA boundary with unchanged Core arbitration and failure semantics; full verification and eight products pass; actual-diff P1 and governance P2 close S27. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect all primary/secondary DMA request reads, clock advancement, wait/grant/trace order and CPU prefetch gates; keep PIT/PIC and CPU INTA explicitly assigned to S28-S29. |

## T540 S27 Acceptance

NXVM P1 `0c1bba56a` leaves Core-owned wait quanta, bus-ready gate,
HOLD/grant/release, bounded DMA memory cycle and CPU prefetch in their
original order. Board callbacks supply DMA clock advancement, copied pending
request and chip advancement through the one existing DMA path. The actual
staged diff was 16 NXVM paths, 185 lines added and 21 removed, including the
new boundary gate, scheduler fixture and eight optimized products.

Focused DMA, refresh, transaction and prefetch tests passed. Full x64 and
x86 repository-only units passed **469/469** each, specialized gates passed,
and all four external boot checkpoints passed once per width (**8/8**).
Eight Release products have the expected PE format, zero `.debug` sections
and recorded SHA-256 hashes. Documentation governance and staged diff checks
passed. The [S27 evidence](../../etc/evidence/t540-s27-board-dma-arbitration.md)
records the exact owner split and product hashes. S27 is accepted; S28 owns
the remaining PIT/PIC post-prefetch tail. T540 remains open.

## T540 S28 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S28, the next linear S after accepted S27. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Move PIT clock/chip advancement and PIC refresh/trace in the post-prefetch arbitration tail to one board-owned callback, preserving the original call and signal order. |
| Non-goals | CPU PIC INTA/locality (S29), mixed plan/reset (S30), neutral Core move (S31), timer formula, profile/INI/firmware/media changes, second guest clock or generic event framework. |
| Reference Baseline | Accepted S27 P1 `0c1bba56a`, [S25 arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md) and S27 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
| Files And ABI Surface | Inspect `devices/{machine_scheduler,board_advance,machine_board,machine}` and PIT/PIC/trace tests. Board owns PIT clock domains, both PIT chips, PIC refresh and their traces; Core supplies elapsed source ticks and remains the sole timeline/phase owner. |
| Applicable Rules | Preserve D4 refresh, DMA wait/grant and Core prefetch before the board PIT/PIC tail; retain primary then optional auxiliary PIT, PIT trace, master/slave PIC refresh and PIC trace order. No CPU INTA migration in this S. |
| Verification | Focused PIT output/IRQ0, auxiliary PIT, PIC cascade/phase, scheduler, DMA and trace tests; full x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles/width, and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Scheduler calls one board tail with source ticks after prefetch; it no longer directly advances PIT clocks/chips or refreshes PIC. Board state and signals retain one owner. |
| Asset Needs | Existing owner-provided external firmware/media for boots/products only; no new or changed ROM, disk, font or INI. |
| Reporting Requirements | Record original and new PIT/PIC order, trace and IRQ effects, actual line delta, focused/full checks, eight boot outcomes and product hashes; transfer PIC CPU INTA/locality to S29. |
| Stop Conditions | Stop before changing timer counts, moving Core guest time to board, putting CPU INTA in board, editing Shared/MyNES or changing protected assets. |
| Exit Criteria | One board-owned PIT/PIC tail after Core arbitration with unchanged signals and trace order; full verification and eight products pass; actual-diff P1 and governance P2 close S28. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect primary/auxiliary PIT and PIC refresh across all scheduler, board callback and reset paths; leave CPU PIC INTA/HOLD locality explicitly for S29. |

## T540 S28 Acceptance

NXVM P1 `826d5e29c` retains early PIT clock-domain conversion as a copied
primary/auxiliary tick pair, then advances both PIT chips, refreshes PIC and
emits the original traces after Core DMA service and prefetch. The staged
diff was 16 NXVM paths, 173 lines added and 17 removed, including the new
boundary gate, scheduler fixture and eight optimized product updates.

Full x64 and x86 repository-only units passed **469/469** each; the
specialized gates passed; all four external boot checkpoints passed once per
width (**8/8**). Eight Release products have the expected PE format, zero
`.debug` sections and recorded SHA-256 hashes. Documentation governance
and staged diff checks passed. The [S28 evidence](../../etc/evidence/t540-s28-board-pit-pic-tail.md)
records exact call order and hashes. S28 is accepted; S29 receives CPU PIC
INTA and DMA-HOLD locality. T540 remains open.

## T540 S29 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S29, the next linear S after accepted S28. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Source-inspect and isolate CPU PIC INTA and DMA-HOLD locality: Core retains CPU delivery and prefetch invalidation, board supplies only PIC/DMA signal facts and electrical wiring. |
| Non-goals | Mixed plan/reset (S30), neutral Core physical move (S31), timing formula, profile/INI/firmware/media changes, second interrupt route or generic event bus. |
| Reference Baseline | Accepted S28 P1 `826d5e29c`, [S25 arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md), S28 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
| Files And ABI Surface | Inspect `devices/{machine_board,machine_scheduler,cpu_bus,machine,transaction}` plus PIC/INTA and prefetch-locality tests. Expose only copied line/request facts across Core/board; no board CPU pointer or Core PIC pointer. |
| Applicable Rules | Preserve PIC phase/cascade, CPU INTA sampling/acknowledge and DMA HOLD/prefetch invalidation order. Board may emit line events; Core alone decides CPU interrupt entry and locality effects. |
| Verification | Focused PIC phase/cascade/INTA, DMA HOLD, CPU prefetch and scheduler tests; full x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles/width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | No board-owned CPU execution mutation or Core-owned PIC wiring; one bounded signal boundary and no duplicate INTA path. |
| Asset Needs | Existing owner-provided external firmware/media for boots/products only; no new or changed ROM, disk, font or INI. |
| Reporting Requirements | Record original/receiver call graph, interrupt and locality order, actual line delta, focused/full checks, eight boot outcomes and product hashes; transfer mixed plan/reset to S30. |
| Stop Conditions | Stop before changing PIC/DMA electrical semantics, CPU interrupt timing or prefetch rules, editing Shared/MyNES, or changing protected assets. |
| Exit Criteria | One source-audited CPU/board signal boundary with unchanged PIC INTA and HOLD locality; full verification and eight products if code changes; actual-diff P1 and governance P2 close S29. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect all PIC IRQ/INTA callbacks and DMA HOLD/prefetch invalidations, including XT versus AT paths and reset; leave plan/reset assigned to S30. |

## T540 S29 Acceptance

NXVM P1 `ace7c0fa6` gives the board the original PIC pending scan and
acknowledge operations as copied values. Core still starts, values and
commits the sole CPU INTA transaction. A D4 refresh edge now notifies the
named Core CPU-bus event, which performs the same prefetch-locality
invalidation at the original call site. DMA HOLD acknowledge locality stays
in Core. The staged diff was 19 NXVM paths, 172 lines added and 9 removed,
including the updated historical CPU/PIC authority gate and its full
negative fixture, new S29 gate, and eight optimized product updates.

Focused scheduler, PIC phase/lifecycle, prefetch locality and historical
negative controls passed. Final-source x64 and x86 complete units passed
**469/469** each; specialized gates passed; all four external boot
checkpoints passed once per width (**8/8**). Eight Release products have
the expected PE format, zero `.debug` sections and recorded SHA-256 hashes.
Documentation governance and staged diff checks passed. The
[S29 evidence](../../etc/evidence/t540-s29-pic-cpu-locality.md) records the
call order and hashes. S29 is accepted; S30 receives mixed plan/reset.
T540 remains open.

## T540 S30 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S30, the next linear S after accepted S29. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only for intake; a later Shared move requires its own scoped P. MyNES remains read-only. |
| Objective | Inspect mixed machine plan/construction/reset sources, identify exact neutral Core and board-only owners, and implement only bounded owner separation needed before S31 neutral Core relocation. Split oversized source moves into linear numeric S receivers if inspection proves the batch too large. |
| Non-goals | Physical Core move (S31 or later), board family extraction (S32 onward), controller timing formulas, profile/INI/firmware/media changes or a new framework. |
| Reference Baseline | Accepted S29 P1 `ace7c0fa6`, [S25 arbitration intake](../../etc/evidence/t540-s25-arbitration-intake.md), dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and accepted S22-S29 evidence. |
| Files And ABI Surface | Read `devices/{machine,machine_plan,machine_board,entry_plan_interface,rom_mapping_interface,trace_interface}` plus constructors, reset fixtures and profile bindings. Record exact owner, reset lifetime and rollback for each mixed field/call. |
| Applicable Rules | One Core timeline/executor/transaction owner, one board wiring/chip owner, frozen profile plan, no raw Core state exported to board or guest device state copied into Core. Preserve construction and reset order and failure rollback. |
| Verification | For source-only intake: complete owner/call inventory, documentation governance and diff check. For code: focused creation/reset/plan/rollback, full dual-width units, specialized gates, eight boot checkpoints and eight optimized products with PE/hash/no-debug proof. |
| Expected Markers | Each mixed plan/reset effect has a unique owner and finite receiver; no unassigned Core/board field remains before physical relocation. |
| Asset Needs | Existing owner-provided external firmware/media only if code receivers rebuild/test products; no new protected assets or INI edits. |
| Reporting Requirements | Record source line inventory, original call/reset/rollback order, owner decisions, bounded linear S receivers, actual diff and verification; preserve T540 open status. |
| Stop Conditions | Stop before moving an oversized mixed source wholesale, changing chip/reset semantics without source review, editing MyNES, or crossing Shared scope in one NXVM commit. |
| Exit Criteria | Audited finite Core/board plan/reset ownership and each bounded receiver implemented or explicitly assigned; no hidden dual path; verification matches actual code scope and P1/P2 close S30. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect all construction, reset, failure cleanup and plan/profile inputs, including XT, 5170, Model 40 and default; no board-specific branch may silently enter neutral Core. |

## T540 S30 Acceptance

NXVM P1 `4f9f72b28` source-inspected the mixed `machine`, `machine_plan`,
`machine_board`, entry, ROM and trace implementations. The original
validation→Core construction→board topology→single rollback, cold-reset
stage order, processor-only reset and image/alias destruction obligations
are recorded in the [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md).
The former one-piece plan/reset move is divided into linear S31–S37
owner-sized receivers, S38 neutral Core relocation, then S39 onward board
extraction. Only unadmitted prospective numbering changed.

This is documentation-only: no production, test, firmware, INI or executable
input changed. The accepted S29 x64/x86 469/469 units, specialized gates,
8/8 boots and eight optimized products remain the source baseline.
Documentation governance and diff checks passed. S30 is accepted; T540
remains open.

## T540 S31 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S31, the next linear S after accepted S30. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Separate IBM-PC topology/timing plan validation and frozen composition from neutral CPU/time/transaction Core construction inputs, keeping one plan and no duplicated mutable state. |
| Non-goals | Core constructor effects (S32), board constructor/topology application (S33), reset (S34), teardown (S35), ROM/entry/trace qualification (S36), physical move (S38), device timing/profile/INI/firmware/media changes. |
| Reference Baseline | Accepted S30 P1 `4f9f72b28`, [S30 plan/reset intake](../../etc/evidence/t540-s30-plan-reset-intake.md) and unchanged S29 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md) and S30 intake. |
| Files And ABI Surface | Inspect `devices/{machine_plan,machine,machine_interface}` and all four profile constructors. One board-owned frozen plan may pass only validated immutable CPU, time-axis, clock and transaction inputs to neutral Core. No parallel plan parser, copy or profile-name switch. |
| Applicable Rules | Preserve validation-before-allocation, declaration uniqueness/timing grade, topology constraints, configuration-owned retirement qualification copy and `create_from_plan` success-only plan publication. |
| Verification | Focused plan/profile/construction-invalid/rollback tests; full x64/x86 repository-only units, specialized/documentation gates, one boot per four profiles/width and eight optimized 0540 products with PE/hash/no-debug proof if code changes. |
| Expected Markers | One plan owner and one immutable neutral Core input boundary; no copied mutable controller state or new Core branch on machine name. |
| Asset Needs | Existing owner-provided external firmware/media only for code validation; no new or changed protected binary or INI. |
| Reporting Requirements | Record validation/copy source diff, owner/lifetime decisions, focused/full checks and products; hand off constructor side effects to S32-S33. |
| Stop Conditions | Stop before implementing a second plan, changing timing grades, moving board topology into neutral Core, editing Shared/MyNES or broadening to constructor/reset effects. |
| Exit Criteria | A single validated frozen plan with neutral Core inputs and no parallel path; complete verification for the actual code scope; actual-diff P1 and governance P2 close S31. |
| Original Owner Request | Build neutral `x86/core` and reusable IBM-PC board components with flat ownership, linear numeric S and no patch-over-patch path. |
| Similar-Issue Sweep | Inspect all config/plan copies, constructor entry points, plan topology declaration and all four profile inputs for duplicated or mutable machine facts. |

## T540 S31 Acceptance

Actual-diff review accepts NXVM P1 `02c505858`. The VM's extra configuration
and timing-rule copies are deleted; the one board construction plan receives
values directly from the profile source. Its topology/timing validation no
longer repeats the neutral transaction preflight that Core itself performs
before allocation. The original create/apply/rollback route remains singular.
The [S31 evidence](../../etc/evidence/t540-s31-frozen-plan-boundary.md) records
ownership, test and artifact proof. Full x64/x86 units pass **469/469** per
width, specialized gates pass, all eight external boot checkpoints pass once,
and eight optimized 0540 products have correct PE width and no `.debug`
sections. No Shared, MyNES, INI, firmware source or timing-grade change.
S31 is accepted; S32 owns constructor effects and S33 owns board creation.
T540 remains open.

## T540 S32 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S32, next linear S after accepted S31. |
| Admission And Approval | The owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES remain read-only. |
| Objective | Isolate neutral Core constructor preflight, allocation, CPU/FPU, transaction, timeline, memory and port construction from IBM-PC board-only construction effects, preserving one create route and exact early exits. |
| Non-goals | Board controller creation and topology application (S33), reset (S34), teardown (S35), ROM/entry/trace work (S36), physical Shared move (S38), timing formulas, profile/INI/firmware/media changes. |
| Reference Baseline | S31 P1 `02c505858`, [S31 evidence](../../etc/evidence/t540-s31-frozen-plan-boundary.md) and dual-width 469/469 units, specialized gates, 8/8 external boot checkpoints, eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/machine.c`, its private state, existing constructor callers and direct tests. Extract only CPU/time/transaction/bus/memory initialization into one private neutral Core owner, without adding a second public constructor or exposing board state. |
| Applicable Rules | Preserve validation-before-allocation, original error codes and null output, copied retirement qualification lifetime, CPU/FPU teardown, one Core timeline, and first-failure rollback. Board construction order remains S33's receiver. |
| Verification | Focused invalid-config, CPU/FPU, allocation/rollback and plan tests; complete x64/x86 repository-only units, specialized/documentation gates, one boot per profile/width, and eight optimized 0540 products with PE/hash/no-debug proof for code changes. |
| Expected Markers | One neutral constructor phase with no machine-name or controller-personality branch; unchanged board controller create order and no duplicate Core state or second create route. |
| Asset Needs | Existing owner-provided external firmware/media for boot validation only; no protected input or owner INI change. |
| Reporting Requirements | Record exact constructor call/failure order, affected callers, actual diff, full verification and products; assign board-only construction and topology to S33. |
| Stop Conditions | Stop before a new framework, board state in neutral Core, changed timing or profile semantics, Shared/MyNES edits, or an oversized mixed constructor rewrite that cannot be cleanly bounded. |
| Exit Criteria | Neutral Core preflight/allocation and setup have one owner, all original early failures and rollback survive, focused/full checks pass, and reviewed P1 plus governance P2 close S32. |
| Original Owner Request | Build reusable neutral `x86/core` and flat IBM-PC board layers without code duplication, using linear numeric S deliveries. |
| Similar-Issue Sweep | Check every create entry point, allocation failure, CPU/FPU and memory/port setup branch, copied qualification lifetime, and partial initialization cleanup. |

## T540 S32 Acceptance

Actual-diff review accepts NXVM P1 `c5e8022d7`. One private neutral phase
now owns CPU/FPU, time, transaction, bus, port and memory setup, while the
original public create entry points and the board controller order remain
unchanged. The [S32 evidence](../../etc/evidence/t540-s32-neutral-construction.md)
records exact early failures and the net +18 source lines. Complete x64/x86
units pass **469/469** per width, specialized gates pass, **8/8** single-run
external boots pass, and eight optimized 0540 products have correct PE width
and no `.debug` sections. No Shared/MyNES, INI, firmware or timing-grade
change. S33 owns board construction and topology application; T540 stays open.

## T540 S33 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S33, next linear S after accepted S32. |
| Admission And Approval | Owner's automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Isolate IBM-PC board controller creation and frozen plan topology application around the one neutral Core construction route, preserving success-only publication and first-failure rollback. |
| Non-goals | Cold/processor-only reset (S34), teardown restructuring (S35), ROM/entry/trace (S36), physical Shared move (S38), profile values, timing formulas, INI/firmware/media changes. |
| Reference Baseline | S32 P1 `c5e8022d7`, [S32 evidence](../../etc/evidence/t540-s32-neutral-construction.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_plan.c,machine_board.c}` and direct plan/rollback tests. Keep one private board assembly phase after the neutral create result; `create_from_plan` still validates before allocation and applies topology through the sole existing route. |
| Applicable Rules | Preserve port checkpoint; DMA before PIC before PIT; PIT0 to IRQ0 binding; XT PPI versus AT 8042 exclusive branch; PIT1 unbound until post-reset wiring; topology order memory aliases, absent/parity/D4/display/DMA/RTC/FDC/HDC; abort on first failure, destroy one partial machine, leave result null. |
| Verification | Focused board registration, plan-invalid, topology, rollback and four-profile tests; full x64/x86 repository-only units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 products with PE/hash/no-debug proof if code changes. |
| Expected Markers | One board constructor phase and one topology apply route, no duplicated port registration or controller state, and no board dependency in the neutral Core phase. |
| Asset Needs | Existing owner-provided external firmware/media for the fixed boot checkpoint only; no new protected input or owner INI change. |
| Reporting Requirements | Record actual controller/topology order and failure rollback diff, complete checks, artifact hashes and any work retained for S34-S36. |
| Stop Conditions | Stop before generic board framework, second constructor/rollback, changed clock or device behavior, Shared/MyNES edits, or an oversized mixed reset/destructor rewrite. |
| Exit Criteria | Board create and topology apply each have one owner and one route, all original failure/publication semantics survive, verification passes, and actual-diff P1 plus governance P2 close S33. |
| Original Owner Request | Extract a neutral reusable x86 Core and flat IBM-PC board mechanisms with no divergence or patch-over-patch code. |
| Similar-Issue Sweep | Review every board port registration, XT/AT controller choice, auxiliary PIT, topology setter, failure return and plan publication caller across four profiles. |

## T540 S33 Acceptance

Actual-diff review accepts NXVM P1 `134c8e722`. The private board-create phase
keeps the original controller and port order; the sole plan validation,
topology application and rollback route now resides in the board-plan source.
The [S33 evidence](../../etc/evidence/t540-s33-board-construction.md) records
the exact owner/failure order and net +14 production lines. Final-source
x64/x86 units pass **469/469** per width, specialized gates pass, the eight
single-run external boot checkpoints pass, and all eight optimized 0540
products have correct PE width and no `.debug` sections. Shared, MyNES,
owner INIs, firmware and timing grades are unchanged. S33 is accepted; S34
receives reset ownership. T540 remains open.

## T540 S34 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S34, next linear S after accepted S33. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Split the one cold reset at its existing Core/board sequence points while retaining the separate CPU-local processor reset, firmware failure lifecycle and exact board signal order. |
| Non-goals | Constructor/topology changes (accepted S31-S33), teardown restructure (S35), entry/ROM/trace qualification (S36), physical Shared move (S38), timing/profile/INI/firmware/media changes. |
| Reference Baseline | S33 P1 `134c8e722`, [S33 evidence](../../etc/evidence/t540-s33-board-construction.md), dual-width 469/469 units, specialized gates, 8/8 boot checkpoints and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c}` and reset/firmware/processor-pulse callers and tests. One cold-reset public route may call private Core and board stages at original sequence points; no second reset implementation. |
| Applicable Rules | Preserve CPU/FPU→port/memory/D4→keyboard→DMA/RTC→board state→FDC/HDC→PIC/PIT→post-PIT wiring→video→Core counters/transaction/timeline/clocks→providers→firmware→STOPPED/trace. Firmware failure remains INITIALIZED; processor-only reset preserves RAM, board devices and scheduled time. |
| Verification | Focused cold-reset, 8042 processor-pulse, firmware-failure, XT/AT/Model40 D4/PIT/IRQ regressions; full x64/x86 repository-only units, specialized/documentation gates, one boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof for code change. |
| Expected Markers | One Core reset owner, bounded board reset effects in original order, one processor-only CPU-local reset, no duplicated state or second guest clock. |
| Asset Needs | Existing owner-provided external firmware/media for fixed boot checkpoints only; no new protected input or owner INI change. |
| Reporting Requirements | Record exact reset stage/order diff, firmware-error lifecycle, processor-only preservation, focused/full checks and product hashes; assign teardown to S35. |
| Stop Conditions | Stop before a second reset route, board state in neutral Core, timing-grade change, Shared/MyNES edits or oversized teardown/ROM changes outside this S. |
| Exit Criteria | Core/board cold-reset ownership is explicit with original order and failure result, processor-only reset is unchanged, verification passes and actual-diff P1 plus governance P2 close S34. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique owners and no patch-over-patch compatibility routes. |
| Similar-Issue Sweep | Check every reset entry point, board post-PIT callback, firmware-failure return, 8042 pulse, D4/XT/AT signal and partial reset observation across four profiles. |

## T540 S34 Acceptance

Actual-diff review accepts NXVM P1 `728553d8e`. The one cold-reset path now
calls a private board-device phase at its original sequence point; the former
board-state wrapper was inlined rather than preserved as a second reset path.
The [S34 evidence](../../etc/evidence/t540-s34-reset-boundary.md) records the
exact order and net -2 production lines. Processor-only reset and firmware
failure lifecycle remain unchanged. Final-source x64/x86 units pass **469/469**
per width, specialized gates pass, all eight single-run external boot
checkpoints pass, and eight optimized 0540 products have correct PE width and
no `.debug` sections. No Shared/MyNES, owner INI, protected firmware or timing
grade changed. S35 receives teardown; T540 remains open.

## T540 S35 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S35, next linear S after accepted S34. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Split the sole partial-failure and final destruction sequence by neutral Core versus IBM-PC board ownership, including immutable ROM image versus alias, without another destructor. |
| Non-goals | Constructor/reset changes (accepted S31-S34), ROM/entry/trace qualification (S36), physical Shared move (S38), timing/profile/INI/firmware/media changes. |
| Reference Baseline | S34 P1 `728553d8e`, [S34 evidence](../../etc/evidence/t540-s34-reset-boundary.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c,machine.h}`, ROM mapping ownership, every early create/apply failure and direct teardown tests. Keep `core_machine_destroy` the one public route; one private board-finalize stage may release only board-owned resources. |
| Applicable Rules | Retain reverse-safe partial teardown, firmware provider revocation before any release, board PIT/HDC/FDC/DMA/RTC/keyboard/PIC/VADP order, neutral CPU/FPU/port/memory/trace/bus release, and exactly-once freeing only for immutable ROM mappings with `owns_image`; aliases never free another mapping's image. |
| Verification | Focused partial-constructor, port rollback, plan-apply failure, ROM alias/owner and final destructor tests; full x64/x86 repository-only units, specialized/documentation gates, one boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof for code change. |
| Expected Markers | One public destructor, one bounded private board teardown and one neutral Core teardown path; no second cleanup owner or ROM image double-free. |
| Asset Needs | Existing owner-provided external firmware/media for fixed boot checkpoints only; no new protected input or owner INI change. |
| Reporting Requirements | Record exact teardown order, early-failure reachability, ROM image/alias ownership, focused/full checks and product hashes; assign ROM/entry/trace qualification to S36. |
| Stop Conditions | Stop before second destructor, moved ownership without rollback proof, generic board framework, Shared/MyNES edits or oversized ROM/entry rewrite. |
| Exit Criteria | Partial and final destruction use the one public route with explicit Core/board owners and original order, no double free, required verification passes, and actual-diff P1 plus governance P2 close S35. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique owners and no patch-over-patch compatibility routes. |
| Similar-Issue Sweep | Check all early create failures, topology rollback, public destroy callers, every board chip finalizer, ROM alias `owns_image` flag and post-destroy state across all four profiles. |

## T540 S35 Acceptance

Actual-diff review accepts NXVM P1 `5e0f86e2a`. The sole public destroy route
now delegates the unchanged board release order to one private board-owner
stage. Immutable ROM teardown uses the existing route rollback before memory
finalization, so only `owns_image` mappings release bytes and the old second
image loop is gone. The [S35 evidence](../../etc/evidence/t540-s35-teardown-boundary.md)
records early-failure reachability, exact order and net-zero production lines.
Final-source x64/x86 units pass **469/469** per width, specialized gates pass,
the eight single-run external boot checkpoints pass, and all eight optimized
0540 products have correct PE width and no `.debug` sections. No Shared/MyNES,
owner INI, protected firmware or timing grade changed. S36 receives the
entry/ROM/trace qualification; T540 remains open.

## T540 S36 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S36, next linear S after accepted S35. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Qualify the existing prepared-entry, immutable ROM mapping and bounded trace surfaces as neutral Core boundaries; remove only proven board-state access that would block physical relocation. |
| Non-goals | Rework accepted constructor/reset/destructor paths, change firmware bytes or boot entry, create a second mapping table/trace buffer, perform the physical Shared move (S38), or change timing/profile/INI/media behavior. |
| Reference Baseline | S35 P1 `5e0f86e2a`, [S35 evidence](../../etc/evidence/t540-s35-teardown-boundary.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Source-inspect `src/app-nxvm/devices/{entry_plan_interface.c,rom_mapping_interface.c,trace_interface.c,machine.c,machine.h}` plus direct tests and board/firmware callers. Retain their one existing route; edit only a concrete dependency violating the Core/board handoff. |
| Applicable Rules | Core owns prepared CPU entry, memory routes, copied immutable ROM image and alias/rollback, and bounded transaction/CPU trace; board/firmware supply bytes, entry request and typed device effects without writable Core internals. Preserve reset-overlay priority, preload bounds and failure rollback. |
| Verification | Source/dependency and caller audit, focused entry/ROM/trace regressions; for code change, full x64/x86 repository-only units, specialized/documentation gates, one boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof. For source-only acceptance, record evidence and governance checks without unnecessary binary rewrite. |
| Expected Markers | A finite per-file ownership verdict, no board state in neutral Core implementation, one mapping table and trace owner, and exact retained or removed dependencies. |
| Asset Needs | Existing owner-provided external firmware/media only if a code change invokes boot checks; no new protected input or owner INI change. |
| Reporting Requirements | Record actual dependency/caller diff or justified no-diff, bounds/rollback/alias/trace findings, verification and remaining S37 audit receivers. |
| Stop Conditions | Stop before an invented framework, second ROM/trace route, changed firmware semantics, Shared/MyNES edits or oversized physical relocation. |
| Exit Criteria | All three boundaries are source-qualified for neutral Core, any concrete board-state leak is repaired and verified, and actual-diff P1 plus governance P2 close S36. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique owners and no patch-over-patch compatibility routes. |
| Similar-Issue Sweep | Inspect each prepared-entry caller, ROM source/alias/rollback path, trace emission caller and board/firmware adaptation for illicit Core-to-board dependency. |

## T540 S36 Acceptance

Actual-diff review accepts NXVM P1 `53fdcef56`. The
[S36 source audit](../../etc/evidence/t540-s36-entry-rom-trace-boundary.md)
qualifies prepared entry, immutable ROM and bounded trace by state owner and
caller direction. The only concrete implementation cleanup removes one unused
board helper include from ROM mapping; no algorithm or public contract was
changed. The three files still compile against the present mixed `machine.h`,
which S37 must ledger and S38 must physically separate; this S does not
claim the Shared move is complete. Final-source x64/x86 units pass **469/469**
per width, specialized gates pass, all eight single-run external boot
checkpoints pass, and eight optimized 0540 products have correct PE width
and no `.debug` sections. No Shared/MyNES, owner INI, protected firmware or
timing grade changed. T540 remains open.

## T540 S37 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S37, next linear S after accepted S36. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Audit the actual S31-S36 source changes and all construction, cold/processor reset, teardown and rollback callers; repair any concrete gap, then freeze a finite neutral Core file ledger for S38. |
| Non-goals | Premature physical Shared move, new generic device framework, second construction/reset/destruction route, profile/clock/firmware/media/INI behavior change. |
| Reference Baseline | S36 P1 `53fdcef56`, [S36 evidence](../../etc/evidence/t540-s36-entry-rom-trace-boundary.md), S31-S35 evidence, dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../../etc/architecture/t540-board-adapter-ledger.md). |
| Files And ABI Surface | Inspect S31-S36 source diffs, `src/app-nxvm/devices/{machine.c,machine.h,machine_plan.c,machine_board.c,entry_plan_interface.c,rom_mapping_interface.c,trace_interface.c}` and every direct caller/test; enumerate neutral Core files, required headers and exact board adapters for S38. |
| Applicable Rules | One frozen board plan before allocation, one neutral constructor plus board phase, exact cold reset and CPU-only reset, one public destroy, owner-only ROM release, typed board signals/deadlines and no Core-to-profile dependency. |
| Verification | Source/diff/caller and dependency audit; focused regressions for any correction, full x64/x86 repository-only units, specialized/documentation gates, one boot per profile/width and eight optimized 0540 products if code changes; source-only evidence and governance if not. |
| Expected Markers | Finite list of neutral Core files and direct test receivers, explicit retained board files/adapters, no unresolved mixed-owner branch or duplicate path, and a concrete S38 move order. |
| Asset Needs | Existing owner-provided external firmware/media only if corrections require boot checks; no new protected input or owner INI change. |
| Reporting Requirements | Record each S31-S36 owner verdict and caller/rollback finding, actual correction diff or no-diff reason, frozen file ledger, test/artifact decision and remaining physical dependency. |
| Stop Conditions | Stop before a guessed Shared dependency, broad header rewrite without per-field owner proof, Shared/MyNES edit or silent behavioral downgrade. |
| Exit Criteria | Every S31-S36 path and dependent caller is audited, concrete defects repaired/verified, finite S38 relocation ledger recorded, and actual-diff P1 plus governance P2 close S37. |
| Original Owner Request | Establish neutral `x86/core` and flat reusable IBM-PC board layers with no divergence or patch-over-patch implementation. |
| Similar-Issue Sweep | Check all four profile compositions, early failures, ROM aliases, firmware callbacks, 8042 processor pulse, D4/XT/AT wiring and all direct tests for misplaced ownership. |

## T540 S37 Acceptance

Source-only P1 `1e68c2e9f` records the
[S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md).
The review found no new guest-behavior regression in S31-S36: frozen plan,
single construction/reset/destruction routes and ROM rollback remain intact.
It also found that the current mixed `machine.c`/`machine.h` cannot be moved
blindly: F0000h alias policy, firmware-less reset fallback, D4 shutdown,
planar memory veto, XT/8042 input dispatch and board clock instances still
live across the intended private-state cut. These are explicitly allocated
to bounded S38-S42; the physical neutral move is now prospective S43, with
board extraction S44 onward. This is a relocation-precondition finding, not
a claim that S38-S43 are already implemented. No source, Shared/MyNES, INI,
firmware, media or executable input changed. `git diff --check` and NXVM
documentation governance passed; S36's 469/469 per width and 8/8 boots remain
the last code baseline. T540 stays open.

## T540 S38 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S38, next linear S after accepted S37. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Put the IBM-PC F0000h firmware role and CPU high reset-alias composition with board/firmware assembly, leaving generic Core immutable ROM registration and operation guarding untouched. |
| Non-goals | High-ROM RAM fallback (S39), D4 shutdown/input dispatch (S40), clock/state split (S41-S42), physical Shared move (S43), ROM byte or CPU reset semantics change, second provider binding. |
| Reference Baseline | S37 P1 `1e68c2e9f`, [S37 ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md), S36 dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c,machine_firmware.c,machine.h,rom_mapping_interface.c}` and firmware/ROM alias tests. Relocate the existing F0000h alias derivation and binding continuation by owner; keep one public firmware-provider bind and the existing Core mapping operations. |
| Applicable Rules | Firmware supplies copied ordinary ROM bytes; board selects the F0000h role and high reset-vector alias; Core owns one immutable mapping table, reset-overlay priority and reverse rollback. Preserve 8086/8088/80186 no-high-alias and 80286/80386 alias behavior plus success-only publication. |
| Verification | Focused firmware bind/rollback, reset-ROM alias, immutable mapping and four-profile tests; full x64/x86 repository-only units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | No F0000h board role in the neutral firmware operation implementation; no second alias table, provider bind, firmware invocation or ROM copy. |
| Asset Needs | Existing owner-provided external firmware/media for fixed boot checks only; no new protected input or owner INI change. |
| Reporting Requirements | Record exact moved functions/callers, ROM alias and failure rollback equivalence, tests/artifact hashes and retained S39-S43 work. |
| Stop Conditions | Stop before a new firmware framework, changed reset mapping, Shared/MyNES edit or moving mixed private state under an unproven header. |
| Exit Criteria | Board/firmware composition solely owns the F0000h role and alias choice, Core solely owns mapping storage/rollback, behavior and checks pass, and actual-diff P1 plus governance P2 close S38. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique state/data ownership and no patch-over-patch path. |
| Similar-Issue Sweep | Check every firmware bind/reset, 286/386 alias, fallback/no-firmware test and failure rollback caller across four profiles. |

## T540 S38 Acceptance

Actual-diff review accepts NXVM P1 `69057d2e9`. The
[S38 evidence](../../etc/evidence/t540-s38-board-firmware-alias.md) records
the F0000h source/alias functions and one public firmware bind continuation
at the board owner. The two old failure-cleanup branches collapsed to one
ROM rollback and provider revocation. Generic firmware operation guards and
the immutable ROM mapping table remain at Core; no extra binding or alias
route exists. Production code net deletes 13 lines. Final-source x64/x86
units pass **469/469** per width, specialized gates pass, all eight single-run
external boot checkpoints pass, and eight optimized 0540 products have
correct PE width and no `.debug` sections. Shared/MyNES, owner INIs,
protected firmware and timing grades are unchanged. S39 receives the
distinct RAM fallback/resize boundary; T540 remains open.

## T540 S39 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S39, next linear S after accepted S38. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Move the firmware-less high reset RAM alias and the planar-parity memory-reconfiguration veto to board composition while keeping Core's checked memory-route and allocation mechanics single-owned. |
| Non-goals | Firmware ROM alias already accepted S38, D4 shutdown/input dispatch (S40), named board clocks/private-state split (S41-S42), physical Shared move (S43), new asset/ROM inputs or memory-size policy change. |
| Reference Baseline | S38 P1 `69057d2e9`, [S38 evidence](../../etc/evidence/t540-s38-board-firmware-alias.md), [S37 ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c,machine.h,memory_interface.c}` and firmware-less reset/memory-reconfigure tests. Move the existing high mapping composition to the board-create phase before publication; separate board veto from neutral RAM route/allocation only where needed. |
| Applicable Rules | For 286/386 with at least 1 MiB, preserve the original high physical alias to F0000h backing and its failure status; 8086/8088/80186 remain unchanged. Do not duplicate RAM bytes. Parity-configured board continues to reject resizing, and Core alone validates mapping bounds and performs allocation/cold reset. |
| Verification | Focused reset-ROM fallback, memory resize, planar parity and partial-create rollback tests; full x64/x86 repository-only units, specialized/documentation gates, one external boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | No F0000h board-address composition or planar-parity field check inside the eventual neutral Core portion; one Core memory reconfigure operation and one board veto, no shadow memory state. |
| Asset Needs | Existing owner-provided external firmware/media for boot checks only; no new protected input or owner INI change. |
| Reporting Requirements | Record exact alias/reset/resize order, failure rollback, focused/full checks, artifact hashes and retained S40-S43 work. |
| Stop Conditions | Stop before a second memory map, guessed profile frequency, Shared/MyNES edit, changed RAM reset contents or a broad private-state move outside this S. |
| Exit Criteria | Board-only address/veto choices no longer reside in neutral Core flow, checked route/allocation behavior is unchanged, verification passes, and actual-diff P1 plus governance P2 close S39. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique owners and no patch-over-patch paths. |
| Similar-Issue Sweep | Check no-firmware fixtures, 8086/286/386 reset vectors, memory resize callers, parity/D4 topology and early construction failures across four profiles. |

## T540 S39 Acceptance

Actual-diff review accepts NXVM P1 `d96502c6c`. The [S39 evidence](../../etc/evidence/t540-s39-board-memory-boundary.md)
records the unchanged high reset RAM alias and failure rollback after its move
to board creation, and the single public resize operation with a board-owned
planar-parity veto and private Core memory operation. Production code net adds
15 lines for this owner seam; no duplicate RAM or route exists. Final-source
x64/x86 units pass **469/469** per width, specialized gates pass, and all
eight single-run external boot checkpoints pass. Eight optimized 0540
products have correct PE width and no `.debug` sections. Shared/MyNES,
owner INIs, protected firmware and timing grades are unchanged. S40 receives
the distinct D4/input boundary; T540 remains open.

## T540 S40 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S40, next linear S after accepted S39. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Move the D4 processor-shutdown reset choice and XT/8042 native keyboard input dispatch behind the IBM-PC board owner while preserving the one neutral Core execution/input path. |
| Non-goals | CPU shutdown generation or reset implementation, KBC/XT chip semantics, new input queue, S41 named clock split, S42 private-state split, S43 physical Shared move, firmware/assets/INI changes. |
| Reference Baseline | S39 P1 `d96502c6c`, [S39 evidence](../../etc/evidence/t540-s39-board-memory-boundary.md), [S37 ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c,machine.h}` and D4/XT/KBC callers/tests. Keep the existing public input ABI, move only board dispatch implementation, and use one bounded board choice for shutdown without copying board state. |
| Applicable Rules | Core owns CPU shutdown consumption and processor-only reset; board owns whether D4 converts shutdown to reset. XT PPI and AT 8042 retain their exact event ordering and one input route. Do not expose chip or board pointers to neutral Core. |
| Verification | Focused D4 shutdown and native keyboard input tests; full x64/x86 repository-only units, specialized/documentation gates, one external boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | Neutral Core run no longer branches on D4; `machine.c` has no public native input dispatch. The mixed board constructor remains explicitly assigned to S42. There is no mirrored state or new public API. |
| Asset Needs | Existing owner-provided external firmware/media for boot checks only; no new protected input or owner INI change. |
| Reporting Requirements | Record exact CPU shutdown priority, input dispatch/effect order, focused/full checks, artifact hashes and retained S41-S43 work. |
| Stop Conditions | Stop before a second input route/queue, new board framework, changed CPU reset behavior, Shared/MyNES edit or a broad private-state move outside this S. |
| Exit Criteria | D4 and XT/8042 board decisions have one owner, Core CPU execution stays neutral, behavior and checks pass, and actual-diff P1 plus governance P2 close S40. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique state/data ownership and no patch-over-patch path. |
| Similar-Issue Sweep | Check D4 shutdown/reset, non-D4 stop, XT scan delivery and 8042 keyboard/mouse input callers and four-profile boots. |

## T540 S40 Acceptance

Actual-diff review accepts NXVM P1 `399dad3e3`. The
[S40 evidence](../../etc/evidence/t540-s40-board-shutdown-input.md) records the
single board-owned D4 shutdown-reset choice and the verbatim relocation of
five public native input functions. CPU shutdown priority and processor-only
reset, XT/8042 dispatch, scan-set observation and mouse rejection remain
unchanged. The scheduler probe exposed and resolved an incorrect first
callback-owner choice before acceptance; the corrected callback receives a
`const core_machine *`, not the replaceable scheduler probe owner. Production
source net adds 12 lines. Final-source x64/x86 units pass **469/469** per
width, specialized gates pass, all eight single-run external boot checkpoints
pass, and eight optimized 0540 products have correct PE width and no `.debug`
sections. Shared/MyNES, owner INIs and protected inputs are untouched.
S41 receives the distinct clock-domain boundary; T540 remains open.

## T540 S41 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S41, next linear S after accepted S40. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Place the six actually named board-device clock-domain initialization/reset operations (DMA, PIT, auxiliary PIT, RTC, VADP and KBC) with the board advance/deadline owner while retaining Core's sole guest timeline and provider clock. FDC/HDC have no independent named clock domains. |
| Non-goals | Changing clock ratios, oscillator sources, device deadlines, CPU retirement timing, profile frequency choices, S42 private-state split, S43 physical Shared move, firmware/assets/INI changes. |
| Reference Baseline | S40 P1 `399dad3e3`, [S40 evidence](../../etc/evidence/t540-s40-board-shutdown-input.md), [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). |
| Files And ABI Surface | Inspect `src/app-nxvm/devices/{machine.c,machine_board.c,machine_scheduler.c,board_advance.c,board_deadline.c,machine.h}` and clock-domain unit/profile tests. Move the existing named-device clock initialization/reset calls by owner only; retain the one current time axis and provider clock. |
| Applicable Rules | Board owns each named device clock domain and its reset order; Core owns guest tick settlement, CPU retirement and provider time. No new clock copy, implicit conversion or fabricated physical rate. |
| Verification | Focused clock/reset/deadline and four-profile tests; full x64/x86 repository-only units, specialized/documentation gates, one external boot per profile/width and eight optimized 0540 products with PE/hash/no-debug proof. |
| Expected Markers | No named board-device clock initialization/reset in the neutral Core portion; one exact clock-domain owner and no changed rate/deadline formula. |
| Asset Needs | Existing owner-provided external firmware/media for boot checks only; no new protected input or owner INI change. |
| Reporting Requirements | Record the exact clock fields, initialization/reset order, failure rollback, tests/artifact hashes and retained S42-S43 work. |
| Stop Conditions | Stop before a second clock axis, unverified frequency, Shared/MyNES edit, broad private-state move or changed controller timing beyond the declared owner cut. |
| Exit Criteria | Named device clocks have board ownership, Core time remains sole and unchanged, behavior and checks pass, and actual-diff P1 plus governance P2 close S41. |
| Original Owner Request | Establish neutral `x86/core` and reusable IBM-PC board layers with unique clock and state owners, without patch-over-patch paths. |
| Similar-Issue Sweep | Check every named clock create/reset caller, reset order, PIT/RTC/DMA/KBC/FDC/HDC deadline mapping and four-profile boots. |

## T540 S41 Acceptance

Actual-diff review accepts NXVM P1 `b33ee1220`. The
[S41 evidence](../../etc/evidence/t540-s41-board-clock-domains.md) records
the six existing device-clock domains moved to their board advance owner
without changing ratios, initialization/reset order, phase or deadline
formula. Core retains its single guest timeline and provider clock. FDC/HDC
were verified not to have separate named clock domains; none were invented.
The T388 physical-timebase inventory now reads the moved source while
retaining all required checks. Production source net adds 20 lines for the
bounded owner operations. Final-source x64/x86 units pass **469/469** per
width, specialized gates pass, all eight single-run external boot checkpoints
pass, and eight optimized 0540 products have correct PE width and no `.debug`
sections. Shared/MyNES, owner INIs and protected inputs are untouched.
T540 remains open.

## T540 S42 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S42, next linear S after accepted S41. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Inspect every remaining mixed `machine.h` field, `machine.c` function, provider callback and direct test consumer; establish exact neutral Core versus IBM-PC board ownership and divide the oversized private-state/source split into finite linear receivers before moving code. |
| Non-goals | Blindly moving the large mixed source/private header, modifying CPU/chip semantics, creating mirrored state or a second machine, changing timing/ROM/INI, or physically moving to Shared before the private split is proven. |
| Reference Baseline | S41 P1 `b33ee1220`, [S41 evidence](../../etc/evidence/t540-s41-board-clock-domains.md), [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). S37's S43 physical move is prospective and must be renumbered only after the actual bounded split receivers are known. |
| Files And ABI Surface | Read `src/app-nxvm/devices/{machine.c,machine.h,machine_board.c,machine_plan.c,board_advance.c,board_deadline.c}` and every direct `machine.h` include/test. Create an owner/receiver ledger; do not edit Shared/MyNES. |
| Applicable Rules | One machine, one constructor/publication, one guest-time axis. Core owns CPU, memory/port transaction, timeline and execution; board owns chip instances, topology, named clocks and wiring. No cross-layer pointer leak or copied fact. |
| Verification | Source and caller sweep, owner matrix, `git diff --check` and NXVM documentation governance. No product binary rebuild for a source-only intake. Actual later split receivers require full dual-width units, gates, boots and eight products. |
| Expected Markers | A finite field/function/test allocation, explicit single-owner dataflow and linear S numbers; no new code path in this intake. |
| Asset Needs | None for read-only source inventory. No protected input or owner INI change. |
| Reporting Requirements | Record current field/function counts, direct include/caller surface, exact neutral/board receiver sizes and revised physical-move sequence. |
| Stop Conditions | Stop before moving a mixed private structure or function without an exact owner, second state copy, new generic framework or Shared/MyNES edit. |
| Exit Criteria | The complete remaining mixed state/source is assigned to bounded linear S receivers with no unallocated rows, governance passes, and S42 source-only P1 plus P2 closure is recorded. |
| Original Owner Request | Prepare reusable neutral x86 Core and IBM-PC board mechanisms so four fixed PC Apps and future PC110 can share them without divergent machine paths. |
| Similar-Issue Sweep | Check every `machine.h` direct include, constructor/reset/run/destroy call, board callback and product test fixture for hidden board-state coupling. |

## T540 S42 Acceptance

Source-only NXVM P1 `78974cff4` records the
[private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md).
The audit found one mixed `core_machine` private structure and a mixed
`machine.c`; 18 production sources under `src/app-nxvm/devices` and 114
product test files/fixture headers directly include the private header.
Every contiguous state group and remaining mixed constructor/callback stage
has one Core or board owner and a numeric receiver. S43 handles constructor
and callback extraction, S44-S47 the board attachment/private-state split,
and S48 the now-prospective physical neutral move. The earlier S37 S43 move
was a plan, never executed. No code, test, Shared/MyNES, binary or owner
asset changed. `git diff --check` and NXVM documentation governance pass;
S41's dual-width 469/469 units and eight boot checkpoints remain the last
code baseline. T540 remains open.

## T540 S43 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S43, next linear S after accepted S42. |
| Admission And Approval | Owner's standing automatic admission for bounded numeric T540 S work applies. Target NXVM only; Shared and MyNES stay read-only. |
| Objective | Put the remaining XT/KBC/PPI callbacks, board-config validation and sole board constructor in the existing `machine_board.c` owner; keep Core `machine.c` to neutral construction and lifecycle orchestration. |
| Non-goals | Private-state field move (S44-S47), physical Shared relocation (S48), new board framework, second machine/create route, timing or chip behavior change. |
| Reference Baseline | S42 P1 `78974cff4`, [S42 ledger](../../etc/architecture/t540-s42-private-state-ledger.md), S41 dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 ownership ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | `src/app-nxvm/devices/{machine.c,machine_board.c,machine.h}` plus exact static inventories/tests that name moved functions. Internal linkage only; public create/configuration ABI unchanged. |
| Applicable Rules | One candidate allocation, one board port checkpoint, unchanged chip order, one success publication and one rollback/destructor. XT/8042 signaling remains board-owned. |
| Verification | Full x64/x86 repository-only units, specialized/documentation gates, one external boot per fixed profile/width, eight optimized 0540 products with PE/hash/no-debug evidence; actual-diff review before P1/P2 closure. |
| Expected Markers | No board-specific constructor or keyboard/PPI callback definition in neutral `machine.c`; no second create or board state mirror. |
| Asset Needs | Existing owner-provided firmware/media only for unchanged boot checks; no new input or owner INI edit. |
| Reporting Requirements | Exact moved functions/lines, constructor failure and callback-order equivalence, tests/artifact hashes, retained S44-S48 work. |
| Stop Conditions | Stop before a generic framework or unsafe private-state move; split into next linear numeric receiver if actual diff exceeds one owner boundary. |
| Exit Criteria | One board constructor owns all remaining board callbacks/validation; neutral create invokes it once; dual-width verification and artifacts pass; actual P1 and governance P2 are pushed with a clean tree. |
| Original Owner Request | Extract independent chips and board mechanisms without diverging the four fixed PC products. |
| Similar-Issue Sweep | Check XT and AT keyboard creation/reset, PPI speaker/NMI, port registration rollback, destruction after partial board creation and all four profile boots. |

## T540 S43 Acceptance

Actual-diff review accepts NXVM P1 `8645c1c74`; the
[S43 evidence](../../etc/evidence/t540-s43-board-constructor.md) records the
245-line move from `machine.c` to the existing board owner, two private
declarations, and eight static gates retargeted to that owner without
relaxing their checks. One resolved-memory-size read replaced a duplicate
configuration calculation after successful allocation. No parallel create,
board registration, rollback or destructor route appeared. Final-source
x64/x86 repository-only units pass **469/469** each, specialized gates pass,
and the four fixed-profile external boot probes pass **8/8**, once per width.
Eight optimized 0540 products have the expected PE widths and no `.debug`
sections, with hashes in the evidence. Shared/MyNES, owner INIs and
firmware/media inputs are untouched. T540 remains open.

## T540 S44 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S44, next linear S after accepted S43. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Establish one board-owned attachment for named device clocks and the actual topology/timing fields now flat in `core_machine`, moving each fact once and keeping the Core clock/timeline separate. |
| Non-goals | Chip-instance and IRQ/port move (S45), D4/refresh/callback split (S46), neutral private header completion (S47), physical Shared move (S48), new device framework or changed time formula. |
| Reference Baseline | S43 P1 `8645c1c74`, [S43 evidence](../../etc/evidence/t540-s43-board-constructor.md), [S42 ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Audit actual references to six named device clocks, KBC timing/input, keyboard/display topology, DMA bindings, RTC/CMOS and FDC/HDC topology across board, controller, plan and tests before editing. Add one owner attachment only if every moved reader/writer has one route. |
| Applicable Rules | No copied clock phase, no dual topology state, no board pointer inside CPU/chip, one success publication and one rollback. Core keeps provider clock and single guest timeline; board owns device clocks. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | Named clocks/topology no longer occupy flat Core state; board attachment has one lifetime and no parallel path. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI modification. |
| Reporting Requirements | Exact moved fields, direct readers/writers, constructor/reset/finalization order, tests and artifact hashes; allocate any oversized remainder to the next linear numeric S before implementation. |
| Stop Conditions | Stop before creating a second state owner, mirroring a clock phase, altering board timing ratios or moving a mixed header into Shared. |
| Exit Criteria | Actual bounded state move has one board owner, all callers and fixtures use it, dual-width validation passes, P1/P2 pushed and tree clean. |
| Original Owner Request | Make independent x86 Core and reusable IBM-PC boards without diverging four PC products. |
| Similar-Issue Sweep | Audit every named clock consumer, configuration/reset path, deadline/advance callback, topology reader and affected private fixture. |

## T540 S44 Acceptance

Actual-diff review accepts NXVM P1 `784e95e40`. The
[S44 evidence](../../etc/evidence/t540-s44-board-state.md) records 27 board
clock/topology/timing fields moved from flat `core_machine` storage to one
board-owned attachment. Allocation precedes board clock/device setup;
finalization releases that attachment after chip teardown, including
partial-construction rollback. Core provider clock and timeline, chip
algorithms, frozen plan, public ABI and VM's separate FDC binding did not
change. Complete x64/x86 repository-only units pass **469/469** each,
specialized gates pass, and the fixed-profile external boot matrix passes
**8/8**, once per profile/width. All eight optimized 0540 products have
the expected PE widths and no `.debug` sections. Shared/MyNES and owner
INIs are untouched. T540 remains open.

## T540 S45 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S45, next linear S after accepted S44. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the PIC pair and its PIT0/RTC IRQ-source bindings from flat `core_machine` to the sole board attachment, preserving one PIC lifetime and callback/registration order. |
| Non-goals | Other chip groups (S46–S52), D4/refresh/board callback split (S53), final neutral private header (S54), physical Shared move (S55), new device framework or chip/timing behavior change. |
| Reference Baseline | S44 P1 `784e95e40`, [S44 evidence](../../etc/evidence/t540-s44-board-state.md), [S42 private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, specialized gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 ownership ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | PIC state is read in three production and fifty direct test files. The measured [receiving plan](../../etc/architecture/t540-s42-private-state-ledger.md) assigns PIT, DMA, RTC, FDC, HDC, keyboard and VADP to S46–S52 before code. S45 edits only the PIC group, its direct fixtures and exact source inventories. |
| Applicable Rules | One board attachment, no mirrored chip or IRQ state, one registration/rollback route; Core CPU, memory, transaction and timeline stay neutral. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | PIC pair and two PIC IRQ-source bindings no longer occupy flat Core storage; board constructor/finalizer alone own their lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Inventory and exact moved fields, constructor/reset/finalization equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip group into S45, a second PIC owner, altered wiring or unsupported Shared import. |
| Exit Criteria | PIC owner move is complete, tests and artifacts pass, P1/P2 pushed and tree clean; S46–S55 retain their recorded receivers. |
| Original Owner Request | Prepare independently reusable x86 Core and IBM-PC boards without diverging the four PC products. |
| Similar-Issue Sweep | PIC/PIT/DMA/RTC/FDC/HDC/KBC/XT PPI/XT keyboard/VADP chip lifetime, IRQ signals, port registration and failed construction. |

## T540 S45 Acceptance

Actual-diff review accepts NXVM P1 `c47f4fb7d`. The
[S45 evidence](../../etc/evidence/t540-s45-pic-owner.md) records the PIC
master/slave and PIT0/RTC IRQ-source bindings moved into the sole board
attachment, with unchanged construction/reset/IRQ/port/destruction calls.
Core CPU bus still sees only bounded PIC callbacks. Complete x64/x86
repository-only units pass **469/469** each, specialized gates pass, and
the four fixed-profile external boot probes pass **8/8**, once per width.
Eight optimized 0540 products have the expected widths and no `.debug`
sections; hashes are in the evidence. The large direct-test diff reflects
field references, not a parallel PIC implementation. Shared/MyNES and
owner INIs are untouched. T540 remains open.

## T540 S46 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S46, next linear S after accepted S45. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the PIT pair and auxiliary-PIT configured state from flat `core_machine` into the sole board attachment; preserve one PIT lifetime and Gate/OUT/IRQ/refresh wiring. |
| Non-goals | DMA, RTC, FDC, HDC, keyboard and VADP (S47–S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), chip/timing behavior change. |
| Reference Baseline | S45 P1 `c47f4fb7d`, [S45 evidence](../../etc/evidence/t540-s45-pic-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | PIT state has four production and eight direct test file consumers. Retarget only these, the board constructor/reset/deadline/advance/finalizer, and exact source inventories. |
| Applicable Rules | One board attachment, no copied PIT phase or duplicated Gate/OUT state; keep chip callback order, IRQ0 and DMA refresh signal wiring unchanged. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | PIT pair and auxiliary configured state no longer occupy flat Core storage; board alone owns chip lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved fields, references, callback/registration/reset/finalization equivalence, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip into S46, changing PIT timing or adding a second owner/access path. |
| Exit Criteria | PIT owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Both PIT instances, IRQ0, refresh, speaker, auxiliary PIT, board reset and failed construction. |

## T540 S46 Acceptance

Actual-diff review accepts NXVM P1 `5a34e2acf`. The
[S46 evidence](../../etc/evidence/t540-s46-pit-owner.md) records the primary
and auxiliary PIT pair plus configured bit moved into the sole board
attachment. Frozen plan ratios and all Gate/OUT, IRQ0, DMA refresh,
speaker, auxiliary PIT, reset and finalization behavior remain unchanged.
Complete x64/x86 repository-only units pass **469/469** each; specialized
gates pass. The four fixed-profile external boot probes pass **8/8**, once
per width. Eight optimized 0540 products have the expected PE widths and no
`.debug` sections; hashes are in the evidence. Shared/MyNES and owner INIs
are untouched. T540 remains open.

## T540 S47 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S47, next linear S after accepted S46. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the DMA latch and primary/secondary controller instances from flat `core_machine` into the sole board attachment; preserve one DMA lifetime and all DRQ/refresh/terminal-count wiring. |
| Non-goals | RTC, FDC, HDC, keyboard and VADP (S48–S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), DMA behavior or timing change. |
| Reference Baseline | S46 P1 `5a34e2acf`, [S46 evidence](../../etc/evidence/t540-s46-pit-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Current DMA-instance references occur in three production and thirteen direct test files. Retarget only these and their exact source inventories. |
| Applicable Rules | One board attachment, no copied DMA phase or controller state; keep chip registration, request/acknowledge, refresh, reset and destruction order unchanged. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | DMA latch and both controller instances no longer occupy flat Core storage; board alone owns their lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved fields and references, constructor/reset/finalization equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip into S47, altering DMA semantics, or adding a second owner/access path. |
| Exit Criteria | DMA owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Primary/secondary DMA, latch, refresh, FDC/HDC DRQ, terminal count, board reset and failed construction. |

## T540 S47 Acceptance

Actual-diff review accepts NXVM P1 `61a53ec3e`. The
[S47 evidence](../../etc/evidence/t540-s47-dma-owner.md) records the DMA latch
and primary/secondary controllers moved into the sole board attachment.
Construction, request/acknowledge, refresh, FDC/HDC DRQ, deadline, reset and
finalization calls retain the existing order and chip behavior. Complete
x64/x86 repository-only units pass **469/469** each; specialized gates pass.
The four fixed-profile external boot probes pass **8/8**, once per width.
Eight optimized 0540 products have the expected PE widths and no `.debug`
sections; hashes are in the evidence. Shared/MyNES and owner INIs are
untouched. T540 remains open.

## T540 S48 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S48, next linear S after accepted S47. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the RTC chip instance and selected-register latch from flat `core_machine` into the sole board attachment; preserve one RTC lifetime and existing CMOS/IRQ/deadline wiring. |
| Non-goals | FDC, HDC, keyboard and VADP (S49–S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), RTC behavior or time formula change. |
| Reference Baseline | S47 P1 `61a53ec3e`, [S47 evidence](../../etc/evidence/t540-s47-dma-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Current RTC-instance/latch references occur in three production and ten direct test files. Retarget only these and exact source inventories. |
| Applicable Rules | One board attachment, no copied RTC/CMOS phase or selected-register state; keep reset, port, IRQ and finalization order unchanged. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | RTC chip and selected register no longer occupy flat Core storage; board alone owns their lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved fields and references, constructor/reset/finalization equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip into S48, altering RTC semantics, or adding a second owner/access path. |
| Exit Criteria | RTC owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | RTC/CMOS port selection, IRQ8, deadline, reset, board seed and failed construction. |

## T540 S48 Acceptance

Actual-diff review accepts NXVM P1 `15fb83f58`. The
[S48 evidence](../../etc/evidence/t540-s48-rtc-owner.md) records the RTC chip
pointer and selected-register latch moved into the sole board attachment.
Port installation/rollback, seed, deadline, advance, IRQ, reset and
finalization calls retain the existing order and chip behavior. The
multi-session test now compares actual RTC instances instead of the addresses
of pointer slots. Complete x64/x86 repository-only units pass **469/469**
each; specialized gates pass. The four fixed-profile external boot probes
pass **8/8**, once per width. Eight optimized 0540 products have the
expected PE widths and no `.debug` sections; hashes are in the evidence.
Shared/MyNES and owner INIs are untouched. T540 remains open.

## T540 S49 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S49, next linear S after accepted S48. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the FDC controller instance from flat `core_machine` into the sole board attachment; preserve one FDC lifetime and its DMA/IRQ/media-change wiring. |
| Non-goals | HDC, keyboard and VADP (S50–S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), FDC chip behavior or timing change. |
| Reference Baseline | S48 P1 `15fb83f58`, [S48 evidence](../../etc/evidence/t540-s48-rtc-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | FDC-instance access spans three production files, twenty direct C test files and one test-boundary script. `topology->fdc` in the frozen plan is a distinct configuration value and must not be moved. |
| Applicable Rules | One board attachment, no copied FDC command/DRQ/IRQ state; preserve controller construction, media binding, port routes, reset, DMA request and finalization order. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | The FDC controller no longer occupies flat Core storage; board alone owns its lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved field/references, constructor/reset/finalization equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip into S49, altering FDC semantics, or adding a second owner/access path. |
| Exit Criteria | FDC owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | FDC ports, drive selection, DMA/IRQ, media-change observation, deadline, reset and failed construction. |

## T540 S49 Acceptance

Actual-diff review accepts NXVM P1 `7e21979dd`. The
[S49 evidence](../../etc/evidence/t540-s49-fdc-owner.md) records the sole FDC
controller instance moved into the board attachment, with frozen topology
and integration observation kept distinct. DMA/IRQ/media-change wiring,
ports, deadline, reset, failed-construction rollback and finalization retain
their existing order. Complete x64/x86 repository-only units pass **469/469**
each; specialized gates pass. The four fixed-profile external boot probes
pass **8/8**, once per width. Eight optimized 0540 products have the
expected PE widths and no `.debug` sections; hashes are in the evidence.
Shared/MyNES and owner INIs are untouched. T540 remains open.

## T540 S50 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S50, next linear S after accepted S49. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the HDC controller instance from flat `core_machine` into the sole board attachment; preserve ATA, XT Xebec and Model-40 personalities on their existing one-controller path. |
| Non-goals | Keyboard and VADP (S51-S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), HDC personality, media or timing changes. |
| Reference Baseline | S49 P1 `7e21979dd`, [S49 evidence](../../etc/evidence/t540-s49-fdc-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Inventory direct HDC-instance consumers before changing source. The frozen plan's HDC configuration and integration observers are distinct values, not controller instances. Keep public chip and Core interfaces unchanged. |
| Applicable Rules | One board attachment, no copied command/DRQ/IRQ/media state; preserve construction, route installation, reset, deadline, media service and destruction order across all three personalities. |
| Verification | Full x64/x86 units, specialized/documentation gates, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | HDC controller no longer occupies flat Core storage; board alone owns its lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved field/references, personality and failure-path equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling another chip into S50, altering HDC semantics, or adding a second owner/access path. |
| Exit Criteria | HDC owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | ATA, Xebec and Model-40 route/IRQ/DRQ/media/deadline/reset/finalization consumers. |

## T540 S50 Acceptance

Actual-diff review accepts NXVM P1 `997d28483`. The
[S50 evidence](../../etc/evidence/t540-s50-hdc-owner.md) records the sole HDC
controller instance moved into the board attachment. ATA, XT Xebec and
Model-40 personalities keep their existing ports, media, DRQ/IRQ, deadline,
reset and failure rollback. Complete x64/x86 repository-only units pass
**469/469** each; specialized gates and focused HDD/ATA integration pass.
The four fixed-profile external boot probes pass **8/8**, once per width.
Eight optimized 0540 products have the expected PE widths and no `.debug`
sections; hashes are in the evidence. Shared/MyNES and owner INIs are
untouched. T540 remains open.

## T540 S51 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S51, next linear S after accepted S50. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the AT KBC and XT PPI-keyboard instance state from flat `core_machine` into the sole board attachment, preserving their separate hardware identities and one input/IRQ/reset path per topology. |
| Non-goals | VADP (S52), board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), keyboard command, BAT, input or timing behavior changes. |
| Reference Baseline | S50 P1 `997d28483`, [S50 evidence](../../etc/evidence/t540-s50-hdc-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates, focused HDD/ATA integration and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Inventory direct KBC and XT keyboard instance consumers before changing source; keep frozen topology and copied input events distinct from live chip state. Public Core and chip interfaces remain unchanged. |
| Applicable Rules | One board attachment, no copied command/output/BAT/typematic or XT PPI state; preserve constructor, port/input routing, IRQ, reset, deadline and teardown order. |
| Verification | Full x64/x86 units, specialized/documentation gates, keyboard-focused regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | KBC and XT keyboard instances no longer occupy flat Core storage; board alone owns their lifetimes. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved fields/references, AT/XT event and failure-path equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling VADP into S51, altering keyboard semantics, or adding a second owner/access path. |
| Exit Criteria | Keyboard owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | 8042 command/output/BAT/IRQ and XT PPI-keyboard input/reset/deadline consumers. |

## T540 S51 Acceptance

Actual-diff review accepts NXVM P1 `b28f48550`. The
[S51 evidence](../../etc/evidence/t540-s51-keyboard-owner.md) records both AT
8042 and XT PPI-keyboard instances moved into the sole board attachment,
without merging their distinct hardware identities. Port/input dispatch,
IRQ/NMI/speaker wiring, reset, deadline and teardown retain the existing
order. Complete x64/x86 repository-only units pass **469/469** each;
specialized gates pass. The four fixed-profile external boot probes pass
**8/8**, once per width. Eight optimized 0540 products have the expected PE
widths and no `.debug` sections; hashes are in the evidence. Shared/MyNES
and owner INIs are untouched. T540 remains open.

## T540 S52 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S52, next linear S after accepted S51. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the sole VADP instance from flat `core_machine` into the board attachment while preserving one guest video state, memory/port mapping and copied snapshot owner. |
| Non-goals | Board electrical/callback split (S53), final neutral header (S54), physical Shared move (S55), CGA/EGA/VGA rendering, palette, geometry or timing behavior changes. |
| Reference Baseline | S51 P1 `b28f48550`, [S51 evidence](../../etc/evidence/t540-s51-keyboard-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Inventory direct VADP instance consumers before changing source; keep frozen display topology and copied presentation frames distinct from live chip state. Public Core and chip interfaces remain unchanged. |
| Applicable Rules | One board video owner, no copied mode/VRAM/frame state; preserve construction, port/memory routes, reset, deadline, snapshot and finalization order. |
| Verification | Full x64/x86 units, specialized/documentation gates, display-focused regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | VADP instance no longer occupies flat Core storage; board alone owns its lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Exact moved field/references, CGA/EGA/VGA and rollback equivalence, diff size, tests and artifact hashes. |
| Stop Conditions | Stop before pulling D4/other latches into S52, altering display semantics, or adding a second owner/access path. |
| Exit Criteria | VADP owner move and focused behavior are complete; tests/artifacts pass, P1/P2 pushed and tree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | VADP ports, VRAM mappings, snapshot, frame generation, reset and resource teardown. |

## T540 S52 Acceptance

Actual-diff review accepts NXVM P1 `855045aac`. The
[S52 evidence](../../etc/evidence/t540-s52-vadp-owner.md) records the sole
VADP instance moved into the board attachment while copied snapshots remain
presentation values. Port and VRAM mapping, clock advance, reset and
finalization retain the existing order. Complete x64/x86 repository-only
units pass **469/469** each; specialized gates pass. The four fixed-profile
external boot probes pass **8/8**, once per width. Eight optimized 0540
products have the expected PE widths and no `.debug` sections; hashes are
in the evidence. Shared/MyNES and owner INIs are untouched. T540 remains open.

## T540 S53 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S53, next linear S after accepted S52. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Correct the S51-overlooked XT keyboard chip pointer so board alone owns its creation, input, advance/deadline, reset and destruction. Audit and split the oversized remaining D4, absent-memory, parity, speaker, refresh and callback row into linear numeric receivers before changing those electrical states. |
| Non-goals | The electrical/callback moves (S54-S59), final neutral private header (S60), physical Shared move (S61), new board framework, new chip state, changed keyboard/D4/NMI/refresh/speaker behavior or timing. |
| Reference Baseline | S52 P1 `855045aac`, [S52 evidence](../../etc/evidence/t540-s52-vadp-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Move only the residual XT keyboard chip pointer and its exact consumers; record each other flat board field, owner, consumer, reset and callback lifetime for S54-S59. Core operation guards and copied plan inputs remain distinct from board electrical state. |
| Applicable Rules | One board owner, no copied latch or callback path; preserve D4 shutdown, parity/NMI, refresh HOLD, speaker and absent-memory order with existing Core operation guards. |
| Verification | Full x64/x86 units, specialized/documentation gates, focused electrical regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | XT keyboard chip leaves flat Core storage; each remaining board-owned latch/callback has one bounded numeric receiver, and neutral Core retains only bounded operations. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Field-by-field owner and callback lifecycle, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop and split if the actual field/callback surface exceeds one bounded S; do not move neutral Core guards into the board. |
| Exit Criteria | XT chip owner correction passes tests/artifacts with P1/P2 pushed and tree clean; the electrical/callback receivers are recorded as S54-S59, not falsely claimed complete. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | D4, absent-memory, parity, speaker, refresh and callback install/revoke consumers. |

## T540 S53 Acceptance

The overlooked XT keyboard chip pointer is now board-owned through its one
construction, input, deadline, advance, reset and teardown path. P1
`e313e4660` is pushed. The original combined electrical/callback assignment
was split on the measured dependency surface rather than falsely declared
complete. Dual-width full units pass 469/469, specialized gates pass, and
all eight fixed-profile boots pass once each. The optimized 0540 products are
rebuilt and PE/no-debug verified; the [S53 evidence](../../etc/evidence/t540-s53-xt-keyboard-and-electrical-intake.md)
contains exact hashes and the S54-S61 receiving map. T540 remains open.

## T540 S54 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S54, next linear S after accepted S53. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only planar-parity configuration, Port-B and NMI latch state from flat Core storage into the sole board attachment; preserve the existing port, memory-fault and speaker signal path. |
| Non-goals | D4 platform/NMI (S55), D4 refresh/DMA (S56), XT speaker state (S57), absent-memory routes (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), behavior/timing change. |
| Reference Baseline | S53 P1 `e313e4660`, [S53 evidence](../../etc/evidence/t540-s53-xt-keyboard-and-electrical-intake.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | `machine.h`, board state/port/memory/NMI owners, direct private test consumers and existing static gates. Preserve one checked Core memory operation and the existing board callback lifetime. |
| Applicable Rules | One board owner, no mirror/compatibility branch; preserve parity latch, NMI delivery and Port-B ordering. |
| Verification | Full x64/x86 units, specialized/documentation gates, parity-focused regression, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | No planar-parity mutable state remains flat in `core_machine`; one board state owns its configuration and latches. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Field/consumer inventory, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop for a new parity semantic conflict, second owner or cross-target change; split if the actual surface exceeds a bounded S. |
| Exit Criteria | P1/P2 pushed; both-width verification, eight boots and artifact evidence pass; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Parity memory fault, Port-B read/write, reset, NMI and speaker wiring consumers. |

## T540 S54 Acceptance

All five planar-parity mutable fields are board-owned with no Core mirror.
The memory-fault, Port-B, NMI and speaker wiring order is unchanged. P1
`8712ec968` is pushed. Both-width full units pass 469/469, specialized gates
pass, and all eight fixed-profile boots pass once each. The optimized 0540
products are rebuilt and PE/no-debug verified. The [S54 evidence](../../etc/evidence/t540-s54-planar-parity-owner.md)
records the one x86 timeout under concurrent load, the clean isolated test,
the full clean rerun, exact diff and artifact hashes. T540 remains open.

## T540 S55 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S55, next linear S after accepted S54. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only D4 platform configuration, Port-B, IOCHK/failsafe and NMI latch state to the sole board attachment. Preserve D4 Port-B, failsafe and NMI signal semantics. |
| Non-goals | D4 refresh/DMA state (S56), XT speaker (S57), absent memory (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), D4 timing or behavior change. |
| Reference Baseline | S54 P1 `8712ec968`, [S54 evidence](../../etc/evidence/t540-s54-planar-parity-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Flat D4 platform fields in `machine.h`, board state/port/reset/NMI consumers and direct private tests. Preserve the Core-owned firmware operation guard and neutral memory transaction boundary. |
| Applicable Rules | One board owner, no copied latch, unchanged D4 Port-B and NMI order and failure behavior. |
| Verification | Full x64/x86 units, specialized/documentation gates, D4/parity focused regression, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | No D4 platform/Port-B/NMI mutable state remains flat in `core_machine`; one board state owns its configuration and latches. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Field/consumer inventory, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop for a new D4 semantic conflict, second owner or cross-target change; split if actual surface exceeds one bounded S. |
| Exit Criteria | P1/P2 pushed; both-width verification, eight boots and artifact evidence pass; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | D4 configuration, Port-B read/write, IOCHK/failsafe latches, reset/shutdown and NMI consumers. |

## T540 S55 Acceptance

The six D4 platform/Port-B/NMI fields are board-owned with no Core mirror.
Failsafe PIT, speaker, reset/shutdown, latch-clear and NMI order is unchanged.
P1 `8ca51989a` is pushed. Both-width full units pass 469/469, specialized
gates pass, and all eight fixed-profile boots pass once each. The optimized
0540 products are rebuilt and PE/no-debug verified; the [S55 evidence](../../etc/evidence/t540-s55-d4-platform-owner.md)
records exact diff and artifact hashes. The three D4 refresh request fields
remain for S56. T540 remains open.

## T540 S56 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S56, next linear S after accepted S55. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move D4 refresh pending/pulse/address electrical latches to the sole board attachment while Core continues to service the one bounded HOLD/arbitration operation. Audit DMA refresh producer/consumer wiring for a single owner. |
| Non-goals | XT speaker (S57), absent memory (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), altered DMA grant or guest-time behavior. |
| Reference Baseline | S55 P1 `8ca51989a`, [S55 evidence](../../etc/evidence/t540-s55-d4-platform-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Three flat D4 refresh fields, PIT/DMA board producer and neutral scheduler deadline/arbitration readers. Keep copied request and success-only completion across the Core/board boundary. |
| Applicable Rules | Board owns refresh signal state; neutral Core owns bus grant, HOLD and memory transaction, not a second refresh latch. |
| Verification | Full x64/x86 units, specialized/documentation gates, refresh/DMA focused regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | No D4 refresh electrical latch remains flat in `core_machine`; one board state owns pending, pulse and address. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Producer/consumer and completion audit, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop for a new timing/ownership conflict, second owner or cross-target change; split if actual surface exceeds one bounded S. |
| Exit Criteria | P1/P2 pushed; both-width verification, eight boots and artifact evidence pass; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | PIT refresh output, DMA request, HOLD/acknowledge, deadline, reset and observer consumers. |

## T540 S56 Acceptance

The three D4 refresh electrical fields are board-owned; Core continues to
service only the bounded request/HOLD/transaction/completion contract. P1
`71efa3bb5` is pushed. Both-width full units pass 469/469, the updated
existing refresh ownership verifier and both specialized gate sets pass, and
all eight fixed-profile boots pass once each. The optimized 0540 products
are rebuilt and PE/no-debug verified. The [S56 evidence](../../etc/evidence/t540-s56-d4-refresh-owner.md)
records the gate correction, exact diff and artifact hashes. T540 remains
open.

## T540 S57 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S57, next linear S after accepted S56. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move XT PPI speaker configuration, timer gate, data enable and observed output to the sole board attachment; retain the existing PIT/PPI/Port-B signal path. |
| Non-goals | Absent-memory routes (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), speaker waveform or host-audio behavior change. |
| Reference Baseline | S56 P1 `71efa3bb5`, [S56 evidence](../../etc/evidence/t540-s56-d4-refresh-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Four flat speaker fields, board PPI/PIT/Port-B consumer, reset and copied observation; no host presenter or chip waveform edit. |
| Applicable Rules | One board owner for speaker electrical state; copied observation remains a value, not a second state path. |
| Verification | Full x64/x86 units, specialized/documentation gates, speaker-focused regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | No XT speaker mutable field remains flat in `core_machine`; board alone owns gate, data and output state. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Signal consumer audit, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop for a speaker semantic conflict, second owner or cross-target change; split if actual surface exceeds one bounded S. |
| Exit Criteria | P1/P2 pushed; both-width verification, eight boots and artifact evidence pass; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | XT PPI gate/data, PIT speaker output, Port-B, reset and copied speaker observation. |

## T540 S57 Acceptance

The four XT PPI speaker fields are board-owned; the existing PPI/PIT/Port-B
signal and copied observation paths are unchanged. P1 `1825a1859` is pushed.
Both-width complete units pass 469/469, both specialized gate sets pass,
and all eight fixed-profile boots pass once each. The optimized 0540 products
are rebuilt and PE/no-debug verified. The [S57 evidence](../../etc/evidence/t540-s57-speaker-owner.md)
records the exact diff and artifact hashes. T540 remains open.

## T540 S58 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S58, next linear S after accepted S57. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only the configured absent-memory windows to the sole board attachment while retaining the existing typed Core memory route, open-bus priority and rollback contract. |
| Non-goals | Callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), absent-memory behavior or priority change. |
| Reference Baseline | S57 P1 `1825a1859`, [S57 evidence](../../etc/evidence/t540-s57-speaker-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Existing absent-memory window array and its board-owned configuration/read/write/query consumers; no new route API or fallback policy. |
| Applicable Rules | Board owns physical absent-window configuration; Core owns typed memory arbitration, not a second copy. |
| Verification | Full x64/x86 units, specialized/documentation gates, affected absent-memory regressions, one external boot per profile/width, eight optimized 0540 PE/no-debug products and actual-diff evidence. |
| Expected Markers | No configured absent-memory array remains flat in `core_machine`; board alone owns each window and the route owner pointer retains its lifetime. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Reporting Requirements | Window registration/rollback and pointer-lifetime audit, exact diff, tests and artifact hashes. |
| Stop Conditions | Stop for a changed open-bus priority, second owner or cross-target change; split if actual surface exceeds one bounded S. |
| Exit Criteria | P1/P2 pushed; both-width verification, eight boots and artifact evidence pass; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Absent-window configuration, Core typed memory registration, route owner pointer, open-bus fallback and rollback. |

## T540 S58 Acceptance

The absent-memory slots are board-owned; Core's fallback memory route keeps
its existing priority and a stable slot owner pointer. P1 `930d35bec` is
pushed. Both-width complete units pass 469/469, both specialized gate sets
pass, and all eight fixed-profile boots pass once each. The optimized 0540
products are rebuilt and PE/no-debug verified. The [S58 evidence](../../etc/evidence/t540-s58-absent-memory-owner.md)
records rollback, lifetime, exact diff and artifact hashes. T540 remains open.

## T540 S59 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S59, next linear S after accepted S58. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Audit and correct the existing board callback install/revoke and firmware binding owners so neutral Core only invokes bounded providers and the board retains its F0000h alias choice, without a second lifetime or direct chip access. |
| Non-goals | Neutral private header (S60), physical Shared move (S61), firmware behavior/ROM change, callback ABI redesign or new board framework. |
| Reference Baseline | S58 P1 `930d35bec`, [S58 evidence](../../etc/evidence/t540-s58-absent-memory-owner.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Existing `board_*_provider` slots and `board_owner`, board constructor/finalizer, firmware provider/context binding and F0000h alias choice. No new public API. |
| Applicable Rules | Core owns operation guards and bounded invocation; board owns callback production and revocation. Preserve shutdown callback ABI used by test schedulers. |
| Verification | Source audit of every install/revoke and firmware path; focused affected regressions; complete x64/x86 units, specialized/documentation gates; rebuild eight products and run one boot per profile/width only if executable inputs change. |
| Expected Markers | Callback owners remain valid through invocation, with no callback after board release; an explicit slot-clearing pass is unnecessary when destroying the whole machine. One firmware invocation/ROM table and one board alias choice remain, with no duplicate path. |
| Asset Needs | Existing external firmware/media for any affected boot checks only; no owner INI edit. |
| Reporting Requirements | Complete callback/firmware owner inventory, exact diff or justified no-change result, tests and artifact determination. |
| Stop Conditions | Stop for conflicting ownership, a needed ABI redesign or cross-target change; split if actual surface exceeds one bounded S. |
| Exit Criteria | P1 if code changes, P2 evidence pushed; affected verification passes; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Board install/revoke, destruction order, outstanding event callbacks, firmware bounded invocation and F0000h alias registration. |

## T540 S59 Acceptance

The [S59 callback/firmware audit](../../etc/evidence/t540-s59-callback-firmware-audit.md)
finds one installation point for 14 Core-facing providers, no callback after
the synchronous board teardown, one guarded firmware binding and one board
reset-alias decision with rollback. It correctly rejects a redundant
provider-slot clearing pass. No executable input changed: focused tests pass
8/8 per width, complete units pass 469/469 per width and both specialized
gate sets pass. S58's eight optimized artifacts and 8/8 boot checkpoints
remain the executable baseline. T540 remains open.

## T540 S60 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S60, next linear S after accepted S59. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Measure the remaining mixed private/public header and direct consumers, then split an oversized neutralization into finite owner receivers before changing code. |
| Non-goals | A one-step header rewrite, physical Shared relocation (now S65), new Core/device framework, behavior/timing change, firmware/media/INI change or cross-target edits. |
| Reference Baseline | [S59 evidence](../../etc/evidence/t540-s59-callback-firmware-audit.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units and gates; S58's eight booted 0540 artifacts. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Source-only inventory of private/public Core and board headers, named chip/D4/plan dependencies and direct source/test consumers; architecture ledger, proposal and admission/evidence documents only. |
| Applicable Rules | No named PC chip, board topology, D4, speaker or profile include in neutral Core's private header; retain only Core state and bounded board attachment/provider contracts. Preserve one owner and test boundaries. |
| Verification | Source inventory, current complete x64/x86 unit and gate baseline, documentation governance and diff checks. No executable input or artifact changes. |
| Expected Markers | Finite numeric receivers for D4, plan, private/public header and physical move; no false claim that the current Core header is already neutral or independently compilable. |
| Asset Needs | Existing external firmware/media for affected boot checks only; no owner INI edit. |
| Reporting Requirements | Measured remaining include/type inventory, numeric receiving allocation and explicit no-build artifact determination. |
| Stop Conditions | Stop or split if actual surface exceeds one bounded receiver, a needed Shared edit appears, or behavior would change. |
| Exit Criteria | Source intake and updated authoritative plan pushed; documentation governance passes; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Every direct board type/include in `machine.h`, neutral Core sources and direct tests; no copied chip state or bypass. |

## T540 S60 Acceptance

The [measured neutral-header intake](../../etc/evidence/t540-s60-neutral-header-intake.md)
finds 384 private and 694 public interface lines, with 133 and 174 direct
includers. The old single-step header/Shared move was not executed. D4,
frozen-plan, private/public interface and physical relocation now have
distinct linear numeric receivers S61-S65; board extraction begins at S66.
No executable input changed. S60's own dual-width 469/469 units and S59's
specialized gates verify the baseline; S58's eight booted 0540
artifacts remain current. T540 remains open.

## T540 S61 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S61, next linear S after accepted S60. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move the D4-specific mutable memory value to the board owner, retaining one Core checked-memory route and the existing parity/mapping effects. Inspect the actual 49 reference sites before changing storage. |
| Non-goals | Frozen-plan split (S62), header/interface neutralization (S63-S64), Shared move (S65), D4 behavior/timing change or new callback framework. |
| Reference Baseline | [S60 measured intake](../../etc/evidence/t540-s60-neutral-header-intake.md), [receiving ledger](../../etc/architecture/t540-s42-private-state-ledger.md), current dual-width units/gates and S58's booted eight-product baseline. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md). |
| Files And ABI Surface | Existing D4 memory state, D4 memory adapter and direct test consumers; preserve its current public configuration contract. |
| Applicable Rules | One board owner for D4-specific state, one neutral Core memory/bus operation owner, no mirrored latches or profile branch in Core. |
| Verification | Full x64/x86 units, specialized/documentation gates, affected D4 tests, one external boot per profile/width and eight optimized 0540 PE/no-debug products if code changes. |
| Expected Markers | No D4-specific mutable value remains flat in `core_machine`; parity, mapping, reset and fault observations follow the existing single path. |
| Asset Needs | Existing external firmware/media for affected boot checks only; no owner INI edit. |
| Reporting Requirements | Exact D4 reference and ownership diff, tests, boot/artifact hashes. |
| Stop Conditions | Split into further numeric S tasks if D4 storage and mechanism cannot be moved safely as one bounded receiver. |
| Exit Criteria | P1/P2 pushed if code changes; affected verification passes; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | D4 configuration, address remap, parity fault, reset, consumer and destroy paths. |

## T540 S61 Acceptance

The sole D4 mutable value is board-owned; Core still publishes the same
two replacement routes and parity/write observers atomically. P1
`ab3c8f437` is pushed. Focused D4/Model 40 units pass 23/23 per width,
complete units pass 469/469 per width, the corrected existing route verifier
and both specialized gate sets pass, and eight fixed-profile boots pass once
each. Eight optimized 0540 products are PE/no-debug verified. The [S61
evidence](../../etc/evidence/t540-s61-d4-memory-owner.md) records exact diff,
ownership and hashes. T540 remains open.

## T540 S62 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S62, next linear S after accepted S61. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Separate the existing frozen plan's board topology and D4/media/display/FDC inputs from neutral Core timing declarations while preserving one plan, one application order and one rollback. |
| Non-goals | Private/public header completion (S63-S64), physical Shared relocation (S65), a second plan or parser, timing/behavior change, profile/INI/media change. |
| Reference Baseline | S61 P1 `ab3c8f437`, [S61 evidence](../../etc/evidence/t540-s61-d4-memory-owner.md), [S60 measured intake](../../etc/evidence/t540-s60-neutral-header-intake.md), dual-width units/gates and eight boots. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | `core_machine_plan`, its constructor/apply/freeze path, neutral timing and board topology/type consumers plus directly affected tests/static inventories. Measure actual coupling before editing; split if larger than one safe owner cut. |
| Applicable Rules | One frozen plan and one success-only publication; Core consumes neutral declarations, board consumes topology. No copied mutable configuration or second application path. |
| Verification | Full x64/x86 units, specialized/documentation gates, focused plan/rollback tests, one external boot per profile/width and eight optimized 0540 PE/no-debug products if code changes. |
| Expected Markers | Board-specific plan types do not force the neutral Core header to include controllers; there is one plan object and unchanged application order. |
| Asset Needs | Existing external firmware/media for affected boot checks only; no owner INI edit. |
| Reporting Requirements | Exact plan consumer/type inventory, ownership diff or numeric sub-split, tests and artifact decision. |
| Stop Conditions | Split into further numeric S tasks if one receiver would be oversized or require a second plan. |
| Exit Criteria | P1/P2 pushed if code changes; affected verification passes; worktree clean. |
| Original Owner Request | Prepare reusable x86 Core and IBM-PC boards with one clear hardware owner. |
| Similar-Issue Sweep | Plan create/configure/freeze, topology apply, board and Core timing consumers, failure rollback and direct tests. |

## T540 S62 Acceptance

P1 `25a350639` removes the live full-plan copy and dead linear declaration
lookup. The neutral machine keeps only indexed, validated timing declarations;
the sole board attachment owns controller timing and explicit-DMA-clock
provenance. The original plan remains the only construction input and the
existing validation, application order and rollback remain intact. The
[S62 evidence](../../etc/evidence/t540-s62-frozen-timing-declarations.md)
records the 31-addition/49-removal diff, both-width 469/469 complete units,
both specialized gate sets and all eight single-run external boot terminals.
Eight optimized 0540 products are PE/no-debug verified. T540 remains open.

## T540 S63 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S63, next linear S after accepted S62. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Remove board-only definitions and chip includes from the neutral Core private header, keeping one owning board header and direct dependencies for each consumer. |
| Non-goals | Public interface redesign (S64), physical Shared move (S65), new device framework, behavior/timing/profile/INI/media changes. |
| Reference Baseline | S62 P1 `25a350639`, [S62 evidence](../../etc/evidence/t540-s62-frozen-timing-declarations.md), [S60 measured intake](../../etc/evidence/t540-s60-neutral-header-intake.md). |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Private `machine.h`, sole `machine_board_state.h`, and their actual direct consumers. Inventory each board-only definition/include before editing; split oversized receivers into later linear S numbers. |
| Applicable Rules | A neutral Core header names no board chip type or private board state; consumers include only the owner they actually use. No second state or forwarding wrapper. |
| Verification | Complete x64/x86 units, specialized/documentation gates, focused header/owner tests, one boot per profile/width and eight optimized 0540 PE/no-debug products if code changes. |
| Asset Needs | Existing external firmware/media for boot checks only; no owner INI edit. |
| Stop Conditions | A private-header slice is larger than one safe owner cut or forces a public ABI redesign; record and assign the remainder to linear S. |
| Exit Criteria | P1/P2 pushed if code changes; affected checks pass; worktree clean. |
| Similar-Issue Sweep | All private-header direct includers, CMake source inventories, unit/integration diagnostics and hidden chip dependencies. |

## T540 S63 Acceptance

P1 `4d6443833` moved the board-only private definitions, chip includes and
function declarations to the existing sole board header, then corrected six
real transitive include consumers. The [S63 evidence](../../etc/evidence/t540-s63-private-header-boundary.md)
records the 90-addition/93-removal structural diff, final x64/x86 469/469
units, both 82-target specialized gate sets, all eight single-run external
boot terminals and optimized PE/no-debug 0540 products. It also records the
resolved intermittent shared modal-window test observation. T540 remains
open; S64 receives the public interface and independent compile proof.

## T540 S64 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S64, next linear S after accepted S63. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Split PC-specific types and includes from the public Core interface and prove neutral Core compiles independently of App board/product headers. |
| Non-goals | Physical Shared source move (S65), new board framework, behavior/timing/profile/INI/media change or second creation path. |
| Reference Baseline | S63 P1 `4d6443833`, [S63 evidence](../../etc/evidence/t540-s63-private-header-boundary.md), [S60 measured intake](../../etc/evidence/t540-s60-neutral-header-intake.md). |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | `machine_interface.h` and its direct consumers, current neutral source inventory and bounded compile target. Measure its 694-line/174-includer surface before editing; split further into linear numeric S if one owner cut is unsafe. |
| Applicable Rules | No PC topology or product type in the neutral public contract; exactly one function implementation and one construction path. Board contracts retain their real owner. |
| Verification | Independent strict neutral compile plus complete x64/x86 units, specialized/documentation gates, affected boot matrix and optimized eight-product PE/no-debug output if code changes. |
| Asset Needs | Existing external boot inputs only; do not alter owner INI. |
| Stop Conditions | Public interface contains multiple distinct owner cuts too large for one S; record finite split before modifying those cuts. |
| Exit Criteria | P1/P2 pushed if code changes; independent compile proof and affected checks pass; worktree clean. |
| Similar-Issue Sweep | Public header transitive includes, direct source/test consumers, target source lists and static gates. |

## T540 S64 Acceptance

The [S64 intake](../../etc/evidence/t540-s64-public-interface-intake.md)
measured the public interface's 694 lines/174 direct includers, overlapping
152-file configuration, 48-file topology and 47-file board-operation
consumer groups, and six direct `machine.c` board calls. It assigns
separate, strictly numeric S65-S69 receivers before the now-S70 physical
Shared move; S71 onward receives board extraction. No production source,
test, build, owner INI, external input or EXE changed. Both complete
repository-only unit suites pass 469/469 on the unchanged S63 baseline.
This is not an independent-Core-build claim. T540 remains open.

## T540 S65 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S65, next linear S after accepted S64. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Separate neutral Core construction inputs from board-selected PIC/PIT/DMA/KBC/XT and other configuration values within the one validated/frozen plan. |
| Non-goals | Board public value/operation split (S66-S67), remaining Core reset/NMI/finalization handoff (S68), independent compile (S69), physical Shared move (S70), behavior/timing/profile/INI/media change. |
| Reference Baseline | S63 P1 `4d6443833`, [S64 measured intake](../../etc/evidence/t540-s64-public-interface-intake.md) and [S63 evidence](../../etc/evidence/t540-s63-private-header-boundary.md). |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Private `core_machine_executor_config` contains only RAM/CPU/FPU/bus/retirement/time/provider-clock inputs. Board composition owns the existing public create and test-allocation pipeline, deriving one temporary neutral value from the unchanged frozen plan; no 152-caller ABI rewrite. Board public configuration ownership completes in S66. |
| Applicable Rules | One immutable plan, one neutral Core configuration, one board configuration, no runtime mirror or forwarding shim and no second create/rollback path. |
| Expected Markers | Neutral constructor never receives `core_machine_config` or a device clock plan. Board preflight still rejects invalid clock/topology before allocation; one public construction path and existing rollback order remain. |
| Verification | Complete x64/x86 units, specialized/documentation gates, focused plan/rollback tests, one external boot per profile/width and eight optimized 0540 PE/no-debug products if code changes. |
| Asset Needs | Existing external firmware/media only for boots; owner INI untouched. |
| Stop Conditions | The measured construction ABI and 152-file consumer group cannot be safely changed in one owner cut; divide unaccepted work into later linear numeric S. |
| Reporting Requirements | Report actual changed paths/line counts, allocation/clock regression results, complete units/gates, eight single boots and product identity; do not claim independent Core compilation yet. |
| Exit Criteria | P1/P2 pushed if code changes; affected checks pass; worktree clean. |
| Similar-Issue Sweep | All direct plan/configuration consumers, board construction, failure rollback and CMake/static gates. |
| Original Owner Request | Build neutral reusable x86 Core and flat IBM-PC common/AT/XT components without duplicate state, frameworks or changed product behavior. |

## T540 S65 Acceptance

Actual-pushed P1 `c6ce9b84c` has exactly eighteen NXVM paths: three
production, one test, two static gates, four documents and eight EXEs.
`git show --check` passes, and P1 equals `origin/master` at review. Core's
constructor receives only the thirteen-field neutral value, with one
board composition/create/rollback route. Complete units pass 469/469 per
width, focused construction/clock/plan tests 6/6 per width, both specialized
gate sets and documentation checks pass. Exactly one boot/profile/width
passes, 8/8; all eight optimized 0540 PE products have no compiler-debug
sections. Shared/MyNES, owner INI and external inputs remain unchanged.
See [S65 evidence](../../etc/evidence/t540-s65-neutral-construction-input.md).
This accepts S65 only: public board values/operations and four direct
Core reset/clock/NMI/finalization calls remain with their named receivers.

## T540 S66 Accepted Admission Record

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S66, next numeric S after accepted S65. |
| Admission And Approval | Owner standing automatic bounded-S admission; NXVM only, Shared/MyNES read-only. |
| Objective | Give public board construction, topology and observation values their explicit board owner, removing their definitions and concrete chip dependencies from the neutral Core interface. |
| Non-goals | Runtime operation changes, neutral validator implementation cut (S67), Core lifecycle handoff (S68), independent compile (S69), physical Shared move (S70), timing/behavior/profile/INI/asset changes. |
| Reference Baseline | S65 P1 `c6ce9b84c`; [S64 intake](../../etc/evidence/t540-s64-public-interface-intake.md) and [S65 evidence](../../etc/evidence/t540-s65-neutral-construction-input.md). |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Move board constants, clock plan, full construction config, keyboard/XT enums, controller timing rules, display/RTC/parity/D4/DMA/media topology and copied observations into `machine_board_interface.h`. Move their adjacent board API declarations atomically (including by-value XT fault enum); repair only concrete consumers. Move private board create-test/clock-plan declarations from `machine.h` to board state. Keep function bodies, symbols, enum values and layouts unchanged; no reverse include or forwarding header. |
| Applicable Rules | One definition per value, no forwarding compatibility header, copied observations, one frozen input plan, explicit includes and unchanged numeric/ABI semantics. |
| Expected Markers | Neutral execution/time values remain Core-owned; board value definitions have one public board interface and every concrete consumer includes that owner. No duplicate definition or runtime mirror. |
| Verification | Complete x64/x86 units, specialized/documentation gates, affected constructor/plan/observation tests, eight optimized 0540 products if production inputs change and one external boot per profile/width. |
| Asset Needs | Existing BYOB inputs only; no acquisition, external-master or owner-INI edit. |
| Stop Conditions | A type-dependent declaration cannot be split without an explicit receiving owner, or the measured cut changes behavior/ABI; re-plan the unaccepted boundary rather than add a shim. |
| Reporting Requirements | Report actual value/caller inventory, source/test diff counts, verification and remaining boundaries; do not claim physical Shared extraction or independent Core build. |
| Exit Criteria | Complete P1 pushed, actual-diff review, governance P2 pushed, required checks and artifacts correct, worktree clean. |
| Similar-Issue Sweep | All direct public interface consumers, transitive concrete chip includes, plan construction, copied observations and boundary gates. |
| Original Owner Request | Build reusable neutral x86 Core and flat IBM-PC common/AT/XT components with single ownership and no patch-layered architecture. |

## T540 S66 Acceptance

The neutral public contract shrinks from 694 to 305 lines. All six board
declaration blocks are verbatim at the new explicit owner; 210 consumer files
change only their includes. Private board construction seams move with their
owner. No runtime implementation, numeric value, layout, Shared or MyNES
source, owner INI or external asset changes. Five remaining neutral validators
defined in board plan are explicitly assigned to S67; S68-S70 remain open.

Both-width complete units pass **469/469** each, specialized and documentation
gates pass, eight optimized/debug-stripped 0540 products are rebuilt and the
eight receiving external boot rows pass. See the
[S66 evidence](../../etc/evidence/t540-s66-public-board-interface.md).
Coordinator review accepts pushed P1 `455e920fc`: exactly 231 NXVM paths,
`git show --check` passes, all 210 concrete consumers have zero non-include
changes, and local HEAD equals `origin/master` with a clean worktree at review.
The six declaration blocks remain verbatim, board-private seams retain their
signatures, and static gates inspect the receiving owner. This closes S66,
not T540 or the independent Core/board extraction.

## T540 S67 Acceptance

Coordinator actual-change review accepts pushed P1 `5db0fc37b`: exactly
14 NXVM paths, `git show --check` passes, and HEAD equals `origin/master`
with a clean worktree at review. All five committed complete definitions
match their originals, with one neutral owner and unchanged declarations and
callers. Source/gate delta is 75 added / 63 removed across three paths:
relocation spacing and ten prevention lines, not new runtime logic.

Complete units pass 469/469 on each width; both specialized sets, exact
dependency inventory and documentation/diff gates pass. All eight optimized,
compiler-debug-stripped 0540 products are rebuilt and each has one qualified
external boot, exit zero. Shared, MyNES, INIs and asset masters are unchanged.
The [S67 evidence](../../etc/evidence/t540-s67-neutral-validation-owner.md) records
the allocation, artifact identities and limits. S67 is accepted, not T540;
S68 receives the measured four lifecycle handoffs, ahead of independent
compilation and Shared physical relocation.

## T540 S68 Accepted Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S68, next linear numeric S after accepted S67. |
| Admission And Approval | Owner standing automatic bounded-S admission; NXVM only. Shared six components, MyNES, owner INIs and external assets remain read-only. |
| Objective | Remove neutral Core's four direct board lifecycle/signal calls and concrete board-state include through the existing single composition boundary. |
| Non-goals | Independent full Core compilation (S69), physical Shared relocation (S70), IBM-PC board extraction, new reset/lifecycle paths, public API, profile/INI/media changes or timing/behavior changes. |
| Reference Baseline | Accepted/pushed S67 P1 `5db0fc37b` and its [evidence](../../etc/evidence/t540-s67-neutral-validation-owner.md). Actual source intake finds reset-devices and reset-clocks in cold reset, NMI refresh on unmask, and finalization in the sole destructor. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md) and [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | Existing private `machine.h`, neutral `machine.c`, board composition `machine_board.c`, applicable tests and existing boundary gate. Bind the four existing phase functions once as private callbacks; invoke each at its existing point and remove the board-state include. No dispatch wrapper, phase enum, callback framework or public API. |
| Applicable Rules | One board allocation, one reset path and one destructor; neutral Core knows only the bounded phase contract. Preserve phase order, firmware-failure behavior, processor-only reset and partial-construction rollback. Do not route these callbacks through replaceable test `board_owner`; use the actual machine parameter, as the existing shutdown-reset contract does. |
| Verification | Exact four-phase caller/definition inventory and body/order comparison; construction failure and reset/NMI/destruction regressions; complete x64/x86 units; specialized, dependency and documentation gates; eight optimized/debug-stripped 0540 products and one qualified external boot per profile/width. |
| Expected Markers | No concrete board-state include or direct named board phase call remains in neutral source. The same four board implementations and single composition binding remain, including finalization after partial allocation. No chip state, guest clock or lifecycle state is copied. |
| Asset Needs | Existing owner-provided external inputs only; no acquisition, asset-master or owner-INI modification. |
| Reporting Requirements | Record phase order, binding point, null/partial-construction behavior, actual source/test/gate diff, verification, artifact identities and remaining S69-S70 boundaries. |
| Stop Conditions | Callback binding loses the board when clock initialization fails, changes processor-only or firmware reset semantics, requires a second destructor, or depends on test-replaced owner context; revise the unaccepted boundary before coding. |
| Exit Criteria | Required checks/artifacts/boots pass; complete P1 pushed; coordinator actual-commit review and governance P2 pushed; worktree clean. T540 remains open. |
| Original Owner Request | Reusable neutral x86 Core and flat IBM-PC common/AT/XT components with unique state and execution owners, strict numeric S tracking and no layered patches. |
| Similar-Issue Sweep | All direct board references in neutral Core, fourteen existing provider bindings, allocation before/after clock initialization, neutral-create failures, cold/processor-only reset, NMI unmask and the sole destroy route. |

Coordinator review accepts pushed S68 P1 `c473ddfdf`. The
[receiving evidence](../../etc/evidence/t540-s68-board-lifecycle-handoff.md)
records four private phase bindings, unchanged board bodies/order, complete
469/469 units per width, specialized/dependency/documentation checks, eight
rebuilt 0540 products and eight one-shot external boot checkpoints. Shared,
MyNES and owner INIs remain unchanged. S69-S70 and T540 remain open.

## T540 S69 Accepted Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S69, next numeric S after accepted S68. |
| Admission And Approval | Standing automatic bounded-S admission; NXVM-only build/test proof. Shared six components, MyNES, owner INIs and external assets remain read-only. |
| Objective | Prove that the actual neutral Core sources compile and link without IBM-PC board implementations or the existing all-chip executor aggregate. |
| Non-goals | Physical Shared relocation (S70), IBM-PC board extraction, new executor or copied production implementation, CPU/device semantics or timing changes, public API and product configuration changes. |
| Reference Baseline | Accepted S68 P1 `c473ddfdf` and its evidence. Intake finds that current `core-machine-executor` links all board chips and the existing lifecycle fixture constructs a board; neither proves independent neutrality. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md) and [source ledger](../../etc/architecture/t540-s37-core-relocation-ledger.md). |
| Files And ABI Surface | NXVM CMake/test/static-proof files. Compile the same sixteen neutral sources: clock, CPU bus, debug, entry plan, firmware invocation, machine, scheduler, memory/port/ROM/trace/retirement interfaces, timeline and memory/port/transaction primitives. Use existing headers and private neutral constructor. No source copy or new public contract. |
| Applicable Rules | One production source and guest executor. The test-only receiving target links only declared CPU/FPU/Lib dependencies, not board composition, display or peripheral chips. Synthetic input belongs to its test; existing board fixtures retain their original meaning. |
| Verification | Both-width standalone link/run proof and link-input inventory; full units and specialized/dependency/documentation/diff gates. If an executable input changes, rebuild eight optimized/debug-stripped 0540 products and verify one external boot per row; otherwise retain and hash-check accepted S68 artifacts. |
| Expected Markers | A real test executable reaches neutral construction, reset, bounded CPU/time execution, memory/port/ROM/debug observation and destruction without board symbols. Static inventory prevents the all-chip target from masquerading as neutrality. |
| Asset Needs | None for unit proof; use only code-owned synthetic data. No firmware/media acquisition, external master or INI change. |
| Reporting Requirements | Exact source/header and link dependencies, exercised API/phase ownership, complete verification and whether product inputs or artifacts changed. State any remaining blocker before claiming independent compilation. |
| Stop Conditions | Linking requires board implementations or a duplicate executor, proof substitutes stubs for Core logic, existing tests lose coverage, or the bounded diff requires broader production changes. Reallocate the measured gap before claiming success. |
| Exit Criteria | Actual independent proof plus complete required checks; P1 pushed, coordinator actual-commit review and governance P2 pushed; clean tree. T540 stays open. |
| Original Owner Request | Neutral x86 Core and flat IBM-PC common/AT/XT ownership for the future independent PC Apps, without mirrored state, layered patches or suffix S numbering. |
| Similar-Issue Sweep | Candidate source includes and undefined symbols, transitive CMake chip links, existing executor fixture, board callbacks and null bindings, trace variants and primitive/display source ownership. |

Coordinator review accepts pushed S69 P1 `0fbfa9a7b`. The
[independent receiving evidence](../../etc/evidence/t540-s69-independent-neutral-core.md)
records sixteen actual-source objects, eighteen required headers, no board
archive dependency, both-width Debug/Release execution and 470/470 complete
units per width. Product sources and all eight S68 EXE identities are unchanged.
Physical relocation and public board-consumer boundaries remain open.

## T540 S70 Accepted Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S70, next unused numeric S after accepted S69. |
| Admission And Approval | Owner standing automatic bounded-S authorization within T540 extraction; NXVM source/test/build evidence only. Shared, MyNES, owner INIs and external assets remain read-only. |
| Objective | Remove every production board borrow of the CPU execution context for NMI and processor-reset signals; Core alone accepts these signals and the existing CPU/run path consumes them. |
| Non-goals | Physical Shared relocation, new CPU signal state, lifecycle queue or executor, CPU/timing/guest behavior change, RAM/port/firmware boundary changes and MyNES builds. |
| Reference Baseline | Accepted S69 P2 `7c0c8c458`. Actual intake finds three NMI and one reset call in board composition still borrow `executor_cpu_execution`; independent linkage did not prove public consumer boundaries. |
| Candidate Proposal | [T540 integration](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md), [S69 proof](../../etc/evidence/t540-s69-independent-neutral-core.md). |
| Files And ABI Surface | Existing neutral machine/interface, board composition, neutral receiving smoke and lifecycle prevention gate. Add only bounded opaque-machine NMI acceptance and processor-reset signal operations. Preserve CPU-owned mask/latch/reset flag and original call sites. |
| Applicable Rules | One CPU execution lifetime and run-loop reset consumer; no CPU pointer crosses the board boundary. Signal acceptance runs synchronously on the executor thread like A20; it is not a host lifecycle request or arbitrary-thread API. |
| Verification | Neutral masked/unmasked NMI acceptance, null input and queued processor reset consumed before retirement; existing XT/parity/D4/KBC tests; full x64/x86 units, independent proof and specialized/dependency/documentation gates; rebuild eight 0540 products and one external boot per profile/width. |
| Expected Markers | No production board CPU-context borrow for these signals; existing NMI latches update only after accepted NMI; reset has no second pending flag and reaches the original processor-only reset consumer. |
| Asset Needs | Existing external boot inputs only, unchanged owner INIs and overlay media. No acquisition or asset-master write. |
| Reporting Requirements | Exact similar-issue caller inventory, public lifetime/thread/mask contract, code-size delta, tests, artifact identities, boot checkpoints and remaining physical-relocation prerequisites. |
| Stop Conditions | Original mask/latch or reset priority changes, a second state/queue is needed, broader private access must be hidden rather than owned, or any unrelated consumer/artifact changes. Re-plan the unaccepted boundary first. |
| Exit Criteria | Complete scoped signal cut and all required proof/artifacts/boots; pushed P1, coordinator actual-commit review and pushed governance P2, clean worktree. T540 stays open. |
| Original Owner Request | Independently reusable Core and flat IBM-PC board components, unique state/production path, automatic numeric S progression without layered patches. |
| Similar-Issue Sweep | All production CPU NMI/reset calls, callback adapters and latch-on-accept sites; classify CPU internals and same-owner tests separately. Exact eighteen-header consumer inventory covers 306 source/test/build paths and retains memory/port, firmware and provider-binding cuts before physical relocation. |

Coordinator actual-change review accepts pushed S70 P1 `ba659381f`.
All four board CPU signal callers use the opaque Core operations; original
mask/latch predicates and the CPU-owned reset consumer remain unchanged.
Both-width complete units pass 470/470; independent Debug/Release linkage,
specialized gates and all eight one-shot external boot checkpoints pass.
Eight optimized debug-stripped 0540 products are committed. Shared, MyNES,
owner INIs and external masters have no change. The
[S70 evidence](../../etc/evidence/t540-s70-core-signal-boundary.md) records
artifact identities, receiving limits and remaining pre-relocation classes.
S70 is accepted; T540 remains open with no next implementation admitted here.

## T540 S71 Accepted Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S71, next numeric receiver after accepted S70. |
| Admission And Approval | Owner standing automatic bounded-S authorization within T540; NXVM-only source, tests, gates, documents and eight artifacts. Shared, MyNES, INIs and external masters remain read-only. |
| Objective | Core alone prepares/publishes RAM aliases and parity storage; board composition supplies copied ranges and electrical callbacks without borrowing RAM layout. |
| Non-goals | Physical Shared move, port-construction checkpoints, firmware publication, provider attachment ownership, timing or guest behavior changes and MyNES builds. |
| Reference Baseline | S70 P2 `d1fe5c0aa`; two production raw RAM mapping callers and one planar parity enable/release pair remain in board/plan composition. D4 already uses the Core memory-route transaction. |
| Candidate Proposal | [T540](../../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../../etc/architecture/t540-s42-private-state-ledger.md), [S71 intake](../../etc/evidence/t540-s71-memory-construction-boundary.md). |
| Files And ABI Surface | Existing memory interface/implementation, board/plan construction, neutral and parity receiving tests, controller-authority and Port-B boundary gates. Move the copied alias descriptor to neutral memory contract; one bounded all-or-none alias batch, no RAM pointer or second map. |
| Applicable Rules | Unique RAM mapping/parity state and existing range validator; construction-only executor-thread operations. Board keeps CPU reset-address choice, parity electrical latch and success-only board publication. |
| Verification | Synthetic alias read/write and rollback, invalid/frozen registration, parity port-conflict cleanup and retry; full x64/x86 units, independent Core proof, specialized/dependency/documentation/diff gates; eight optimized stripped 0540 builds and one external boot per profile/width. |
| Expected Markers | No board raw RAM mapping or parity allocation/release calls; existing memory storage and validators remain sole owners. Failed alias batch leaves prior mapping count intact; failed parity port publication removes only the newly prepared memory registration. |
| Asset Needs | Existing external boot media with unchanged owner INIs and overlays only; no acquisition or master writes. |
| Reporting Requirements | All production raw memory construction hits, copied-value contract, exact code-size delta, regression results, artifact hashes and remaining port/firmware/attachment cuts. |
| Stop Conditions | Reset alias priority changes, parity owner collides with an existing published registration, duplicate state or rollback path is needed, or another target must change. Re-plan before implementation beyond this scope. |
| Exit Criteria | Complete memory construction class with passing required proof/artifacts/boots, complete pushed P1 and actual coordinator review, governance P2 push and clean worktree. T540 remains open. |
| Original Owner Request | Flat independently reusable Core and IBM-PC board components, one state owner/path, no pointer facade or layered patches; numeric S progression automatically admitted. |
| Similar-Issue Sweep | Search all NXVM production for raw memory mapping and parity enable/release calls. Neutral memory implementation is the owner, same-owner tests are valid; D4 already uses public route transaction. Port checkpoint and board-issued I/O hits are explicitly retained for the next receiver. |

Coordinator actual-change review accepts pushed S71 P1 `9050dfd69` across all
22 delivered paths. The original RAM range validator and mapping storage remain
the sole owners; failed batches restore the prior count. Board reset aliases
retain their address/priority and parity publication retains success-only board
commit plus Core owner-qualified rollback. Both-width full units pass 470/470;
independent Core proofs, specialized gates and all eight one-shot external boot
checkpoints pass. Eight optimized debug-stripped 0540 EXEs are committed;
Shared, MyNES, owner INIs and external masters are unchanged. The
[S71 evidence](../../etc/evidence/t540-s71-memory-construction-boundary.md) records
identities and limits. S71 is accepted; T540 remains open for measured port,
firmware/attachment and physical relocation receivers. No next implementation
is admitted by this governance closure.

## T540 S72 Acceptance

Coordinator actual-change review accepts pushed P1 `53c2d4085`: all sixteen
changed files match the NXVM-only packet, including three source/test/gate
paths and eight rebuilt products. Constructor order and error returns remain
unchanged; Core batches and the sole destructor retain failure ownership.
The [S72 evidence](../../etc/evidence/t540-s72-port-construction-owner.md) records
complete allocation-failure coverage, x64/x86 470/470 units, specialized gates,
independent Core proofs, eight one-shot boots and artifact identities.
Shared, MyNES and INIs are unchanged. S72 is accepted; T540 remains open for
reset-I/O, firmware/attachment and physical relocation. No next implementation
is admitted by this governance closure.

## T540 S81 Acceptance

Coordinator accepts immediately pushed P1 `c01ee5ede` after actual review of
all 26 NXVM source/test/build/document/artifact paths. The frozen plan publishes
Core and the actual board only after complete application; every caller is
reconnected and the sole destructor invalidates the driver lease. There is no
getter, duplicate state, allocation or cleanup path. [S81 evidence](../../etc/evidence/t540-s81-plan-board-publication.md)
records dual-width 470/470 units/gates, nine injected negatives, eight neutral
executions, eight single INI boots and optimized stripped 0540 identities.
The initial negative-test timeout remains recorded and the complete rerun
passes with every check preserved. Committed artifacts equal the verified
files; scope, identifiers, links and retained ledger dispositions pass review.
Shared, MyNES, INIs and external masters are unchanged. S81 is accepted;
T540 remains open for public board operations/configuration-fixture callers,
direct-test classification and physical neutral Core/IBM-PC relocation.
No next implementation packet is admitted by this governance closure.

## T540 S80 Acceptance

Coordinator accepts immediately pushed P1 `24fb33b1e` after reading the actual
25-path NXVM source/test/build/document/artifact diff. All nineteen callbacks
consume the actual board allocation; copied publication and Core finalization
remain unique. Reset, firmware rollback, event ordering and null-context NMI
semantics are preserved. [S80 evidence](../../etc/evidence/t540-s80-board-callback-context.md)
records dual-width 470/470 units/gates, seven injected negatives, eight neutral
Core executions, eight single boot checkpoints and stripped 0540 identities.
Committed artifact blobs equal the verified files. Scope, identifiers, document
links and retained ledger dispositions pass review. Shared, MyNES, owner INIs
and external master inputs are unchanged. S80 is accepted; T540 remains open
for the full public board-handle caller cut, direct-test classification and
physical neutral Core/IBM-PC relocation. No next packet is admitted here.

## T540 S79 Accepted Delivery

Coordinator actual pushed-diff review accepts P1 `8717d3af2`: all 37 paths
match the NXVM-only packet. Nineteen callbacks have one copied publication;
invalid, duplicate and frozen requests preserve the previous value. Core's
finalization order, scheduler/firmware algorithms and fixture context identity
are preserved. The [S79 evidence](../../etc/evidence/t540-s79-copied-attachment-binding.md)
records dual-width 470/470 units/gates, six injected negatives, eight neutral
Core executions, eight one-shot boot checkpoints and stripped 0540 identities.
Committed artifact blobs match the verified worktree. Documentation structure,
links, complete ledger disposition, task identifiers and actual scope pass.
Shared, MyNES, INIs and external master inputs are unchanged. S79 is accepted;
T540 remains open for opaque-board ownership, direct-test classification and
physical Core/IBM-PC relocation. This governance closure admits no next packet.

## T540 S78 Acceptance

Coordinator actual pushed-diff review accepts design P1 `5d707a0ba`:
five NXVM documentation files record the complete nineteen-callback boundary,
six production board owners, direct-test classification and lifetime order.
The [S78 intake](../../etc/evidence/t540-s78-attachment-owner-intake.md) separates
existing implementation from the opaque-handle/copied-binding target and
assigns the whole callback cut to prospective S79. Documentation governance,
changed local links, diff/scope and all eight S77 artifact hashes pass.
Two stale proposal links are corrected. Source/tests, Shared, MyNES, owner
INIs and artifacts are unchanged; no new runtime test or rebuild is claimed.
S78 is accepted. T540 remains open for attachment implementation, test
classification and physical Core/IBM-PC relocation. No next implementation
packet is admitted by this pure-governance acceptance.

## T540 S77 Acceptance

Coordinator actual pushed-diff review accepts P1 `bd0256b47`: all 58 paths
match the NXVM-only packet. Four Core entries supply the single copied read
tick; all forty typed callbacks are reconnected, and both Port-B routes use
their unchanged helper algorithm. No clock mirror, running getter or second
dispatch path is added. The [S77 evidence](../../etc/evidence/t540-s77-port-read-time-input.md)
records x64/x86 470/470 complete units, specialized and injected-negative
gates, eight independent neutral executions, eight one-shot boot checkpoints
and optimized stripped product identities. Artifact hashes remain exact.
Shared, MyNES, owner INIs and external master inputs remain unchanged.
S77 is accepted; T540 stays open for attachment ownership, direct-test
classification and physical Core/IBM-PC relocation. This governance closure
admits no next implementation packet.

## T540 S76 Acceptance

Coordinator actual pushed-diff review accepts P1 `8a4e3bd12`: Core validates
and publishes the complete copied table once; board retains source/seam policy
and receives deadline qualification as a value. All 22 paths stay within NXVM
scope, including eight optimized stripped 0540 products. The
[S76 evidence](../../etc/evidence/t540-s76-timing-publication-boundary.md)
records x64/x86 470/470 units, specialized gates, independent neutral linkage,
eight single external boots and full artifact hashes. Shared, MyNES and owner
INIs remain unchanged. The resolved modal-test observation is recorded without
a claimed root cause or shared-code change.

S76 is accepted; T540 remains open. S77 intake owns the distinct Running Port-B
I/O-cycle observation before attachment ownership, test classification and
physical Core/IBM-PC relocation. This pure-governance closure admits no next
implementation packet.

## T540 S75 Acceptance

Coordinator actual pushed-diff review accepts P1 `b39d60c31`: six existing
board input/display functions use the copied Core lifecycle operation, with
all original states/statuses preserved; both READY bodies move verbatim into
the sole neutral scheduler. The whole board-source-family gate and four
injected negatives pass. No API, duplicate state or runtime path is added.
The [S75 evidence](../../etc/evidence/t540-s75-lifecycle-ready-boundary.md)
records x64/x86 470/470 complete units, specialized gates, eight independent
Core-link executions, eight one-shot external boots and stripped 0540 hashes.
Shared, MyNES, INIs and external master inputs are unchanged.
S75 is accepted. S76 intake receives the complete timing publication and
observation group: one Running Port-B clock read, three deadline qualification
reads and both plan publication sites. Attachment, test classification and
physical Core/IBM-PC relocation remain required before T540 closure.
This pure-governance acceptance admits no next implementation S.

## T540 S74 Acceptance

Coordinator actual pushed-diff review accepts runtime/test/artifact P1
`7997202a6` and complete gate correction P2 `eb6e1db17`. Core owns firmware
publication and rollback; board policy uses copied coverage and atomic ROM
window operations. Whole-production inspection and five injected negatives
close the private-state coverage gap without another runtime path.
The [S74 evidence](../../etc/evidence/t540-s74-firmware-publication-boundary.md)
records x64/x86 470/470 complete units, specialized gates, independent Core
proofs and eight one-shot boots. All eight optimized stripped 0540 artifact
hashes remain exact. Shared, MyNES and owner INIs are unchanged.
S74 is accepted; T540 remains open for attachment ownership, remaining scalar
boundaries, direct-test classification and physical Core/IBM-PC relocation.
This governance closure admits no next implementation S.

## T540 S73 Acceptance

Coordinator actual-change review accepts pushed P1 `dd5b611f2`: all sixteen
changed files match the NXVM-only packet. The three board refresh writes use
the existing PIT algorithm; no Core/private port dependency, new API or timing
change remains in that batch. The [S73 evidence](../../etc/evidence/t540-s73-board-refresh-pit.md)
records construction/repeated-reset matrix proof, x64/x86 470/470 units,
specialized gates, independent Core proofs, eight one-shot boots and artifact
identities. All eight current hashes match that evidence. Shared, MyNES and
INIs are unchanged. S73 is accepted; T540 remains open for firmware/attachment,
direct-test classification and physical relocation. No next implementation
is admitted by this governance closure.

## S87 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S87, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic admission of bounded linear T539 S deliveries and required oversized CPU sources to be split by actual responsibility. This follows accepted S86. |
| Objective | Extract only the CPU bus, instruction-effect and copied retirement-observation rows from the mixed execution-context test into one Shared CPU receiver. |
| Non-goals | CPU create/reset/prepared-entry/lifecycle rows, NMI, prefetch, paging, INVLPG, public board wiring, CPU semantics/timing, production APIs, firmware, assets, INI and executable inputs. |
| Reference Baseline | `42ec9a014` after S86; the ledger assigns execution-context bus/observation rows to S87 and reserves lifecycle for S88 and signal/prefetch/paging for S89. |
| Candidate Proposal | [Independent Shared chips](../../history/M5-T539-independent-shared-chips.md) and the revised [CPU work package ledger](../../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split `cpu_execution_context_smoke.c` by existing static-function boundary. Move the already CPU-only `cpu_bus_fixture.h` to Shared and update its one retained NXVM FLAGS consumer to include it directly. Add one Shared successor with only bus/observation helpers and a minimal main; retain all other functions once in the NXVM residual source. Update registrations/manifests/static inventories; no production or public ABI change. |
| Applicable Rules | NXVM architecture and coding authorities; Shared architecture/coding/execution/documentation rules; one owner per test behavior and no duplicate CPU setup path. |
| Verification | Record the exact function allocation; prove the receiver uses only the existing CPU bus fixture; run focused x64/x86 receiver and residual tests, repository-only x64/x86 unit suites, T332 where applicable, Shared manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` receiver, one Shared CPU bus fixture, a residual NXVM test with the remaining functions, no copied bus/observation function in both files, and no new fixture/API. |
| Asset Needs | None; repository-only test ownership split. |
| Reporting Requirements | Record moved and retained static functions, source line totals, static-gate impact, test evidence and artifact determination. |
| Stop Conditions | Stop and report if a candidate row needs public Core/board wiring, a new Shared fixture/API, or duplicated function/fixture setup. |
| Exit Criteria | All admitted bus/observation rows have one Shared receiver, every remaining execution-context row stays once in the residual source with S88/S89 allocation, and required verification is recorded. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S87 consumes only CPU bus/effect/retirement observation functions. S88 owns create/reset/prepared-entry/timing/lifecycle; S89 owns NMI, prefetch, paging-control and INVLPG. |

## S87 Acceptance

The former `cpu_bus_cases()` function is now solely the Shared
`cpu_execution_bus` receiver. Its one CPU-local fixture is likewise solely
Shared; NXVM's residual context and FLAGS-local tests include it directly, and
the former App fixture copy and function call are deleted. Lifecycle/timing,
signal/prefetch/paging and public-board rows remain once in NXVM for S88/S89.

Focused x64/x86 Shared and NXVM receivers, T332, Shared manifest/corpus and
CPU/PIC authority gates pass. The original terminal-hosted full-unit runs
misreported the long CMake negative gate as failed. Detached, process-owned
full-unit verification fixes that execution artefact: x64 **440/440** and x86
**440/440** both pass with exit code zero. This is test/CMake/documentation-only
work: no production/API, firmware, asset, INI or EXE input changed. See the
[S87 evidence](../../etc/evidence/t539-s87-execution-bus-receiver.md). S87 is
accepted; T539 remains open for S88-S98.

## S88 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S88, automatically admitted continuation. |
| Objective | Extract only CPU reset, prepared-entry, instance-lifecycle and timing rows from the residual execution-context test into bounded Shared receivers. |
| Non-goals | NMI, prefetch, paging, INVLPG, public board wiring, production APIs, firmware, assets, INI and executables. |
| Files And ABI Surface | Split only existing static functions from `cpu_execution_context_smoke.c`; reuse existing CPU-local fixtures or add no fixture/API unless the current code proves one is necessary. Retain every S89 row once in NXVM. |
| Verification | Record exact function allocation; run focused x64/x86 successor and residual tests, complete x64/x86 repository-only units, relevant static gates, manifests/corpus, authority, documentation governance and diff checks. |
| Exit Criteria | Every admitted lifecycle/timing row has one Shared receiver, all non-admitted rows remain once in NXVM, no duplicate fixture/setup path or public ABI is introduced, and both full unit runs pass. |

## S88 Acceptance

The CPU-only reset, prepared-entry, two-instance isolation and repeat-timing
rows now have one Shared lifecycle receiver. The NXVM residual no longer
contains a timing or lifecycle implementation; it retains only named S89 and
later work. Focused receivers and all static gates pass; detached process-owned
full units pass **441/441** on x64 and x86. No production/API, firmware, asset,
INI or EXE input changed. See [S88 evidence](../../etc/evidence/t539-s88-execution-lifecycle-receiver.md).
S88 is accepted; T539 remains open for S89-S100.

## S89 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S89, automatically admitted continuation. |
| Objective | Move only the CPU-owned NMI mask/delivery and prefetch-reservation rows from the residual execution-context test. |
| Non-goals | Paging/INVLPG, protected fault/pending-event rows, public IRQ/board wiring, firmware, production APIs, assets, INI, executables and unrelated residual CPU rows. |
| Files And ABI Surface | Split existing static functions by actual CPU responsibility; reuse existing CPU-local fixture and create no shared framework, board adapter or public API. Retain any concrete board path once in NXVM under a named later receiver. |
| Verification | Record exact allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, applicable static gates, manifest/corpus, authority, documentation governance and diff checks. |
| Exit Criteria | Every admitted NMI/prefetch row has one Shared owner, every excluded row remains once in NXVM, neither width regresses, and full unit suites pass. |

## S89 Acceptance

`cpu_signal_case()` and `cpu_prefetch_case()` now have exactly one Shared
receiver, `cpu_execution_signal_prefetch`. The NXVM residual deletes both
functions and calls, retaining only named S90/S91 work. Focused x64/x86
receivers, T332, authority, manifest/corpus, documentation governance and
diff gates pass; detached full unit suites pass **442/442** on x64 and x86.
No production/API, firmware, asset, INI or executable input changed. See the
[S89 evidence](../../etc/evidence/t539-s89-signal-prefetch-receiver.md). S89 is
accepted; T539 remains open for S90-S100.

## S90 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S90, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S90 is the next linear package after accepted S89. |
| Objective | Move only 80186 LGDT availability, paging-control and INVLPG rows from the residual execution-context test to bounded Shared CPU receivers. |
| Non-goals | Protected fault/pending-event rows, public paging/board wiring, firmware, production APIs, assets, INI, executables and unrelated residual CPU rows. |
| Reference Baseline | `30fa60856`, accepted S89; the residual source contains S90 paging/INVLPG and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split only `cpu_80186_lgdt_gate()`, `cpu_paging_prepare()`, `cpu_paging_control_gate()`, `cpu_paging_control_forms()`, `cpu_paging_invlpg_case()`, `cpu_paging_invlpg_rejection()` and `cpu_paging_cr0_mutable_controls()` to one Shared successor. Reuse the CPU-local fixture; no production, public API, framework or board adapter changes. |
| Applicable Rules | NXVM and Shared architecture/coding/execution/documentation rules; one owner per CPU-only test behavior, no duplicate fixture path and no public board assertion in Shared. |
| Verification | Record allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, T332, manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` paging/INVLPG receiver, one NXVM residual source with only debug and S91 rows, no duplicate helper/function and no new fixture/API. |
| Asset Needs | None; repository-only ownership migration. |
| Reporting Requirements | Record moved and retained functions, focused/full test evidence, static-gate impact and artifact determination. |
| Stop Conditions | Stop and report if a row requires public Core paging, physical mapping, IRQ/board wiring, a new Shared API or a duplicate fixture path. |
| Exit Criteria | Every admitted paging/INVLPG row has one Shared owner, all excluded rows remain once in NXVM, neither width regresses, and full unit suites pass. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S90 consumes only CPU-local paging/INVLPG and 80186 gate rows. S91 retains fault/pending-event rows; public Core paging and board IRQ paths remain named NXVM receivers. |

## S90 Acceptance

The 80186 LGDT-gate, paging-control, INVLPG and CR0 mutable-control helpers
now have one Shared `cpu_execution_paging` receiver. NXVM deletes all seven
helpers and calls, retaining only debug and named S91 fault/event work.
Focused x64/x86 receivers, T332, authority, manifest/corpus, documentation
governance and diff gates pass; detached full units pass **443/443** on x64
and x86. No production/API, firmware, asset, INI or executable input changed.
See [S90 evidence](../../etc/evidence/t539-s90-execution-paging-receiver.md).
S90 is accepted; T539 remains open for S91-S100.

## S91 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S91, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S91 is the next linear package after accepted S90. |
| Objective | Move only CPU-owned protected interrupt preparation, UD cache-preservation and pending-event rollback rows from the residual execution-context test. |
| Non-goals | Public Core paging, PIC IRQ delivery, board transactions, firmware, production APIs, assets, INI, executables and unrelated CPU rows. |
| Reference Baseline | `d8e3665ec`, accepted S90; the residual source contains debug and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split existing CPU-local static functions by responsibility, reusing the existing CPU bus fixture with no framework, board adapter or public API. |
| Applicable Rules | NXVM and Shared architecture/coding/execution/documentation rules; one owner per CPU-only behavior and no duplicate fixture path. |
| Verification | Record allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, T332, manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` fault/event receiver, a residual NXVM test containing only debug, and no new fixture/API. |
| Asset Needs | None; repository-only ownership migration. |
| Reporting Requirements | Record moved/retained functions, test evidence, static-gate impact and artifact determination. |
| Stop Conditions | Stop and report if a row requires public Core paging, PIC/board wiring, a new Shared API or a duplicate fixture path. |
| Exit Criteria | Every admitted fault/event row has one Shared owner, excluded rows remain once in NXVM, neither width regresses, and full unit suites pass. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S91 consumes only artificial-IDT CPU fault/event rows; public IRQ and Core paging paths remain named NXVM receivers. |

## S91 Acceptance

The three protected fault/event helpers now have one Shared
`cpu_execution_fault_event` receiver. NXVM deletes the duplicate helpers and
transfers the T337 #UD inventory to that receiver; its independent debug API
test remains. Focused x64/x86 and all static gates pass; detached units pass
**444/444** on both widths. No production/API, firmware, asset, INI or EXE
input changed. See [S91 evidence](../../etc/evidence/t539-s91-execution-fault-event-receiver.md).
S91 is accepted; T539 remains open for S92-S100.

## S92 Acceptance

The CPU-only protected data-access, far-transfer and outer-return receivers,
with their sole CPU-local fixtures, now have one Shared owner in
`test/x86/devices/cpu`. NXVM removes the duplicated targets and source copies.
Its retained PIC/board receivers continue to own interrupt delivery and use
the Shared fixtures without recreating CPU setup.

Focused x64/x86 receivers, T317/T332, CPU/PIC authority, Shared
manifest/corpus and documentation governance pass. Detached full unit suites
pass **447/447** on x64 and x86. This is test/CMake/documentation-only work:
no production/API, firmware, asset, INI or EXE input changed. See the
[S92 evidence](../../etc/evidence/t539-s92-protected-receivers.md). S92 is
accepted; T539 remains open for S93-S100.

## S93 Acceptance

The CPU-only 16-bit task-switch, 32-bit task-switch decode and 32-bit
task-state receivers, with their sole fixture, now have one Shared owner in
`test/x86/devices/cpu`. NXVM deletes the duplicate targets and source copies.
The retained PIC board receiver consumes the Shared fixture while continuing
to own actual interrupt routing; cross-width, paging and TSS I/O paths remain
separate NXVM receivers.

Focused x64/x86 receivers, T317/T332, CPU/PIC authority, Shared
manifest/corpus and documentation governance pass. Detached full unit suites
pass **450/450** on x64 and x86. This is test/CMake/documentation-only work:
no production/API, firmware, asset, INI or EXE input changed. See the
[S93 evidence](../../etc/evidence/t539-s93-task-state-receivers.md). S93 is
accepted; T539 remains open for S94-S100.

## S94 Acceptance

The eight CPU-only bit-scan/test, double-shift, IMUL2, MOVX, sign-extend,
SETcc and LEA receivers, with their sole operand-probe fixture, now have one
Shared owner in `test/x86/devices/cpu`. NXVM deletes the duplicate targets and
source copies. The named `core_machine_*` receivers remain NXVM board owners;
no production code, public API, firmware, asset, INI or executable input
changed.

Focused x64/x86 Shared and retained board receivers pass **16/16** on each
width. T317/T332, CPU/PIC authority, Shared manifest/corpus and documentation
governance pass. Detached full unit suites pass **458/458** on x64 and x86.
See the [S94 evidence](../../etc/evidence/t539-s94-cpu-instruction-receivers.md).
S94 is accepted; T539 remains open for S95-S100.

## S95 Acceptance

The direct CPU-only operand/address and prefix-attribute receivers now have
one Shared owner in `test/x86/devices/cpu`, reusing the existing Shared
instruction fixture. NXVM deletes their duplicate targets and source copies.
The Core-machine operand/address and prefix-attribute board paths remain
separate NXVM owners; no production code, public API, firmware, asset, INI or
executable input changed.

Focused x64/x86 Shared and retained-board tests pass **3/3** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **460/460** on x64 and x86. See the
[S95 evidence](../../etc/evidence/t539-s95-operand-prefix-receivers.md). S95 is
accepted; T539 remains open for S96-S100.

## S96 Acceptance

The direct CPU-only legacy-LOCK and immediate-IMUL encoding receivers now
have one Shared owner in `test/x86/devices/cpu`, reusing the existing Shared
instruction fixture. NXVM deletes the duplicate targets and source copies.
`core_machine_legacy_lock_s1_smoke` remains NXVM because it owns real port and
IOPL board wiring; no blanket LOCK compatibility path was added.

Focused x64/x86 Shared and retained-board tests pass **3/3** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **462/462** on x64 and x86. See the
[S96 evidence](../../etc/evidence/t539-s96-lock-imul-receivers.md). S96 is
accepted; T539 remains open for S97-S100.

## S97 Acceptance

The three direct CPU-only control-transfer receivers now have one Shared owner
in `test/x86/devices/cpu`. S97 initially classified the IDT privilege receiver
as NXVM from its `device_support.h` include; S99 corrected that finding when it
proved the include supplied only a generic bit-test macro. No production code,
public API, firmware, asset, INI or executable input changed.

Focused x64/x86 successor and retained-IDT tests pass **4/4** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **465/465** on x64 and x86. See the
[S97 evidence](../../etc/evidence/t539-s97-residual-cpu-classification.md). S97
is accepted; T539 remains open for S98-S100.

## S98 Acceptance

Shared `src/x86/devices/cpu` remains the sole CPU timing implementation. The
timing/catalog runners remain their one NXVM owner because each composes an
App machine/profile, publishes board-time observations, or produces a
generated result contract. S98 adds no synthetic Shared machine fixture, no
second timing route and no copied formula table.

The CPU timing-manifest catalog, x64/x86 full units (**465/465** each), T317,
CPU/PIC authority, Shared manifest/corpus and documentation governance pass.
See the [S98 evidence](../../etc/evidence/t539-s98-timing-catalog-boundary.md).
S98 is accepted; T539 remains open for S99-S100.

## S99 Acceptance

S99 corrected S97's include-only false boundary: the IDT privilege CPU
receiver now has one Shared owner. Its former App macro use is an equivalent
local bit test, not a new API. The retained PIC-board receiver directly
includes the canonical Shared fixture; the last NXVM fixture forwarding header
is deleted. IDT CPU semantics and PIC delivery remain separate one-owner tests.

Focused x64/x86 Shared IDT and retained PIC-board tests pass **2/2** on each
width. T317, CPU/PIC authority, Shared manifest/corpus and documentation
governance pass. Detached full unit suites pass **466/466** on x64 and x86.
See the [S99 evidence](../../etc/evidence/t539-s99-final-cpu-path-cleanup.md).
S99 is accepted; T539 remains open for S100.

## S100 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S100, automatically admitted final continuation. |
| Admission And Approval | The owner approved automatic, bounded, strictly linear numeric T539 continuation tasks; S100 follows accepted S99. |
| Objective | Prove final CPU test ownership: each CPU-only receiver and fixture has one Shared owner; each machine, PIC, board-time and result-publication receiver has one named NXVM owner; no retired App CPU test path survives. |
| Non-goals | Further CPU behavior/timing changes, new Shared APIs/frameworks, profile changes, firmware, assets, INI and executable changes. |
| Reference Baseline | `dfe846cbc`, accepted S99. |
| Candidate Proposal | [Independent Shared chips](../../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Read-only inventory and evidence unless a real duplicate path is found. Verify no App target directly links `x86-cpu`, no App fixture forwards the Shared CPU fixture, and every retained App CPU-named receiver has a Core-machine/board/result owner. |
| Applicable Rules | One owner per fact and behavior; Shared owns chip semantics and CPU-only fixtures, NXVM owns composition, board signal routes and product result artifacts. |
| Verification | Full x64/x86 unit suites; full integration suite; T317/T332/static gates; Shared manifest/corpus; CPU/PIC authority; documentation governance; diff and inventory checks. |
| Exit Criteria | The complete receiver map is evidenced, both unit widths and integration pass, no duplicate path remains, and T539 can close without an unallocated CPU boundary. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |

## S100 Acceptance

The final inventory found one remaining CPU-only EFLAGS receiver. It is now
the Shared `x86.cpu_eflags_local` receiver and uses the canonical CPU bus
fixture. No App target directly links `x86-cpu`; the remaining App sources
with CPU fixtures are named machine/board owners and include canonical Shared
fixtures directly rather than forwarding them. The timing ledger/manifests
remain NXVM because they compose machine/profile time, board inputs and
published result artifacts; no second Shared timing runner was created.

The native modal-window unit test was also correctly declared CTest-exclusive:
it owns a real nested Win32 message loop and was the only full-suite race. Its
coverage and assertions remain unchanged.

Full detached unit suites pass **467/467** on x64 and x86; the x64 external
integration suite passes **20/20**. T317, T332, CPU/PIC authority, Shared
manifest/corpus, documentation governance and diff checks pass. This is
test/CMake/documentation-only work: no production/API, firmware, asset, INI
or executable input changed, so no executable rebuild is required. See the
[S100 evidence](../../etc/evidence/t539-s100-whole-cpu-acceptance.md). S100 is
accepted. Its follow-up source-copy gap is owned by active S101; T539 remains
open.

## S101 Acceptance

S101 removes the exact nine-file obsolete App CPU corpus. The source implementation
is now solely `src/x86/devices/cpu`; NXVM retains `cpu_bus.c` only as the named
board adapter. The CPU/PIC authority gate now rejects any reintroduced historical
CPU source, and the CPU-boundary negative fixture begins with a clean owned work
tree so a prior run cannot masquerade as a source duplicate. The two legacy
verification tools now inspect the canonical CPU paths.

The similar-issue sweep found no live App CPU copy or include. The only remaining
old-path spelling is an intentionally injected forbidden include in the Shared
negative test; `cpu_bus.c` references are the retained board adapter, not a CPU
implementation. The finite chip ledger has no unallocated disposition.

Focused x64/x86 CPU-boundary and decoder-ledger checks pass. Full repository-only
unit suites pass **467/467** on x64 and x86; the x64 external integration suite
passes **20/20**. T317, T332, CPU/PIC authority, x86 corpus, all six manifests,
executor closure, documentation governance and `git diff --check` pass. This
changes only deleted dead source, static verification, tests, tools and evidence:
no runtime source, public API, firmware, asset, INI or executable input changes,
so no EXE rebuild is required. See [S101 evidence](../../etc/evidence/t539-s101-cpu-source-cleanup.md).

## S86 Acceptance

The 257-line CPU-only debug-state receiver—MOV-DR, debug exceptions and data
breakpoints—now has one Shared `cpu_debug_state` owner using the established
instruction fixture. Its NXVM source and target are removed;
`machine_debug_state_board_smoke` remains the named public-board receiver.
T332 now maps the debug receiver through the canonical `devices/cpu/` entry.

Focused successors and T332 pass on x64 and x86. Both current full
repository-only unit logs contain 439 passing tests and zero failed-test
records. CPU/PIC authority, Shared manifest/corpus, documentation governance
and diff checks pass. This is test/CMake/documentation-only work: no
production/API, firmware, asset, INI or EXE input changed, so no executable
rebuild is required. See the [S86 evidence](../../etc/evidence/t539-s86-debug-state-receiver.md).
S86 is accepted; T539 remains open for S87-S98.

## S85 Acceptance

The 392-line CPU-only CLTS/SMSW/LMSW/MOV-CR receiver now has one Shared
`cpu_control_state` owner using the established instruction fixture. Its NXVM
source and target are removed; `machine_control_state_board_smoke` remains the
named public board receiver.

The same intake found that T332's static lifecycle gate still resolved S83/S84
Shared receivers as obsolete App paths. Its source resolver now recognizes the
canonical `test/x86/devices/` inventory prefix, so the 44-owner gate verifies
the real Shared files without a parallel inventory. Focused receivers and T332
pass on x64/x86; each 438-case unit suite was executed. The x64 run recorded
the existing `unit.vm-runner-error-propagation-smoke` flake and x86 recorded
the existing `x86.cpu_movs` flake; both pass immediately in isolated reruns.
Shared manifest/corpus, CPU/PIC authority, documentation governance and diff
checks pass. This is test/CMake/documentation-only work: no production/API,
firmware, asset, INI or EXE input changed, so no executable rebuild is
required. See the [S85 evidence](../../etc/evidence/t539-s85-control-state-receiver.md).
S85 is accepted; T539 remains open for S86-S98.

## S84 Acceptance

The four CPU-only system-table receivers now have one Shared owner:
`cpu_descriptor_system`, `cpu_dttr_s61`, `cpu_lgdt_lidt`, and
`cpu_sgdt_sidt`. The former NXVM copies (1,068 source lines) are retired; no
board path moved. The descriptor receiver no longer includes an App header:
its two generic CR0 bit-macro uses are equivalent local expressions, without
a Shared API addition.

Each successor passes on x64 and x86, the 437-case unit suite was executed on
both widths, and every migration receiver passes in focused reruns. The x64
parallel run recorded five pre-existing string-test flakes and the x86 runs
recorded one `cpu_movs` flake; all six passed immediately in isolation. The
x64 serial run recorded the existing runner-error-propagation flake, which
also passed immediately in isolation. Shared manifest/corpus, CPU/PIC
authority, documentation governance and diff checks pass. This is
test/CMake/documentation-only work: no production/API, firmware, asset, INI
or EXE input changed, so no executable rebuild is required. See the
[S84 evidence](../../etc/evidence/t539-s84-system-table-receivers.md). S84 is
accepted; T539 remains open for S85-S94.

## S40 Acceptance

Actual pushed NXVM P1 `e4a7615d1` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at actual-commit review. The
two original ARPL sources (910 lines) are retired; every base and S53 case
has a CPU or public-board receiver, including register/memory forms, illegal
encodings, protected faults and PIC IRQ delivery. Eight code/test/build/gate
paths add 689/remove 941 lines (net -252); evidence is separate. CPU tests
link only `x86-cpu`; board tests use public machine and real PIC operations.
Complete x64/x86 builds and units pass 413/413 per width; all 66 specialized
gates and the 412-row T344 matrix pass on both widths. Six unchanged Shared
manifests, documentation governance and diff checks pass. No production/API,
Shared, firmware, INI or EXE input changed. See [S40 evidence](../../etc/evidence/t539-s40-arpl-migration.md).
S40 is accepted; 56 original direct-private `.c` consumers plus the common
fixture header remain assigned to S41-S51. T539 stays open.

## S39 Acceptance

Actual pushed NXVM P1 `07019f588` has exactly fourteen scoped paths, passes
`git show --check`, and equals `origin/master` at review. The 170 original
scalar IN/OUT contexts and sixty original INS/OUTS contexts retain CPU or
public-board receivers; the separate port-ownership behaviors retain their
board receiver. The thirteen code/test/build/gate paths add 1,239 and remove
1,564 lines (net -325); the 44-line evidence report is separate. There is no
production/API or EXE input change. Complete x64/x86 builds and repository-
only units pass 413/413 each. All 66 specialized gates pass per width,
including T317/T332/T337/T344, CPU/PIC authority and the 412-row direct
matrix. Six unchanged Shared manifests pass within eleven manifest tests;
documentation governance passes. See [S39 evidence](../../etc/evidence/t539-s39-port-io-migration.md).
S39 is accepted; 58 direct-private `.c` consumers plus the common fixture
header remain assigned to S40-S51. T539 stays open.

## S31 Acceptance

Actual pushed NXVM P1 `38bc5b10c` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 335 original
contexts retain one receiver: 324 CPU-owned and eleven board-owned. Code,
test, build and gate changes add 1,289/remove 1,468 lines (net -179); the
49-line evidence report is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors. Complete x64/x86 units pass 397/397 each;
T317/T332/T344 and CPU/PIC gates, 396-row direct matrix, six unchanged
Shared manifests, and documentation governance pass. No production/API or
executable input changed. [S31 evidence](../../etc/evidence/t539-s31-imul-group2-migration.md)
contains the original-case receiving map. The S31 packet is closed; 74
original private-test consumers remain assigned to S32-S42. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

## S30 Acceptance

Actual pushed NXVM P1 `442088410` has exactly 23 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 549 original
contexts retain one receiver: 539 CPU-owned and ten board-owned. The 19
code/test/build paths add 1,429/remove 1,506 lines (net -77); six CPU tests
link only `x86-cpu`, while six board tests retain real faults or IRQ through
Core-machine. Complete x64/x86 units pass 395/395 each, 66 specialized gates
pass per width, six unchanged Shared manifests verify, and documentation
governance passes. No production/API or executable input changed. See
[S30 evidence](../../etc/evidence/t539-s30-bit-condition-extension-migration.md).
The S30 packet is closed; 76 original private-test consumers remain assigned
to S31-S42. S43-S45 still own lifetime, physical relocation and whole-CPU
acceptance.

## S32 Acceptance

Actual pushed NXVM P1 `9f785a551` has exactly eight scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 775
original contexts retain one receiver: 767 CPU-owned and eight board-owned.
Code/test/build/gate changes add 1,536/remove 1,589 lines (net -53); the
57-line P1 evidence draft is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors; board tests use public machine operations.
Complete x64/x86 builds and units pass 399/399 on each width; all 66
specialized gates pass per width, including T317/T332/T337/T344, CPU/PIC
authority, the 398-row direct matrix, six unchanged Shared manifests and
documentation governance. No production/API or executable input changed.
[S32 evidence](../../etc/evidence/t539-s32-legacy-alu-lock-migration.md) contains
the original-case receiving map. The S32 packet is closed; 72 original
private-test consumers remain assigned to S33-S42. S43-S45 retain lifetime,
physical relocation and whole-CPU acceptance.

## S33 Acceptance

Actual pushed NXVM P1 `6ca7f61ac` has exactly six scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. The
original 19 first-group functions retain one receiver each: 200 CPU-only
instruction executions and 23 public board contexts. Code/test/build/gate
changes add 1,056/remove 1,041 lines (net +15); the 43-line evidence report
is separate. The CPU test links only `x86-cpu` and compiles with warnings as
errors. The board test uses public machine operations for 11 protected
faults and twelve real-mode divide deliveries. Historic T316/T401 first-group
success markers moved to the CPU receiver. Complete x64/x86 builds and units
pass 401/401 on each width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 400-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S33 evidence](../../etc/evidence/t539-s33-inc-dec-first-group-migration.md)
contains the original-case receiving map. The S33 packet is closed; 72
original private-test consumers remain assigned to S34-S42, including the
unmigrated S34/S35 functions in `core_machine_inc_dec_smoke.c`. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

## S34 Acceptance

Actual pushed NXVM P1 `3b2d17d97` has exactly seven scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 13
original second-group functions retain one receiver each: 111 CPU instruction
executions and eight public board faults. Two originally mixed functions were
split at their fault sections. Code/test/build changes add 843/remove 881
lines (net -38); the 44-line evidence report is separate. One CPU test links
only `x86-cpu`; one board test uses public machine operations. The shared
test-only board-fault helper removes duplicate S33/S34 checking logic while
preserving the 23 S33 board cases. Complete x64/x86 builds and units pass
403/403 per width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 402-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S34 evidence](../../etc/evidence/t539-s34-test-add-adc-sbb-migration.md)
contains the receiving map. S34 is closed; the last 19 `inc_dec` functions
remain assigned to S35. The original private-test consumer inventory remains
at 72 until that file is retired. S43-S45 still own lifetime, physical
relocation and whole-CPU acceptance.

## S35 Acceptance

Actual pushed NXVM P1 `0fc194460` has ten scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 19
remaining functions have one receiver: 325 CPU instruction executions and
twelve public board contexts. The mixed source is retired; the CPU receiver
links only `x86-cpu`, and the board receiver uses public machine operations.
The shared test-only divide fixture removes repeated S33/S35 delivery checks
while preserving S33's cases. Code/test/build changes add 463/remove 733
lines (net -270); the 45-line evidence report is separate. Complete x64/x86
builds and units pass 404/404 per width, all 66 specialized gates pass per
width, including T317/T332/T337/T344, CPU/PIC authority, the 403-row direct
matrix, six unchanged Shared manifests and documentation governance. No
production/API or executable input changed. [S35 evidence](../../etc/evidence/t539-s35-final-inc-dec-migration.md)
contains the receiving map. S35 is closed; 71 original private-test consumers
remain assigned to S36-S45. S46-S48 retain lifetime, physical relocation and
whole-CPU acceptance.

## S36 Acceptance

Actual pushed NXVM P1 `607fdea7a` contains exactly 17 scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. Four private-state FLAGS tests are retired, including the PUSHF/POPF
source includer. All original case families have one CPU or public-board
receiver; the six replacement tests pass on x64 and x86. Code/test/build/gate
changes add 1,562/remove 1,982 lines (net -420). Complete x64/x86 units pass
406/406 each; both specialized gate aggregates, T317/T332/T337/T344,
CPU/PIC authority, 405/404-row direct matrices, six unchanged Shared
manifests, and documentation governance pass. No production/API or executable
input changed. [S36 evidence](../../etc/evidence/t539-s36-flags-migration.md)
records the receiving map. S36 is accepted; 66 original direct-private `.c`
test consumers plus one shared fixture header remain assigned to S37-S45.

## S37 Acceptance

Actual pushed NXVM P1 `3c826c32d` has exactly thirteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. The two original mixed-owner MOVS/LODS sources are retired; all 119
original contexts have a CPU or public-board receiver, with complementary
protected-fault assertions rather than duplicate production paths. Eleven
code/test/build/gate paths add 1,065/remove 1,327 lines (net -262); the
evidence and inventory documentation are separate. Both CPU receivers link
only `x86-cpu`; board receivers use public machine operations and real PIC,
guest descriptors, faults and interrupt frames. Complete x64/x86 units pass
408/408 each; both 66-target specialized gate aggregates, the 407-row T344
direct matrix, six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared source/test, firmware, INI or EXE
input changed. [S37 evidence](../../etc/evidence/t539-s37-string-transfer-migration.md)
records the receiving map. S37 is accepted; 64 original direct-private `.c`
test consumers plus one fixture header remain assigned to S38-S45.

## S38 Acceptance

Actual pushed NXVM P1 `ae76a7c84` has exactly fifteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. All 203 original STOS/SCAS/CMPS contexts retain CPU or public-board
receivers. The three mixed-owner sources are retired; six replacement tests
preserve historical success markers. Thirteen code/test/build/gate paths add
1,682/remove 2,034 lines (net -352); the two evidence documents are separate.
Complete x64/x86 builds and units pass 411/411 per width. Specialized gates
pass 66 x64 and 68 x86 targets, including T317/T332/T337/T344, CPU/PIC
authority and direct matrices of 410/409 rows. All eleven selected manifest
tests, including six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared, firmware, INI or EXE input
changed; the regenerated x86 EXE build side effect was removed. The
[S38 evidence](../../etc/evidence/t539-s38-string-scan-compare-migration.md)
records the receiving map. S38 is accepted; 61 direct-private `.c` test
consumers plus one fixture header remain assigned to S39-S45.

## S41 Acceptance

Actual pushed NXVM P1 `99ab4c002` contains exactly eighteen scoped paths,
passes `git show --check`, and equals `origin/master` at actual-commit review.
The 899-line mixed BOUND source is retired. Its 247-line CPU receiver links
only `x86-cpu`; its 335-line board receiver uses public machine operations,
guest table construction and the real PIC. All original families retain one
receiver: width/profile, size attributes, invalid forms, segment routes,
signed boundaries, SIB/SS, VM86, real/protected faults and IRQ delivery.
The BOUND-local predecode fixes the previously reproduced 32-bit register-only
form from internal CPU error to terminal #UD without a new path or API.

Eight code/test/build/gate paths add 609/remove 922 lines (net -313); evidence
and task state are separate. Complete x64/x86 builds and units pass 414/414
each. Both 67-target specialized gate aggregates pass per width, including
T317/T332/T337/T344, CPU/PIC authority and the 413-row direct matrix. Six
unchanged Shared manifests, documentation governance and diff checks pass.
Because CPU production changed, all four runnable profiles have rebuilt,
optimized, stripped T539 x64/x86 EXEs; their hashes and no-INI-change proof
are recorded in [S41 evidence](../../etc/evidence/t539-s41-bound-migration.md).
S41 and S42 are accepted. Fifty-two direct-private `.c` consumers plus the
shared fixture header remain assigned to S43-S51. T539 remains open.

## S42 Acceptance

Coordinator actual-change review accepts pushed NXVM P4 `2935b5886`: exactly
the three named table-register smoke sources and registrations are retired;
CPU-only forms remain in their three `x86-cpu` receivers, while real guest and
board contexts remain in `machine-table-register-board-smoke`. Test sources
add 109/remove 1,370 lines (net -1,261); gate declarations add 33/remove 44.
No production, Shared, firmware, INI or executable input changed. Complete
x64/x86 repository-only units pass 415/415 per width; specialized gates pass
66/66 per width. T317 has 35 strict receivers; T332 recognizes the three
CPU-local instruction fixtures; T344 classifies the one public board
constructor (123 total). Documentation governance and diff checks pass, and
the reviewed commit equals `origin/master`. See [S42 evidence](../../etc/evidence/t539-s42-table-register-migration.md).

## S43 Acceptance

P1 `4ff59cd5c` moves every descriptor/table/cache case from the mixed source
to the CPU-only `cpu_descriptor_system_smoke.c`; its retained original source
now contains only S45's `SMSW/LMSW/CLTS/MOV CR` control-state cases. The new
receiver links only `x86-cpu`; no descriptor case remains in the S45 input.
The nine scoped P1 paths pass `git show --check` and equal `origin/master`.
No production/API, Shared, firmware, INI or executable input changed.

Complete repository-only units pass 457/457 on both x64 and x86. Both widths'
specialized aggregate passes, including T317 (36 strict CPU receivers), T332
(36 fixture owners), T337, T344, T345, CPU/PIC authority, direct-matrix,
manifest and documentation-governance gates. See
[S43 evidence](../../etc/evidence/t539-s43-descriptor-system-migration.md).
S43 is accepted; S44 owns descriptor queries and S45 alone owns the retained
control-state source portion. T539 remains open.

## S44 Acceptance

S44 retires the two 1,748-line mixed LAR/LSL and VERR/VERW sources. The
CPU-only receivers retain every instruction-local selector, visibility,
operand, prefix, rollback, VM86 and LDT result without a `core_machine`
dependency. The retained 80386 board timing runner owns the four real
page-granularity LSL rows (register/memory: 21/25/22/26 ticks); public Core
receivers retain descriptor-table and IRQ delivery behavior. No production,
Shared, firmware, INI or executable input changed.

Repository-only units pass 416/416 on x64 and x86. Both specialized aggregates
pass, including T317 (36 strict CPU receivers), T332 (36 fixture owners),
T337, T344, T388, CPU/PIC authority, direct matrix, manifest and documentation
governance gates. The focused timing runner passes on x86; S44 evidence records
the receiver map and verification. S44 is accepted; S45 owns control state and
T539 remains open.

## S45 Acceptance

| Field | Contract |
| --- | --- |
| Identifier / mode | M5 T539 S45, accepted implementation. |
| Admission and approval | The owner granted automatic admission for each bounded T539 S. This packet admits the next bounded CPU-control-state batch. |
| Objective | Retire direct-private CPU access from CLTS/SMSW/LMSW/MOV-CR control-state tests while retaining one CPU-local or public-board receiver for every original case. |
| Precise scope | Consume `core_machine_clts_s62_smoke.c`, `core_machine_msw_s63_smoke.c`, and only `dt_test_msw_and_control_registers()` from `core_machine_descriptor_system_smoke.c`. Cover CR0 TS/PE effects, CR2/CR3 reads and writes, real/protected/VM86 and CPL behavior, register and memory operands, prefixes/LOCK, faults and IRQ delivery. |
| Non-goals | No CPU production change, new public API, Shared change, firmware/asset/INI/EXE update, timing reinterpretation, or migration of any remaining descriptor-system case. |
| Reference baseline | `CURRENT.md`; `t539-cpu-work-packages.md`; `t539-cpu-incremental-inventory.md`; S43/S44 evidence; the existing x86 CPU and public Core-machine test contracts. |
| Candidate implementation | Move instruction-local control semantics to an `x86-cpu` receiver using CPU-only fixtures. Retain actual fault delivery, memory boundary and PIC IRQ observations in public Core-machine receivers. Delete the old private sources only after the exact map is complete. |
| Files and ABI surface | Test, CMake gate, task-state and evidence paths only. Production and ABI surface remain unchanged. |
| Applicable rules | NXVM architecture/coding/documentation guides and shared execution, architecture, coding and documentation rules named by `docs/nxvm/README.md`. |
| Verification | Both affected targets build and pass on x86/x64; repository-only units pass 416/416 per width; T317/T332/T337/T344/T345, CPU/PIC authority, direct-matrix, manifest, documentation and diff gates pass. |
| Expected markers | Retain `M5:T316:S68:CLTS:OK` and `M5:T316:S69:MSW:OK`, or record their exact successor receiver markers in the evidence map. |
| Asset needs | None; repository-only unit fixtures only. No executable rebuild is required. |
| Reporting requirements | Record a complete original-case-to-receiver map, counted test-path delta, retained owner path, focused and full verification, and a similar-issue sweep. |
| Stop conditions | Stop for a new production/ABI requirement, an unmapped original case, a disagreement between retained behavior and the CPU authority, or any necessary scope beyond the named control-state cases. |
| Exit criteria | All named original cases have exactly one receiver; no named source retains direct-private CPU access; no duplicate production path is introduced; required verification and actual-change review pass. |
| Original owner request | Split CPU migration into small, traceable S tasks; automatically admit each, preserve clean ownership boundaries, and avoid patch-on-patch extraction. |
| Similar-issue sweep | Audit CLTS/SMSW/LMSW/MOV CR across all supported CPU profiles, operand and prefix forms, CR0/CR2/CR3 effects, privilege/VM86/fault rollback, memory and PIC delivery. |

The two complete mixed sources and the retained descriptor-system block are
retired with no duplicate execution path.  The CPU receiver owns only
instruction-local state; the public board receiver owns actual PIC delivery,
interrupt frames and the existing early-80386 board option.  The exact map and
dual-width evidence are recorded in
[S45 evidence](../../etc/evidence/t539-s45-control-state-migration.md). T539
remains open for S46 and later packets.

## S46 Acceptance

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S46, continuation implementation. |
| Admission And Approval | The owner granted automatic admission for each bounded T539 S. This packet admits the next finite debug-state migration batch on 2026-09-30. Allowed target: NXVM only. |
| Objective | Retire direct-private CPU access from MOV DR and TF/#DB test receivers while retaining one CPU-local or public-board receiver for every original semantic case. |
| Non-goals | No CPU production change, new public API, Shared/MyNES change, firmware, asset, INI or EXE update, timing reinterpretation, or migration of any S47-plus source. |
| Reference Baseline | This Current packet; [CPU work packages](../../etc/architecture/t539-cpu-work-packages.md); [incremental inventory](../../etc/evidence/t539-cpu-incremental-inventory.md); S45 evidence; existing x86 CPU fixture and public Core-machine contracts. |
| Candidate Proposal | Move MOV-DR register semantics, profile/operand/prefix/privilege rejection and synthetic state rollback to CPU-only fixtures. Retain actual exception vector delivery, #DB/#UD/IRQ ordering, trap frames, PIC acknowledgement and protected-IDT observation in public Core-machine receivers. Delete the old mixed sources only after the complete receiving map is proven. |
| Files And ABI Surface | Consume only `core_machine_debug_mov_s59_smoke.c` and `core_machine_tf_db_s60_smoke.c`; add bounded CPU and board receivers plus CMake/gate/evidence/state paths as needed. Production and ABI surface remain unchanged. |
| Applicable Rules | NXVM architecture and coding guides; shared execution, architecture, coding and documentation rules named by `docs/nxvm/README.md`. The tests retain one owner per fact and no parallel execution path. |
| Verification | Build affected x86/x64 unit targets; run complete `ctest -L unit -j8` once per width; run T317/T332/T337/T344/T345, CPU/PIC authority, direct-matrix, manifest and documentation gates; run `git diff --check`. |
| Expected Markers | Preserve the original test success coverage or record exact successor markers and original-case-to-receiver mapping in S46 evidence. |
| Asset Needs | None; repository-only unit fixtures only. No executable rebuild is required. |
| Reporting Requirements | Record every original case family, receiving owner, test-path delta, retained public board path, full verification and a similar-issue sweep. |
| Stop Conditions | Stop for a new production/ABI requirement, an unmapped original case, a disagreement between retained behavior and CPU authority, or necessary scope beyond MOV DR and TF/#DB. |
| Exit Criteria | All named original cases have exactly one receiver; neither named source retains direct-private CPU access; no duplicate production path is introduced; required verification and actual-change review pass. |
| Original Owner Request | Implement chip/device architecture extraction in small automatically admitted S tasks with explicit ownership, preservation and no patch-on-patch duplication. |
| Similar-Issue Sweep | Audit MOV DR and TF/#DB across supported CPU profiles, register forms, prefixes/LOCK, real/protected/VM86 privilege, rollback, exception vectors, trap frames and PIC ordering. |

The two named direct-private sources are retired. `cpu-debug-state-smoke`
owns MOV-DR transfer/rejection and CPU-local debug-register behavior through
the shared instruction fixture; `machine-debug-state-board-smoke` owns real
and protected exception delivery, interrupt frames and PIC ordering through
the public Core-machine boundary. Complete x64/x86 unit suites pass 416/416
per width, and the current specialized-gate aggregate passes per width. The
T332 fixture inventory classifies the new CPU receiver explicitly; no
production/API, Shared, firmware, INI or executable input changed. The
[S46 evidence](../../etc/evidence/t539-s46-debug-state-migration.md) retains the
complete receiver map and validation record. S46 is accepted; T539 remains
open for S47 and later packets.

## S47 Acceptance

Actual pushed NXVM P1 `583ba9a6c` retires the five S3--S7 direct-private
protected-mode smoke consumers.  Their official targets now share one public
bootstrap fixture and preserve the five historical markers.  Architectural
gate, external/NMI, outer-stack, IRET/RETF and call-gate contracts retain
their public-board receivers; non-architectural cache mutations are retired,
not recreated as a test seam.  The 80286/80386 timing runners retain only the
renamed timing recipe for S50.

The commit changes 18 NXVM paths, adding 1,028 and removing 1,297 lines
(net -269), with no production/API, Shared, firmware, INI or executable input.
Complete repository-only units pass 512/512 on x64 and x86; both widths'
current specialized gates, the T344 constructor classification, documentation
governance and diff checks pass.  The actual commit equals `origin/master` at
review.  [S47 evidence](../../etc/evidence/t539-s47-protected-16-fixture-migration.md)
contains the receiver map and retained timing boundary.  S47 is accepted;
T539 remains open for S48--S67.

## S48 Acceptance

Actual pushed NXVM P1 `a61178820` retires the 776-line private call-gate
privilege-entry smoke and replaces it with one public 80386 board receiver.
The receiver reaches ring 3 through guest GDT/LTR/IRET instructions, tests
32-bit outer call-gate parameter frames, and observes DPL, gate-type and target
descriptor rejection through real #GP delivery.  The shared bootstrap gains
one initial default-32 construction path; it does not expose a mutable CPU
seam.  Cache corruption, forced exception-delivery and shutdown-flag rows are
retired because they are not architectural guest states.  The smaller
call-gate smoke remains solely with the 80286 timing includer in S63.

Nine NXVM paths add 346/remove 814 lines (net -468), with no production/API,
Shared, firmware, INI or executable input change.  The affected marker passes
on x64 and x86; complete repository-only units pass 416/416 per width; both
specialized-gate aggregates, T344 classification, documentation governance and
diff checks pass.  No EXE rebuild is required for test/CMake/docs-only input.
See [S48 evidence](../../etc/evidence/t539-s48-call-gate-privilege-entry-migration.md).
S48 is accepted; T539 remains open for S49--S67.

## S49 Acceptance

Actual pushed NXVM P1 `19ea7597e` splits the former 1,181-line mixed
control-transfer smoke at its instruction-family boundary.  The new
CPU-local receiver owns all short/near Jcc, direct short/near JMP,
LOOP/LOOPE/LOOPNE and JCXZ/JECXZ forms, their 16/32-bit code/address forms,
the four real-mode CPU profiles, 80286 near-Jcc #UD and target-limit
atomicity.  It links only `x86-cpu` and uses the CPU instruction fixture; no
Core private machine field is exposed.  The renamed retained source contains
only the pending S50 near-call/return and S51 far-transfer families, each
once.

Eight NXVM paths add 366/remove 398 lines (net -32), with no production/API,
Shared, firmware, INI or executable input change.  Complete repository-only
units pass 417/417 on x64 and x86.  Both specialized-gate aggregates,
including the 36-owner T332 lifecycle check and the 105-row retained T344
matrix, documentation governance and diff checks pass per width.  No EXE
rebuild is required for test/CMake/docs-only input.  See [S49
evidence](../../etc/evidence/t539-s49-control-transfer-branch-migration.md).
S49 is accepted; T539 remains open for S50--S69.

## S50 Acceptance

Actual pushed NXVM P1 `64c261cf7` retires the direct-private near-transfer
families into one `x86-cpu` receiver: direct and register-indirect CALL,
RET/RET-immediate, 16/32-bit forms, indirect JMP and target-limit rollback.
The CPU fixture explicitly supplies stack and instruction-retirement inputs;
the retained Core source has no S50 helper, invocation or marker and now
contains only S51 far-transfer families.

Six NXVM paths add 203/remove 185 lines (net +18), with no production/API,
Shared, firmware, INI or executable input change.  Complete repository-only
units pass 418/418 on x64 and x86.  Both specialized-gate aggregates pass,
including T317 strict compilation, the 37-owner T332 lifecycle check and the
105-row retained T344 matrix; documentation governance and diff checks pass.
No EXE rebuild is required for test/CMake/docs-only input.  See [S50
evidence](../../etc/evidence/t539-s50-control-transfer-near-migration.md).
S50 is accepted; T539 remains open for S51--S69.

## S51 Acceptance

Actual pushed NXVM P1 `50cd002c1` retires the final mixed direct-private
control-transfer source.  One `x86-cpu` receiver retains protected immediate
and indirect far CALL/JMP, same-CPL RETF selector rejection and rollback, all
four real-mode far/near forms, boundary far-pointer behavior and terminal
reserved-`FF` #UD behavior.  It links only the CPU instruction fixture; no
Core machine or board-private field is exposed.  The source is deleted, so no
parallel control-transfer path remains.

Ten NXVM paths add 462/remove 649 lines (net -187), with no production/API,
Shared, firmware, INI or executable input.  The two protected-16 board callers
only receive their already-required `default32 = false` test-helper argument,
which restores the current helper contract without changing their scenarios.
Fresh x64/x86 builds and complete repository-only units pass 418/418 per
width.  The specialized gates, 38-owner T332 lifecycle check, 104-row T344
inventory, documentation governance, direct-private sweep and diff checks
pass.  No EXE rebuild is required for test/CMake/docs-only input.  See [S51
evidence](../../etc/evidence/t539-s51-control-transfer-receiver-map.md).  S51 is
accepted; T539 remains open for S52--S69.

## S52 Acceptance

Actual pushed NXVM P1 `bec9e0a70` retires both direct-private IDT and
privilege-entry sources.  Their cases now have exactly three receivers:
software `INT` DPL/gate/frame and rollback semantics are CPU-local;
external-IRQ DPL bypass is an independent PIC board receiver; and full-Core
delivery plus handler continuation uses only public Core operations.  No
receiver borrows executor or shared-PIC fields, and no parallel private setup
path remains.

Ten NXVM paths add 487/remove 398 lines (net +89), with no production/API,
Shared, firmware, asset, INI or executable input change.  Focused x64/x86
receivers and complete repository-only units pass 419/419 per width.  Both
specialized-gate aggregates, the 39-owner T332 lifecycle inventory, the
103-row T344 matrix, documentation governance, direct-private sweep and diff
checks pass.  No EXE rebuild is required.  See [S52
evidence](../../etc/evidence/t539-s52-idt-privilege-receiver-map.md).  S52 is
accepted; T539 remains open for S53--S69.

## S53 Acceptance

P1 `89eeef93c` retires the 934-line protected far/data pair into two CPU-local
receivers and two PIC board receivers.  The CPU cases retain descriptor/cache,
data-access and all-or-nothing fault assertions; the board cases additionally
prove real PIC IRR-to-ISR acknowledgement and interrupt-frame publication.
No production/public API, Shared, firmware, asset, INI or EXE input changes.

The focused receivers and complete repository-only x64/x86 unit suites pass.
T317 confirms 41 strict CPU compile commands, T332 confirms 41 fixture owners,
T344 confirms 101 direct constructors; direct-private, documentation-governance
and diff checks pass.  The [S53 receiver map](../../etc/evidence/t539-s53-protected-far-data-receiver-map.md)
records the exact allocation.  S53 is accepted; T539 remains open for S54-S69.

## S54 Acceptance

P1 `6d65ce0cc` retires the 1,003-line outer-return pair into exactly two
receivers. `cpu_outer_return_smoke` owns 80286/80386 outer RETF and IRET
frames, prefix and width forms, cached segment restoration and exact
all-or-nothing exception observations. `machine_outer_iret_pic_board_smoke`
alone owns real PIC IRR-to-ISR acknowledgement and the externally delivered
outer-IRET interrupt frame. The CPU receiver constructs neither a Core machine
nor a PIC; the board receiver uses only the public CPU-bus/PIC contract.

Ten NXVM paths add 575/remove 1,019 lines (net -444); no production/API,
Shared, firmware, asset, INI or executable input changes. Focused receivers
and complete repository-only x64/x86 units pass 421/421 per width. All 66
specialized gates pass, including the 41-owner T332 lifecycle check, 41-command
T317 strict compilation audit and 101-row T344 fixture-shape inventory;
documentation governance and diff checks pass. No EXE rebuild is required.
The [S54 receiver map](../../etc/evidence/t539-s54-outer-return-receiver-map.md)
records the exact allocation. S54 is accepted; T539 remains open for S55-S69.

## S55 Acceptance

S55 retires the 16-bit/task-gate half of `core_machine_task_switch_smoke.c`.
The CPU-only receiver covers direct and indirect task JMP, task CALL/GDT task
gate, LDT, nested return, IDT/double-fault task gates and every named selector,
presence, busy, short-TSS, stack and LOCK failure route. The sole board
receiver covers a real PIC IRQ0 IRR→ISR acknowledgement after a task switch.
All S55-only private helpers are deleted; the residual legacy source contains
only S56's four 80386 operand/address-size forms. Complete units pass 423/423
on x64 and x86; all 66 specialized gates, the repaired CPU-boundary negative
test, Lib manifest and documentation governance pass. No production/API,
Shared, firmware, asset, INI or EXE input changed, so no EXE rebuild is
required. The [S55 receiver map](../../etc/evidence/t539-s55-task-switch16-receiver-map.md)
records the exact allocation. S55 is accepted; T539 remains open for S56-S69.

## S75 Acceptance

S75 replaces the board's embedded CPU, decoder and execution layout with one
`core_machine_cpu_execution_context *` lifetime. `cpu.c` owns its allocation
and destruction; the board only creates it with its CPU-bus provider, binds its
existing profile/FPU/diagnostic inputs, and passes that same opaque context to
prepared entry, reset, execution and destruction. No public mutable CPU
accessor, layout mirror or compatibility path was added.

The CPU test fixture is the only private-layout consumer: it borrows the
opaque owner's CPU/decoder exclusively to prepare historical instruction-state
inputs. Production headers now import the opaque CPU interface rather than the
private decoder definition. Complete repository-only units pass 426/426 on
both x64 and x86; lifetime, CPU/PIC authority and historical-fixture gates pass
on both widths. Because production executable inputs changed, all four
admitted NXVM profiles have rebuilt 0539 x64/x86 Release artifacts. The
[S75 evidence](../../etc/evidence/t539-s75-opaque-cpu-lifetime.md) records the
boundary and artifact hashes. S75 is accepted; T539 remains open for S76-S82.

## S81 Acceptance

S81 moved the five 1,804-line CPU-only `MOVS`/`LODS`/`STOS`/`SCAS`/`CMPS`
test suites to one Shared CPU owner and deleted the NXVM duplicates. Real Core
memory, interruptibility, and PIC/IRQ observations remain NXVM board tests.
Complete repository-only units pass 427/427 on x64 and x86. Shared
manifest/corpus, CPU/PIC authority, documentation governance, and diff checks
pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input; the 0539 artifacts remain current. See the
[S81 evidence](../../etc/evidence/t539-s81-string-receivers.md). S81 is
accepted; T539 remains open for S82-S86.

## S82 Acceptance

S82 moved the two 642-line CPU-only scalar/string port suites and their sole
68-line CPU-bus fixture to one Shared owner, then deleted NXVM duplicates.
Actual port routing, permission, and PIC/IRQ observations remain named NXVM
board tests. Complete repository-only units pass 429/429 on x64 and x86.
CPU/PIC authority, Shared manifest/corpus, documentation governance, and diff
checks pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input. See the [S82 evidence](../../etc/evidence/t539-s82-port-receivers.md).
S82 is accepted; T539 remains open for S83-S94 after the protected/system
work-package split.

## S83 Acceptance

S83 moved the four 1,143-line protected descriptor-operand suites and their
sole 121-line descriptor-query fixture to one Shared CPU owner. NXVM removes
the duplicate tests and its static inventories use the same Shared targets.
The named ARPL/BOUND public-board receivers retain real board semantics.
Complete repository-only units pass 433/433 on x64 and x86. CPU/PIC authority,
Shared manifest/corpus, documentation governance, and diff checks pass. This
test/CMake-only scope changes no firmware, asset, INI, or EXE input. See the
[S83 evidence](../../etc/evidence/t539-s83-descriptor-operands.md). S83 is
accepted; T539 remains open for S84-S94.

## S80 Acceptance

S80 moved three 2,105-line CPU-only segment/data tests to one Shared owner and
deleted NXVM duplicates. Public Core memory/fault/IRQ observations remain NXVM
tests. See the [S80 evidence](../../etc/evidence/t539-s80-segment-data-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S80 is accepted;
T539 remains open for S81-S86.

## S79 Acceptance

S79 moved five 1,883-line CPU-only segment-stack/far-pointer tests to one
Shared owner and deleted NXVM duplicates. Board-specific descriptor,
memory/fault, and IRQ observations remain NXVM tests. See the
[S79 evidence](../../etc/evidence/t539-s79-segment-stack-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S79 is accepted;
T539 remains open for S80-S86.

## S78 Acceptance

S78 moved the four 1,858-line CPU-only basic-stack tests to one Shared owner
and deleted the duplicate NXVM paths. Real Core stack/fault/IRQ cases remain
explicit NXVM board tests. The [S78 evidence](../../etc/evidence/t539-s78-basic-stack-receivers.md)
records receiver ownership and verification.

Complete repository-only units pass 427/427 on x64 and x86, and Shared
manifest/corpus, CPU/PIC authority, documentation governance, and diff checks
pass. Test/CMake-only scope leaves S76's 0539 product artifacts current. S78 is
accepted; T539 remains open for S79-S86.

## S77 Acceptance

S77 established the first Shared CPU-only test receiver group: ten arithmetic,
FLAGS, rotate, and direct-register sources plus their one real fixture now live
under `test/x86/devices/cpu/`.  The historic NXVM fixture is only a forwarding
include for explicitly assigned S79--S85 callers; it carries no state or test
registration.  Board paths stayed NXVM and are listed in the
[S77 evidence](../../etc/evidence/t539-s77-cpu-arithmetic-receivers.md).

The final 427-test repository-only unit suite passed on both x64 and x86.
Shared corpus/manifest, CPU/PIC authority, documentation governance, and diff
checks pass. No runtime CPU or product input changed, so the S76 0539 artifacts
remain current. S77 is accepted; T539 remains open for S78--S82.

## S76 Acceptance

S76 established `src/x86/devices/cpu/` as the canonical nine-file CPU corpus.
`x86-cpu-shared` is the sole implementation target; NXVM's historic
`x86-cpu` spelling is a CMake alias, not a wrapper or duplicate object path.
All active NXVM consumers and static authority gates now use the canonical
Shared headers/sources. The retained App CPU files are inert historical source
only; S81 owns their physical deletion.

Shared corpus, opaque contract, CPU-bus negative controls and focused timing
runners pass on both widths. Complete repository-only NXVM units pass 427/427
on each width. All eight 0539 profile/width artifacts were rebuilt and their
PE architectures plus hashes are recorded in the [S76 evidence](../../etc/evidence/t539-s76-shared-cpu-cutover.md).
S76 is accepted; T539 remains open for S77-S82.

## S74 Acceptance

`machine_cpu_profile_gate_smoke.c`, `machine_fpu_escape_smoke.c` and
`machine_fpu_interface_s65_smoke.c` are now the sole Core-machine receivers
for the remaining profile/FPU inputs.  `support/machine_cpu_fixture.h` is the
one private prepared-state fixture; all 31 test-only consumers use it, with no
compatibility include or second fixture path.  The CPU-boundary, T332 lifecycle
and T344 shape gates recognize the successor names and still reject the private
fixture outside its intended boundary.

Focused successor receivers and the CPU-boundary negative gate pass on x86 and
x64.  All affected consumers rebuilt on both widths.  Complete repository-only
unit suites pass 426/426 on x86 in 100.66 seconds and 426/426 on x64 in 99.72
seconds; Types, T344, T332, VM lifecycle, CPU/PIC authority, T388,
documentation-governance and diff checks pass per width.  No production/API,
Shared, firmware, asset, INI or executable input changed, so no product binary
rebuild is required.  See the [S74 receiver map](../../etc/evidence/t539-s74-remaining-consumer-fixture-map.md).
S74 is accepted; T539 remains open for S75-S77.

## S73 Acceptance

`machine_80386_timing_manifest_runner.c` is the sole Core-machine receiver
for the 80386 timing-manifest context. It retains source formulas,
manifest/catalog rows and measured results without adding a generated
production dependency or a second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 98.16 seconds and 426/426 on x64 in 98.08 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S73 receiver map](../../etc/evidence/t539-s73-80386-timing-receiver-map.md)
records the exact scope. S73 is accepted; T539 remains open for S74-S77.

## S72 Acceptance

`machine_80286_timing_manifest_runner.c` and its direct
`machine_call_gate_smoke.c` includer are the sole Core-machine receiver/include
closure for the 80286 timing-manifest context. They retain source formulas,
manifest/catalog rows, measured results and the existing public call-gate
construction without adding a generated production dependency or a second
recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 105.59 seconds and 426/426 on x64 in 105.52 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S72 receiver map](../../etc/evidence/t539-s72-80286-manifest-call-gate-receiver-map.md)
records the exact scope. S72 is accepted; T539 remains open for S73-S77.

## S71 Acceptance

`machine_80286_instruction_timing_ledger_smoke.c` and
`machine_80386_protected_io_timing_smoke.c` are the sole Core-machine
receivers for their 80286 ledger and 80386 protected-I/O timing contexts.
They retain the source formulas, protected-port rows and measured results
without adding a generated production dependency or a second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 103.32 seconds and 426/426 on x64 in 103.50 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S71 receiver map](../../etc/evidence/t539-s71-80286-protected-io-timing-receiver-map.md)
records the exact scope. S71 is accepted; T539 remains open for S72-S77.

## S70 Acceptance

`machine_80186_instruction_timing_ledger_smoke.c` and
`machine_80186_timing_manifest_runner.c` are the sole 80186 Core-machine
timing receivers. They retain the source formulas, generated manifest catalog
and measured results without adding a generated production dependency or a
second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 and x64. T344 registration and historical fixture shapes, T332
lifecycle, VM-machine lifecycle, Core CPU/PIC authority, T388 lexeme and
physical eligibility, documentation governance and diff checks pass. No
firmware, asset, INI or EXE input changed, so no product binary rebuild is
required. The [S70 receiver map](../../etc/evidence/t539-s70-80186-timing-receiver-map.md)
records the exact scope. S70 is accepted; T539 remains open for S71-S77.

## S69 P1 Acceptance

P1 corrected the private Shared `x86_video_active_ega_aperture()` failure
contract before the timing receiver rename. The helper now reports invalid
arguments and each of its planar-offset, write-observer and containment
callers has one explicit failure result. Valid aperture maps, offsets and
dirty observation are unchanged. The P1 commit is `429407e84`; dual-width EGA
focused tests, complete 426/426 unit suites, all applicable gates and
documentation governance passed. No public API, asset, INI or EXE input
changed. P2 is recorded by the S69 acceptance below.

## S69 Acceptance

P1 is the approved Shared precondition and P2 makes
`machine_8086_instruction_timing_ledger_smoke.c` and
`machine_8086_timing_manifest_runner.c` the sole 8086 Core-machine timing
receivers. The latter is compiled once per explicit 8086/8088 profile, without
duplicating its recipe executor or generated-catalog path. Formula rows,
timing results and decoder inventory remain unchanged.

Focused x64/x86 8086/8088 ledger, manifest, result and decoder-ledger tests
pass; complete repository-only unit suites pass 426/426 on x86 and x64. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No firmware, asset, INI or EXE
input changed, so no product binary rebuild is required. The
[P1 Shared evidence](../../etc/evidence/t539-s69-p1-shared-video-aperture.md) and
[P2 receiver map](../../etc/evidence/t539-s69-8086-timing-receiver-map.md)
record the exact scope. S69 is accepted; T539 remains open for S70-S77.

## S68 Acceptance

`machine_instruction_timing_smoke.c`, `machine_instruction_timing_ledger_smoke.c`,
`machine_legacy_timing_normalization_s2_smoke.c` and
`machine_t359_s2_timing_smoke.c` through `machine_t359_s6_timing_smoke.c`
are the sole named Core-machine receivers for the common timing baseline.
They retain the original formula rows, execution-provider observations and
normalization assertions without changing a timing algorithm or production API.

Focused x64/x86 receivers pass and retain `M5:T265:S3:INSTRUCTION-TIMING:OK`,
`M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`,
`M5:T362:S2:LEGACY-TIMING-NORMALIZATION:OK`, and all T359 S2-S6 markers.
Complete repository-only unit suites pass 426/426 on x86 and x64. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical-eligibility gates,
documentation governance and diff checks pass. This is test/CMake/documentation
work only: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required. The [receiver map](../../etc/evidence/t539-s68-timing-baseline-receiver-map.md)
records the complete allocation. S68 is accepted; T539 remains open for S69-S77.

## S67 Acceptance

`machine_vm86_delivery_smoke.c`, `machine_vm86_iret_smoke.c`,
`machine_vm86_lgdt_lidt_s5_smoke.c` and
`machine_hardware_delivery_s3_smoke.c` are the sole named public Core-machine
receivers for the VM86 dependency group. They retain VM86 interrupt frames,
LGDT/LIDT validation, paging faults and real hardware IRQ delivery; direct
includers reuse the same fixture rather than introduce another execution path.

Focused x64/x86 receivers pass and emit `M5:T539:S67:VM86:OK` together with
their retained historical VM86 markers. Complete repository-only unit suites
pass 426/426 on x64 and x86. T344 registration and historical fixture shapes,
T332 lifecycle, VM-machine lifecycle, Core CPU/PIC authority, documentation
governance and diff checks pass on both widths. This is test/CMake/documentation
work only: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required. The [receiver map](../../etc/evidence/t539-s67-vm86-receiver-map.md)
records the complete allocation. S67 is accepted; T539 remains open for S68-S72.

## S66 Acceptance

`machine_interrupt_entry_smoke.c` is the unique named Core-machine receiver
for GDT/IDT gate construction, software-INT entry, PIC/NMI delivery and
fault escalation.  Its direct S66 software-INT includer reuses this setup;
the S67 hardware-delivery includer receives only the mechanical new filename
and retains all VM86 behavior for its own package.

Focused x64/x86 receiver and both includers pass and emit
`M5:T539:S66:INT-ENTRY:OK`.  Complete repository-only unit suites pass 426/426
on both widths.  T332 lifecycle, T344 registration and historical fixture
shapes, VM lifecycle, Core CPU/PIC authority, documentation governance and
`git diff --check` pass on both widths.  This is test/CMake/documentation-only
work: no production/API, Shared, firmware, asset, INI or EXE input changed, so
no EXE rebuild is required.  The [receiver map](../../etc/evidence/t539-s66-int-entry-receiver-map.md)
records the allocation.  S66 is accepted; T539 remains open for S67-S72.

## S65 Acceptance

`machine_protected_iret_smoke.c` is the unique named Core-machine receiver for
the protected IRET dependency group.  It preserves real/protected/outer IRET
frame construction, validation and all-or-nothing fault delivery through public
machine setup.  Its sole direct includer,
`core_machine_iret_s51_smoke.c`, reuses that receiver rather than duplicating
the descriptor and frame setup.

Focused x64/x86 receiver and includer runs pass and emit
`M5:T539:S65:PROTECTED-IRET:OK`.  Complete repository-only unit suites pass
426/426 on both widths.  T332 lifecycle, T344 registration and historical
fixture shapes, VM lifecycle, Core CPU/PIC authority, documentation governance
and `git diff --check` pass on both widths.  This is test/CMake/documentation-
only work: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required.  The [receiver map](../../etc/evidence/t539-s65-protected-iret-receiver-map.md)
records the allocation.  S65 is accepted; T539 remains open for S66-S72.

## S64 Acceptance

`machine_cli_sti_interrupt_smoke.c` is the unique named Core-machine receiver
for real/protected/VM86 CLI/STI construction, IF/shadow, PIC IRQ/mask and
guest-visible frame behavior. Its direct S64 includers—80286 CLI/STI, HLT and
INT-to-IRET-to-IRQ composition—reuse that receiver rather than rebuilding a
second execution-provider or IRQ setup. Later IRET and software-INT sources
received only the required mechanical filename update; their behavior remains
allocated to S65 and S66.

Focused x64/x86 runs emit `M5:T539:S64:CLI-STI-INTERRUPT:OK`; final serial
repository-only units pass **426/426** on x64 and x86. T332 lifecycle, T344
registration and fixture shape, VM lifecycle, Core CPU/PIC authority,
documentation governance and `git diff --check` pass. This is test/CMake/
documentation-only work: no production/API, Shared, firmware, asset, INI or
EXE input changed, so no EXE rebuild is required. The [receiver map](../../etc/evidence/t539-s64-cli-sti-receiver-map.md)
records every context. S64 is accepted; T539 remains open for S65-S72.

## S63 Acceptance

`machine_tss_iomap_port_authorization_smoke.c` is now the sole named
receiver for TSS I/O-map authorization. It preserves real protected entry,
`LTR`, bitmap bounds, CPL/IOPL decisions, #GP delivery and actual public port
provider effects. No task-switch fixture or second authorization path was
introduced.

Focused x64/x86 runs emit `M5:T539:S63:TSS-IOMAP:OK`; final serial
repository-only units pass **426/426** on both widths. T344 shape and
registration, Core CPU/PIC authority, VM lifecycle, documentation governance
and `git diff --check` pass. This is test/CMake/documentation-only work: no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. The [receiver map](../../etc/evidence/t539-s63-tss-iomap-receiver-map.md)
records every authorization context. S63 is accepted; T539 remains open for
S64-S69.

## S62 Acceptance

The final residual task-switch receiver is now
`machine_task_switch_cross_width_smoke.c`, replacing the misleading mixed
`core_machine_task_switch_smoke.c` name and target. It alone executes the
eight 16→32 and 32→16 direct/nested/task-gate/return cases, preserving each
old/new TSS image, TR and busy state, backlink/NT state, and nested IRET
return. The existing 80286 and 80386 timing runners include that same source
under their private `main` rename; no duplicate fixture was introduced.

Focused x64/x86 runs pass and emit `M5:T539:S62:TASK-CROSS-WIDTH:OK`. Final
serial repository-only unit suites pass **426/426** on both x64 and x86.
T317/T332, T344 shape/declaration/direct-compilation/registration, Core
CPU/PIC authority and machine lifecycle gates pass on both widths;
documentation governance and `git diff --check` pass. This is
test/CMake/documentation-only work: no production/API, Shared, firmware,
asset, INI or EXE input changed, so no EXE rebuild is required. The
[receiver map](../../etc/evidence/t539-s62-task-cross-width-receiver-map.md)
records all eight contexts. S62 is accepted; T539 remains open for S63-S69.

## S61 Acceptance

The ordinary TSS32 far-JMP and nested TSS32 far-CALL rows with a pending IRQ
now have one board/Core receiver, `machine_task_switch32_paging_smoke.c`.
It uses guest I/O to initialise and unmask the master PIC, injects a real
keyboard byte through the public Core board input, and observes the resulting
KBC-to-PIC-to-CPU delivery while executing the real task transfer. The
legacy mixed runner no longer contains its direct-private pending-IRQ helper.

Focused x64/x86 receivers and the retained corpus pass. Complete serial unit
suites pass **426/426** on x64 and **426/426** on x86. The directly invoked
T344 historical-shape and Core lifecycle scripts pass; documentation
governance and `git diff --check` pass. The CMake/Ninja batch invocation for
the remaining static targets stalled without CPU progress on this host, so it
is deliberately not recorded as a passing result. This is test and
documentation-only work: no production/API, Shared, firmware, asset, INI or
EXE input changed, so no EXE rebuild is required. The [receiver map](../../etc/evidence/t539-s61-task-switch32-pending-irq-receiver-map.md)
records the two contexts and their sole delivery route. S61 is accepted;
T539 remains open for S62-S69.

## S60 Acceptance

The eleven nested CALL/JMP, task-gate, nested-return and rejection rows now
have one CPU-local receiver, `cpu_task_switch32_state_smoke`.  It executes
real guest table loads and task transfers over copied CPU memory, including a
correctly aligned task-gate descriptor and the actual LDT descriptor limit.
The mixed legacy runner retains only S61's two pending-IRQ contexts.

Focused x64/x86 receivers and the retained corpus pass.  The complete x64
suite passes 426/426.  The x86 suite's 424 unaffected rows passed in the full
serial run; its two transient failures each passed immediately when rerun in
isolation, including the fixed-path CPU-boundary negative gate. T317/T332,
T344, Core CPU/PIC authority and lifecycle checks, documentation governance
and `git diff --check` pass.  This is test/CMake/documentation-only work; no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. The [receiver map](../../etc/evidence/t539-s60-task-switch32-nesting-receiver-map.md)
records the exact allocation. S60 is accepted; T539 remains open for
S61-S69.

## S59 Acceptance

The two direct TSS32 paging contexts now have one public-Core receiver,
`machine_task_switch32_paging_smoke.c`.  It builds real guest page tables,
GDT, IDT and TSS images through public physical-memory writes, performs guest
`LGDT`/`LIDT`/`LMSW`/`LTR`, and observes a mapped target transition plus an
unmapped target-TSS `#PF` through the public diagnostic and copied snapshot
contracts.  The retained mixed source contains neither S59 page-table state
nor private IDTR mutation.  The [receiver map](../../etc/evidence/t539-s59-task-switch32-paging-receiver-map.md)
records the exact boundary.

Focused x64/x86 receivers and the retained task-switch corpus pass. Complete
repository-only unit suites pass 426/426 on x64 and x86 when serially run;
T317, T332, T344, Core CPU/PIC authority and lifecycle gates, documentation
governance and `git diff --check` pass on both widths. This is
test/CMake/documentation-only work: no production/API, Shared, firmware,
asset, INI or EXE input changed, so no EXE rebuild is required. S59 is
accepted; T539 remains open for S60-S69.

## S56 Acceptance

The four functional 80386 `66h`/`67h` task-JMP contexts now have one
CPU-local receiver, `cpu_task_switch32_decode_smoke.c`, sharing the established
S55 TSS16 fixture. The legacy mixed source retains only the encoding recipes
needed by the S65 timing-runner includer; it no longer invokes these functional
rows. The [receiver map](../../etc/evidence/t539-s56-task-switch32-decode-map.md)
records the boundary.

Focused x64/x86 receivers and the retained mixed runner pass. Complete units
pass 424/424 on x64 and x86; all 66 specialized gates, documentation governance
and `git diff --check` pass. This is test/CMake/documentation-only work: no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. S56 is accepted; T539 remains open for S57-S69.

## S57 Acceptance

Direct TSS32 baseline, operand/address forms, descriptor/LDT state and fault
rows now have one CPU-local receiver, `cpu_task_switch32_state_smoke.c`. Its
fixture executes `LGDT` and `LTR`; it does not fabricate a TR cache. The
[receiver map](../../etc/evidence/t539-s57-task-switch32-state-map.md) records
the exact 17 contexts. Its then-residual mixed source retained only the later
exceptional and cross-width rows.

Focused x64/x86 receivers and the retained mixed runner pass. Complete units
pass 425/425 on x64 and x86; documentation governance and `git diff --check`
pass. This is test/CMake/documentation-only work: no production/API, Shared,
firmware, asset, INI or EXE input changed, so no EXE rebuild is required. S57
is accepted; T539 remains open for S58-S69.

## S58 Acceptance

The direct TSS32 debug-trap word and LOCK direct/indirect task-JMP cases now
have one CPU-local receiver, `cpu_task_switch32_state_smoke.c`; its genuine
LGDT/LTR setup supplies the task state without a fabricated TR cache. The
residual mixed source no longer invokes any of these five contexts. A first
principles fixture check established that paging does not belong in that
receiver: its CPU bus has no Core physical-translation binding, so S59 owns
the paging rows through a public Core receiver instead.

Focused x64/x86 receivers and the retained mixed runner pass. Complete x64/x86
units pass 425/425 each. T317 and T332 pass on both widths with 44 strict
CPU receivers/fixture owners; documentation governance and `git diff --check`
pass. This is test/CMake/documentation-only work: no production/API, Shared,
firmware, asset, INI or EXE input changed, so no EXE rebuild is required.
S58 is accepted; T539 remains open for S59-S69.

## S29 Acceptance

Actual pushed NXVM P1 `86fe95201` has exactly 13 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 28 original
operand contexts and all twelve original prefix groups retain CPU or board
receivers; two fault contexts have complementary observations. x64 and x86
builds and complete units pass 389/389 each, along with 66 specialized gates,
extended CPU-boundary negatives, six unchanged manifests and documentation
governance. The nine test/build paths add 1,514/remove 1,311 lines; no
production/API or executable input changed. See
[S29 evidence](../../etc/evidence/t539-s29-operand-prefix-migration.md). S29 is
accepted; 82 original private consumers remain assigned to S30-S42. S43 owns
opaque lifetime, S44 physical Shared relocation and S45 whole-CPU acceptance.

## S28 Acceptance

Actual-commit review accepts pushed NXVM P1 `fa092b95e`: the 244 original
execution contexts and three metadata queries retain CPU/board receivers;
the inverted 286 test result and incorrect EAX expectation are corrected.
Full x64/x86 units pass 387/387 each, with 66 specialized gates, six unchanged
manifests and documentation/diff checks. Ten test/build paths add 2,065/remove
1,530 lines, net +535; no production or executable input changed. See
[S28 evidence](../../etc/evidence/t539-s28-segment-migration.md). The S28 packet
is closed, not carried into S29.

CPU extraction itself is not accepted. The
[inventory](../../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 56 original direct-private `.c` test consumers and one fixture
header to S41-S51. Embedded CPU lifetime remains until S52; physical Shared
relocation is S53; whole CPU acceptance is S54. S41 owns the unresolved
32-bit BOUND observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S44 | Accepted: descriptor-query cases move to CPU-local LAR/LSL and VERR/VERW receivers; the retained 80386 board timing runner covers LSL 21/25/22/26 source ticks. Units 416/416 per width. No production or asset change. |
| T539 S40 | Accepted: NXVM P1 e4a7615d1 migrates ARPL ownership; original base/S53 cases retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S39 | Accepted: NXVM P1 07019f588 migrates port I/O ownership; all 230 original scalar/string contexts retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S38 | Accepted: NXVM P1 ae76a7c84 migrates STOS/SCAS/CMPS ownership; all 203 original contexts retain CPU/board receivers; units 411/411 per width. No production or asset change. |
| T539 S31 | Accepted: NXVM P1 38bc5b10c migrates immediate IMUL and Group-2 test ownership; all 335 original contexts retain CPU/board receivers; units 397/397 per width. No production or asset change. |
| T539 S30 | Accepted: NXVM P1 442088410 migrates bit/condition/extension ownership. All 549 original contexts retain CPU/board receivers; units 395/395 per width. No production or asset change. |
| T539 S29 | Accepted: NXVM P1 86fe95201 migrates operand/address and S64 prefix ownership. All 28 operand contexts and twelve prefix groups retain receivers; units 389/389 per width. No production or asset change. |
| T539 S28 | Accepted: NXVM P1 fa092b95e migrates segment selector/SREG MOV test ownership. All 244 original contexts and three queries retained; units 387/387 per width. No production or asset change. |
| T539 S27 | Accepted: NXVM P1 42d6c86e0 migrates far-pointer test ownership. All 117 original contexts retained; units 385/385 per width. No production or asset change. |
| T539 S26 | Accepted: NXVM P1 587a91af9 migrates segment-stack test ownership. All 164 original contexts retained; units 382/382 per width. No production or asset change. |
| T539 S25 | Accepted: NXVM P1 2cb8b64f7 migrates ENTER/LEAVE test ownership. All 53 original contexts retained; units 380/380 per width. No production or asset change. |
| T539 S24 | Accepted: NXVM P1 ff09b22a3 migrates GPR stack test ownership. All 198 original contexts retained; units 379/379 per width. No production or asset change. |
| T539 S23 | Accepted: NXVM P1 5c2835936 migrates XCHG test ownership. All 101 original instruction contexts retained; units 376/376 per width. No production or asset change. |
| T539 S22 | Accepted: NXVM P1 9e5382872 migrates GPR MOV/MOFFS test ownership. All 277 original contexts retained; units 375/375 per width. No production or asset change. |
| T539 S21 | Accepted: NXVM P1 1049b9021 migrates LEA/MOVX test ownership and divides the oversized instruction batch. Units 373/373 per width. Production, EXEs and INIs unchanged. |
| T539 S20 | Accepted: NXVM P1 af06a6259 qualifies copied observations, debug/reset adapters and board access. Units 371/371 per width. |
| T539 S19 | Accepted: NXVM P1 f1b43af46 qualifies CPU bus transactions, failure effects and imports. Units 371/371 per width. |
| T539 S18 | Accepted: NXVM P1 0067d80c4 restores the incremental baseline and preserves pending migrations. Units 370/370 per width; default integration 20/20 per width; tools-off 45/45; six vendor boots once. |

The [proposal](../../proposals/m5-shared-chip-extraction.md),
[finite ledger](../../etc/evidence/t539-chip-migration-ledger.md),
[CPU boundary](../../etc/architecture/t539-s18-cpu-extraction.md) and
[task history](../../history/M5-T539-independent-shared-chips.md) retain the
complete scope, original requirements, earlier acceptance and receiving proof.
The historical S18-S20 prospective numbering is superseded only for unadmitted
packages by the current work plan.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0540 EXEs were rebuilt by
T540 S92 with unchanged owner INIs; S92 evidence records their hashes, PE
architecture and verification limits. The 0539 pairs remain in Git history.
Run native desktop test suites without cross-tree overlap.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
cadaf0990 and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by the CPU batches.

The six [Queue](../../states/QUEUE.md) candidates retain dependency order. Acceptance does
not claim indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
