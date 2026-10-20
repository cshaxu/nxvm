# T3 S22: COP Mode-Banked DMA And Stable Sort

## Boundary And Model

S22 consumes the complete mode-banked DMA and sort receiver frozen by the
[S20 residual contract](t3-s20-residual-contract.md).  It changes only
MyRaidenII Core and its existing Board unit.  No Shared component, public
interface, worker, queue, host API, ROM, or reference implementation is
imported or changed.

The COP now owns exactly one 512-entry setup bank.  The existing raw 16-bit
mode register at `47e` remains the sole selector.  `478`, `47a`, and `47c`
byte-merge only the selected bank's source, size, and destination fields;
`476` remains scalar.  An out-of-range raw mode is rejected before any bank
access.  Reset clears every bank.

The pre-existing one pending command is replaced, rather than supplemented,
by one discriminated pending operation: command, DMA, or sort.  It prevents
configuration changes until the existing safe-point advances it and is
cleared after either success or failure, or reset.  All data movement uses the
pre-existing checked COP-to-Core DMA cycle helper.

Modes `009` and `00e` use the approved nominal L2 units:
`source * 64`, `destination * 64`, and
`((size + 1) * 32) - destination` bytes.  A negative range is rejected before
subtraction.  The transfer is forward, two bytes at a time, retaining the
natural overlap order of the established Core DMA route.  Mode `116` repeats
the existing `428/42a` dword for `(size + 1) * 16` bytes at `source * 64`.
Modes `118--11f` use 32-byte units and explicitly reject a nonzero destination.

The `6fe` sort trigger copies all lookup offsets and signed 32-bit keys before
publishing output.  Parameter 1 is stable ascending, parameter 2 stable
descending, and other parameters preserve the original table ordering while
still completing the read/write path.  Temporary merge-sort storage lives only
for the safe-point operation.  This is a reference-derived L2 functional
model, not a physical COP timing claim.

Video mode `14`, palette mode `15`, brightness modes `80--87`, unresolved
macros, and real-set startup remain outside S22 and retain their named T3
receivers.

## Existing-Test Reuse And Proof

The existing `myraidenii.core/board-bus-smoke` executable remains the sole
receiver.  It already creates the actual V30/Core/COP attachment and routes
all MMIO through the real Board memory path.  S22 extends that fixture rather
than adding a second device harness or a per-mode test executable.

The added matrix proves selected-bank byte writes, bank isolation, invalid
raw-mode refusal, pending-operation admission, zero-length transfer, invalid
range preservation, forward overlap, both fill unit classes, nonzero long-fill
destination rejection, signed sort direction, stable equal keys, unknown sort
parameter ordering, and reset cancellation.  A 20-bit Core address wrap is
valid hardware behavior, not a fabricated DMA fault injection; the test does
not mislabel it as an error path.

## Archived Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | MyRaidenII M1 T3 S22; sequential coordinator/executor |
| Admission And Approval | Owner delegated ordinary same-T planning; S20 freezes this complete local receiver |
| Objective | Complete one mode-bank setup owner, one pending-operation owner, memory copy/fill modes, and lookup-table sorting |
| Non-goals | Shared changes; Video/palette/brightness DMA; residual macros; ROM startup; rendering; sound; host threads; generic frameworks |
| Reference Baseline | Clean pushed `930075de1`, S20 residual contract, and S21 RNG receiver |
| Candidate Proposal | `proposals/m1-board-cop.md`, dependency receiver 2 |
| Files And ABI Surface | `src/myraidenii/core/cop.c`, existing `test/myraidenii/unit/core/board_bus_smoke.c`, and MyRaidenII records; no public ABI |
| Applicable Rules | MyRaidenII guide/source policy/architecture/coding plus shared execution, architecture, coding, document, and contributing rules |
| Verification | Cached dual-width builds; actual Board unit; whole-repository dual-width units; protected-input guards; documentation and actual-diff review |
| Expected Markers | 512-bank isolation, full raw bounds, byte lanes, one pending operation, copy/fill/sort success, invalid-range/reset/overlap/stability proof |
| Asset Needs | No ROM/media import or published-artifact change; preserve protected inputs and existing releases |
| Reporting Requirements | Exact source/test delta, retained owners, L2 limits, verification, and remaining receivers |
| Stop Conditions | Shared/API need, source-policy conflict, protected-input mutation, or evidence contradicting the frozen contract |
| Exit Criteria | Complete bounded receiver, dual-width proof, coordinator actual-diff review, pushed delivery, and clean worktree |
| Original Owner Request | Continue MyRaidenII under governing rules with cohesive batches and no per-mode patch accumulation |
| Similar-Issue Sweep | All banks, setup byte lanes, busy/reset, copy/fill units, sort ties/directions/unknown parameter, and raw-mode bounds |

## Qualification And Limits

Focused x64/x86 Board-unit runs and the full MyRaidenII label both pass before
the whole-repository qualification.  Final complete units pass 687/687 with
no failures or skips per width: x64 in 108.13 seconds and x86 in 121.22
seconds.  The aggregate ran sequentially with the existing four-worker,
600-second-budget policy; neither architecture was restarted.

Final native `LastTest.log` SHA-256:

| Width | SHA-256 |
| --- | --- |
| x64 | `DEA0E43A62E8AE97B778DE0874D96283D33E921C9EB78EAD7276743187B9F2C5` |
| x86 | `FF176D5BAA472DD0BE4455D34D37D35EFBEB5052472377500F82DD19BAD547F4` |

Actual source delta is +286/-45 in `core/cop.c`; actual existing-unit delta
is +167/-2 in `board_bus_smoke.c`.  The source expansion replaces the prior
two-field pending state with the one discriminated owner and adds the complete
approved receiver; it does not introduce a second path.  The test expansion
extends the existing real Board executable rather than creating a duplicate
fixture or executable.  Frozen source and test SHA-256 respectively are
`6D0695B2BB8FFE822E755AC48B186CBFDA8F2053E46F05821D64504E8A243FA1` and
`773706DA431E6F6D98F6DC262AEC43E992E09BDF455E0159785F7A1EF29680D0`.

The protected release/media guard was rerun after restoring the two unrelated
MyNES EXEs rewritten by the full build; all guarded assets and media now match
their frozen hashes.  MyRaidenII has no frontend or published EXE yet, so no
new binary is released by this Core-only S.  No desktop, RDP, external
integration, or playable-game result is claimed by this local receiver.
