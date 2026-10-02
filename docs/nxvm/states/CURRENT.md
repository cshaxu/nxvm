# Project Status

## Current Work

M5 T540 S70 is active: close the board-to-Core CPU signal boundary.
S1-S69 are accepted.
T540 remains open for neutral Core relocation and
IBM-PC board extraction.
The oversized former S12 port batch is split into linear receivers. Shared
Core and board code have not moved.
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
linked work plan owns the current sequence. The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
and [S101 evidence](../etc/evidence/t539-s101-cpu-source-cleanup.md) record
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
| T540 S69 | Active: independently link the actual neutral source with only CPU/FPU/Lib dependencies and synthetic owned inputs. |

## T540 S1 Acceptance

S1 inspected all four existing profile construction paths, the retained board
adapters and the present product regressions.  Its audited result is
[`t540-ibmpc-board-audit.md`](../etc/architecture/t540-ibmpc-board-audit.md):
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
[`t540-board-adapter-ledger.md`](../etc/architecture/t540-board-adapter-ledger.md).
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [board audit](../etc/architecture/t540-ibmpc-board-audit.md), and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
header or second chip implementation. [S4 evidence](../etc/evidence/t540-s4-chip-path-migration.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [board audit](../etc/architecture/t540-ibmpc-board-audit.md), and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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

The [neutral Core cut](../etc/architecture/t540-s5-neutral-core-cut.md)
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
| Admission And Approval | Automatic bounded-S authorization applies. S6 intake found that moving only timeline/transaction would expose mutable private Core layouts or add a transient wrapper; the [S5 cut amendment](../etc/architecture/t540-s5-neutral-core-cut.md) records the narrower first board/Core seam. NXVM is the only changed target; Shared and MyNES are read-only receiving reviews. |
| Objective | Remove the fixed port 92h callbacks and registration from the generic memory mechanism; install the identical read/write route once from the existing IBM-PC board owner during the same machine construction step. |
| Non-goals | A20 electrical behavior change, changing which current profile has 92h, a new public getter/setter or adapter framework, moving private timeline/transaction state alone, ROM/INI/asset changes, or MyNES edits. |
| Reference Baseline | S5 acceptance `de382a478`, complete 0540 product artifact pairs. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S5 Core cut](../etc/architecture/t540-s5-neutral-core-cut.md), and [board ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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

The [S6 evidence](../etc/evidence/t540-s6-a20-board-port.md) records the sole
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

The [Core/board handoff](../etc/architecture/t540-s7-core-board-handoff.md)
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
reviewed against the S8 packet and the [S8 evidence](../etc/evidence/t540-s8-port-route-batch.md).

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
controller ports. The [S9 evidence](../etc/evidence/t540-s9-dma-port-routes.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../etc/architecture/t540-s7-core-board-handoff.md), and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
[S10 evidence](../etc/evidence/t540-s10-kbc-port-routes.md) records the
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../etc/architecture/t540-s7-core-board-handoff.md), and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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

[S11 evidence](../etc/evidence/t540-s11-fdc-port-routes.md) records the
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and [S11 evidence](../etc/evidence/t540-s11-fdc-port-routes.md). |
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

[S12 evidence](../etc/evidence/t540-s12-rtc-port-routes.md) records the
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
staged diff check pass. The [S13 evidence](../etc/evidence/t540-s13-board-port-b-routes.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
[S14 evidence](../etc/evidence/t540-s14-hdc-port-routes.md) records the failure
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S7 handoff and S12 refinement](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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

P1 `b656b0456` is pushed. The [S15 evidence](../etc/evidence/t540-s15-vadp-port-routes.md)
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
| Admission And Approval | The owner's automatic admission for bounded numeric T540 subtasks applies. [S16 source intake](../etc/architecture/t540-s7-core-board-handoff.md) separates VADP, D4 and ROM by their distinct memory owner/rollback boundaries. Target: NXVM only; Shared and MyNES remain read-only. |
| Objective | Give VADP a bounded Core memory route and copied inspection boundary for candidate CGA/planar mappings, observer and display snapshot, without `t_ram` in the VADP adapter. |
| Non-goals | D4 parity/windows (S17), immutable ROM/reset aliases (S18), KBC/DMA memory cycles (S19), video register semantics, frame geometry/timing, new generic device framework, profile/INI/firmware/media changes, physical Core or board relocation. |
| Reference Baseline | Accepted S15 P1 `b656b0456` and P2 `700d7f0c9`; both-width full units 467/467, focused EGA integration 1/1 per width, complete specialized gates and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S16 source refinement](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and [S15 evidence](../etc/evidence/t540-s15-vadp-port-routes.md). |
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

P1 `1fc8b2054` is pushed. The [S16 evidence](../etc/evidence/t540-s16-vadp-memory-routes.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md), and [S16 evidence](../etc/evidence/t540-s16-vadp-memory-routes.md). |
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
conflict, observer capacity, retry, reset and teardown. The [S17 evidence](../etc/evidence/t540-s17-d4-memory-routes.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md), and [S17 evidence](../etc/evidence/t540-s17-d4-memory-routes.md). |
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

The [S18 evidence](../etc/evidence/t540-s18-rom-memory-routes.md) records x64
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S19 refinement](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md), and [S18 evidence](../etc/evidence/t540-s18-rom-memory-routes.md). |
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
arguments remain assigned to S20. The [S19 evidence](../etc/evidence/t540-s19-a20-fallback-routes.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff and S19 refinement](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md), and [S19 evidence](../etc/evidence/t540-s19-a20-fallback-routes.md). |
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
before/after positions. The [S20 evidence](../etc/evidence/t540-s20-dma-core-cycle.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [Core/board handoff](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and [S20 evidence](../etc/evidence/t540-s20-dma-core-cycle.md). |
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
PIC service. The [S21 intake](../etc/evidence/t540-s21-scheduler-pic-intake.md)
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
| Reference Baseline | Accepted S21 P1 `fa9aa0c27` and [source intake](../etc/evidence/t540-s21-scheduler-pic-intake.md); S20 complete units 469/469 per width, 76/76 specialized gates, four external boots once per width and eight Release products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and [S21 source intake](../etc/evidence/t540-s21-scheduler-pic-intake.md). |
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
and advances time. The [S22 evidence](../etc/evidence/t540-s22-copied-board-deadlines.md)
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
| Reference Baseline | Accepted S22 P1 `dbef7f81e`, [S22 evidence](../etc/evidence/t540-s22-copied-board-deadlines.md) and [S21 source intake](../etc/evidence/t540-s21-scheduler-pic-intake.md); dual-width units 469/469, specialized gates 75/75, eight external boots and eight Release products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S23 refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S21/S22 evidence. |
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
[S23 evidence](../etc/evidence/t540-s23-board-peripheral-advance.md) records
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
| Reference Baseline | Accepted S23 P1 `545086ce9`, [S23 evidence](../etc/evidence/t540-s23-board-peripheral-advance.md), [source intake](../etc/evidence/t540-s21-scheduler-pic-intake.md): dual-width units 469/469, gates 76/76, eight boots and eight Release products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S23 source-refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and board adapter ledger. |
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
[S24 evidence](../etc/evidence/t540-s24-board-readiness.md) records the
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
| Reference Baseline | Accepted S24 P1 `8f1b7a68c`, [S24 evidence](../etc/evidence/t540-s24-board-readiness.md), [S21 intake](../etc/evidence/t540-s21-scheduler-pic-intake.md); dual-width units 469/469, all specialized gates, eight external boots and eight Release products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S25 refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md) and board adapter ledger. |
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
[S25 intake](../etc/evidence/t540-s25-arbitration-intake.md) records exact
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
| Reference Baseline | Accepted S25 P1 `3f14bf9d2`, [arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md) and S24 accepted dual-width 469/469 units, all specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [board adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
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
and staged diff checks passed. The [S26 evidence](../etc/evidence/t540-s26-board-refresh-request.md)
records the owner split and exact products. S26 is accepted; S27 receives the
remaining DMA clock/request/wait/grant/prefetch boundary. T540 remains open.

## T540 S27 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S27, the next linear S after accepted S26. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Give board code DMA clock/request/chip effects while Core alone owns cycle wait, HOLD/grant, transaction arbitration and CPU prefetch reservation, preserving their exact order. |
| Non-goals | PIT/PIC tail (S28), CPU PIC INTA/locality (S29), mixed plan/reset (S30), neutral Core move (S31), timing formula, profile/INI/firmware/media changes or a generic bus framework. |
| Reference Baseline | Accepted S26 P1 `0d5cac0f9`, [S25 arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md) and S26 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
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
passed. The [S27 evidence](../etc/evidence/t540-s27-board-dma-arbitration.md)
records the exact owner split and product hashes. S27 is accepted; S28 owns
the remaining PIT/PIC post-prefetch tail. T540 remains open.

## T540 S28 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S28, the next linear S after accepted S27. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Move PIT clock/chip advancement and PIC refresh/trace in the post-prefetch arbitration tail to one board-owned callback, preserving the original call and signal order. |
| Non-goals | CPU PIC INTA/locality (S29), mixed plan/reset (S30), neutral Core move (S31), timer formula, profile/INI/firmware/media changes, second guest clock or generic event framework. |
| Reference Baseline | Accepted S27 P1 `0c1bba56a`, [S25 arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md) and S27 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
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
and staged diff checks passed. The [S28 evidence](../etc/evidence/t540-s28-board-pit-pic-tail.md)
records exact call order and hashes. S28 is accepted; S29 receives CPU PIC
INTA and DMA-HOLD locality. T540 remains open.

## T540 S29 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S29, the next linear S after accepted S28. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only; Shared and MyNES are read-only. |
| Objective | Source-inspect and isolate CPU PIC INTA and DMA-HOLD locality: Core retains CPU delivery and prefetch invalidation, board supplies only PIC/DMA signal facts and electrical wiring. |
| Non-goals | Mixed plan/reset (S30), neutral Core physical move (S31), timing formula, profile/INI/firmware/media changes, second interrupt route or generic event bus. |
| Reference Baseline | Accepted S28 P1 `826d5e29c`, [S25 arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md), S28 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S25 intake. |
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
[S29 evidence](../etc/evidence/t540-s29-pic-cpu-locality.md) records the
call order and hashes. S29 is accepted; S30 receives mixed plan/reset.
T540 remains open.

## T540 S30 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S30, the next linear S after accepted S29. |
| Admission And Approval | The owner's automatic admission for bounded numeric T540 S tasks applies. Target NXVM only for intake; a later Shared move requires its own scoped P. MyNES remains read-only. |
| Objective | Inspect mixed machine plan/construction/reset sources, identify exact neutral Core and board-only owners, and implement only bounded owner separation needed before S31 neutral Core relocation. Split oversized source moves into linear numeric S receivers if inspection proves the batch too large. |
| Non-goals | Physical Core move (S31 or later), board family extraction (S32 onward), controller timing formulas, profile/INI/firmware/media changes or a new framework. |
| Reference Baseline | Accepted S29 P1 `ace7c0fa6`, [S25 arbitration intake](../etc/evidence/t540-s25-arbitration-intake.md), dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and accepted S22-S29 evidence. |
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
are recorded in the [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md).
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
| Reference Baseline | Accepted S30 P1 `4f9f72b28`, [S30 plan/reset intake](../etc/evidence/t540-s30-plan-reset-intake.md) and unchanged S29 dual-width 469/469 units, specialized gates, eight external boots and eight optimized products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md), [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md) and S30 intake. |
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
The [S31 evidence](../etc/evidence/t540-s31-frozen-plan-boundary.md) records
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
| Reference Baseline | S31 P1 `02c505858`, [S31 evidence](../etc/evidence/t540-s31-frozen-plan-boundary.md) and dual-width 469/469 units, specialized gates, 8/8 external boot checkpoints, eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
unchanged. The [S32 evidence](../etc/evidence/t540-s32-neutral-construction.md)
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
| Reference Baseline | S32 P1 `c5e8022d7`, [S32 evidence](../etc/evidence/t540-s32-neutral-construction.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
The [S33 evidence](../etc/evidence/t540-s33-board-construction.md) records
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
| Reference Baseline | S33 P1 `134c8e722`, [S33 evidence](../etc/evidence/t540-s33-board-construction.md), dual-width 469/469 units, specialized gates, 8/8 boot checkpoints and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
The [S34 evidence](../etc/evidence/t540-s34-reset-boundary.md) records the
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
| Reference Baseline | S34 P1 `728553d8e`, [S34 evidence](../etc/evidence/t540-s34-reset-boundary.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
image loop is gone. The [S35 evidence](../etc/evidence/t540-s35-teardown-boundary.md)
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
| Reference Baseline | S35 P1 `5e0f86e2a`, [S35 evidence](../etc/evidence/t540-s35-teardown-boundary.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
[S36 source audit](../etc/evidence/t540-s36-entry-rom-trace-boundary.md)
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
| Reference Baseline | S36 P1 `53fdcef56`, [S36 evidence](../etc/evidence/t540-s36-entry-rom-trace-boundary.md), S31-S35 evidence, dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [adapter ledger](../etc/architecture/t540-board-adapter-ledger.md). |
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
[S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md).
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
| Reference Baseline | S37 P1 `1e68c2e9f`, [S37 ledger](../etc/architecture/t540-s37-core-relocation-ledger.md), S36 dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). |
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
[S38 evidence](../etc/evidence/t540-s38-board-firmware-alias.md) records
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
| Reference Baseline | S38 P1 `69057d2e9`, [S38 evidence](../etc/evidence/t540-s38-board-firmware-alias.md), [S37 ledger](../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [S30 intake](../etc/evidence/t540-s30-plan-reset-intake.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). |
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

Actual-diff review accepts NXVM P1 `d96502c6c`. The [S39 evidence](../etc/evidence/t540-s39-board-memory-boundary.md)
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
| Reference Baseline | S39 P1 `d96502c6c`, [S39 evidence](../etc/evidence/t540-s39-board-memory-boundary.md), [S37 ledger](../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). |
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
[S40 evidence](../etc/evidence/t540-s40-board-shutdown-input.md) records the
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
| Reference Baseline | S40 P1 `399dad3e3`, [S40 evidence](../etc/evidence/t540-s40-board-shutdown-input.md), [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). |
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
[S41 evidence](../etc/evidence/t540-s41-board-clock-domains.md) records
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
| Reference Baseline | S41 P1 `b33ee1220`, [S41 evidence](../etc/evidence/t540-s41-board-clock-domains.md), [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [refined handoff](../etc/architecture/t540-s7-core-board-handoff.md) and [S37 relocation ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). S37's S43 physical move is prospective and must be renumbered only after the actual bounded split receivers are known. |
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
[private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md).
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
| Reference Baseline | S42 P1 `78974cff4`, [S42 ledger](../etc/architecture/t540-s42-private-state-ledger.md), S41 dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 ownership ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S43 evidence](../etc/evidence/t540-s43-board-constructor.md) records the
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
| Reference Baseline | S43 P1 `8645c1c74`, [S43 evidence](../etc/evidence/t540-s43-board-constructor.md), [S42 ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, specialized gates, 8/8 boots and eight optimized 0540 products. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S44 evidence](../etc/evidence/t540-s44-board-state.md) records 27 board
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
| Reference Baseline | S44 P1 `784e95e40`, [S44 evidence](../etc/evidence/t540-s44-board-state.md), [S42 private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, specialized gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and [S42 ownership ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | PIC state is read in three production and fifty direct test files. The measured [receiving plan](../etc/architecture/t540-s42-private-state-ledger.md) assigns PIT, DMA, RTC, FDC, HDC, keyboard and VADP to S46–S52 before code. S45 edits only the PIC group, its direct fixtures and exact source inventories. |
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
[S45 evidence](../etc/evidence/t540-s45-pic-owner.md) records the PIC
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
| Reference Baseline | S45 P1 `c47f4fb7d`, [S45 evidence](../etc/evidence/t540-s45-pic-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S46 evidence](../etc/evidence/t540-s46-pit-owner.md) records the primary
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
| Reference Baseline | S46 P1 `5a34e2acf`, [S46 evidence](../etc/evidence/t540-s46-pit-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S47 evidence](../etc/evidence/t540-s47-dma-owner.md) records the DMA latch
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
| Reference Baseline | S47 P1 `61a53ec3e`, [S47 evidence](../etc/evidence/t540-s47-dma-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S48 evidence](../etc/evidence/t540-s48-rtc-owner.md) records the RTC chip
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
| Reference Baseline | S48 P1 `15fb83f58`, [S48 evidence](../etc/evidence/t540-s48-rtc-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S49 evidence](../etc/evidence/t540-s49-fdc-owner.md) records the sole FDC
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
| Reference Baseline | S49 P1 `7e21979dd`, [S49 evidence](../etc/evidence/t540-s49-fdc-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S50 evidence](../etc/evidence/t540-s50-hdc-owner.md) records the sole HDC
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
| Reference Baseline | S50 P1 `997d28483`, [S50 evidence](../etc/evidence/t540-s50-hdc-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates, focused HDD/ATA integration and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S51 evidence](../etc/evidence/t540-s51-keyboard-owner.md) records both AT
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
| Reference Baseline | S51 P1 `b28f48550`, [S51 evidence](../etc/evidence/t540-s51-keyboard-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S52 evidence](../etc/evidence/t540-s52-vadp-owner.md) records the sole
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
| Reference Baseline | S52 P1 `855045aac`, [S52 evidence](../etc/evidence/t540-s52-vadp-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
rebuilt and PE/no-debug verified; the [S53 evidence](../etc/evidence/t540-s53-xt-keyboard-and-electrical-intake.md)
contains exact hashes and the S54-S61 receiving map. T540 remains open.

## T540 S54 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S54, next linear S after accepted S53. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only planar-parity configuration, Port-B and NMI latch state from flat Core storage into the sole board attachment; preserve the existing port, memory-fault and speaker signal path. |
| Non-goals | D4 platform/NMI (S55), D4 refresh/DMA (S56), XT speaker state (S57), absent-memory routes (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), behavior/timing change. |
| Reference Baseline | S53 P1 `e313e4660`, [S53 evidence](../etc/evidence/t540-s53-xt-keyboard-and-electrical-intake.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
products are rebuilt and PE/no-debug verified. The [S54 evidence](../etc/evidence/t540-s54-planar-parity-owner.md)
records the one x86 timeout under concurrent load, the clean isolated test,
the full clean rerun, exact diff and artifact hashes. T540 remains open.

## T540 S55 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S55, next linear S after accepted S54. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only D4 platform configuration, Port-B, IOCHK/failsafe and NMI latch state to the sole board attachment. Preserve D4 Port-B, failsafe and NMI signal semantics. |
| Non-goals | D4 refresh/DMA state (S56), XT speaker (S57), absent memory (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), D4 timing or behavior change. |
| Reference Baseline | S54 P1 `8712ec968`, [S54 evidence](../etc/evidence/t540-s54-planar-parity-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
0540 products are rebuilt and PE/no-debug verified; the [S55 evidence](../etc/evidence/t540-s55-d4-platform-owner.md)
records exact diff and artifact hashes. The three D4 refresh request fields
remain for S56. T540 remains open.

## T540 S56 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S56, next linear S after accepted S55. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move D4 refresh pending/pulse/address electrical latches to the sole board attachment while Core continues to service the one bounded HOLD/arbitration operation. Audit DMA refresh producer/consumer wiring for a single owner. |
| Non-goals | XT speaker (S57), absent memory (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), altered DMA grant or guest-time behavior. |
| Reference Baseline | S55 P1 `8ca51989a`, [S55 evidence](../etc/evidence/t540-s55-d4-platform-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
are rebuilt and PE/no-debug verified. The [S56 evidence](../etc/evidence/t540-s56-d4-refresh-owner.md)
records the gate correction, exact diff and artifact hashes. T540 remains
open.

## T540 S57 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S57, next linear S after accepted S56. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move XT PPI speaker configuration, timer gate, data enable and observed output to the sole board attachment; retain the existing PIT/PPI/Port-B signal path. |
| Non-goals | Absent-memory routes (S58), callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), speaker waveform or host-audio behavior change. |
| Reference Baseline | S56 P1 `71efa3bb5`, [S56 evidence](../etc/evidence/t540-s56-d4-refresh-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
are rebuilt and PE/no-debug verified. The [S57 evidence](../etc/evidence/t540-s57-speaker-owner.md)
records the exact diff and artifact hashes. T540 remains open.

## T540 S58 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S58, next linear S after accepted S57. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Move only the configured absent-memory windows to the sole board attachment while retaining the existing typed Core memory route, open-bus priority and rollback contract. |
| Non-goals | Callback/firmware audit (S59), neutral header (S60), physical Shared move (S61), absent-memory behavior or priority change. |
| Reference Baseline | S57 P1 `1825a1859`, [S57 evidence](../etc/evidence/t540-s57-speaker-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
products are rebuilt and PE/no-debug verified. The [S58 evidence](../etc/evidence/t540-s58-absent-memory-owner.md)
records rollback, lifetime, exact diff and artifact hashes. T540 remains open.

## T540 S59 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S59, next linear S after accepted S58. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Audit and correct the existing board callback install/revoke and firmware binding owners so neutral Core only invokes bounded providers and the board retains its F0000h alias choice, without a second lifetime or direct chip access. |
| Non-goals | Neutral private header (S60), physical Shared move (S61), firmware behavior/ROM change, callback ABI redesign or new board framework. |
| Reference Baseline | S58 P1 `930d35bec`, [S58 evidence](../etc/evidence/t540-s58-absent-memory-owner.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units, gates and 8/8 boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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

The [S59 callback/firmware audit](../etc/evidence/t540-s59-callback-firmware-audit.md)
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
| Reference Baseline | [S59 evidence](../etc/evidence/t540-s59-callback-firmware-audit.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), dual-width 469/469 units and gates; S58's eight booted 0540 artifacts. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the measured [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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

The [measured neutral-header intake](../etc/evidence/t540-s60-neutral-header-intake.md)
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
| Reference Baseline | [S60 measured intake](../etc/evidence/t540-s60-neutral-header-intake.md), [receiving ledger](../etc/architecture/t540-s42-private-state-ledger.md), current dual-width units/gates and S58's booted eight-product baseline. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md). |
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
evidence](../etc/evidence/t540-s61-d4-memory-owner.md) records exact diff,
ownership and hashes. T540 remains open.

## T540 S62 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S62, next linear S after accepted S61. |
| Admission And Approval | Owner's standing automatic admission applies to bounded numeric T540 S work. NXVM only; Shared and MyNES remain read-only. |
| Objective | Separate the existing frozen plan's board topology and D4/media/display/FDC inputs from neutral Core timing declarations while preserving one plan, one application order and one rollback. |
| Non-goals | Private/public header completion (S63-S64), physical Shared relocation (S65), a second plan or parser, timing/behavior change, profile/INI/media change. |
| Reference Baseline | S61 P1 `ab3c8f437`, [S61 evidence](../etc/evidence/t540-s61-d4-memory-owner.md), [S60 measured intake](../etc/evidence/t540-s60-neutral-header-intake.md), dual-width units/gates and eight boots. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S62 evidence](../etc/evidence/t540-s62-frozen-timing-declarations.md)
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
| Reference Baseline | S62 P1 `25a350639`, [S62 evidence](../etc/evidence/t540-s62-frozen-timing-declarations.md), [S60 measured intake](../etc/evidence/t540-s60-neutral-header-intake.md). |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
real transitive include consumers. The [S63 evidence](../etc/evidence/t540-s63-private-header-boundary.md)
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
| Reference Baseline | S63 P1 `4d6443833`, [S63 evidence](../etc/evidence/t540-s63-private-header-boundary.md), [S60 measured intake](../etc/evidence/t540-s60-neutral-header-intake.md). |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and the [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
| Files And ABI Surface | `machine_interface.h` and its direct consumers, current neutral source inventory and bounded compile target. Measure its 694-line/174-includer surface before editing; split further into linear numeric S if one owner cut is unsafe. |
| Applicable Rules | No PC topology or product type in the neutral public contract; exactly one function implementation and one construction path. Board contracts retain their real owner. |
| Verification | Independent strict neutral compile plus complete x64/x86 units, specialized/documentation gates, affected boot matrix and optimized eight-product PE/no-debug output if code changes. |
| Asset Needs | Existing external boot inputs only; do not alter owner INI. |
| Stop Conditions | Public interface contains multiple distinct owner cuts too large for one S; record finite split before modifying those cuts. |
| Exit Criteria | P1/P2 pushed if code changes; independent compile proof and affected checks pass; worktree clean. |
| Similar-Issue Sweep | Public header transitive includes, direct source/test consumers, target source lists and static gates. |

## T540 S64 Acceptance

The [S64 intake](../etc/evidence/t540-s64-public-interface-intake.md)
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
| Reference Baseline | S63 P1 `4d6443833`, [S64 measured intake](../etc/evidence/t540-s64-public-interface-intake.md) and [S63 evidence](../etc/evidence/t540-s63-private-header-boundary.md). |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
See [S65 evidence](../etc/evidence/t540-s65-neutral-construction-input.md).
This accepts S65 only: public board values/operations and four direct
Core reset/clock/NMI/finalization calls remain with their named receivers.

## T540 S66 Accepted Admission Record

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S66, next numeric S after accepted S65. |
| Admission And Approval | Owner standing automatic bounded-S admission; NXVM only, Shared/MyNES read-only. |
| Objective | Give public board construction, topology and observation values their explicit board owner, removing their definitions and concrete chip dependencies from the neutral Core interface. |
| Non-goals | Runtime operation changes, neutral validator implementation cut (S67), Core lifecycle handoff (S68), independent compile (S69), physical Shared move (S70), timing/behavior/profile/INI/asset changes. |
| Reference Baseline | S65 P1 `c6ce9b84c`; [S64 intake](../etc/evidence/t540-s64-public-interface-intake.md) and [S65 evidence](../etc/evidence/t540-s65-neutral-construction-input.md). |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[S66 evidence](../etc/evidence/t540-s66-public-board-interface.md).
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
The [S67 evidence](../etc/evidence/t540-s67-neutral-validation-owner.md) records
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
| Reference Baseline | Accepted/pushed S67 P1 `5db0fc37b` and its [evidence](../etc/evidence/t540-s67-neutral-validation-owner.md). Actual source intake finds reset-devices and reset-clocks in cold reset, NMI refresh on unmask, and finalization in the sole destructor. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md). |
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
[receiving evidence](../etc/evidence/t540-s68-board-lifecycle-handoff.md)
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
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md) and [source ledger](../etc/architecture/t540-s37-core-relocation-ledger.md). |
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
[independent receiving evidence](../etc/evidence/t540-s69-independent-neutral-core.md)
records sixteen actual-source objects, eighteen required headers, no board
archive dependency, both-width Debug/Release execution and 470/470 complete
units per width. Product sources and all eight S68 EXE identities are unchanged.
Physical relocation and public board-consumer boundaries remain open.

## T540 S70 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T540 S70, next unused numeric S after accepted S69. |
| Admission And Approval | Owner standing automatic bounded-S authorization within T540 extraction; NXVM source/test/build evidence only. Shared, MyNES, owner INIs and external assets remain read-only. |
| Objective | Remove every production board borrow of the CPU execution context for NMI and processor-reset signals; Core alone accepts these signals and the existing CPU/run path consumes them. |
| Non-goals | Physical Shared relocation, new CPU signal state, lifecycle queue or executor, CPU/timing/guest behavior change, RAM/port/firmware boundary changes and MyNES builds. |
| Reference Baseline | Accepted S69 P2 `7c0c8c458`. Actual intake finds three NMI and one reset call in board composition still borrow `executor_cpu_execution`; independent linkage did not prove public consumer boundaries. |
| Candidate Proposal | [T540 integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), [private-state ledger](../etc/architecture/t540-s42-private-state-ledger.md), [S69 proof](../etc/evidence/t540-s69-independent-neutral-core.md). |
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

## S87 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S87, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic admission of bounded linear T539 S deliveries and required oversized CPU sources to be split by actual responsibility. This follows accepted S86. |
| Objective | Extract only the CPU bus, instruction-effect and copied retirement-observation rows from the mixed execution-context test into one Shared CPU receiver. |
| Non-goals | CPU create/reset/prepared-entry/lifecycle rows, NMI, prefetch, paging, INVLPG, public board wiring, CPU semantics/timing, production APIs, firmware, assets, INI and executable inputs. |
| Reference Baseline | `42ec9a014` after S86; the ledger assigns execution-context bus/observation rows to S87 and reserves lifecycle for S88 and signal/prefetch/paging for S89. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the revised [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
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
[S87 evidence](../etc/evidence/t539-s87-execution-bus-receiver.md). S87 is
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
INI or EXE input changed. See [S88 evidence](../etc/evidence/t539-s88-execution-lifecycle-receiver.md).
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
[S89 evidence](../etc/evidence/t539-s89-signal-prefetch-receiver.md). S89 is
accepted; T539 remains open for S90-S100.

## S90 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S90, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S90 is the next linear package after accepted S89. |
| Objective | Move only 80186 LGDT availability, paging-control and INVLPG rows from the residual execution-context test to bounded Shared CPU receivers. |
| Non-goals | Protected fault/pending-event rows, public paging/board wiring, firmware, production APIs, assets, INI, executables and unrelated residual CPU rows. |
| Reference Baseline | `30fa60856`, accepted S89; the residual source contains S90 paging/INVLPG and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
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
See [S90 evidence](../etc/evidence/t539-s90-execution-paging-receiver.md).
S90 is accepted; T539 remains open for S91-S100.

## S91 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S91, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S91 is the next linear package after accepted S90. |
| Objective | Move only CPU-owned protected interrupt preparation, UD cache-preservation and pending-event rollback rows from the residual execution-context test. |
| Non-goals | Public Core paging, PIC IRQ delivery, board transactions, firmware, production APIs, assets, INI, executables and unrelated CPU rows. |
| Reference Baseline | `d8e3665ec`, accepted S90; the residual source contains debug and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
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
input changed. See [S91 evidence](../etc/evidence/t539-s91-execution-fault-event-receiver.md).
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
[S92 evidence](../etc/evidence/t539-s92-protected-receivers.md). S92 is
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
[S93 evidence](../etc/evidence/t539-s93-task-state-receivers.md). S93 is
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
See the [S94 evidence](../etc/evidence/t539-s94-cpu-instruction-receivers.md).
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
[S95 evidence](../etc/evidence/t539-s95-operand-prefix-receivers.md). S95 is
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
[S96 evidence](../etc/evidence/t539-s96-lock-imul-receivers.md). S96 is
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
[S97 evidence](../etc/evidence/t539-s97-residual-cpu-classification.md). S97
is accepted; T539 remains open for S98-S100.

## S98 Acceptance

Shared `src/x86/devices/cpu` remains the sole CPU timing implementation. The
timing/catalog runners remain their one NXVM owner because each composes an
App machine/profile, publishes board-time observations, or produces a
generated result contract. S98 adds no synthetic Shared machine fixture, no
second timing route and no copied formula table.

The CPU timing-manifest catalog, x64/x86 full units (**465/465** each), T317,
CPU/PIC authority, Shared manifest/corpus and documentation governance pass.
See the [S98 evidence](../etc/evidence/t539-s98-timing-catalog-boundary.md).
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
See the [S99 evidence](../etc/evidence/t539-s99-final-cpu-path-cleanup.md).
S99 is accepted; T539 remains open for S100.

## S100 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S100, automatically admitted final continuation. |
| Admission And Approval | The owner approved automatic, bounded, strictly linear numeric T539 continuation tasks; S100 follows accepted S99. |
| Objective | Prove final CPU test ownership: each CPU-only receiver and fixture has one Shared owner; each machine, PIC, board-time and result-publication receiver has one named NXVM owner; no retired App CPU test path survives. |
| Non-goals | Further CPU behavior/timing changes, new Shared APIs/frameworks, profile changes, firmware, assets, INI and executable changes. |
| Reference Baseline | `dfe846cbc`, accepted S99. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
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
[S100 evidence](../etc/evidence/t539-s100-whole-cpu-acceptance.md). S100 is
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
so no EXE rebuild is required. See [S101 evidence](../etc/evidence/t539-s101-cpu-source-cleanup.md).

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
rebuild is required. See the [S86 evidence](../etc/evidence/t539-s86-debug-state-receiver.md).
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
required. See the [S85 evidence](../etc/evidence/t539-s85-control-state-receiver.md).
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
[S84 evidence](../etc/evidence/t539-s84-system-table-receivers.md). S84 is
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
Shared, firmware, INI or EXE input changed. See [S40 evidence](../etc/evidence/t539-s40-arpl-migration.md).
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
documentation governance passes. See [S39 evidence](../etc/evidence/t539-s39-port-io-migration.md).
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
executable input changed. [S31 evidence](../etc/evidence/t539-s31-imul-group2-migration.md)
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
[S30 evidence](../etc/evidence/t539-s30-bit-condition-extension-migration.md).
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
[S32 evidence](../etc/evidence/t539-s32-legacy-alu-lock-migration.md) contains
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
or executable input changed. [S33 evidence](../etc/evidence/t539-s33-inc-dec-first-group-migration.md)
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
or executable input changed. [S34 evidence](../etc/evidence/t539-s34-test-add-adc-sbb-migration.md)
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
production/API or executable input changed. [S35 evidence](../etc/evidence/t539-s35-final-inc-dec-migration.md)
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
input changed. [S36 evidence](../etc/evidence/t539-s36-flags-migration.md)
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
input changed. [S37 evidence](../etc/evidence/t539-s37-string-transfer-migration.md)
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
[S38 evidence](../etc/evidence/t539-s38-string-scan-compare-migration.md)
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
are recorded in [S41 evidence](../etc/evidence/t539-s41-bound-migration.md).
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
the reviewed commit equals `origin/master`. See [S42 evidence](../etc/evidence/t539-s42-table-register-migration.md).

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
[S43 evidence](../etc/evidence/t539-s43-descriptor-system-migration.md).
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
[S45 evidence](../etc/evidence/t539-s45-control-state-migration.md). T539
remains open for S46 and later packets.

## S46 Acceptance

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S46, continuation implementation. |
| Admission And Approval | The owner granted automatic admission for each bounded T539 S. This packet admits the next finite debug-state migration batch on 2026-09-30. Allowed target: NXVM only. |
| Objective | Retire direct-private CPU access from MOV DR and TF/#DB test receivers while retaining one CPU-local or public-board receiver for every original semantic case. |
| Non-goals | No CPU production change, new public API, Shared/MyNES change, firmware, asset, INI or EXE update, timing reinterpretation, or migration of any S47-plus source. |
| Reference Baseline | This Current packet; [CPU work packages](../etc/architecture/t539-cpu-work-packages.md); [incremental inventory](../etc/evidence/t539-cpu-incremental-inventory.md); S45 evidence; existing x86 CPU fixture and public Core-machine contracts. |
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
[S46 evidence](../etc/evidence/t539-s46-debug-state-migration.md) retains the
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
review.  [S47 evidence](../etc/evidence/t539-s47-protected-16-fixture-migration.md)
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
See [S48 evidence](../etc/evidence/t539-s48-call-gate-privilege-entry-migration.md).
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
evidence](../etc/evidence/t539-s49-control-transfer-branch-migration.md).
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
evidence](../etc/evidence/t539-s50-control-transfer-near-migration.md).
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
evidence](../etc/evidence/t539-s51-control-transfer-receiver-map.md).  S51 is
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
evidence](../etc/evidence/t539-s52-idt-privilege-receiver-map.md).  S52 is
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
and diff checks pass.  The [S53 receiver map](../etc/evidence/t539-s53-protected-far-data-receiver-map.md)
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
The [S54 receiver map](../etc/evidence/t539-s54-outer-return-receiver-map.md)
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
required. The [S55 receiver map](../etc/evidence/t539-s55-task-switch16-receiver-map.md)
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
[S75 evidence](../etc/evidence/t539-s75-opaque-cpu-lifetime.md) records the
boundary and artifact hashes. S75 is accepted; T539 remains open for S76-S82.

## S81 Acceptance

S81 moved the five 1,804-line CPU-only `MOVS`/`LODS`/`STOS`/`SCAS`/`CMPS`
test suites to one Shared CPU owner and deleted the NXVM duplicates. Real Core
memory, interruptibility, and PIC/IRQ observations remain NXVM board tests.
Complete repository-only units pass 427/427 on x64 and x86. Shared
manifest/corpus, CPU/PIC authority, documentation governance, and diff checks
pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input; the 0539 artifacts remain current. See the
[S81 evidence](../etc/evidence/t539-s81-string-receivers.md). S81 is
accepted; T539 remains open for S82-S86.

## S82 Acceptance

S82 moved the two 642-line CPU-only scalar/string port suites and their sole
68-line CPU-bus fixture to one Shared owner, then deleted NXVM duplicates.
Actual port routing, permission, and PIC/IRQ observations remain named NXVM
board tests. Complete repository-only units pass 429/429 on x64 and x86.
CPU/PIC authority, Shared manifest/corpus, documentation governance, and diff
checks pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input. See the [S82 evidence](../etc/evidence/t539-s82-port-receivers.md).
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
[S83 evidence](../etc/evidence/t539-s83-descriptor-operands.md). S83 is
accepted; T539 remains open for S84-S94.

## S80 Acceptance

S80 moved three 2,105-line CPU-only segment/data tests to one Shared owner and
deleted NXVM duplicates. Public Core memory/fault/IRQ observations remain NXVM
tests. See the [S80 evidence](../etc/evidence/t539-s80-segment-data-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S80 is accepted;
T539 remains open for S81-S86.

## S79 Acceptance

S79 moved five 1,883-line CPU-only segment-stack/far-pointer tests to one
Shared owner and deleted NXVM duplicates. Board-specific descriptor,
memory/fault, and IRQ observations remain NXVM tests. See the
[S79 evidence](../etc/evidence/t539-s79-segment-stack-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S79 is accepted;
T539 remains open for S80-S86.

## S78 Acceptance

S78 moved the four 1,858-line CPU-only basic-stack tests to one Shared owner
and deleted the duplicate NXVM paths. Real Core stack/fault/IRQ cases remain
explicit NXVM board tests. The [S78 evidence](../etc/evidence/t539-s78-basic-stack-receivers.md)
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
[S77 evidence](../etc/evidence/t539-s77-cpu-arithmetic-receivers.md).

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
PE architectures plus hashes are recorded in the [S76 evidence](../etc/evidence/t539-s76-shared-cpu-cutover.md).
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
rebuild is required.  See the [S74 receiver map](../etc/evidence/t539-s74-remaining-consumer-fixture-map.md).
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
The [S73 receiver map](../etc/evidence/t539-s73-80386-timing-receiver-map.md)
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
The [S72 receiver map](../etc/evidence/t539-s72-80286-manifest-call-gate-receiver-map.md)
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
The [S71 receiver map](../etc/evidence/t539-s71-80286-protected-io-timing-receiver-map.md)
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
required. The [S70 receiver map](../etc/evidence/t539-s70-80186-timing-receiver-map.md)
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
[P1 Shared evidence](../etc/evidence/t539-s69-p1-shared-video-aperture.md) and
[P2 receiver map](../etc/evidence/t539-s69-8086-timing-receiver-map.md)
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
so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s68-timing-baseline-receiver-map.md)
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
so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s67-vm86-receiver-map.md)
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
no EXE rebuild is required.  The [receiver map](../etc/evidence/t539-s66-int-entry-receiver-map.md)
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
so no EXE rebuild is required.  The [receiver map](../etc/evidence/t539-s65-protected-iret-receiver-map.md)
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
EXE input changed, so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s64-cli-sti-receiver-map.md)
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
rebuild is required. The [receiver map](../etc/evidence/t539-s63-tss-iomap-receiver-map.md)
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
[receiver map](../etc/evidence/t539-s62-task-cross-width-receiver-map.md)
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
EXE input changed, so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s61-task-switch32-pending-irq-receiver-map.md)
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
rebuild is required. The [receiver map](../etc/evidence/t539-s60-task-switch32-nesting-receiver-map.md)
records the exact allocation. S60 is accepted; T539 remains open for
S61-S69.

## S59 Acceptance

The two direct TSS32 paging contexts now have one public-Core receiver,
`machine_task_switch32_paging_smoke.c`.  It builds real guest page tables,
GDT, IDT and TSS images through public physical-memory writes, performs guest
`LGDT`/`LIDT`/`LMSW`/`LTR`, and observes a mapped target transition plus an
unmapped target-TSS `#PF` through the public diagnostic and copied snapshot
contracts.  The retained mixed source contains neither S59 page-table state
nor private IDTR mutation.  The [receiver map](../etc/evidence/t539-s59-task-switch32-paging-receiver-map.md)
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
rows. The [receiver map](../etc/evidence/t539-s56-task-switch32-decode-map.md)
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
[receiver map](../etc/evidence/t539-s57-task-switch32-state-map.md) records
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
[S29 evidence](../etc/evidence/t539-s29-operand-prefix-migration.md). S29 is
accepted; 82 original private consumers remain assigned to S30-S42. S43 owns
opaque lifetime, S44 physical Shared relocation and S45 whole-CPU acceptance.

## S28 Acceptance

Actual-commit review accepts pushed NXVM P1 `fa092b95e`: the 244 original
execution contexts and three metadata queries retain CPU/board receivers;
the inverted 286 test result and incorrect EAX expectation are corrected.
Full x64/x86 units pass 387/387 each, with 66 specialized gates, six unchanged
manifests and documentation/diff checks. Ten test/build paths add 2,065/remove
1,530 lines, net +535; no production or executable input changed. See
[S28 evidence](../etc/evidence/t539-s28-segment-migration.md). The S28 packet
is closed, not carried into S29.

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
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

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain the
complete scope, original requirements, earlier acceptance and receiving proof.
The historical S18-S20 prospective numbering is superseded only for unadmitted
packages by the current work plan.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0540 EXEs were rebuilt by
T540 S70 with unchanged owner INIs; S70 evidence records their hashes, PE
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

The seven [Queue](QUEUE.md) candidates retain dependency order. Acceptance does
not claim indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
