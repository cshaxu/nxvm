# T7 S40 COP Command Admission

## Observed Receivers

S39 stopped both host widths at 24,517,538 instructions when the main V30
wrote `0x3bb0` to the COP command port at physical `0x500`.  The reported
linear address was the CPU program counter, not the failed target address.
The Core physical fault code identified the rejected device write.

After the admission gate was removed, a bounded, test-only Core trace reached
the next receiver at the same mapped port.  It identified `0x138e`, a missing
member of the local angle-from-vector command family.  The trace retained only
event metadata during the run and was deleted with its external output after
the aggregate result was recorded; no ROM instruction, byte, or media payload
is retained.

## Local Defect And Model

The local COP classifier already identifies `0x3bb0` as a supported numeric
command.  A second private condition nevertheless required a fully written
eight-word upload slot plus six metadata bytes whose trigger equalled the
command.  The local model never executed those uploaded words; it dispatched
the classified command through its own finite L2 numeric implementation.

This packet removes that unused mirror and its admission gate.  A recognised,
aligned command word now enters the existing pending-operation path directly.
It also completes the already implemented angle family with `0x138e` and
`0x330e`: both use the existing numeric calculation; as with the pre-existing
paired forms, only the result-store variant writes the established byte result.
Unsupported words and requests while any operation is pending remain invalid.
Configuration registers remain ordinary COP configuration storage; their
contents do not claim to model physical COP program RAM.

The same existing fill owner had one independent local failure contract error:
a recognised generic-fill context with a nonzero destination was rejected even
though that form performs no transfer. It now completes successfully without
changing memory. The existing Board smoke drives that real context through the
normal trigger and verifies that its sentinel remains intact; no second DMA
path or generic no-op layer was added.

## L2 Reference Boundary

Inspection of the published MAME Seibu COP implementation at
`src/mame/seibu/seibucop.cpp` and `seibucop_cmd.ipp`, together with the
independently inspected Raiden II MiSTer L2 command-family description, found
recognised command words directly dispatched by the command-write owner and
identified the four angle-family forms as two result-store dispositions.  The
trigger-table lookup is not an execution-admission precondition.  This is an
L2 behavioural comparison only: no external code, tables or ROM data were
copied, and this packet does not claim physical microcode fidelity.

## Verification

- The existing Board smoke proves recognised commands no longer require a
  program-table upload, proves a second command is rejected while the first is
  pending, retains unsupported-command rejection, and covers each added
  angle-family result-store disposition.
- A bounded x64 selected-set probe reached 25,000,000 instructions without a
  CPU, COP or control fault.  It produced four non-identical, non-black frame
  samples.  This crosses the former 24,517,538-instruction receiver and the
  later `0x138e` receiver, but is not yet the packet's required full dual-width
  qualification.

The later 100,000,000-instruction qualification attempts on both widths reach
a deterministic, separate Sound safe-point failure before their budget.  A
temporary non-payload owner/category diagnostic located it in the Sound Z80
DD/FD indexed executor, not in COP admission, the Board control window, or
the host-width path.  That diagnostic and its transient staging were deleted.
S40 therefore transfers the finite Sound receiver to its own packet rather
than guessing a COP repair outside this packet's boundary.  The Probe now
prints `FAULT` and returns nonzero for such a terminal Core fault; it no
longer labels a failed qualification `OK`.

All temporary ROM staging, trace output and scheduled diagnostic work were
removed.  No protected guest payload is retained in this record.
