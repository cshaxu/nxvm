# Native Console pause intermittency

Candidate, not admitted. Investigate the intermittent MyNES native Console
Escape/pause assertion through input delivery, broker takeover and observed
machine state. T562 S3 and T563 S10 recorded failures followed by passing
reruns; NXVM T564 S5 also observed a failure followed by a passing isolated
rerun. This is not yet a proven production defect.

Exit: establish the failure mechanism or explicitly document a bounded
unresolved result; preserve real native assertions and timeout, verify both
architectures and distinguish desktop automation from manual RDP acceptance.
Any required Shared modification needs its own authorized scope.
