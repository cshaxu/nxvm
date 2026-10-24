# PC110: protected bootstrap and real mapping admission

## Admission and dependencies

This candidate follows closed T574.  It requires either a lawful primary
chipset source or a reproducible ordinary-executor trace that reaches the
first PC110 protected-bootstrap device/configuration boundary before any
mapping implementation is admitted.  It depends on the approved external TFT
firmware asset and [T574's trace evidence](../etc/evidence/t574-s1-pc110-c-segment-trace.md).

## Objective

Classify and, only when fully proved, implement the first real PC110 bootstrap
contract preventing firmware from reaching its own video BIOS.  The receiver
may use primary documentation and labelled L2 behavior evidence to select
experiments, but may not copy third-party emulator source.

## Non-goals

No static C-segment alias, host BDA/IVT mutation, BIOS/interrupt interception,
PC/AT fallback, second executor, guessed VL82C420 protocol, raw-ROM commit or
PC110 executable release claim.

## Exit

Record a finite source-labelled failure boundary on both widths and either
deliver its smallest correctly owned contract with direct regression, or close
with a precise missing-source disposition.  Product qualification remains a
separate receiver.
