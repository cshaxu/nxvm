# T3 S15: Complete COP-Owned Register Decode

## Baseline, Universe And Source Grade

Baseline50b7cad23. This reconciles B02/B07/B08's COP-owned physical register
inventory, not all algorithms or the entire low control window. Fixed research
commit c1425a1af00b88aa07477b1b23a30ed5e5f856ff supplies selected-board map facts:
raiden2.cpp SHA-256
FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A;
seibucop.cpp SHA-256
7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9.
These are research-only L2 facts, not an imported implementation/map table,
schematic or physical bus timing claim. Independent grouped ranges below
identify every selected-map handler owned by COP, not every address inside
a function whose name contains COP.

| Physical bytes | One owner/capability after this batch |
| --- | --- |
| 41c--41f | COP angle target/step, explicitly unmodelled; refuse both directions until the numeric receiver. |
| 420--423 | Existing decimal input/publication. |
| 424--425 | Existing decimal mode. |
| 428--42d | Existing raw setup transport for DMA values and RNG limit; algorithms remain pending. |
| 432--43d | Existing upload address/data/metadata and collision hit-base configuration. |
| 444--445 | Existing scale write transport; numeric interpretation pending. |
| 450--45d | COP sort-address/lookup/parameter and palette-brightness configuration, explicitly unmodelled. |
| 476--47d | Existing raw DMA relative/source/size/destination transport, not qualified per-mode DMA execution. |
| 47e--47f | Raw16-bit DMA mode, merged writes and non-mutating stored readback; no clamp or invented readiness bits. |
| 4a0--4ad,4c0--4cd | Existing seven high/low address words. |
| 500--505 | Existing complete-upload/macro admission and sole safe-point receiver. |
| 580--589 | Existing collision results/alias. |
| 590--599 | Existing decimal results. |
| 5a0--5a7 | COP RNG results, explicitly unmodelled; not zero-filled RAM or a guessed PRNG. |
| 5b0--5b5 | COP numeric results, explicitly unmodelled until the numeric receiver. |
| 6fc--6ff | COP DMA/sort triggers, explicitly unmodelled; do not accept a trigger as a no-op. |

Five missing groups total36 bytes. They extend the one existing descriptor
list with one private unmodelled role, not new storage, a second decoder,
objects or a framework. Query claims those devices; both data directions
fail with existing INVALID_STATE, not UNSUPPORTED which would invite ordinary
RAM fallback. In particular6fc lies outside current register storage: refusal
must happen before indexing or mutation. Reset retains decode and no pending
algorithm/state is fabricated. This is explicit missing-capability handling,
not a claim that physical hardware faults or returns this software status.

The complete fixed source inventory contains40 COP map clauses and126 unique
physical bytes, represented by17 local grouped ranges after this batch.
Read-only counting confirms the pinned source hash, not a new source parser,
map generator or imported dispatch table.

The actual mode handler merges a full16-bit word and returns that same word.
Split mode from the neighbouring write-only configuration in the descriptor
list. Rename the private GPR-specific readable role to a plain read/write role;
reuse the existing stored-byte reader for GPR and mode. No forwarding callback,
duplicated mode value, public function or special product condition is needed.
Per-mode source/size/destination contexts remain a B08 implementation receiver;
raw setup storage and mode readback do not qualify that larger mechanism.

## Distinct Owners And Remaining B02 Receiver

470h tile-bank behavior,600h--63fh CRTC,68eh sprite latch,6a0h--6dfh sprite/
bank controls,762h protection output,700h sound and740h/744h/74ch inputs are
Board/Video/Sound owners, not COP merely because the inherited map groups them
near COP calls. Keep6cbh's existing program-bank owner unchanged. Other board
variants'440h/442h comments do not add selected-board COP registers.

The current neutral Machine backing also leaves undeclared low-control holes
as RAM. S15 does not claim to fix the entire400h--7ffh non-RAM window or choose
hardware open-bus bytes. Mandatory T3 B02 receiving work must qualify and own
that whole window, preserving real devices and honest unsupported behavior,
before genuine selected-set startup can qualify. It will also reconcile the
old synthetic failure fixtures which currently use those holes as writable
prefix storage. Do not silently erase this remaining requirement because the
known COP register list is now complete.

## Before-Code Verification And Estimate

Extend the existing Board route test once with the five declared groups:
every byte and every within-range two-byte lane must query as a provider and
reject operational read, observation and write. Repeat decode proof after
reset. An actual guest DMA-trigger store must fault rather than continue.
Prove mode word/byte/upper-bit/inspection/reset/pending behavior and retain
all previous transfer/update/collision/decimal/IRQ/video assertions.

The new41c--41f guard would cause two old0205 increment-failure seeds and one
coordinate-failure seed to fail early during setup. Relocate them, preserving
each exact stage: first increment word failure at476h uses base466h; second
increment word failure at476h uses base464h; coordinate read failure at47ch
uses base45eh after the position write has committed. All earlier operands
remain readable RAM in the present declared fixture; other rows are unchanged.
No fake provider, special test-only production entry or public getter is added.

Focused verification additionally exposes irrelevant fixture setup: two0905
failure rows write an auxiliary coordinate which0905 never reads and which
they never assert. With the newly accurate guard, that old setup write fails
before the intended increment test. Seed coordinates only for0205 or an actual
coordinate-preservation assertion; retain all case rows/expected effects.
This is a setup correction, not deleting an independent behavior check.

Negative-capability ranges also claim complete within-range spans, not only
CPU lane widths. Other device roles retain their original1/2-byte data
contract. The one range helper uses bounded subtraction before comparing
length, avoiding a wide-length addition overflow. Full-span and mixed-prefix
data tests must reject without a RAM bypass; a mixed write retains its already
accepted preceding RAM byte, not an invented rollback.

## Separate Shared Query Finding And Probe Lifecycle

A bounded ROM-free probe using only actual Board/Core public APIs confirms
query4c0h/2=>PROVIDER, query4c0h/4=>ORDINARY_RAM, while read4c0h/4 returns
34120000 after a GPR write. This is the general complete-span classification
gap, not a data-read fallback: Core's actual read/write paths scan narrower
providers before using RAM, whereas the public query does not. Entry preflight
can rely on that classification. It is recorded in the product
[TODO](../../states/TODO.md#medium-shared-complete-span-memory-query-classification)
for explicit Shared review; no Shared source/API is changed or workaround
added to the App. Current COP DMA cycles remain1/2 bytes.

The owned build/myraidenii-t3-s15 probe contains only these generated files:
query_probe.c1447 bytes, SHA-256
82AA2AA071CA246ECCE846D63342CE61CDFE507CA1DA13EE17838C0F6329F9CD;
query_probe.exe677178 bytes, SHA-256
2C061DDF30749A8977ECE0B37444F4509D0A0979F5C95FA058A7630A21DF651A.
Strict C11 compilation and the probe finish successfully; it has no background
worker, protected input, ROM or retained copied reference source. The probe
asserts the current discrepancy only diagnostically, not as a committed
regression requiring incorrect behavior. Archive these neutral facts before
preview/apply cleanup; never keep the executable as a release or test fixture.

## Implementation, Scope And Cleanup Review

All17 grouped ranges now come from the sole existing COP descriptor list.
The five new groups share one private unmodelled role. Read refusal uses the
existing terminal path; write refusal occurs before register indexing and
cannot mutate the byte store even for6fc, beyond its allocated extent.
There are no new functions, objects, mutable state, public APIs or alternate
dispatch paths. GPR/mode readback shares the renamed plain read/write role;
the old GPR-specific role name and duplicate interpretation do not survive.

Negative ranges claim their full span, while real data roles retain their
original lane-width contract. Range containment uses subtraction bounded by
the actual descriptor rather than potentially overflowing length addition.
Every byte and within-range word lane is checked before/after reset; complete
spans and mixed RAM-prefix writes are also checked. The actual V30 trigger
store faults through the real Board/Core route. Mode word/byte/high-bit,
inspection, reset and pending behavior are covered in the same existing unit.
No mock COP, per-register executable or test framework is introduced.

The only old-vector changes are the three declared0205 relocations and
removal of irrelevant0905 coordinate setup. The initial focused run exposed
the latter as a fixture setup failure in rows18/19, not a runtime regression;
after correction all rows and independent accepted-prefix assertions remain.
This distinction prevents a green-but-early-failure coverage downgrade.

Actual source/test delta against50b7cad23 by git diff --numstat:
production21 added/10 removed/net11; existing test92 added/7 removed/net85.
The test removes fewer lines than estimated because all independent old
assertions remain. The positive source adds the missing physical decode and
capability distinction rather than a guessed algorithm. Public headers,
Shared, other Apps, CMake, manifests and artifacts are unchanged. This local
source/test package has no manifest to regenerate.

The probe's two terminal processes finished before preview/apply cleanup.
The exact verified build/myraidenii-t3-s15 root contained only two owned
generated files,678625 bytes. Default preview deleted nothing; Apply with
TasksExited and RequireClean then removed that root. Neither persistent cache,
any Git data, configuration, media nor another task was removed. Build now
contains only .ccache/.ninja; necessary neutral results/hashes are retained.

Frozen production SHA-256:
CB744B64D4787D51B93619EB573F9EDA90250372E66FD3C2F1B6E2A46485DDC6;
frozen test SHA-256:
67D146AC9544173E4CB74B962094F93937AFF7277BC6A5ADFE6FADE7A56EAEA7.

Estimated source+20--40/-8--20; test+70--120/-10--25; records+120--190.
Both original Ninja/cache graphs run strict affected builds and all687 unit
tests sequentially, four workers,600-second aggregate containment. Record
actual hashes/delta, docs/whitespace, unchanged ten releases/protected inputs
and terminal owned work. No ROM run or playable-game qualification is claimed.

## Final Qualification

Both original cached Ninja graphs complete the strict affected builds. Final
full units pass687/687 per width, with zero failures/skips: x64 119.11s,
x86 126.84s. The final source/test identities above remain unchanged.
LastTest.log SHA-256, x64:
2E24EDF10567146217E56FAF18C7C611F764A160D3CC07EBB76176B8D02D7742;
x86:
76CED778DED9FEBBA5FB4A80D783D467702D33F53FBCD9F5263227BED12ED0F6.

Post-test guards match all10 release identities,33 tracked assets and6
protected media-role records against accepted S10/S49 evidence. There is no
MyRaidenII release yet and this local COP unit is not linked into those ten
products; no unaffected binary rebuild or timestamp refresh is warranted.
No Shared, other App, external media, INI, CMake or manifest changes occur.
The separate Shared full-span query issue remains an explicit TODO, not fixed
or hidden by this batch. Geometry/math approval and the original remaining
numeric,2a05,DMA,low-control-window/startup receivers remain open.

Desktop, RDP, real-ROM startup, playable game and full integration qualification
are not executed or claimed. Full unit success does not close T3 or M1.

## Archived S15 Packet

Coordinator acceptance follows actual git show review of P1 fb5e14f7b:
all six changed files match the scoped receiver. The decoder has one owner;
read/write refusal precedes storage access, mode uses existing storage, and
all retained failure vectors reach the declared stages. Tests reuse the real
Board/Core route, not a new production path. No algorithm completion, full
integration or game qualification is inferred from the unit results.
P1 totals387 additions/18 removals/net369: production21/10, test92/7 and
records274/1. Documentation exceeds the initial120--190 estimate because it
archives the complete16-field packet, fixed inventory and newly discovered
Shared query evidence/receiving obligation; executable scope did not expand.
This governance acceptance changes no source, tests, binaries or inputs.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S15 after accepted S14 50b7cad23; single-session sequential coordinator/executor roles. |
| Admission And Approval | Owner's standing complete MyRaidenII implementation approval and delegated same-T ordering. MyRaidenII Core/test/records only; no Shared math or other public capability approval is inferred. |
| Objective | Reconcile the complete selected-board COP-owned register inventory against actual installed routes. Claim every known missing COP range and reject unmodelled reads/writes explicitly instead of RAM success. Qualify the existing DMA mode's raw16-bit readback through the same stored-word owner. |
| Non-goals | No Shared/API/source import, geometry/PRNG/DMA/sort algorithm, open-bus guess, blanket COP ownership of Video/Board controls, full low-control-window fence or gameplay qualification. These mechanisms retain mandatory original receivers. |
| Reference Baseline | Clean pushed 50b7cad23; actual current Board and COP decoder and accepted S10--S14 paths. Fixed MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff map/ownership/readback facts only. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s1-board-contract.md B02/B07/B08; S12 missing numeric routes and S14 alias/failure-stage review. |
| Files And ABI Surface | Existing cop.c, existing Board unit, Current/evidence/index and directly discovered Shared query-classification debt in this product's TODO. Private range role only, no public header or state/storage expansion. Estimate production+20--40/-8--20; test+70--120/-10--25; documentation+120--190. Refine before material scope increase. |
| Applicable Rules | Execution full finite-family disposition/actual review/P lifecycle and failure-stage sweep; Architecture COP-only ownership and one descriptor/route/read-write policy, no RAM fallback for a known unmodelled device; Coding reuse stored readback/private role and existing Board fixture, no generic decoder manager; source policy facts not reference implementation/table; Document explicit remaining Board/Video/control-hole boundaries and model grade. |
| Verification | All bytes, within-range word lanes and full spans of missing41ch--41fh,450h--45dh,5a0h--5a7h,5b0h--5b5h,6fch--6ffh query as provider and reject operational/observational reads and writes; mixed RAM-prefix transfers also cannot bypass refusal. Preserve normal byte/word device widths while unmodelled negative-capability ranges claim their full span. Actual guest DMA-trigger write faults, never no-ops. Existing47eh/47fh word/byte/full-bit/read/inspect/reset/busy behavior is proved. Retarget three old state-update failure vectors to preserve their exact read stages. Both stable graphs build affected targets and run all687 units sequentially with four workers and600-second aggregate containment. Check docs/whitespace, artifact/input hashes and cleanup. |
| Expected Markers | Five missing groups use one private unmodelled role in the sole COP descriptor list. Writes reject before indexing storage, including6fch outside the current byte store. GPR and DMA mode share plain stored read/write behavior. The guard is not an implemented algorithm or a claim of hardware open-bus behavior. |
| Asset Needs | ROM-free tests and read-only fixed research responses. Owned build/myraidenii-t3-s15 only if necessary,32MiB/5-minute no-progress/60-minute phase budgets. No ROM or EXE needed; preserve ten current releases and protected inputs. |
| Reporting Requirements | Before code report actual missing routes, owner distinctions and estimate. Preserve complete inventory/dispositions, precise failure-stage addresses, actual source/test delta and dual-width results/hashes; complete scoped P push, actual coordinator review and governance P. |
| Stop Conditions | Shared/public capability, protected input, licensed reference body/table, per-game bypass, invented timing/open-bus value, or widening into unreviewed Board/Video algorithms. Keep full low-control decode gap as an explicit T3 B02 receiver, not an implicit completion claim. |
| Exit Criteria | Every COP-owned address in the fixed selected-board map has an explicit route and capability disposition; new guards, existing paths and raw mode readback have actual-bus proof. Both final unit sets and scope/quality/cleanup checks pass and complete P delivery/review is pushed. S15 closes only this receiver, not T3/M1. |
| Original Owner Request | Implement complete MyRaidenII using single-session dual-role governance, coherent batches and existing test reuse, while Shared changes require explicit review. |
| Similar-Issue Sweep | Separate the inherited map's COP, Video and Board handlers by actual owner, not its function name. Audit every new role before register-store indexing, all read/write/inspection lanes and reset. Relocate old position/increment/coordinate failure vectors only where their accepted prefix changes; retain every independent assertion. Coordinate fixture writes are required only for0205 or an explicit coordinate-preservation assertion; do not write an unrelated0905 field into newly guarded device space during setup. |
