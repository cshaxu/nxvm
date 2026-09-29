# T539 S19 CPU Bus Boundary

Baseline 193fd1962; Current owns admission and acceptance. This batch reviews
and qualifies the S18 bus extraction, rather than reimplementing it.

## Scope And Initial Findings

The nine CPU files have no concrete RAM/port/PIC/transaction or firmware
interception dependency. The previous CPU/PIC gate inspected only five files
and PIC tokens. Its existing check now covers all nine files, disallows board
and firmware tokens, and limits includes to CPU-owned headers, Types and the
public FPU contract. The registered CPU boundary negative test copies the
gate's inputs into its owned build-tree fixture, accepts the baseline, then
rejects eight injected violations in each of the nine CPU files (72 controls).
Each rejection must report its intended diagnostic. Includes use an exact CPU
header allowlist; a broad cpu*.h pattern would incorrectly admit a future
board-owned cpu_bus.h. That loophole is now covered explicitly.

CPU-context tests already cover port widths 1/2/4, read/write, provider failure,
completion after operand publication, copied fault records and INTA failure.
They do not prove board transaction trace ordering. Existing board lifecycle
tests covered reset cancellation and memory commit before retirement, but did
not directly cover all port failure boundaries.

Extended the same board lifecycle test with 18 cases: three widths, two
directions and success/endpoint failure/admission failure. Success retains the
transaction until explicit completion. Endpoint failure preserves the endpoint
call's side effect, cancels the transaction, then cancels the external cycle;
it does not publish the read result. Admission failure calls no endpoint and
does not cancel the unrelated DMA owner. Exact event counts and ordering forbid
extra commit/completion events. No production source or public API changed.

The same lifecycle test now covers six memory cases (read/write crossed with
success, provider failure, and transaction admission failure). Assertions check
the DATA provenance and transaction kind, exact BEGIN/COMMIT or BEGIN/CANCEL,
preserved provider-side effects on failure, and unchanged DMA ownership on
rejection. A separate observation read checks returned data with no transaction
or operational provider count. FPU extension trace checks both successful
BEGIN/COMMIT and rejection without cancelling the active DMA transaction.

The PIC phase test retains the original CPU execution proof that INTA commits
before interrupt stack writes. Its additional cascaded IRQ14 case exercises
the actual board CPU bus: admission rejection leaves the output vector and
both ISR registers unchanged, retains the pending request and DMA owner, and
emits no transaction events. Retry after DMA cancellation returns 2Eh, sets
master ISR2/slave ISR6, and emits exactly BEGIN/COMMIT with the committed vector.
Both expanded regressions pass individually on x64.

## Verification In Progress

The x64 regression binary passes. The first exact CTest filter matched no tests;
the registered name is unit.core-machine-transaction-lifecycle-s4-smoke. That
empty selection is not verification. The subsequent full x64 suite passes
370/370 in 27.46 seconds; the sequential x86 suite passes 370/370 in 25.21
seconds (command exit 0). Remaining work includes memory/INTA/extension proof mapping,
negative boundary checks, complete required gates and actual-diff acceptance.

After registering the negative test and tightening the header allowlist, both
existing build trees reconfigure successfully with no executable work. Full
units pass 371/371 on x64 (31.66 seconds) and x86 (35.13 seconds), combined
command exit 0. The registered negative controls pass in 10.89/10.92 seconds.
Diff whitespace checks pass. Remaining S19 work is the memory/INTA/extension
proof mapping, remaining gates and actual-diff acceptance; this is not closure.

After the memory/FPU/cascaded-INTA additions, complete units again pass 371/371
on x64 (28.18 seconds) and x86 (26.40 seconds), combined exit 0. The specialized
aggregate first rejected the newly direct-constructed PIC bus fixture as
unclassified (104 actual versus 103 registered sources). Its exact source and
board-bus purpose are now registered, with both count and duplicate checks
retained; no production or test assertion was weakened. The revalidation passes
all 66 aggregate steps, exit 0, including the 370-row strict compilation matrix.
Final requirement-to-proof mapping, remaining release checks and S19 review are
still pending at this checkpoint.

## Final Executor Review

| Contract | Direct evidence |
| --- | --- |
| Neutral CPU imports and no firmware interception | All nine CPU files inspected by the strengthened CPU/PIC gate; copied-fixture baseline and 72 specific negative controls. Board cpu_bus.c remains outside the chip. |
| CPU address masking, reset intent and observation intent | cpu_execution_context_smoke.c cpu_bus_cases runs the five CPU profiles; checks physical masks, reset-fetch intent for 286/386, nonexecuting preview and preserved copied observations. |
| Physical memory transaction and partial effect | machine_transaction_lifecycle_s4_smoke.c memory matrix tests success, provider failure, admission failure, DATA provenance and observation-only behavior. The original reset-cancel and commit-before-retirement assertions remain. |
| Reset mapping and external prefetch order | machine_transaction_s2_smoke.c executes the mapped reset vector, checks three wrapped prefetch cycles versus one after reset, BEGIN before COMMIT, DATA/PREFETCH provenance and no duplicate FETCH transaction. |
| Port transfer versus completion | CPU-context tests verify successful operand publication before completion and no completion on rejection; lifecycle board matrix checks 18 width/direction/outcome cases and exact cancellation order. |
| PIC pending and INTA | CPU-context tests cover success/failure at the CPU caller; PIC phase test proves master INTA before stack writes and cascaded IRQ14 admission rejection/retry with both ISR registers and transaction vector. |
| FPU command trace | Lifecycle test checks successful command BEGIN/COMMIT and rejected command preserving a DMA owner. This callback records the external command; it does not implement FPU instructions. |
| Sole owner and retained behavior | Reviewed cpu_bus.c callbacks, port completion and transaction/external-cycle trace against the retained S18 boundary; S19 changes no production instruction, timing, provider, allocation or routing code. |

Executor reviewed the added tests and CMake changes, including fixture lifetime,
restoration after each negative injection, exact header allowlist and explicit
constructor classification. No old test or assertion was removed. Existing
direct board access in these board-owned tests is intentional; CPU private
consumer migration remains assigned to S20-S29, not hidden as S19 completion.

Final six source/test manifests pass unchanged, documentation governance passes,
and git diff --check passes. Both complete unit suites and all 66 specialized
steps are recorded above. No production/link input changed, so the accepted
eight S18 0539 executables remain current; no new external integration run or
binary replacement is warranted for this test/gate-only delivery. MyNES, INIs,
external assets and Shared corpus are untouched. T539/CPU acceptance remains
open through the later packages. Implementation is ready for P1 and subsequent
coordinator actual-commit review; this record alone does not accept the S.

Counted implementation footprint: six test/build/gate files, +334/-9 lines
(net +325), excluding documentation. Growth is the explicit boundary matrices
and negative controls, not a new production layer; production delta is zero.

## Coordinator Acceptance

Reviewed actual P1 `f1b43af46` after push: all nine changed paths belong to
NXVM; no production replacement or duplicated owner is introduced. Test
matrices retain failure side effects instead of assuming rollback, and gate
negative controls restore their copied fixture after each rejection. Original
tests and execution scope are preserved. Verification satisfies this bounded
bus package. S19 is accepted; S20-S32 retain the rest of CPU migration and
T539 remains open. Artifact baseline remains S18, since link inputs are identical.
