# T3 S20: Residual COP, RNG And DMA Contract Reconciliation

## Baseline And Research Boundary

Baseline b2f8b9311 accepts S19. Production currently implements18 of S9's22
initial uploaded IDs: three transfers, two updates, nine numeric operations
and four collision operations. The remaining four and the entire S15 mapped
RNG/DMA/sort capability domain are reconciled here, not selected by another
first-failing instruction. Upload alone still does not prove execution or
the complete playable game's operation universe. This S changes records only.

Read-only fixed MAME revision c1425a1af00b88aa07477b1b23a30ed5e5f856ff:

| Unit | Fresh SHA-256 |
| --- | --- |
| raiden2.cpp | FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A |
| seibucop.cpp | 7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9 |
| seibucop.h | 69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962 |
| seibucop_dma.ipp | 71C21FDCF6D35611F665416C8D02952DBCCA7C54366632817A8819921397EC04 |

The previously pinned command unit B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC
is also read at that revision for the four residual IDs. These are LGPL
research facts, not imported/transliterated bodies, tables, code dependencies
or physical proof. Responses exist only in process memory; no payload or ROM
is downloaded to the worktree. Source comments and hacks are not specifications.

## Complete Residual Capability Ledger

| Capability / selected map | Present implementation | Factual candidate contract and mandatory receiver |
| --- | --- | --- |
| 2a05 | Explicit command refusal | For slot i, read unsigned word delta from B1+1eh+4i; read word at B0+6+4i and write its sum with delta as a dword there; then independently read word B0+1eh+4i and write its sum as a dword there. Both write widths and the second output matter. Reconcile as a declared reference-derived L2 choice or obtain stronger evidence; do not silently narrow writes, sign-extend delta or call this hardware-exact. Receiver must prove both outputs, carry/overlap and every accepted prefix. |
| 1905 | Explicit command refusal | Reference contains an upload/comment entry but no executing handler. No-op success is not established. Its semantic-reconciliation receiver retains non-eligibility until actual selected-set use and an independent model/evidence are available. |
| 9100/9900 | Explicit command refusal | Reference comments suggest alternate sine/cosine and different output offsets, but its actual dispatch does not implement either. Comments about unused routines are not proof that the selected game's paths are unreachable. Preserve both IDs and their reconciliation receiver, not redirects to8100/8900 or guessed results. |
| RNG limit42c--42d, results5a0--5a7 | Limit raw writes stored; results refused | Four result-word aliases, candidate value from guest time modulo(limit+1); widen limit before addition so ffff means65536, not zero. Selected map makes the limit write-only despite an unused reference getter. Reference says RNG was verified only for68k games: V30 model must be explicit L2. One Core timeline sample per read, no host random source/seed/clock mirror; prove all byte/word/observe aliases, reset, full limit and actual guest reads. |
| DMA mode47e--47f | Full16 raw readback accepted | Mode remains raw16; source/size/destination are mode-selected, not one flat triple. Source declares512 contexts. Bounds-check the full mode before any context access; do not mask invalid values into a valid bank or clamp existing readback. Preserve mode selection and every other bank/reset/lane/busy behavior. |
| DMA setup476--47d, fill428--42b | Flat raw setup only | Relative address476 and fill low/high428/42a are scalar owners. Source478, size47a, destination47c need one512-entry context store and word-byte merging. Retire their flat authoritative path, not retain mirrored bank fields. Unknown-mode trigger behavior is separate from storing a valid context. |
| Memory DMA09/0e, trigger6fc | Refused | Candidate src=source*64, dst=destination*64, end=(size+1)*32, bytes=end-dst. Check end<dst before subtraction; no enormous unsigned-underflow transfer. Zero length is a valid empty range, not an unknown-operation no-op. Forward word-by-word order preserves overlap and failures; each completed prefix is retained.0e is reference-used by other games, not proven selected-set use. |
| Fill116 and118--11f, trigger6fc | Refused | Repeated dword from fill low/high. Address=source*64;116 uses(size+1)*16 bytes,118--11f use(size+1)*32. The latter reference refuses nonzero destination by returning silently; qualification must make invalid setup explicit rather than fabricate completion. Actual DMA reads/writes remain one Core bus owner; mode-specific units stay distinct. |
| Video DMA14, trigger6fc | Refused; Video has raster only | Reference copies0x2800 bytes into private tilemap output, not RAM. It hard-codes sourcecfc0 to d000 and admits odd size/destination may explain it. Do not import that address test. Required receiver freezes real descriptor inputs/general address relationship and Video-owned1400h-word latch, then proves delivery/failure/reset independently of rendering. |
| Palette DMA15, trigger6fc | Refused; Video has no palette latch | Reference copies1000h bytes to a private2048-word palette output and itself questions ignored length. Video owns that latch, COP owns source/bus delivery, Board composes them. Source RAM, accepted palette, future RGB frame and host palette are distinct owners. No duplicate palette in COP or pretend RAM-to-RAM completion. Fixed-size behavior can only be an explicit qualified L2 choice. |
| Brightness DMA80--87; setup45a/45c and relative476 | Refused | Candidate bounded memory transfer with the same range units as09. Mode5 blends RGB555 source/relative target, preserving source bit15 as an exemption; mode4 includes explicitly guessed other-game behavior. Other reference modes pass source words through. Freeze independent channel/rounding/flag oracles and exact selected-mode eligibility before implementation; do not copy the reverse-engineered fade function/table or mistake guessed modes for measured hardware. |
| Sort450--459, trigger6fe | Refused | High/low object-base450/452, lookup-base454/456 and parameter458. Trigger word is entry count. Read lookup u16 offsets and signed32 keys at object-base+offset. Parameter1 ascending,2 descending, stable ties; other reference parameters preserve order but still access/write the table. All inputs precede output publication; failure retains only accepted writes. Bounded temporary scratch is algorithm work, not permanent state or a second queue. |

All21 reference DMA modes are enumerated above (two private-video, eight
brightness, two memory-copy, one short-fill and eight long-fill). No mode,
residual ID or mapped group is removed from the T3 ledger because initial
observation did not reach it. This is a source-grade contract, not21 admitted
algorithms or proof that all other raw mode values have hardware semantics.

## Ownership And Existing Time Capability

Actual local COP still has one copied pending macro and one checked Core DMA
helper, serviced after CPU write commit. Adding triggers must extend this
single bounded operation admission/service owner, not execute nested DMA in
the MMIO write or create a second worker/queue. A private discriminated pending
operation distinguishes macro, DMA and sort with only their actual payload;
replace the old nonzero-command busy predicate, do not add a mirrored busy bit.
One retirement clears the pending operation after success/failure; reset cancels
it. Setup cannot change under pending execution. Temporary sort allocation is
released on every exit, not retained as a hidden prepared-state object.

Per-mode setup requires real independent hardware configuration state. A single
512-context array of source/size/destination (3072 bytes) is not duplicate
storage; the current flat authoritative bytes must stop owning those fields.
Existing raw mode remains the sole selector. Invalid full-mode indexes must
fail before mutation and preserve every valid context. This completes a missing
capability; no current per-mode algorithm was previously qualified by S15.

An initial read-only suspicion of a Shared time gap is resolved by inspecting
all actual interfaces: get_elapsed_ticks and capture_time_observation reject
RUNNING, but existing public get_timeline_observation does not. Its copied now
is read directly from the one Core timeline, without advancing it or borrowing
mutable layout. Use that capability on the execution owner, or while quiescent;
do not treat it as an unsynchronized cross-thread lease. No Shared change,
relaxed lifecycle check, new getter or memory-callback signature is required.
Do not maintain an App elapsed counter, use trace/retirement observers as a
functional clock, or introduce a periodic update cache merely to serve RNG.

The chosen candidate RNG time is timeline.now at actual MMIO dispatch. On the
current macro-proportional16MHz model it is explicitly nominal guest time,
not the reference's literal physical CPU total_cycles or a newly qualified
V30 RNG circuit. No physical clock or randomness-quality claim follows.
Tests must execute a real guest RNG load while RUNNING, not only external
paused reads; also prove repeated observational reads do not move time.

COP should deliver Video DMA through one private sink composed by Board;
Video owns accepted tilemap/palette latches and reset. The existing raster
phase is distinct necessary state, not a full output buffer substitute. No
Lib frame or public Shared ABI change is justified. Rendering later consumes
the same accepted Video state rather than introducing another DMA path.

## Dependency-Ordered Implementation Receivers

These are bounded named receivers, not allocated future S identifiers:

1. Complete RNG using the existing public timeline and complete alias/reset/
   lane/guest proof. Expected local production+20--40/-2--5, existing Board
   tests+80--140/-2--8; no Shared edit or new executable. Keep this independent
   prerequisite moving without waiting for unknown macro semantics.
2. Complete mode-banked setup and the common pending-operation extension with
   general memory DMA/fill and sorting. Expected local production+180--280/
   -25--55, existing tests+180--280/-10--25. Full512-bank isolation, invalid
   mode/lane admission, signed stable sort, zero/negative-size and overlap/
   failure/reset cases consume one owner-local batch, not one mode per S.
3. Qualify Video-owned private DMA latches and palette/brightness receiving
   semantics. First collect bounded real descriptor facts for14's source
   discrepancy; absence of data is not permission to copy its magic address
   patch. Expected production+120--220/-10--30 and tests+140--220/-5--20,
   refined after that contract. Preserve raw RAM and rendering ownership.
4. Reconcile all four initial residual IDs and finite real-set initialization/
   startup checkpoints with the completed paths.2a05 requires both outputs
   and explicit width choice, not the incomplete one-output description;
   the three unimplemented reference IDs remain non-eligible until qualified.
   Bounded observations refine this complete ledger rather than select the
   next single opcode repair. No T3 exit is granted by a budget or later PC.

All code receivers reuse existing checked1/2/4-byte DMA, result/admission,
actual COP fixture and Board executable. No public code entry is added only
for tests. Extend old tables with literal oracles, not per-mode executables,
copied tables, a device framework or a general transaction manager. Each code
receiver freezes actual units/failure order, runs strict cached dual-width
proof and required full units, then actual-change review and scoped delivery.
The figures are estimates, not permission to execute before admission.

## Verification And Limits

Read-only checks directly reconcile the complete selected map, current and
legacy dispatch, new DMA source unit, declared context dimensions, all mode
branches and public Core time/callback implementation. Normalizing legacy
same-looking algorithms is not an implementation plan: the second2a05 write,
two fill units, private video output, stability and RNG sampling distinctions
are preserved. Source-specific guessed/hard-coded forms retain named receivers
and exact blockers instead of quietly inheriting successful no-op behavior.

No production/test/build/API/manifest/artifact changes occur. Frozen S19 COP
3A9AA4F1D2E33474C779C0CA53E21D435B7E7EB5DC09F5A75CF69517BF4D5FB0
and Board test AC169849A1FBEF858A88FB07DFE6D6500E8017184883791F28810B1BBC4BE0BD
retain the actual S19 full unit qualification687/687 per width; no new suite,
RNG execution, DMA output, ROM startup or game pass is claimed here. Source/
test delta is zero. Document/whitespace, exact unchanged-source, ten-release,
33-asset and six-media-role guards precede delivery. No task temp directory,
research payload or owned background work exists; only persistent caches
remain. T3 and the full playable MyRaidenII goal remain open.

Final documentation governance and git diff --check pass. The exact two
frozen code hashes, ten releases,33 tracked guards and six media-role inputs
are verified unchanged. The implementation request therefore needs no Shared
approval for RNG; ordinary local implementation can proceed after admission.

## Archived S20 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S20 after accepted S19 b2f8b9311; sequential coordinator/executor. |
| Admission And Approval | Owner's full implementation goal and delegated same-T planning permit this bounded prerequisite research. MyRaidenII records only; no Shared or production edit is inferred. |
| Objective | Reconcile the complete remaining initial-trigger and mapped COP capability batch: 2a05/1905/9100/9900, RNG, mode-banked DMA/fill/video/brightness, sorting and their Board/Video/time ownership, then freeze cohesive implementation receivers rather than successive first-fault patches. |
| Non-goals | No imported reference body/table, production/test modification, guessed success for unknown macros, raw-ROM inspection/patch, clock mirror, Shared interface change, physical precision or full gameplay qualification. |
| Reference Baseline | Clean pushed b2f8b9311; S1 B07/B08, S9 initial22 IDs, S15 whole map and S19 accepted18 initial IDs. Pinned MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff driver/COP/command/DMA facts; actual current Core time, memory and attachment interfaces. |
| Candidate Proposal | ../proposals/m1-board-cop.md retains all original COP/DMA/startup gates. Research freezes the full residual domain, not another implementation T or reduced completion standard. |
| Files And ABI Surface | Current, one indexed residual contract/evidence record, receiving proposal and necessary task history only. Source/test/ABI/artifact delta zero. Estimate records +130--210/-0--15, refine for exact complete packet/contract proof. |
| Applicable Rules | Architecture governance: single banked-register/safe-point/time/Video state owners and explicit cross-boundary capability gaps. Coding governance: reuse actual transport/fixtures, no forwarding framework or guessed reference special case. Execution: full batch coverage ledger, bounded source research, complete P/actual-change review and no unnecessary rebuild for unchanged code. Source/document rules: fixed hashes, facts only, truthful qualifications and historical records retained. |
| Verification | Fresh fixed-source map/functions/hashes; all four initial residual IDs and all mapped capability groups get dispositions, operands/state/publication/failure context and receivers. Inspect actual running-time query refusal, Core callback lifetime, DMA source anomaly, mode index bounds, sort widths/ties and existing test owners. Verify code/test unchanged, retained S19 dual-width687/687 identities, docs/whitespace, ten release/33 asset/six media-role hashes and build-only caches. No new dynamic pass is claimed. |
| Expected Markers | Exact residual ledger and dependency-ordered batch plans; unresolved source behavior is not a no-op. RNG cannot use an external-only getter or mirrored clock; Video buffers have their real owner; raw16 DMA mode cannot index a512-slot array unchecked. |
| Asset Needs | Reference text in memory only; no ROM, temporary tree, process or payload retained. Build contains only persistent .ninja/.ccache; ten current releases and protected inputs remain untouched. |
| Reporting Requirements | Report actual architectural gaps, exact scope/estimate for any required Shared approval and local follow-on batches, before/after dispositions, source identities, unchanged qualification and complete delivery/review. |
| Stop Conditions | Any production/Shared/API modification, source derivation/import, protected content, invented clock/algorithm/precision or silent transfer/drop of a T3 requirement. Ask only the required bounded Shared decision; continue independent reconciliation. |
| Exit Criteria | Whole residual batch has a reviewable factual/L2/non-eligible contract and concrete receivers, minimal requested interface options and independent proof plans. Product-only complete P and coordinator acceptance pushed, worktree clean. T3 remains open; this design S does not implement RNG/DMA or qualify startup. |
| Original Owner Request | Implement complete playable MyRaidenII under single-session dual-role governance; group coherent work, reuse tests and review Shared changes explicitly. |
| Similar-Issue Sweep | All per-mode setup words, mapped triggers/results, runtime-time consumers and reference hard-coded cases, all four unresolved uploaded IDs versus current dispatch and legacy alternatives; preserve distinct Video/Board/COP owners and no copied algorithms/tables. |
