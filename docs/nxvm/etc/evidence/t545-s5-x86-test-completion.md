# T545 S5 x86 Test Completion

Baseline 05427402a. Shared test ownership repair and NXVM receiver records;
production/API, deployed EXEs and queued CPU manual repairs stay unchanged.

## Initial Relocation Ledger

Three 80186/80286/80386 decoder inventory producers in IBMPC only call the CPU
lexical decoder and write code-owned output; move them unchanged into x86.
Their exact accepted counts and JSON predicates survive both standalone and
historical NXVM consumer paths. This is lexical inventory, not semantic/timing
qualification.

The existing IBMPC timeline probe combines heap ordering/cancel/reset/nested
scheduling with board/PIT/PIC/CPU progress. Move neutral heap assertions into
an x86-local test and retain board signal-chain assertions and original marker
at IBMPC. Audit remaining chip/CPU/Core and xasm/Debug contracts before closure;
file references or green tests alone do not prove complete owner-local coverage.

Coordinator refinement: machine_entry_plan_smoke checks only Core entry/preload
transaction and immutable mapping. Its sole board dependency is construction.
Move it to x86 and use neutral_create with equivalent CPU/memory/tick values;
retain every success/failure/state/memory assertion and original target/marker.
Port route atomicity is currently exercised through board endpoints, not a
neutral local route test; add owner-local cases to the existing neutral probe.

## Current Proof And Remaining Inventory

Three decoder files move byte-for-byte with their JSON/count predicates. The
entry-plan test retains every assertion and original target/marker; only its
include/config/constructor becomes neutral. The new timeline test receives
every old heap assertion and adds capacity, rejected-token preservation,
stale cancellation, monotonic advance and sequence-exhaustion boundaries.
IBMPC retains its machine/PIT/PIC progress/trace assertions and original marker.
neutral_link gains direct route batch rollback, wired-OR, owner removal,
read/write failure publication, deterministic time/overflow/reset checks.

Independent Release x86 builds pass both widths. The six changed/relocated
core and decoder cases pass 6/6 per width; the initial complete x64 suite
passes 172/172 (56.20s); x86 also passes 172/172 (70.79s). Subsequent KBC
configuration/serial additions pass on both widths, along with fresh manifests.
Receiving/full-unit verification remains pending. Root reconfiguration completed, but the same initial
multi-target make invocation had parsed its old target list and rejected the
new timeline target; a subsequent explicit build verifies the regenerated
target, rather than concealing the failure. Regenerated receiving timeline,
board timeline, entry-plan and neutral Core targets now execute successfully.

The same KBC-owned fixture now checks input/test pin projections, reset output,
poll-delayed replies, preserved serial delay, copied multi-byte admission,
whole-capacity rejection and timed set-2 translation. Original transport,
parameter-interleaving and AUX saturation cases remain intact. All changes
are test-only; no controller or board algorithm/API is altered.

An API-reference diagnostic also identifies direct-observer, timing/deadline,
CPU execution-adapter, KBC configuration/serial, DMA/FDC/HDC handshake and
video validation candidates. Inspect actual producer/consumer assertions and
source for each before disposition; many implementation callbacks are exercised
through higher-level tests of the same owner. Neither API-reference absence nor
a passing suite settles that inventory. S5 remains active and is not ready for
acceptance; S6 and final S7 remain unimplemented.

## Continued Gap Review

The Core-owned debug test now directly proves code-base/default-size observation,
linear/real memory read/write correspondence, all three watchpoint set/get/clear
contracts and invalid outputs preserving state. Existing instruction-triggered
watchpoint behavior remains. Rational clock tests add six literal phase-aware
inverse-deadline cases, invalid inputs, read-only observation and overflow output
preservation. Neutral Core tests add idle/future/immediate deadline advancement
and disabled L1 escape without injecting host time. All pass both widths.

HDC's own existing fixture now transfers a full Xebec DMA read/write record and
aborts each direction with early terminal count; it checks DRQ/IRQ, media commit
counts, response status and no data leaking after completion. PIC copied-register
observation and invalid outputs are local, with request selection proving that
caller edits do not alter chip state. Both updated cases and manifests pass on
both widths. Production source remains unchanged.

Further ownership review finds machine_fpu_interface_s65_smoke in test/ibmpc
already constructs a neutral Core and uses no board device, but imports generic
Core reset/exception fixtures through IBMPC paths. It now moves to x86, using
the already-owned debug/exception fixtures and the exact three bind/freeze/reset
calls formerly wrapped by the board helper. Its entire CPU/FPU matrix, exception
predicates, target and output markers remain unchanged. No fixture is borrowed
from IBMPC or duplicated. Historical NXVM registrations follow the new source.
The migrated matrix builds and passes on both widths; it additionally checks
Core's public FPU-profile/state observation and invalid output arguments against
every constructed variant. The former instruction/exception assertions remain.
Actual board scheduler attachment forwarding stays IBMPC; do not move its PIT,
DMA, media or PIC wiring assertions into a neutral test. Remaining diagnostic
callback/type matches still need semantic disposition, not blanket acceptance.

## Additional Core Contract Proof

The existing neutral firmware callbacks now exercise their actual active
configure/reset capabilities: configure rejects runtime RAM access without
publishing output, reset performs checked RAM/port reads and writes, and an
optional copied-ROM alias is installed through the active configuration context.
A separate neutral instance preserves the original one-ROM test input and
checks alias bytes plus rejected inactive context operations. No production
BIOS service or firmware path is introduced.

Memory-provider tests now remove just the overlay owner while preserving the
independent ROM mapping, reject invalid removal, and exercise A20 set/observe.
Attachment tests use a neutral deadline producer (not PC devices): L1 progress
stops immediately when its class changes, blocked fast advance stays inert,
unblocked advance reaches the exact deadline, and persistent L1 is bounded by
the existing 16-step host-control limit. That bound is not a claimed device
duration. CPU's own contract test adds all five profile names/early semantics
and coalesced shutdown request consumption. These cases pass both widths.

The additions reuse existing owner-local tests and callbacks. Independent tests
remain self-contained; remaining inventory, receiving builds and complete-unit
verification are required before acceptance. Current standalone registration
has 173 cases after moving the preserved CPU/FPU matrix from IBMPC.

Fresh complete standalone runs pass 173/173 per width (14.14s x64, 13.81s x86).
A later constructor rollback case injects a configure failure after both ROM
and alias creation, verifies provider/mapping/route rollback, then retries
successfully. Its initial public-memory-query assertion failed on both widths
because INITIALIZED is outside that API's stopped/paused contract. Core-owned
construction tests use the actual private physical route query at that stage;
the rollback predicate is retained, not weakened, and production is unchanged.

Receiving configuration exposed one remaining IBMPC target_link_libraries for
the moved CPU/FPU target. It is removed, since x86 owns that target and its sole
Core link. This was a test/build receiver gap, not a production-library change;
fresh receiving generation/build must prove the correction before closure.

The corrected receiving x64/x86 x86 suites, board timeline and three historical
decoder producers build successfully. An intermediate full x64 repository run
passes 532/532 (119.25s), but predates the last CECG negative-case addition and
is not final acceptance proof. The CECG owner test now independently rejects
each of the five invalid configuration fields and proves previous configuration
preservation; this passes both standalone widths. Receiving rebuilds follow.

## Reconciled Contract Families

The frozen coverage unit is an owned mechanism/contract, not every spelling of
an implementation helper and not a new manual-complete instruction oracle.
Public-header reference diagnostics are reconciled against actual code paths:
record-result and pin/attachment structs preceding callback parentheses are
types, not missing callable APIs. Child execution/clock/validation helpers have
local proof through the actual same-package operations below, not App tests.

| Owned family | x86-local proof / before-after disposition |
| --- | --- |
| CPU profile, execution and bus | cpu_contract checks profile identities/early semantics, reset state, shutdown consumption and fallback timing bound; per-instruction bus fixtures, control_state/control-transfer, software INT, protected/VM86 delivery, execution_fault_event and context isolation tests retain instruction, bus, exception and rollback assertions. Private consume/dispatch helpers run through these real execution paths. Queued T544 manual repairs remain excluded, not represented as newly qualified. |
| CPU timing/decoder forms | Existing family timing ledgers and qualification-key cases remain at x86; all three relocated lexical producers keep their exact counts/output. Lexical acceptance is not timing or retirement correctness. |
| CPU/FPU integration | fpu_contract/fpu interface state and migrated machine_fpu_interface_s65 retain profile combinations, ESC/WAIT/fault/deadline/reset contracts. Public FPU profile/state observations are direct; genuine board IRQ routing remains IBMPC. |
| Core construction/lifecycle/attachment | neutral_link, configuration, stopped_lifecycle, attachment_phases and memory_reconfigure cover candidate validation, frozen bindings, callback order, independent destruction and failure retention. Attachment deadline/L1 admission now has neutral direct proof. |
| Core memory, ROM, entry and firmware | memory_inspection/device_registration, immutable ROM/aliases, neutral_link and migrated entry_plan cover readonly backing, overlay fallback/removal, A20, freeze guards, preload atomicity and firmware stage/rollback. DMA memory transactions now verify effects, error cancellation and successful reuse without a board fixture. |
| Core ports | neutral_link proves batch conflict rollback, wired-OR, owner removal and failed read/write preserving published state. Byte-bank tests add two-lane read/write, shared tick, last-lane read failure and illegal width/wrap rejection. CPU/Core I/O timing cases retain actual IN/OUT behavior. |
| Core time/clock/trace | timeline moves heap order/nested/cancel/reset locally and adds capacity/stale-token/exhaustion; rational_clock checks phase, split/reset and inverse deadlines. Explicit time, immediate/future/blocked deadlines, bounded L1 and enabled/disabled trace cases have local proof. Board clock/IRQ composition stays IBMPC. |
| PIC/PIT/RTC/DMA/PPI | Existing per-chip contract/command/waveform/register/transfer cases retain literal outputs, gate/timing/reset/bus failure boundaries. PIC snapshots are now direct; DMA release is observed through terminal, failure, reset and active-channel results, not a name-only hit. |
| Keyboard/KBC/mouse | keyboard command/parameter/repeat matrices, XT keyboard, mouse and controller contracts own replies, signals and transport. KBC pins, response polls, serial delays, capacity and copied batches now have direct local proof. Native host input belongs to Lib, not these tests. |
| FDC/HDC | FDC contract/record/cause/allocation fixtures sample pins and provider result types, execute command/status/data/terminal boundaries and observe emitted IRQ/DRQ. HDC taskfile/record/allocation and new Xebec DMA/early TC cases own personalities and media outcomes; physical disk/image persistence stays Lib/IBMPC. |
| Video | video allocation/inspection/text/status, CGA/EGA/planar/CECG and CRTC boundary cases retain register, memory, geometry and timing outputs. All five CECG validity conditions now reject atomically. Host frames/presentation are not retested here. |
| xasm/Debug | xasm32/contract/bounds and debug_output/linear/machine transcripts retain lexical, byte, formatting and failure predicates, including real Common paused-lease integration with an x86-owned driver. Native monitor policy stays Common/Product. |

The explicit owner gaps are repaired in existing fixtures; five pure producer/
Core files move rather than fork, and neutral timeline assertions split from
retained board assertions. No unique old assertion, integration checkpoint or
CPU family is removed. Fixture adaptations use equivalent neutral constructor
values and expand the exact original bind/freeze/reset calls at the new owner;
no foreign-test include remains.
This disposition does not claim measured 100-percent branch coverage or newly
complete manual semantics. Full receiving units, gates and actual review still
decide acceptance.

## Final Verification And Review

Current independent Release suites pass 173/173 on x64 (11.94s) and x86
(18.21s), with current changed cases and manifests verified. Complete receiving
units pass 532/532 per width (68.68s x64, 70.81s x86). All twelve relevant
x86/IBMPC manifest, corpus, boundary/Types, ownership and negative probes pass
per width. Three original decoder JSON files have identical SHA-256 across
both standalone and both historical receiving paths; embedded count predicates
also pass. A final EOF-only cleanup changes no assertion or executable code.

Sequential executor/coordinator actual-diff review confirms the scope map,
five tracked relocations, unchanged decoder bytes, retained old predicates,
neutral constructors/fixtures, sole source targets and absence of production,
App/INI/media or public API change. No remaining owner-local gap is hidden by
a moved test; existing CPU manual repairs retain their original queued owner.
Full-field hardware qualification and external checkpoints remain S7/Queue
requirements, not claims of this test-owner batch.

Tracked Shared code/build: 19 paths, +712/-107 (net +605), measured by staged
Git numstat for C/H/CMake, excluding README/manifests/generated/artifacts. Most
growth is direct missing boundary/failure proof in reused cases; only the
split neutral timeline adds an executable. There is no new framework or
duplicate production path. NXVM receiving build diff is four source-path
replacements (+4/-4). Test-only inputs require no EXE rebuild; 0545/0043 remain.
Documentation governance, source/test identities and staged diff checks pass.
Deliver Shared test changes, then NXVM receiving/evidence changes, then the
normal NXVM coordinator acceptance P. S5 closes only with that acceptance;
T stays open for S6 and S7.
