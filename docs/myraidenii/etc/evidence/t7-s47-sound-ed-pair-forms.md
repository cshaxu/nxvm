# T7 S47 Sound ED Pair Forms

## Authority And Scope

The pinned original Zilog Z80 CPU User Manual UM008011-0816, recorded in the
[research register](../research/references.md), is the L3 authority for the
following complete ED families: `ADC HL,ss`, `SBC HL,ss`, `LD (nn),dd`, and
`LD dd,(nn)`.  The arithmetic forms take 15 T-states and define full
`S/Z/H/PV/N/C` results; the absolute pair transfers take 20 T-states.  No
guest ROM bytes, addresses, traces, external tables, or reference source code
are copied here.

S46's ledger originally misnamed the already-present `ED 52` implementation
as `ADC HL,DE`.  Its existing subtract argument proves it is correctly
`SBC HL,DE`; that documentation label is corrected as part of this receiver.

## One Local Decoder Path

The sole Sound ED decoder now owns all sixteen forms.  It selects the existing
BC/DE/HL/SP pair accessors from the opcode's pair field, uses the existing
memory read/write helpers for both transfer directions, and retains their
first-failure propagation.  The private 16-bit arithmetic helper now takes an
explicit full-flag mode: ordinary `ADD HL,ss` preserves its established
partial-flag contract, while ED `ADC` computes full result sign, zero and
overflow flags and includes carry in half-carry.  The existing subtract branch
already supplies the complete `SBC` flag contract.

No Sound map, Board route, device, clock owner, public header, Shared
component, queue, thread, product path, release artifact, or ROM probe changed.

## Compact Regression

One ROM-free Sound smoke covers all four pair selections for both arithmetic
families, both directions of all four absolute transfers, and explicit ADC
overflow/half-carry and SBC borrow/negative flag observations.  Existing
ordinary-add coverage remains in place.  The Board unsupported-decoder fixture
now uses still-unimplemented `ED 47`, preserving the real Board-to-Sound error
path rather than treating a newly supported form as invalid.

Focused Sound/Board tests and the complete twelve-test MyRaidenII unit label
pass on x64 and x86.  This is a finite CPU receiver only; it makes no selected
ROM startup, gameplay, audio, video, or silicon-timing claim.
