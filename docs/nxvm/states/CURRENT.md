# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S1 | Active: inventory retained CPU-family qualification and establish its finite convergence ledger and fresh unit baseline. |

T543 is closed at dd9af8951; its history retains the accepted four-App baseline.
T544 consumes the former first Queue candidate, not another structural split.

### Active S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T544 S1, after task-level T543 closure and immutable Git subject reconciliation. |
| Admission And Approval | Owner's current request, "admit and execute the first T task", 2026-10-05; NXVM-owned planning/inventory records admitted. Read Shared source/tests only; no Shared implementation changes until concrete owner review. |
| Objective | Freeze the five implemented CPU models, legal-form/state/timing audit units, source and regression owners, and initial repair batches; collect a fresh complete repository-only unit baseline. |
| Non-goals | No CPU instruction fix, new 80188/486 implementation, board/INI/UX change, MyNES change, Lib/Common edit, sibling import, or claim of complete CPU conformance from tests alone. |
| Reference Baseline | dd9af8951; T543 closed, eight verified 0543 PC artifacts unchanged. Existing ignored receiving build caches retained for incremental qualification. |
| Candidate Proposal | [Retained CPU qualification](../proposals/m5-retained-cpu-qualification.md); [T544 ledger](../history/M5-T544-retained-cpu-qualification.md), S1 inventory batch. |
| Files And ABI Surface | NXVM docs proposal, Current, Queue, Roadmap and task ledger only. Read src/x86/chips/cpu, x86/core, test/x86, test/ibmpc composition, four App profiles, cmake/nxvm and tools/nxvm. No public ABI or runnable input change. |
| Applicable Rules | Execution: one numeric S/packet, complete P, actual-change coordinator review; Documentation: Current sole status and finite ledger; Architecture/Coding skills and project authorities: one CPU/time owner, neutral dependencies and original table style; source policy: external manuals/references read-only. |
| Verification | Verify-CpuTimingManifestContract.ps1; complete unit suite once per x64/x86 receiving baseline via RunTestAggregate.ps1, jobs 4, deadline 300 seconds each, sequential native runs; documentation governance, packet/link review and git diff --check. No integration or EXE rebuild needed for this design-only S. |
| Expected Markers | Five actual CPU models; 4,906 canonical timing keys structurally accounted for; historical planning statuses distinguished from fresh generated results; both complete unit suites exit 0; unresolved semantic/source proof not labelled accepted. |
| Asset Needs | No asset changes. Prior five-CPU original-manual identity/source record used as evidence index, not fresh page verification; manuals stay external. Units use repository inputs only. |
| Reporting Requirements | Confirm scope, report actual inventory and any newly demonstrated gap; final delivery links ledger and pushed P, verification and remaining proof batches. Never infer L1 from a historical planning status. |
| Stop Conditions | Preserve unrelated work; report missing executable tests, failed baseline, new unupgradable L1 or proposed downgrade; no Shared mutation without owner-reviewed repair, no false CPU or physical-time claim. |
| Exit Criteria | Complete inventory batch has finite disposition, exact owners/receivers and initial S plan; required baseline/gates pass; coordinator reviews actual documentation diff and pushes acceptance. T remains open. |
| Original Owner Request | Admit and execute the first queued T; retain all CPU families and correctly classify exact/manual L3, model/range L2 and order-only L1 without unnecessary code or duplicate paths. |
| Similar-Issue Sweep | Reconcile every retained CPU enum/binding, all five manifest expansions, decoder producer/runner locations and prior source/state/timing evidence. Check stale proposal paths; do not modify historical records merely for old path names. |

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
