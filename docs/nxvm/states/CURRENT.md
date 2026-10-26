# Project Status

## Current Work

## M5 T580 S54 — Post-restoration Setup and boot route

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; next numeric S54 after accepted S53, one-session coordinator/executor. |
| Admission And Approval | Owner's standing instruction to implement PC110 through T closure, automatic in-scope S admission and source-labelled L2 research/implementation approval. The active persistent goal continues that request; it does not authorize firmware shortcuts or excluded Shared changes. |
| Objective | Consume the ledger's complete post-restoration POST/Setup/boot batch: reconcile static branches and installed receivers, then observe the unchanged BIOS beyond real retained wake/RSM through a named Setup or boot handoff, or identify the complete evidenced receiver/failure class preventing it. Preserve all original guest/product gates. |
| Non-goals | No ROM edits, BIOS-PC interception, synthetic IVT/boot service, fixed device status/retry replies, automatic wake, new executor/clock/renderer, physical-dump prerequisite, unrelated App source change or PC110 publication. |
| Reference Baseline | Clean pushed 63f1f6e5a, accepted S53's actual F12/retained-image/RSM proof on both widths and the unchanged original T580 universe. |
| Candidate Proposal | ../proposals/m5-pc110-qualification.md and ../etc/evidence/t580-s1-pc110-qualification-ledger.md; S53 receiver evidence remains the starting proof, not product completion. |
| Files And ABI Surface | Revised after the evidenced #NM class reconciliation: Shared X86 Core cpu_instructions.c ESC owner and cpu_timing_model.c L2 allocation, existing full-state/alignment/nominal CPU and executor/FPU Machine tests, package README/manifests; NXVM existing real-firmware harness/registration and task docs; receiving eight PC and two Core-linked MyRaidenII artifacts (retain their own versions). No public API/type, Lib/Emulator, FPU arithmetic implementation or MyNES change. Estimate production +50--90/-2--8, Shared tests +120--220/-10--35; preserve original table-style handlers. |
| Applicable Rules | NXVM Task Reading Set, CONTRIBUTING, shared EXECUTION/ARCHITECTURE/CODING/DOCUMENT, NXVM design ARCHITECTURE/CODING and source-policy; architecture-governance followed by coding-governance skills. One execution/state owner, Types vocabulary, unchanged protected inputs and explicit source grades. |
| Verification | Reconcile the finite post-RSM POST, Setup/Flash and IPL branches against actual firmware and installed App routes before selecting a repair. Reuse the existing Core execute-watchpoint/step and same diagnostic, not per-retirement snapshots. Run one bounded real observation per necessary width after observer review; complete all repository units per width before S acceptance, scheduling the expensive Types selftest alone rather than beside builds/BIOS. Run affected static/docs checks; rebuild only genuinely receiving artifacts if production changes are later admitted. |
| Expected Markers | A semantic POST/Setup/boot handoff and its required context, or an evidenced complete receiver/failure-class disposition. A new PC, elapsed budget, successful probe process or further loop iteration is not boot success or a qualification gate. |
| Asset Needs | Existing admitted read-only TFT BIOS/font and derived DSTN inputs, pinned PC110 reference repositories and existing source records. No acquisition, binary mutation, source import or guest-media copy. |
| Reporting Requirements | Report the reconciled route inventory, evidence selecting any repair, before-change design/diff estimate, actual delta, regressions and all unexecuted desktop/guest/panel gates. Evidence: ../etc/evidence/t580-s54-post-restoration-route.md. |
| Stop Conditions | Contradictory source stops speculative repair, not independent research. A necessary Shared change must receive an explicit revised packet/contract; Lib/Emulator remain excluded. Scratch only build/t580-s54-boot, S54-owned, 16 MiB. Earlier observations retain their original 300 s results. The expanded full wake/POST/INT19 replay now has a separately justified 360 s/100M containment budget, after sparse x86 measurements prove the former endpoint precedes a further finite BIOS delay. No guest timing or functional assertion changes. Do not repeat a timeout without new evidence or a corrected observer. |
| Exit Criteria | The whole admitted route batch has a source/owner/actual-execution disposition and any admitted repair proof, full required unit/static qualification, actual-diff review, safe cleanup, complete target-scoped commit/push and coordinator acceptance. All unqualified original product members remain named in the ledger; no partial P from an unfinished batch. |
| Original Owner Request | Continue implementing PC110 to its real qualified product end state under the normal governance/build/test/publish workflow; use justified emulator-reference L2 when exact hardware evidence is unavailable. |
| Similar-Issue Sweep | Reconcile all post-restoration reset/return, ordinary POST, Setup/lower-Flash, IPL/INT19 and terminal failure consumers as a batch, with existing route and lifetime owners. Do not repair one observed instruction while hiding another unclassified member. Preserve caches; archive facts/hashes and clean only owned scratch after all processes exit. |

Coordinator admits S54 under the existing T scope. Executor confirms the
initial observation-only boundary; no production receiver is yet selected.
The first corrected boot observation reaches S53's genuine RSM but exceeds
the 300-second native budget before an INT19 result. The observer now bounds
the post-RSM phase to eight million further instructions (inside the original
100M maximum) and prints a copied final CPU/code context. That is containment
diagnosis only, not a relaxed success predicate or production change.
That observation now ends with 398,666 delivered exceptions (four at RSM)
inside the BIOS unexpected-interrupt handler, without a terminal Core fault.
The existing copied diagnostic now identifies the first post-RSM exception:
device-not-available (#NM) at F000:6F25, bytes DB E3 (FNINIT), with CR0=10h
and the configured 486SX/no-FPU pair. The BIOS is performing its ordinary
coprocessor-presence test. Source reconciliation must cover the complete
absent-FPU ESC/WAIT class and existing C14 tests, not special-case this PC.
At that observation boundary the Intel manual and a later independent hardware
report disagreed on EM-clear behavior; no speculative fix was selected.

Coordinator revises the packet after reconciling C14's unconditional #NM
assumption with independent hardware and pinned 86Box evidence. The owner
permits source-labelled L2 reference models; this is a correction of the
overclaimed EM-clear behavior, not an accuracy downgrade or a fake FPU.
The executor implements one CPU-owned absent-FPU ESC model. EM/TS faults,
WAIT and earlier families remain intact. Valid SX memory operands retain
segment/alignment/paging checks; observed Intel no-FPU stores deliver ones
without numeric state. Reserved forms do not acquire invented arithmetic.
The complete opcode/form/control matrix and source limits are recorded in
the S54 evidence before implementation. All receiving artifacts and required
repository units are part of this revised batch; no partial P is delivered.

The x64 unchanged-BIOS replay now passes retained wake/RSM and the actual
F000:52BD INT19 handoff to F000:E6F2, at 56,679,591 instructions, four
processor resets and the same four delivered exceptions. The additional
source timing omission is corrected at the existing 486 L2 cost owner;
successful no-device ESC receives an explicit one-clock modeled decode
envelope while actual memory/fetch/prefix/bus costs keep their existing
owners. x86 real execution, complete regression and artifact delivery remain
pending. No DOS, Setup UI or original-DSTN qualification is inferred.

The corrected x86 replay initially times out at 300 seconds after RSM, so
that boot-handoff attempt is not accepted. No unchanged replay or speculative
performance patch is selected. The existing harness records host milliseconds and
copied CPU/exception progress only at startup, wake, RSM and sparse million-
instruction diagnostic boundaries; those host values never control guest
time or success. This is the next diagnostic, not another production patch.
Both cached full builds have completed. Isolated Types selftests pass in
107.56 s x64 and 99.84 s x86. The x64 remainder has 710/711 initial passes;
its separate executor SX FPU matrix retained the old unconditional #NM
expectation and is corrected without production change. That test and the
updated manifest pass on recheck, completing evidence for all 712 distinct
x64 units. The x86 remainder passes 711/711 in 131.39 s, likewise completing
712 distinct unit results. The same background controller (session 50342)
then runs the sparse-timing x86 BIOS diagnostic alone. Unaffected MyNES's
generated EXE copies are restored exactly to HEAD
after both builders exited; source, INI and snapshots remain unchanged.

That diagnostic terminates at the original 300-second deadline with useful
progress evidence: real RSM is at 270.414 s, followed by about 1,048,576
instructions per 4.75--5.06 s in the known finite EA86/EA88 BIOS delay,
with no additional exception. At 299.697 s it has executed 55,080,898
instructions, versus the proven x64 INT19 point at 56,679,591. Coordinator
therefore corrects the newly expanded boot-handoff test's containment to
360 seconds; the former 300-second power-only predicate had less work.
This is an evidenced workload-budget revision under automatic in-scope S
planning authority, not a guest-time change or a passing timeout. No runtime
performance patch is justified. The unchanged final functional predicate must
still pass actual BIOS execution on each width before delivery.

Final registered regressions now pass on both widths: FDC/timer/ASMI in
62.42 s x86 / 45.76 s x64; complete retained wake/POST/INT19 in 302.96 s
x86 / 224.16 s x64. Both whole routes retire exactly 56,679,591 instructions,
perform four processor resets, retain the same 512 bytes, return through
RSM and reach the real INT19 service F000:E6F2 without a fifth exception.
The original guest/Setup/panel/release gates remain open. Eight receiving PC
and two Core-linked MyRaidenII artifacts are rebuilt/deployed through
the original two Ninja graphs. All eight PC INI identity integrations pass;
PE widths, versions and deployed hashes are verified in the
[artifact inventory](../etc/evidence/t580-s54-artifacts.json).
Four PC110 previews are rebuilt but remain unpublished. MyNES artifacts and
all PC INI contents match HEAD. Final incremental builds report no work on
both widths. Executor delivery and coordinator acceptance remain separate;
the original whole-product gates are not satisfied by this bounded batch.

| Task | Progress |
| --- | --- |
| T580 S53 | Accepted bounded EC/power/retained-wake receiver: Shared `85a65c3a8`, NXVM `a8561555e`. Source/test and receiving artifacts are pushed; the closure governance commit records coordinator acceptance. |

## Verified Baseline

S53 adds one supported App EC reply/RAM receiver, Chip-owned suspend/button/
resume-event semantics and optional copied Machine physical-key wiring.
Both widths execute the unchanged BIOS through real F12 driver input,
retained processor reset, the same 512 CPU-save bytes and actual RSM. The
registered power integration passes in 186.98 s x64 and 248.64 s x86.
All 712 distinct repository units per width have passing results: the initial
parallel static-test timeouts and unchanged-budget serial corrections remain
explicit in the [S53 evidence](../etc/evidence/t580-s53-power-receiver.md).
Original FDC/timer/ASMI regression and all four deployed INI identities pass
on each width. No whole-tree/desktop qualification is inferred from them.

The eight current PC artifacts retain version 0.5.0580 under
`assets/{my5160,my5170,mydeskpro386,nxvm}/`, with their existing lowercase
INI files unchanged. The artifact target is `vm-0-5-0580`. Four rebuilt PC110
TFT/DSTN previews stay unpublished in the stable Ninja graphs. Lib, Emulator,
X86 Debug, X86 IBM-PC, MyNES and MyRaidenII source/test/artifacts are unchanged.
All 28 S53 temporary files/3,011,103 bytes are removed; both Ninja graphs and
ccache are retained. Working-tree and remote state are checked after closure.

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and its complete
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain original Setup/boot, selected storage/guest, front panel/live input,
lifecycle, source-grade and publication gates. They are not replaced by a
passing wake transaction or a new terminal PC. The next coherent batch
reconciles the real post-restoration Setup/boot route before selecting any
further receiver. Source-labelled L2 remains permitted; no physical-unit dump
is imposed as a blanket prerequisite.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained prior status](../history/M5-T580-retained-status.md) preserves the
earlier packets, failures and baselines as history, not simultaneous active
tasks. [Queue](QUEUE.md) retains unrelated ordered candidates.
