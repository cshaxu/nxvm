# M5 Session Deadline-Consumer Convergence

## Purpose

Audit and close every repository test or product session runner that can
observe `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`. A session-backed runner
must either consume the published Core deadline through the existing VM
waiting entry point or prove its bounded scenario cannot enter that state.

## Frozen Coverage

The initial universe is every `test/vm/**` and `test/integration/**` C source
which calls `core_machine_run(session->core_machine, ...)`, plus the production
VM runner. Core-only fixtures are excluded because they deliberately test
Core's direct API. Each session source receives one disposition: uses the
existing waiting entry point; cannot reach HLT under its named bounded setup;
or is repaired in the sole session consumer path.

## Constraints

Do not restore immediate controller completion, add a VM tick writer, expose a
new timing setter, or create a test-only scheduler. The retained mechanism is
`vm_session_waiting_advance`: it reads Core observation, and Core alone
advances to its own deadline. Tests retain direct Core calls only where their
explicit subject is a Core API.

## Required Sequence

1. Freeze a line-addressed ledger of the production runner and every covered
   session-backed test, including each non-wait proof.
2. Repair all revealed consumer violations by using the existing VM waiting
   entry point; consolidate duplicate local loops only when their semantics
   are identical.
3. Add a narrow regression proving an asynchronous HDC path reaches its
   result through the same session consumer route.
4. Run the complete repository-only unit suite, external-asset integration,
   documentation governance, and the stripped current Release artifact gate.

## Exit Criteria

Every frozen source has a durable disposition. No session-backed product or
test runner assumes asynchronous controller completion or bypasses a published
deadline after HLT. Core remains the sole guest-time/deadline owner, and no
parallel runner or scheduler is introduced.
