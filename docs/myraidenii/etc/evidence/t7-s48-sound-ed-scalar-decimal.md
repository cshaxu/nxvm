# T7 S48 Sound ED Scalar And Decimal Forms

## Authority And Scope

The pinned original Zilog Z80 CPU User Manual UM008011-0816, recorded in the
[research register](../research/references.md), is the L3 authority for the
eight legal `NEG` aliases and the `RRD`/`RLD` nibble exchanges.  It defines the
alias operation and full subtraction flags, the decimal exchanges' accumulator,
memory and carry-preservation rules, and their 8/18-T-state nominal durations.
No guest ROM bytes, addresses, traces, external tables, or third-party source
are copied into the repository.

## One Existing Owner Path

The sole Sound ED executor handles all ten encodings.  `NEG` directly reuses
the existing full-flag byte subtraction helper.  One local decimal helper reads
`(HL)`, computes both replacement nibbles without committing state, writes via
the existing Sound map owner, then commits A and reuses logic flags while
retaining the prior carry.  A failed map read/write therefore does not create a
second map path or partially update A/F.

The omitted accumulator/refresh transfers still require a separate review of
I/R refresh-state ownership.  Port-register, block and other ED families remain
explicitly outside this bounded receiver.

## Regression And Result

The existing ROM-free Sound smoke executes every `NEG` alias and verifies its
result plus complete flags.  Its two decimal cases verify both directions,
accumulator and memory nibble results, and preserved carry through captured AF.
The existing unsupported fixture retains a documented undefined ED opcode, and
the same test now also sends `RRD` through an unowned HL map address to prove
the actual form propagates the existing map category.

Focused Sound/Board units and the full twelve-test MyRaidenII unit label pass
on x64 and x86.  No release binary, selected-ROM probe, gameplay, video, audio
or silicon-level timing qualification is claimed by this CPU-local batch.
