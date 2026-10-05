# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 S3 | Executor audit delivery ready for coordinator review; 8086/8088 qualification and concrete Shared repair approval remain open. |

## Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T544 S3; next numeric S after accepted S2. |
| Admission And Approval | Owner's admitted CPU qualification and automatic sequential S continuation, reaffirmed by the active CPU audit goal. This packet admits NXVM-owned audit records and read-only Shared inspection, not Shared code changes. |
| Objective | Reconcile the entire 8086/8088 F01-F10/F14 form, state, delivery and timing batch against original manuals, current handlers and existing regressions; name every unresolved receiver without a completeness claim from green catalogs. |
| Non-goals | No Lib/Common/x86 source or test modification before concrete owner approval; no new CPU, decoder, timing framework, board clock, App configuration, MyNES or executable change. |
| Reference Baseline | d47399525; accepted S2 boundary evidence, five-family List 1, existing original-manual identities and current CPU/test owners. |
| Candidate Proposal | [T544 proposal](../proposals/m5-retained-cpu-qualification.md); consume the early-family batch in the [convergence ledger](../history/M5-T544-retained-cpu-qualification.md). |
| Files And ABI Surface | NXVM Current, task ledger and new t544-s3-8086-8088-family-audit.md evidence; read-only src/x86/chips/cpu, x86/core and corresponding tests/catalog producers. No ABI change. |
| Applicable Rules | Task Reading Set; shared Execution and Documentation; source/research policy; architecture/coding governance for one CPU decoder, state and timing owner. Original PDF pages must be visually checked for source claims. |
| Verification | Original-page/handler/regression reconciliation and complete batch dispositions; fresh full repository-only units once per host width at S delivery; documentation governance, changed-document links, packet shape and git diff --check. |
| Expected Markers | Every F01-F10/F14 family has a named form/context disposition and receiver; source facts, code discrepancies and unproved contexts remain distinct; no fabricated L3, completeness or repair claim. |
| Asset Needs | Read-only owner archive manuals-nxvm/cpu and retained ignored build/t544-s2-research extraction/render scratch; no external assets imported, changed or published. |
| Reporting Requirements | Report substantive source/code findings and coherent repair proposals; full S delivery only after actual-diff review and required verification. Record original-source identities and code/test diff zero. |
| Stop Conditions | Report unupgradable L1, false higher-grade corrections or unavailable authority. Stop Shared edits until concrete approval; continue safe audit work. Do not overwrite unrelated work. |
| Exit Criteria | Complete early-family audit inventory with direct proof where available and explicit named pending source/regression/repair contexts; full-unit and documentation gates; executor delivery then independent coordinator acceptance. T544 stays open until qualification requirements are actually met. |
| Original Owner Request | CPU audit; preserve all retained CPU families, audit function/state/timing against manual authority, repair coherently rather than per first failure. |
| Similar-Issue Sweep | Sweep byte/word/register/memory and both early CPUs for every arithmetic finding; carry reset, FLAGS, return IP, interrupt inhibition, prefix and bus-transfer contexts from S2 across the full family rather than one example. |

S3's retirement/delivery investigation now records a context-lifetime gap:
asynchronous entry reinitializes instruction data before Core selects the
completed instruction's timing. The working audit names one cross-family
handoff receiver, distinct instruction/delivery time and missing regressions;
three existing timing/observation tests pass per width, without closing it.

Fresh S3 complete units pass once each width: x64 506/506 in 61.92 seconds,
x86 506/506 in 62.56 seconds. Documentation/link/diff checks pass. The finite
receiver matrix and pending source/context dispositions have passed executor
actual-change review; coordinator acceptance remains pending. Neither these
green suites nor the inventory repair CPU defects.

T543 is closed at dd9af8951; its history retains the accepted four-App baseline.
T544 consumes the former first Queue candidate, not another structural split.

S1's complete delivery is
742c31f23; its immutable packet and the [T544 ledger](../history/M5-T544-retained-cpu-qualification.md)
record the inventory, source boundaries and unresolved proof receivers.
Both fresh complete unit suites pass 506/506. No production input changed,
so all eight 0543 PC artifacts and the MyNES pair remain current and untouched.
The [proposal](../proposals/m5-retained-cpu-qualification.md) names the initial
sequence; accepted S2 consumed the cross-family state/delivery batch.

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
retained in be44d5be0. S3 now has the active packet above. Audit closure is not defect repair or
CPU qualification, and does not remove any unresolved row from T544.

The [S3 working audit](../etc/evidence/t544-s3-8086-8088-family-audit.md)
records rendered early-IDIV lower-bound authority and the common-helper
mismatch, existing regression limits and a single-path repair proposal.
All eleven early-family partitions remain tracked in the delivered inventory;
unresolved source and repair contexts remain inside T544.
The same working record now reconciles all eight F01 handlers and base clocks,
identifies original AAM-length/DAA-flags table conflicts, records valid-BCD
transcription checks and the unchecked immediate-fetch failure lead. It also
reconciles MUL/DIV range bounds and confirms that shift undefined metadata has
no current reader. These are partial audit results, not Shared repair or S closure.
F02 now has an inspected form/flag/timing reconciliation, explicit encoding-table
conflicts and regression limits. A finite thirteen-call unchecked-decode inventory
joins F01/F02/F03/F05 under one failure-propagation receiver. Focused ALU tests
pass both widths; they do not replace S3's complete-unit delivery gate.
F03 adds source/code reconciliation for MOV, exchange, LEA and pointer loads;
the broader decoder sweep includes LEA. Five related existing tests pass both
widths, with early-profile and failure-context limits explicitly retained.
F04/F10 now distinguish basic stack/flag behavior from source-dependent FLAGS
images and arbitration. Existing PUSHF/POPF tests include 8088 but mask the
high bits relevant to S2; four selected stack/flag tests pass both widths,
without closing those discrepancies or the complete family batch.
F08 now reconciles normal repeat iteration/termination and existing timing
ownership. The original early multi-prefix interrupt limitation differs from
the current whole-instruction rewind; saved-IP/resumed-prefix proof and the
ten-handler failed-element sweep remain pending. Five selected string tests
pass both widths, without qualifying their missing 8088/delivery contexts.
F09/F14 now distinguish the existing WAIT duration/completion owner from
unproved early TEST sampling. Original ESC memory-read authority exposes an
omission in the no-FPU dispatch; a CPU-bus receiver is recorded separately
from FPU arithmetic support. Four selected tests pass each width; full I/O,
HLT wake and XLAT boundary qualification remain open.
F05 adds normal condition/return/frame reconciliation and a concrete
zero-displacement timing counterexample: final-PC equality cannot distinguish
taken from not-taken. The similar-issue sweep names early/later Jcc, LOOP/JCXZ
and compatibility consumers. Five selected control-transfer tests pass both
widths; Shared repair, failure traces and complete qualification remain open.
F07 adds count/flag and selected exact-formula reconciliation, with existing
rotate coverage limits retained. F09 now has a source/code XLAT word-offset
counterexample: BX+AL is not truncated before translation. The receiver owns
the effective-address calculation, not a blanket change to logical memory.
The existing rotate test passes once each width; Shared code remains unchanged.

## Current Technical Baseline

S3's F06 arithmetic sweep also identifies three word-concatenation expressions
that shift promoted signed DX before widening. The working evidence names
the expression-local repair and all-family regression receiver, distinguishes
it from the unresolved IDIV source boundary, and records no runtime crash
claim. Shared source/test and deployed artifacts remain unchanged.

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
