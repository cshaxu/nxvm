# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S5 | Accepted and closed: 80286 audit delivery 083a8946c; all unresolved source/repair/regression receivers remain open in T544. |

There is no active subtask packet between accepted subtasks.

The [proposal](../proposals/m5-retained-cpu-qualification.md) owns scope;
the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md)
retains batch dispositions and acceptance evidence.

S1-S5 inventories are accepted, not repaired or qualified CPU behavior.
The latest [80286 audit](../etc/evidence/t544-s5-80286-family-audit.md) maps
all ten partitions to source/code/regression evidence and retained receivers.
Source conflicts and S2-S5 mechanism gaps remain inside T544. Coordinator
actual-change review accepts the read-only inventory only, not Shared edits.
Fresh S5 complete units pass once each width: x64 506/506 in 174.97s,
x86 506/506 in 145.33s. Documentation, links, packet and diff checks pass.
Shared source/tests, App configuration, MyNES and artifacts are unchanged;
docs-only work requires no EXE rebuild. Concrete Shared repairs still require
owner review before implementation.

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

T544 remains open after accepted S5. Its next numeric S needs an active packet;
no Shared repair is approved by audit acceptance. The remaining candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
