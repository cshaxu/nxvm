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

Executor review: source checks cover actual metadata, dispatch, length and
LOCK parsing, test-register moves, flags consumers, profile bounds and existing
regression scope. Source conflicts and optional silicon eligibility are gated,
not declared resolved by guessed behavior. The LOCK finding is static evidence;
its new dynamic regression belongs to S3. Documentation governance and
`git diff --check` pass. No runtime tests or builds are claimed for this
documentation-only S; existing ten executable inputs remain unchanged.
