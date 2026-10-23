# PC110 Power Management And APM

## Admission And Dependencies

Unnumbered M5 candidate created by T567 S3.  It follows the completed PC110
input composition and does not reopen it.  Admit only after the source gate
below is satisfied; it is not authorization to infer a controller from U6
physical wiring.

## Source Gate

Before production code, record one reproducible CPU-visible contract for each
admitted behavior: decode/call entry, request bytes or registers, success and
failure results, lifecycle state, reset behavior, wake source and timing.
Read-only original BIOS analysis, repeatable guest traces or a complete
reference implementation may supply a labelled evidence grade.  The
contradictory historical `0ECh/0EDh` attribution is excluded: those ports are
already the VL82C420 configuration bank.

## Objective And Scope

Implement only source-qualified PC110 APM/power-status, suspend, power-off and
wake behavior.  The selected owner must keep CPU-visible device protocol,
firmware services and host lifecycle separate: a guest request cannot be
implemented by directly calling an Emulator lifecycle action.

Expected owners are App-local PC110 firmware/device wiring and existing
Core/X86 NXVM attachment points where their contracts already fit.  A shared
contract change requires a separate review.

## Non-goals And Stop Conditions

No invented mailbox, always-ready reply, rail simulation, MCU firmware import
or execution, host-key shortcut, second lifecycle owner, or U6 assignment to
`0ECh/0EDh`.  Stop and transfer any command whose complete guest-visible
contract remains unknown.

## Suggested S Boundaries

1. CPU-visible APM/firmware contract audit and exact ownership map.
2. One bounded status or power-control receiver, only if its source gate
   passes.
3. Suspend/wake state and real production-path qualification.

## Verification And Exit

Use direct contract tests and the production guest route.  Prove both accepted
and rejected requests, state transitions, wake ownership, reset interaction
and unchanged existing profiles on x64/x86.  Do not publish a MyPC110 artifact
as boot-qualified until the separate firmware-boot task reaches its own gate.
