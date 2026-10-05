# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S6 | Active: complete 80386DX function/state/timing audit, including inherited forms, widths, paging and VM86. Shared implementation remains read-only. |

## Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T544 S6, next numeric S after accepted S5. |
| Admission And Approval | Owner CPU audit admission and automatic sequential S authorization, reaffirmed 2026-10-05: finish T544 original-manual versus implementation instruction/timing audits before any other task. NXVM documentation and read-only Shared inspection only. |
| Objective | Reconcile the whole 80386DX F01-F14 form/state/time batch: 16/32-bit execution/address/stack, inherited and new instructions, real/protected/paged/VM86 modes, descriptors/tasks/delivery/debug and successful/faulted time. |
| Non-goals | No Shared source/test/ABI edit without concrete review; no Lib/Common/App/INI/MyNES/artifact change, new CPU, board physical clock or qualification from a green catalog. |
| Reference Baseline | a8527a828; accepted S1-S5 inventories, List 1 and retained Intel 386DX 1990 original (230985-003); hardware/original-edition cross-checks only when identified and freshly inspected. |
| Candidate Proposal | [T544 proposal](../proposals/m5-retained-cpu-qualification.md); consume the 80386 batch in the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md), retaining every S2-S5 receiver. |
| Files And ABI Surface | NXVM Current, T544 ledger and t544-s6-80386-family-audit.md; read-only x86 CPU/Core, current CPU/composition fixtures and timing catalogs. Source/test/ABI change zero. |
| Applicable Rules | Task Reading Set, Execution/Documentation/source policy, Architecture/Coding one-owner invariants and PDF original-page verification; preserve original table-style handlers and separate source facts from deductions. |
| Verification | Entire finite batch source/code/test disposition or precise retained receiver; selected current predicates only as baseline observations; complete repository-only units once each width at delivery, governance/links/packet/diff and coordinator actual-change review. |
| Expected Markers | Every family, size/mode/privilege/reference and failure class has an explicit source and implementation disposition. Exact values/formulas L3, ranges/models L2; no falsely qualified success or silent L1/downgrade. |
| Asset Needs | Read-only manuals-nxvm/cpu originals; ignored build/t544-s2-research render/extraction scratch and retained default receiving caches; no asset import or redistribution. |
| Reporting Requirements | Confirm scope, report source/implementation/oracle discrepancies and whole-mechanism repair proposals; complete inventory delivery, original identities/pages and verification, not partial P milestones or T closure. |
| Stop Conditions | Unavailable authority, unupgradable L1 or false higher labels must be reported. Stop Shared edits pending concrete owner review while continuing safe audit; preserve unrelated work and remain inside T544. |
| Exit Criteria | Entire finite 80386 batch mapped to direct proof, non-applicability or exact source/code/regression receivers; full units and document checks, executor complete P push and coordinator actual-diff governance acceptance. T544 qualification remains open. |
| Original Owner Request | Complete CPU original-manual versus actual instruction/function/timing correctness audit across all retained CPUs, without switching tasks or first-failure patching. |
| Similar-Issue Sweep | Pair widths/defaults/prefixes, real/protected/VM86/paging, CPL/RPL/DPL, code/data/stack/gates/tasks, arithmetic/count/bit/repeat/ports/debug/FPU, fetch/operand/delivery/retirement and producer/oracle attribution; retain cross-family receivers. |

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

T544 remains open after accepted S5. S6 has the active packet above;
no Shared repair is approved by audit acceptance. The remaining candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
