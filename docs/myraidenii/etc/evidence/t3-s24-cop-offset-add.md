# T3 S24: COP 2A05 Offset Add

## Boundary And Model

S24 implements only the source-grade L2 `2a05` receiver frozen in the
[S20 residual contract](t3-s20-residual-contract.md).  It changes the
MyRaidenII-local COP owner and its existing Board-unit executable.  It adds no
Shared source, public interface, new pending state, queue, thread, ROM input
or product artifact.

For command slot `i`, the COP reads an unsigned 16-bit delta from
`B1 + 1e + 4i`.  It reads the first unsigned word at `B0 + 6 + 4i`, widens the
sum to a dword, and writes it back at that same first address.  It then reads
the independent second word at `B0 + 1e + 4i`, applies the same delta, and
writes its widened dword at the second address.  Every read and write remains
on the established checked Core DMA cycle helper.  The existing command
admission and one pending-operation owner remain the only trigger/safe-point
path.

The result is intentionally functional L2: the evidence establishes the two
word inputs, dword output widths and ordering, but not silicon-exact timing.
Commands `1905`, `9100` and `9900`, Video modes and selected-set startup
remain explicit T3 obligations.

## Existing-Test Reuse And Proof

The existing `myraidenii.core/board-bus-smoke` executable remains the sole
receiver.  It uses the real V30 guest MMIO route for normal slots and the
already-owned direct COP safe-point only for fault-prefix inspection; no test
fixture or test-only production entry point was added.

The extended case proves all three slots, unsigned delta widening and carry,
both dword outputs, a `B1` input that overlaps the first output, rejection of
a second trigger while pending, reset/program re-admission, an overflowing
source address that leaves both outputs unchanged, and a late write whose
first 16-bit lane remains committed while the boundary-crossing lane fails.
The latter is the existing Core DMA prefix contract, not a rollback policy.

## Qualification

Focused `myraidenii.core/board-bus-smoke` passes in both stable Ninja trees.
After the final test strengthening, complete repository-only unit suites pass
687/687 with zero failures/skips: x64 in 106.24 seconds and x86 in 125.78
seconds.  The x64 suite ran outside the restricted sandbox because its
existing build-cleanup safety test legitimately creates a temporary junction
under `build/`; it passed as part of the complete aggregate.  No integration,
desktop/RDP, external-ROM or playable-game result is claimed.

Final `LastTest.log` SHA-256:

| Width | SHA-256 |
| --- | --- |
| x64 | `9E1212CDF6CD736532A0D8C8B36199B4F30434ACC7EBB9127FA7427ED827943D` |
| x86 | `43A6C6F3BAF97D01A8D8DC78B86E2A28B32917B6ACDD2285A7DDA18849BFD9CE` |

Frozen local source/test SHA-256:

| Path | SHA-256 |
| --- | --- |
| `src/myraidenii/core/cop.c` | `B4859F12ABDF45B52783713C097CB2B394A820D77B340D28C5374688F7A788F1` |
| `test/myraidenii/unit/core/board_bus_smoke.c` | `8841F0639C0867E15121ED20E94DE12E99006929C6F923A01891853DDD9061E5` |

The actual production delta is +33/-1; the existing owner test changes
+123/-2.  The test expansion is confined to the one Board route executable
and replaces the obsolete `2a05` refusal assertion.  MyRaidenII has no
runnable frontend or published EXE, so no binary deployment is required.

## Coordinator Acceptance

The actual-diff review confirms one new private handler at the existing COP
owner, one command classification entry and one dispatcher branch.  It keeps
all memory transport, command admission, pending ownership and cleanup on
their existing paths.  No Shared/public ABI, asset/media, external source,
host API, second queue or second execution path is introduced.  Documentation
governance and whitespace checks pass.  S24 closes while M1/T3 remains open
for the named unresolved macros, Video qualification and genuine selected-set
startup.
