# M4 Initial Prompt Correction

## Outcome

Delay MyNes's first cooked monitor prompt until App relays Common's actual
initial stopped state into Session. The first `rom insert <file>` then uses the
same stopped-state command path as every later cartridge insertion.

## Acceptance

The no-ROM startup prints help without accepting input before `STOPPED` reaches
Session, then displays `MyNes>`. Its first valid cartridge insertion requests a
reset. A startup configured with a ROM remains prompt-suppressed until reset
completion. The repair stays in MyNes App and product tests; all imported
SoftPC roots remain byte-identical.
