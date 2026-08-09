# Evidence Policy

Compatibility behavior is implemented from explicit evidence rather than from
plausibility or a reference project's shortcut.

## Authority Order

1. Reproducible behavior of the target DOS program on a legally available
   reference system, paired with an inspectable probe.
2. Intel manuals, Microsoft DOS documentation, RBIL, and original vendor
   documentation for the exact ABI boundary.
3. Generated conformance probes and independently reproduced behavior.
4. Existing emulator behavior, including NXVM, DOSBox, vDos, and external VDMs.
5. Comments, issue reports, or unverified source reading.

Production behavior needs level 1, 2, or 3 evidence. Levels 4 and 5 can choose
the next experiment but cannot alone justify a compatibility claim.

## Required Record

Each nontrivial interrupt, device, timing, loader, or path-semantics decision
records the claim, precise boundary, evidence level, source identity,
reproduction procedure, alternatives, accepted behavior, limitation, and
regression test. Use `docs/evidence/template.md`.

Stop an investigation when its next discriminating experiment is known, the
question is nonblocking, evidence cannot distinguish alternatives, or resolving
it would cross a licensing or asset boundary.
