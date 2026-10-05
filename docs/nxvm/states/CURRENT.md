# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S2 | Accepted and closed: cross-family boundary audit at be44d5be0; qualification and four Shared repair mechanisms remain open in T544. |

T543 is closed at dd9af8951; its history retains the accepted four-App baseline.
T544 consumes the former first Queue candidate, not another structural split.

S1's complete delivery is
742c31f23; its immutable packet and the [T544 ledger](../history/M5-T544-retained-cpu-qualification.md)
record the inventory, source boundaries and unresolved proof receivers.
Both fresh complete unit suites pass 506/506. No production input changed,
so all eight 0543 PC artifacts and the MyNES pair remain current and untouched.
The [proposal](../proposals/m5-retained-cpu-qualification.md) names the initial
sequence; S2 now consumes the cross-family state/delivery batch.

The [S2 audit](../etc/evidence/t544-s2-cross-family-boundary-audit.md)
records original-page/current-code discrepancies in divide-error return IP,
reset CS limit, SS/debug/NMI arbitration, instruction length and outgoing
FLAGS image, 286 POPF/nested faults and 386 RF fault-frame handling. The
retirement observer ordering is reconciled with its explicit contract and
existing rejection regression, not classified as a defect. Fresh S2 complete
units pass 506/506 per width (x64 60.41s, x86 60.39s), once each.
Remaining generation/source contexts are explicitly pending inside T544,
with their family/repair receivers named in the audit. The inventory is not
whole-CPU qualification. Four coherent repair mechanisms await owner review.
Shared repairs have not been admitted or implemented. S2's immutable packet is
retained in be44d5be0. There is no active packet between accepted subtasks;
the next numeric S needs its own packet. Audit closure is not defect repair or
CPU qualification, and does not remove any unresolved row from T544.

## Current Technical Baseline

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

## Runnable Evidence

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

T544 CPU qualification is admitted; the remaining ordered candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
