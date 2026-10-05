# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as a completed CPU audit by owner direction; all CPU repair/proof gaps transferred to the first queued proposal. No implementation task is active. |
| M5 Td S177 | Complete: audit closure, full CPU repair transfer, proposal archive and queue/reference reconciliation. NXVM documentation only. |
| T545 S2 | Accepted: exact eight-corpus import and receiver batch delivered; full units and all ten artifacts verified. S3 external qualification remains; no active packet. |

## Accepted T545 Import

Shared S2 P1 `fc3c73aa1` imports all eight source/test trees exactly from
committed SoftPC `8124e551e841ccdec2ceb7f6a0f6ae5b513a7951`; all Git blob
identities and manifests pass. NXVM-only tests retain 71 mapped receivers;
all original registered assertions and 58 external contexts remain required.
No imported file is locally patched, and owner INI/media/snapshot inputs are
unchanged. Receiver paths, private-layout access and fixture link closure
follow the same owners without a shim or second production path.

Fresh complete units pass 531/531 per width (223.46s x64, 147.44s x86),
MyNES product units pass 43/43 per width, and 23 supplemental gates pass per
width. The x64 specialized aggregate, documentation governance and diff checks
pass. Actual-change review accepts this complete import/receiver batch, not
CPU qualification or fresh external integration. See the
[S2 evidence](../etc/evidence/t545-s2-ibmpc-refresh.md) for provenance, diff
counts, assertions and artifact SHA-256 identities.

Current PC target is `vm-0-5-0545` (0.5.0545), optimized stripped x64/x86
pairs directly in assets/my5160, assets/my5170, assets/mydeskpro386 and
assets/nxvm. MyNES keeps its rebuilt `mynes-0-0-0043` pair (S2 P2
`f010812fd`). All ten PE widths and absence of compiler debug sections
are verified; runtime Debug remains. Superseded PC 0543 EXEs are retained only
in Git history. All owned build/test processes have exited. Receiving caches
and ignored immutable source snapshots remain needed by S3 qualification.

No S is active between accepted subtasks. S3 will run the retained external
integration contexts before T closure; CPU gap repair stays queued.

## Accepted Audit Baseline

Coordinator actual-change review accepts S7 P1 `a3b9951c8` and the complete
[five-family convergence](../etc/evidence/t544-s7-five-family-convergence.md).
All retained instruction/function/timing classes have source, implementation,
gap and regression dispositions; eighteen coherent repair/proof receivers
transfer intact to the queued repair proposal. Fresh full units pass once per
width: x64 506/506 in 260.54s and x86 506/506 in 245.60s. All 58 original integration contexts
pass once, without altered checkpoints. Shared, App and artifacts remain
unchanged. No active packet is retained between accepted subtasks.

The [archived proposal](../history/M5-T544-retained-cpu-qualification-proposal.md)
retains audit scope;
the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md)
retains batch dispositions and acceptance evidence.

S1-S7 inventories are accepted, not repaired or qualified CPU behavior.
The convergence report and earlier family evidence retain all confirmed
mechanism defects, missing contexts and source conflicts. The owner-approved
successor [CPU repair proposal](../proposals/m5-cpu-audit-gap-repair.md) owns
the complete transferred implementation scope; it is queued, not admitted.
Coordinator acceptance covers read-only inventory, not Shared edits or CPU
qualification. Docs-only work needs no EXE rebuild; concrete Shared repairs
still require owner review before implementation.

## Accepted Technical Baseline Before T545 Import

- x86/chips owns independent chips; x86/core owns the neutral execution,
  guest clock and memory engine. Lib/Common/x86 source and tests are unchanged.
- ibmpc/board-common, board-at and board-xt own shared/family PC mechanisms.
  ibmpc/machine owns the sole PC execution/pacing/media/Common adapter;
  ibmpc/product owns one INI, command/Debug/UX, entry and shared version path.
- app-my5160, app-my5170, app-mydeskpro386 and app-nxvm each own their fixed
  composition, firmware/build binding and model assertions. Shared AT
  materialization serves all three AT Apps without merging model definitions.
  Model40 retains its sole D4 owner. No App production graph links a peer App.
- Repository-only family tests/fixtures are under test/ibmpc; App profile
  assertions and integration registration follow the matching App.
  One external family integration harness remains under test/app-nxvm/integration,
  linked to the selected actual binding, not to a production multi-profile path.
- PC-family docs/nxvm, tools/nxvm, version and MTSP stay unified. PC110 is future
  work, not a stub App. MyNES and its 0043 artifacts are unchanged.

## Accepted Runnable Evidence Before T545 Import

All four PC Apps retain optimized stripped 0.5.0543 x64/x86 pairs and their
adjacent owner NXVM.ini directly in assets/my5160, assets/my5170,
assets/mydeskpro386 and assets/nxvm. There is no profile child directory.
Eight PC EXEs plus the unchanged MyNES pair remain; old artifacts are recoverable
from Git history. Rebased media paths retain identical external master files,
access modes and other INI values. Runtime Debug remains present.

Final source deliveries are Shared 4b1948c31 and NXVM c8da42e91.
The ledger records all eight deployed SHA-256 identities and PE widths.
Complete units pass 506/506 per width; all 58 original integration contexts
pass once with unchanged predicates. Both specialized aggregates, 19 supplemental
manifest/corpus/dependency/layout/negative checks per width, documentation
governance and diff checks pass. Actual-change coordinator review accepts the
four complete batches, fixed source graphs and preserved assertions/assets.

No owned build/test process remains active. The existing ignored receiving
configuration caches are retained for incremental verification of the immediate
qualification successor; they are not deployed artifacts or new source paths.

## Next Work

T545 S2 is accepted; S3 external qualification remains before T closure.
The first candidate remains [CPU gap repair](../proposals/m5-cpu-audit-gap-repair.md),
with eighteen mechanism/proof batches and final qualification. Its numeric T
is allocated only on admission; concrete Shared edits require owner review.
T544 is closed as audit, not CPU qualification. Actual-change review accepts
the complete transfer, archived proposal and reconciled references; governance
and link/diff checks pass. No executable inputs changed, so no rebuild is due.
The remaining candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
