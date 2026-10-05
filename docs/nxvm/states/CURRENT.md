# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S4 | Active: 80186 instruction, inherited-state and timing audit; Shared implementation remains read-only pending concrete review. |

## Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T544 S4, next numeric S after accepted S3. |
| Admission And Approval | Owner's CPU audit admission and automatic sequential S authorization, reaffirmed by the active CPU audit goal; read-only Shared inspection and NXVM audit documents only. |
| Objective | Reconcile the 80186 F11 additions and inherited F01-F10/F14 forms, state/delivery differences and exact/range timing against original sources, handlers and existing regressions. |
| Non-goals | No Shared source/test or ABI change, new CPU/80188 alias, board clock, product configuration, MyNES or EXE change; no qualification from green catalogs. |
| Reference Baseline | 946f7a737; accepted S1-S3 inventories, five-family List 1 and owner archive original 1985 manual. |
| Candidate Proposal | [T544 proposal](../proposals/m5-retained-cpu-qualification.md); consume the 80186 batch in the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md). |
| Files And ABI Surface | NXVM Current, task ledger and t544-s4-80186-family-audit.md; read-only CPU/Core and corresponding CPU/composition tests. No ABI change. |
| Applicable Rules | Task Reading Set, shared Execution/Documentation, source/research policy and one decoder/state/timing owner invariants. Visually inspect original PDF pages for source claims. |
| Verification | Complete finite form/context dispositions with direct source/code/regression evidence or named pending receivers; fresh complete repository-only units once per host width at delivery; document/link/packet/diff checks and actual-change review. |
| Expected Markers | Every F11 addition and inherited family has an explicit disposition; numeric/formula L3 and range/model L2 remain distinct; S2/S3 cross-family receivers carried forward. |
| Asset Needs | Read-only manuals-nxvm/cpu originals; ignored build/t544-s2-research render/extraction scratch and retained receiving caches. No asset import/publication. |
| Reporting Requirements | Confirm scope, report substantive discrepancies and coherent repair proposals, then deliver only the complete audit brief with source identities, verification and code/test diff zero. |
| Stop Conditions | Report unavailable authority, unupgradable L1 or false grade corrections; stop Shared edits until concrete approval; continue safe audit. Preserve unrelated changes. |
| Exit Criteria | Complete 80186 audit inventory covering additions and inherited contexts, all unresolved members retained with named receivers; full units and documentation gates; executor delivery followed by coordinator actual-diff acceptance. T qualification stays open. |
| Original Owner Request | Retained CPU audit of function, state and timing; coherent all-family mechanisms, original table-style handlers and no per-first-failure patches. |
| Similar-Issue Sweep | Sweep register/memory, width, immediate sign/count, stack/REP, failure and delivery variants; carry shared arithmetic/decode/retirement/FLAGS/LOCK findings from S2/S3 without assuming identical generation rules. |

The [proposal](../proposals/m5-retained-cpu-qualification.md) owns scope;
the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md)
retains batch dispositions and acceptance evidence.

S1 inventory is accepted at 742c31f23; S2 boundary audit is accepted at
be44d5be0. S3 consumes 8086/8088 F01-F10/F14 as a read-only audit inventory,
not as repaired or qualified CPU behavior. Its
[audit](../etc/evidence/t544-s3-8086-8088-family-audit.md) records all eleven
partitions and thirteen pending mechanism/source/regression batches within
T544. IDIV's numeric source conflict remains unresolved; it is not a proven
silicon-generation rule. No pending receiver was removed or transferred out.

Coordinator review of the actual P1 changes accepts only the admitted audit
delivery. Fresh S3 complete units pass once each width: x64 506/506 in 61.92s,
x86 506/506 in 62.56s. Documentation, links and diff checks pass. Shared
source/tests, App configuration, MyNES and all deployed artifacts are unchanged;
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

T544 remains open after accepted S3. S4 has the active packet above;
no Shared repair is approved by audit acceptance. The remaining candidates stay in
[Queue](QUEUE.md). Common wake-failure and Shared vocabulary follow-ups remain in
[TODO](TODO.md). This structural split does not qualify new hardware or timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine/composition
prerequisites. [T543](../history/M5-T543-four-pc-apps.md) completes the fixed App split.
