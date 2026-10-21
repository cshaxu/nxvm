# T7 S45 Sound Accumulator And Carry Forms

## Scope And Authority

S45 completes the finite base accumulator/carry-flag group admitted by S43 and
selected by S44: `RLCA`, `RLA`, `RRCA`, `RRA`, `DAA`, `CPL`, `SCF`, and `CCF`.
The pinned original Zilog Z80 CPU User Manual UM008011-0816, recorded in the
[research register](../research/references.md), is the L3 authority for their
operations, flag effects and four-T-state execution.  No reference source,
table, guest byte, address, trace, ROM or media data is copied into this
repository.

## One Executor And One Flag Owner

The existing Sound base decoder remains the sole execution owner.  A private
rotate helper consolidates the four common accumulator-rotate effects: preserve
`S`, `Z` and `P/V`; clear `H` and `N`; replace `C` with the outgoing bit.  It
does not replace the existing full logic-flag helper, whose contract is
different.

`DAA` retains the existing arithmetic flags as its input, applies the manual's
add/subtract adjustment rules, and then owns its resulting `S`, `Z`, `P/V`,
`H`, `N` and `C` values.  `CPL`, `SCF`, and `CCF` remain direct base-decoder
forms because their flag effects are distinct and small.  No Board map,
device, timing clock, public interface, Shared component, queue, thread, or
product path changed.

## Regression And Result

The existing ROM-free Sound smoke now executes all eight forms in one compact
program: rotate result/carry and preserved zero checks, decimal adjustment for
add and subtract cases, and flag interaction through `CPL`, `SCF` and `CCF`.
It reruns after reset.  The existing Board smoke continues to prove that the
real Board path propagates a decoder category; its synthetic unsupported input
was changed from now-supported `DAA` to an unimplemented extended form.

Focused Sound/Board tests and the complete twelve-test MyRaidenII unit label
pass on x64 and x86.  One persistent-background selected-set probe per width
continues to report the same finite `DECODER` category.  This is a receiver
transfer only: it neither identifies a guest instruction nor claims startup,
title, audio, gameplay or silicon-level timing.  External staging was empty
after each probe and no probe output was retained.

## Receiving Work

The remaining receiver is the next complete source-authorized decoder family.
It must be selected from the static inventory and primary authority, not from
the category alone.  ED, index, CB and Sound-map expansion remain outside S45.
