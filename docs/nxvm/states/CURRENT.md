# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S2 | Audit delivery: finite cross-family boundary inventory and four coherent repair proposals; awaiting coordinator actual-change review. |

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
Shared repairs have not been admitted or implemented; S2 is not closed.

### Active S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T544 S2, after accepted S1 at 7393eabaf. |
| Admission And Approval | Owner admitted T544 and automatic sequential S admission; current CPU-audit continuation, 2026-10-05. NXVM-owned audit evidence only; Shared source/test edits require concrete owner approval before implementation. |
| Objective | Trace all five CPUs through the finite cross-family reset, metadata/prefix, FLAGS, interrupt inhibition/delivery, exception restart and successful-time publication contracts; identify actual defects and exact missing proof batches. |
| Non-goals | No instruction-family qualification shortcut; no 80188/486 implementation, Lib/Common/MyNES edit, INI/media change, new API, timing downgrade or source import. |
| Reference Baseline | 7393eabaf; S1 five-CPU inventory and two fresh 506/506 unit suites. All eight 0543 PC artifacts unchanged. |
| Candidate Proposal | [T544 proposal](../proposals/m5-retained-cpu-qualification.md); [ledger](../history/M5-T544-retained-cpu-qualification.md), S2 boundary batch. |
| Files And ABI Surface | Read x86/chips/cpu, x86/core execution/retirement, CPU/Core regressions and PC composition; write NXVM Current, task ledger and S2 evidence only. No public ABI or executable input change. |
| Applicable Rules | Execution finite-batch convergence, full unit S gate and actual-change review; Documentation sole Current; architecture/coding skills, project Architecture/Coding one-owner and table-style constraints; source policy and PDF skill for read-only original manuals, OCR navigation plus rendered-page proof. |
| Verification | Direct source/caller/test review for every S2 row; original manual hash and selected page review; complete units once per x64/x86 with RunTestAggregate.ps1, jobs 4, 300-second deadline each, sequential native runs; documentation governance, links and diff check. |
| Expected Markers | Every S2 boundary row has source/callers/regression, scope-limited disposition and repair receiver; observed defects distinguished from unverified hypotheses; no unsupported whole-CPU claim. |
| Asset Needs | Existing original manuals located under external manuals-nxvm/cpu and matched against recorded SHA-256. No asset master write. Ignored build/t544-s2-research holds disposable extraction/render outputs. |
| Reporting Requirements | Confirm scope; report concrete findings with code/manual/test evidence and minimal owner-local repair proposal. Report final audit batch and known unresolved work, not only green tests. |
| Stop Conditions | Do not mutate Shared without concrete approval; report new unupgradable L1 or grade correction; absent/uncertain source stays unqualified; preserve unrelated work and all existing artifacts/configuration. |
| Exit Criteria | Complete S2 boundary inventory reconciled with exact findings/receivers, mandatory verification and actual-diff review; no found defect hidden behind baseline pass. T remains open. |
| Original Owner Request | Continue full CPU audit under T544; preserve five CPU implementations and manual L3/model L2 distinctions, correct shared mechanisms rather than profile workarounds. |
| Similar-Issue Sweep | All five CPU identities, all reset callers and FLAGS image/load callers; prefix admission, interrupt inhibit/NMI/HLT transitions; all fault-versus-retirement and timing publication paths. Named family-specific semantic contexts stay pending S3-S6. |

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
