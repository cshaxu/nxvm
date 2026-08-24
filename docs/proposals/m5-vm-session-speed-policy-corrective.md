# VM Session Speed Policy Corrective

## Defect

T459 S1 let the VM select one tick and call Core time advancement after an HLT
wait. It preserved Core's per-tick event order but still made VM the producer
of guest-clock progress. That violates the sole Core guest-time owner rule.

## Bounded Repair

Keep the two stopped-session Console selections, but remove the Turbo one-tick
branch entirely. VM may only ask the existing virtual-time source for
already-selected ticks; it must not synthesize one. The fixed `Sleep(1)` when
no source tick is available remains Standard's explicit L2 HLT host-load
backoff, not a synchronization algorithm. Turbo remains a selected product
policy with no guest-time fast-forward until a later Core deadline contract
exists; it must use the same safe L2 backoff rather than busy-spin.

## Transfer

Correct host-paced Standard and safe Turbo fast-forward require a
profile-backed physical timebase and Core-composed, guest-observable controller
deadlines. The existing recurring timeline callbacks are not deadlines. The
existing host-paced guest-time contract debt remains the sole receiver.

## Acceptance

No VM speed branch calls `core_machine_advance_time` with a generated tick.
Both selections remain observable and rejected while running. The existing
virtual-time source retains its standard behavior; a focused regression proves
Turbo does not advance a source-less halted Core. Current gate, documentation
governance and the stripped Release artifact pass.
