# T2 S44 Native Packed Scalar Decimal

Admitted at clean `7a6fe123b` after accepted S43. This finite V07 batch
qualifies ADJ4A/ADJ4S for valid packed-decimal arithmetic, not arbitrary
nondecimal silicon behavior, complete elapsed time, Machine or game readiness.
The unchanged existing DAA/DAS handlers already implement these legal results;
there is no justification for a production patch merely to create a diff.
Shared P1 `d6205a514` is pushed. This product-local delivery retains the
complete evidence/packet for coordinator actual committed-change review.

## Primary Reconciliation

NEC V20/V30 October1986 manual SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
printed12-61/12-62, physical PDF106/107, was visually reviewed. ADJ4A's
low-nibble condition is printed `<9` and V is marked affected; ADJ4S's
AC-to-CY assignment is also unsuitable as a literal general-input model.
The historical S18 evidence remains unchanged.

NEC original-uPD70116-inclusive V Series Instruction manual,
U11301EJ5V0UMJ1 fifth edition September2000, SHA-256
`19A69547651AA65E32DF33F1BF84DEA095A00237F680A67B166213FCC6D96F6A`,
printed19/20, physical PDF30/31, was visually reviewed in full. It states
low-nibble `>9` or AC correction, high correction and packed addition/subtraction
meaning, and explicitly marks V undefined. Source:
https://www.ardent-tool.com/CPU/docs/NEC/V20-V30/v_series.pdf .
Renderer warnings omit some symbolic arrows/affected marks, so the extracted
text and original scan supplement visible prose, conditions and U cells;
missing glyphs are not claimed visible. No manual or external implementation
payload enters the repository.

This resolves the legal decimal-result/defined-flag proof prerequisite,
not every interpretation of arbitrary AL/AC/CY inputs. The oracle uses
ordinary signed decimal arithmetic on actual valid operands, rather than
transliterating either conflicting pseudocode. Its AF expectation follows
decimal low-digit correction and CF follows decimal carry/borrow. S/Z/P
follow the complete byte result; only V is masked. Unchanged AH, upper halves,
other registers/flags, segments and next IP are checked in the full CPU image.

The later clock table also disagrees with original ADJBA/ADJ4S counts.
This batch changes no clock source or value: existing unpacked/conversion
nominal tests remain exact; packed-adjust rows remain zero/unattributed,
unallocated with maximum0. Those source/time questions retain V07/V15.

## Independent Complete Legal Operand Batch

The existing decimal test retains all817152 old unpacked/radix-ten inputs,
their expectations, pure previews and every applicable fetch/observation refusal.
The same fixture/checker adds40000 packed inputs: both adjust operations,
each decimal operand0--99 and carry/borrow0/1. Input AL and AC/CY are derived
from the preceding binary arithmetic; the expected result is independently
derived in decimal, then encoded. AH varies independently of the result;
the full CPU comparison preserves it and every unaffected field.

Additional360 deep contexts use all combinations of operand0/9/99, both
operations/carry inputs, absent plus four segment overrides and normal/FFFF
fetch starts. Each performs pure preview, actual execution and refusal at
every observed fetch/observation position, retaining exact CPU rollback,
no data transactions/phases, diagnostic/stop and no completed timing on failure.
This covers low correction, high carry/borrow, zero, segment and code-wrap
seams without constructing a second test framework or production entry.

Exact markers:

```text
V30:DECIMAL:OK cases=817152
V30:NOMINAL-DECIMAL:OK cases=817152
V30:PACKED-ADJUST:OK cases=40360
```

The first focused run failed because the new test incorrectly expected
packed-adjust nominal clocks already existed. The selector has no such row.
Only that new expectation was corrected to the actual zero/unattributed
contract; no old assertion, budget or production eligibility was weakened.
The next focused x64 run passed. Initial images104/105 were DIV pages, not
adjust evidence; actual relevant106/107 were subsequently checked. Owned
temporary images are removed after review, not retained as source authority.
Exactly six owned page renders were removed after review; PDFs, extracted
text, qualification logs and existing persistent build/cache graphs remain.

## Similar-Issue Sweep And Resulting Design

Queries over production, tests and retained task records:

```text
rg -n '^static void (DAA|DAS|AAA|AAS|AAM|AAD)' src/x86-core/cpu/cpu_instructions.c
rg -n 'DAA|DAS|AAA|AAS|AAM|AAD|ADJ4A|ADJ4S|radix' test/x86-core/cpu docs/myraidenii
```

DAA/DAS share existing advance/result/flag/fetch owners and require no
production repair on this complete legal operand domain. AAA/AAS and
documented0A conversions keep their existing complete S18 vectors. Generalized
radices and inconsistent arbitrary decimal flags remain explicit V07
receivers; V15 retains clocks/physical accounting. Intel paths have distinct
semantics and existing regressions, not a new V30-as-Intel alias. Lib,
Emulator, public/private layouts, dispatch, time/state owners and all
executable inputs stay byte-identical to baseline.

Tracked code count uses `git diff --numstat 7a6fe123b --
test/x86-core/cpu/cpu_v30_decimal_smoke.c`: test+56/-15/net41, production0.
The manifest separately changes+1/-1/net0. Positive test growth is the
independent legal packed-decimal proof and reuse of the same complete-context
checker, not another implementation or unused abstraction. No new function,
helper, state, public interface, production path or build registration exists.

## Verification And Release Decision

Final dual-width results are recorded in [results](t2-s44-results.json).
Original cached Release Ninja graphs use the same persistent ccache launcher,
build-j8 and unit-j4. Both all-target builds, five transient focused tests,
678/678 repository units (191 X86 Core), strict498 and Types1389 gates pass,
with zero failures/skips. Full units take112.83/121.18 seconds; the expanded
decimal unit takes1.55464/1.84476 seconds. Existing warning exceptions remain;
this is not a no-warning-exemption claim. Runner22909 is terminal zero.
Executable/build/INI/assets inputs are compared against the exact baseline;
all ten current releases receive fresh PE/size/SHA/debug-section checks against
the accepted [S43 identities](t2-s43-artifacts.json). Unchanged inputs/hashes
require no manufactured binary refresh; every one of ten actual identities
matches S43. Source/test manifests, documentation governance, packet fields
and whitespace checks pass. No external media is read or changed.
No fresh external/native integration, desktop/RDP, physical audio or gameplay
claim is made; this is test-only semantic qualification, not a playable EXE.

Original V01--V17 domain and full T2/proposal/Queue stay open. This result
accepts legal scalar packed decimal, not the remaining arbitrary input,
prefix/alias/8080, complete clocks/max/Machine/board/game receivers.

## Archived Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S44, next unused S after accepted S43; no other MyRaidenII packet. |
| Admission And Approval | Owner request to continue NEC V30 and required facilities; standing automatic S admission and semantic qualification approval. Coordinator admits this bounded test/source-reconciliation batch on 2026-10-18. Targets Shared and MyRaidenII only; no public contract or product behavior change approved. |
| Objective | Consume V07 native ADJ4A/ADJ4S valid packed-decimal scalar batch: reconcile primary NEC descriptions and independently prove all valid two-digit addition/subtraction outcomes, carry/borrow, flags, fetch/wrap/preview/failure effects through existing handlers. |
| Non-goals | No physical time, Machine admission, board/game completion, undocumented nondecimal silicon claim, generalized radix claim, clock conflict resolution, Lib/Emulator/Intel change or receiving artifact rebuild without executable-input change. |
| Reference Baseline | Clean 7a6fe123b; S43 source/test/artifact identities; original NEC1986 manual and NEC2000 U11301EJ5V0UMJ1 primary already pinned by S43. Relevant full pages must be visually checked. |
| Candidate Proposal | ../proposals/m1-v30.md and ../etc/t2-v30-ledger.md V07/V15/V17; qualify a finite remaining semantic batch without retiring other rows or reducing the frozen domain. |
| Files And ABI Surface | Existing test/x86-core/cpu/cpu_v30_decimal_smoke.c and test manifest; MyRaidenII Current/history/ledger/evidence/index. Expected production+0/-0; test approximately+90--130/-5--15. No function, enum, layout, state or production helper addition. A demonstrated production defect requires packet revision before repair. |
| Applicable Rules | Execution packet/one target per P/complete units/actual-diff review; architecture sole CPU/flag/fetch ownership; Coding and local governance skills preserve existing handler and test style, Types vocabulary, independent oracle and old vectors; source policy permits primary facts only, no external code/manual payload; Document authority/retention and exact evidence. |
| Verification | Both original Ninja/ccache Release graphs: focused decimal and existing packed-BCD/ALU/flags/nominal-transfer units, complete all-target build and repository-only unit suite, specialized strict/Types/dependency/manifest gates. Verify unchanged production/build/assets/INI hashes and all ten current release identities; no external media use. Documentation gate and git diff --check. |
| Expected Markers | Existing V30:DECIMAL:OK and V30:NOMINAL-DECIMAL:OK retained; new V30:PACKED-ADJUST:OK with exact independently counted vectors. Full units678/678 each if registration unchanged; strict498, Types1389, no failed/skipped units. |
| Asset Needs | Existing ignored primary PDFs/text and task-owned page renders only. No ROM/media access, modification or acquisition; original current PC0564/MyNES0044 assets stay unchanged when executable inputs stay unchanged. |
| Reporting Requirements | Confirm scope/estimated diff before implementation, report genuine failures and disposition, actual source/test additions/removals/net, exact complete units, source/model boundary, unchanged executable decision, pushed commits and remaining whole-T gaps. |
| Stop Conditions | Stop on primary ambiguity outside legal decimal arithmetic, required public/Lib/Emulator change, new production defect needing packet revision, protected-input change or unrelated worktree overlap. Do not invent arbitrary-input flags or timing. |
| Exit Criteria | All valid operand/carry cases and selected complete-context/failure seams independently pass both widths without weakening817152 old vectors; source differences explicitly classified; all repository units/gates and ten unchanged identities pass; exact diff and similar-issue sweep reviewed, complete target P commits pushed, coordinator acceptance before closure. |
| Original Owner Request | Switch to MyRaidenII context and execute the next S implementing NEC V30 and required facilities toward playable RAIDEN II; not a CPU/game completion claim. |
| Similar-Issue Sweep | Review all six existing DAA/DAS/AAA/AAS/AAM/AAD handlers, decimal tests and source descriptions. Keep unpacked/conversion tests and Intel paths; classify invalid decimal inputs, non-ten radices and contradictory clocks as retained explicit V07/V15 receivers, not inferred qualification. |
