# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S7 | Active: converge all five CPU manual/implementation audit batches and actual timing-source coverage. Shared implementation remains read-only. |

## Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T544 S7, next numeric S after accepted S6. |
| Admission And Approval | Owner automatic sequential S authorization and 2026-10-05 instruction to complete T544 CPU original-manual/function/timing audit before other work. NXVM audit documentation and read-only Shared inspection only. |
| Objective | Converge S1-S6 across 8086, 8088, 80186, 80286 and 80386: original-source conflicts, actual successful/faulted time allocation, all retained mechanism gaps, regression ownership and concrete coherent repair batches. |
| Non-goals | No Shared source/test/API repair without concrete owner review; no new CPU, board clock, Lib/Common/MyNES/INI/media change, green-test completeness claim or transfer of residual CPU gaps outside T544. |
| Reference Baseline | 8295ff789; accepted S1-S6 finite inventories, retained List 1 and identified original manuals. |
| Candidate Proposal | [T544 proposal](../proposals/m5-retained-cpu-qualification.md); converge every batch in the [ledger](../history/M5-T544-retained-cpu-qualification.md). |
| Files And ABI Surface | NXVM Current, T544 ledger and S7 convergence evidence; read-only x86/chips/cpu, x86/core, CPU/composition fixtures and original manuals. Source/test/API changes zero. |
| Applicable Rules | Task Reading Set, Execution/Documentation/source policy, Architecture/Coding single-owner and admission-to-commit invariants; inspect original PDF pages, not OCR alone, and preserve original handlers. |
| Verification | Reconcile every S2-S6 retained class and uncertain/source-conflict result; trace effective mode/width and timing producer precedence; full unit once per width and original integration aggregate once per group at final audit delivery; document checks and actual-change review. |
| Expected Markers | Exact/formula L3, range/model L2 and order-only L1 remain distinct; proven defects, source conflicts, undefined outcomes and missing predicates are not conflated. Each retained class has actual owner, affected variants, repair design and regression receiver. |
| Asset Needs | Read-only original manuals and existing external integration assets; ignored research scratch/default build caches retained for this batch. No import or redistribution. |
| Reporting Requirements | Complete whole-audit convergence and explain actionable defects and source/tier limitations; report failed integration without changing predicates or chasing unrelated board issues; do not close T544 as qualified. |
| Stop Conditions | Unavailable source or unupgradable L1/false higher label is reported; Shared implementation waits for concrete review, while safe audit continues. No other queue task may execute. |
| Exit Criteria | All five finite family audits and every retained receiver converge to a precise disposition with repair/regression owners; actual-source coverage and source conflict matrix complete, required baseline verification recorded, full P delivery and coordinator actual-change acceptance. |
| Original Owner Request | Complete T544 CPU original-manual versus actual instruction/function/timing audit, without prematurely ending or moving to another task. |
| Similar-Issue Sweep | Compare all families and mode/width/reference/failure variants of FLAGS, address/fetch, arithmetic, segment/gate/task, exception/debug, strings/ports/NPX/bus and timing/retirement; no first-failure patching or silent disposition change. |

## Accepted Audit Baseline

Coordinator actual-change review accepts executor S6 P1 `1fc27c7cd` as a
complete read-only inventory. The [80386 audit](../etc/evidence/t544-s6-80386-family-audit.md)
dispositions all fifteen partitions, retaining source conflicts and exact
implementation/regression receivers. Fresh units pass once each width:
x64 506/506 in 68.14s; x86 506/506 in 83.88s. Shared, App and artifacts
remain unchanged. No active packet is retained between accepted subtasks.

The [proposal](../proposals/m5-retained-cpu-qualification.md) owns scope;
the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md)
retains batch dispositions and acceptance evidence.

S1-S6 inventories are accepted, not repaired or qualified CPU behavior.
The latest [80386 audit](../etc/evidence/t544-s6-80386-family-audit.md) and
earlier family evidence retain all mechanism/source conflicts inside T544.
Coordinator acceptance covers read-only inventory, not Shared edits or CPU
qualification. Docs-only work needs no EXE rebuild; concrete Shared repairs
still require owner review before implementation.

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

T544 stays open during S7 cross-family convergence; no Shared repair is approved
by audit acceptance. The remaining candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
