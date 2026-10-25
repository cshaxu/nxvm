# M5 T580 S40 — Generic delay-acceleration design

## Result

No PC110-specific shortcut is admitted.  The selected TFT BIOS's `F000:EA86h`
service repeatedly reads port `61h`, compares bit 4 with the prior sample, and
uses `LOOP` only after an observed transition.  It is therefore not a pure
counter decrement that Core may skip without preserving every intermediate I/O
observation.

## Existing ownership

- CPU owns decode, register/flag effects and the actual I/O instruction.
- `x86_core_machine_run()` publishes time after each retirement.
- `x86_core_machine_advance_to_next_deadline()` intentionally accepts only
  stopped or paused machines.
- The scheduler owns device deadlines and NMI refresh; it has no generic
  promise that a port read remains constant until a known deadline.

## Disposition

Using the paused-only deadline API while running, or forcing a port result or
register count from a PC110 BIOS address, would create an invalid second
execution path.  A future general optimizer would need a restricted loop
contract, Core-owned batch retirement and explicit device observability rules;
it belongs in a separate X86 Core performance task.  S41 instead selects the
already implemented Turbo runner policy for MyPC110.  Turbo changes host
rendezvous/pacing only and preserves every CPU retirement and guest-time
publication.
