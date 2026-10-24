# PC110: protected bootstrap and real mapping admission

## Admission and dependencies

This candidate follows closed T574.  It depends on the approved external TFT
firmware asset and [T574's trace evidence](../etc/evidence/t574-s1-pc110-c-segment-trace.md).
T575 S1's ordinary-executor trace has isolated its first boundary as a
protected data-segment reload, not a chipset mapping operation.

## Objective

Classify and, only when fully proved, implement the first real PC110 bootstrap
contract preventing firmware from reaching its own video BIOS.  The receiver
may use primary documentation and labelled L2 behavior evidence to select
experiments, but may not copy third-party emulator source.  The admitted first
contract is the explicitly selected, strict-by-default Core data-cache reload
policy recorded in [T575 S1 evidence](../etc/evidence/t575-s1-pc110-protected-bootstrap.md).

## Non-goals

No static C-segment alias, host BDA/IVT mutation, BIOS/interrupt interception,
PC/AT fallback, second executor, guessed VL82C420 protocol, raw-ROM commit or
PC110 executable release claim.

## Exit

Record a finite source-labelled failure boundary on both widths and either
deliver its smallest correctly owned contract with direct regression, or close
with a precise missing-source disposition.  Product qualification remains a
separate receiver.  The next boundary after the admitted reload remains a
separate evidence receiver; this proposal does not permit a guessed mapping
or chipset protocol merely because execution proceeds farther.
