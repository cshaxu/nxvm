# M5 T559 Selected 486SX Instruction Semantics

Admitted on 2026-10-13 after owner closure of T558. Admission was recorded in
`29ba073f3`; this main history record is added during S1 to complete the required
task-record topology. [Current](../states/CURRENT.md) remains the sole active
packet authority; the [proposal](../proposals/m5-pc110-486-instructions.md)
defines scope.

S1 audits the actual decoder/profile/test boundaries against the original
sources retained by T558. Its [inventory](../etc/evidence/t559-s1-instruction-inventory.md)
records concrete gaps, I1--I11, sequential batches and remaining system
decisions. No S1
implementation, source import, runnable 486 admission or executable refresh
has occurred. S1 remains open pending coordinator review of this delivery.

Coordinator acceptance: S1 P1 `2afeda156` was reviewed against its actual five
document changes, source references, I1--I11 dispositions, bounded gates and
task scope. Documentation governance and whitespace checks pass. S1 closes as
planning delivery; S2 identity/decode/BSWAP is admitted next. The prior open
wording above records executor delivery, not the current status.

Executor review: source checks cover actual metadata, dispatch, length and
LOCK parsing, test-register moves, flags consumers, profile bounds and existing
regression scope. Source conflicts and optional silicon eligibility are gated,
not declared resolved by guessed behavior. The LOCK finding is static evidence;
its new dynamic regression belongs to S3. Documentation governance and
`git diff --check` pass. No runtime tests or builds are claimed for this
documentation-only S; existing ten executable inputs remain unchanged.

## S2 Executor Delivery

S2 implements I1 and scoped I10: an appended CPU-only 486SX identity and the
existing-table BSWAP path, with old-profile rejection and machine admission
still gated. [Evidence](../etc/evidence/t559-s2-decode-bswap.md) records source,
model-policy limits, failure investigation, actual-change review and artifact
hashes. Complete repository-only units pass 593/593 on each architecture;
eight real product/INI identity integrations pass. The external boot suite is
not claimed here and remains required at T closure.

Production C/H changes are +55/-12 (net +43); test C/CMake changes are
+177/-1 (net +176), excluding manifests, documentation and artifacts. All
eight PC EXEs are updated to 0559; MyNES and Shared six remain unchanged.
Only the separately owner-approved three Shared root preset targets change.
The complete executor delivery is ready for coordinator review, not task
closure. I2--I9 and the remaining I10 sweep keep their planned S owners.

Coordinator acceptance: Shared P1 `aad734fdf` contains exactly the three
approved preset targets; NXVM P2 `c93b778aa` contains the reviewed CPU,
test/build/documentation and eight artifact changes. No Shared-six or MyNES
delta is present. The S2 actual-change audit maps I1 and identity/decode I10
to the direct CPU test, production machine rejection and retained full units.
Both pushes are confirmed. S2 closes; S3 is admitted for I2 and inherited I9
under the owner's automatic Core-only sequence. T559 remains open.

## S3 Executor Delivery

I2 XADD and the ordinary-opcode portion of I9 now have direct form, alias,
flag, LOCK and failure-path proof. Production changes are +44/-4; test
C/CMake changes are +426/-12, excluding manifests/documents/artifacts.
The final complete units pass 594/594 on each width. Eight refreshed 0559
PC EXEs pass product/INI identity, PE-width and version checks; Shared-six,
MyNES and owner inputs remain unchanged. [S3 evidence](../etc/evidence/t559-s3-xadd-lock.md)
records hashes, failed intermediate attempts, final preflight correction and
the explicit inherited RMW TODO transfer. No new production 486 admission or
whole-T closure is claimed. The owner-approved batch-efficiency adjustment
is recorded in the proposal without dropping any qualification requirement.

Coordinator acceptance: reviewed actual c1a04cffb source/test, manifest,
document and eight-artifact changes against I2/scoped I9. The single CPU
execution owner, immutable decoded address, write-permission-before-read,
source-before-destination alias order and existing exception rollback are
preserved. Complete units, eight real product/INI checks, artifact hashes,
scope, documentation and whitespace evidence support this bounded delivery.
The push is confirmed. S3 closes and S4 admits the complete CMPXCHG family;
the unresolved system, whole-sweep and external-boot gates keep T559 open.

## S4 Executor Delivery

I3 CMPXCHG implements both comparison outcomes across byte/word/dword forms,
using the existing CPU comparison, write preflight and destination publication
owners. The unequal memory path still writes the old value; failed reads or
writes do not publish partial CPU state. [S4 evidence](../etc/evidence/t559-s4-cmpxchg.md)
records the full form/failure matrix and actual-change review. Production
changes are +46/-3; test C/CMake changes are +428/-1. Complete units pass
595/595 on both widths and eight refreshed PC EXEs pass product/INI identity,
PE-width and 0559 version checks. Public six, MyNES and owner inputs are
unchanged. This is executor delivery, not whole-T or 486 system qualification.

Coordinator acceptance: reviewed actual c4a08c66e instruction handlers,
metadata/LOCK/dispatch changes, complete CPU test, board metadata expectation,
registration/manifests, evidence and eight artifacts. I3 maps to direct
equal/unequal, alias, write-cycle, permission and failure/retry assertions;
the existing instruction/memory/exception owners remain sole authorities.
Both full unit logs, eight product/INI logs and deployed hashes support this
bounded qualification; push is confirmed. S4 closes and S5 is admitted for
the complete I4--I6 system-instruction boundary group. T559 remains open.

## S5 Executor Delivery

Whole I4--I6 inspection confirms no real cache/TLB owner to connect. The
system receiver now explicitly owns the complete forms and effects; S5 does
not introduce successful no-ops, new fallback errors or false guest UD for
legal operations. Existing passive 486 TR6/TR7 behavior remains an identified
limitation behind the unchanged production admission gate. This follows the
S1 ledger's unavailable-state disposition, not a claim that these semantics
are implemented.

The [S5 evidence](../etc/evidence/t559-s5-system-instruction-boundaries.md)
records original-manual pages, actual ownership, direct retained transfer,
illegal-form, CPL/VM86 and failed-fetch proofs, and review. Production is
unchanged; test C/CMake is +242/-0/net+242. Complete repository units pass
596/596 on both widths. S4's eight EXEs remain current without executable-input
changes. Shared-six, MyNES, INI/media/ROM inputs are unchanged. Executor delivery
awaits coordinator acceptance; I7/I8 and T-level qualification remain open.

Coordinator acceptance of 8e6159de4: inspected all 241 test lines, registration,
manifest delta, task evidence and the explicit receiver/packet reconciliation.
The test's actual GP frame and VM86 stack switch, complete CPU-state comparison
and failed-fetch checks support its limited claims; legal 486 effects are not
asserted as UD or accepted passive storage. Both 596/596 unit logs and unchanged
production/artifact paths support qualification. Push is confirmed. S5 closes
under S1's explicit system disposition; S6 admits I7/I8 source-qualified CPUID
and flags as one batch. T559 remains open.
