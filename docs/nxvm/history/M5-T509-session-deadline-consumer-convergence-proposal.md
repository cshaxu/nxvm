# M5 Session Deadline-Consumer Convergence

## Purpose

Audit and close every repository test or product session runner that can
observe `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`. A session-backed runner
must either consume the published Core deadline through the existing VM
waiting entry point or prove its bounded scenario cannot enter that state.

## Frozen Coverage

The universe is every `test/vm/**` and `test/integration/**` C source which
calls `core_machine_run(session->core_machine, ...)`, plus the production VM
runner. Core-only fixtures are excluded because they deliberately test Core's
direct API. Each session source receives one disposition: uses the existing
waiting entry point; cannot reach HLT under its named bounded setup; or is
repaired in the sole session consumer path.

## Constraints And Completion

No immediate controller completion, VM tick writer, timing setter, test-only
scheduler, or new Core path may be introduced. The retained mechanism is
`vm_session_waiting_advance`: it observes Core state and Core alone advances.
The task completes only when the ledger exhausts the finite source set, the
complete unit/integration gates and documentation governance pass, and the
current stripped Release artifact is reproduced.
