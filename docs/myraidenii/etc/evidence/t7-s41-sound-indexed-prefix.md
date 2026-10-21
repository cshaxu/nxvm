# T7 S41 Sound Indexed-Prefix Receiver

## Boundary

The selected program previously reached a deterministic Sound Z80 DD/FD
receiver after the completed COP admission work.  A temporary category-only
diagnostic identified the existing indexed executor as the owner and was then
removed; it retained no guest instruction, address, byte or media payload.

## Local Correction

The existing executor already owned IX/IY pair operations and indexed-memory
forms, but it did not distinguish all direct IXH/IXL forms from ordinary
prefix-ignored instructions.  It also treated the self-pair add as if its
right operand were HL.

The correction remains in that sole executor:

- direct high/low register loads, transfers, increment/decrement and ALU
  sources select the active IX or IY half;
- indexed-memory forms retain ordinary H/L register semantics;
- `ADD IX/IY,IX/IY` selects the active index pair and uses the existing
  16-bit flag helper;
- an otherwise unaffected prefix executes through the sole ordinary executor,
  with only the prefix clock cost added.

There is no second decoder, Sound bus path, public interface, worker, queue or
ROM-derived table.  Existing unsupported instructions remain unsupported.

## Evidence Grade

The Zilog Z80 CPU User Manual listed in the product research index supplies
the DD/FD register-substitution and indexed-memory instruction contract.  The
local implementation is independently written against that finite contract;
this record makes no claim about the selected ROM's instruction stream beyond
the category-only receiver observation.

## Verification

- The existing Sound smoke adds synthetic, ROM-free coverage for ignored
  prefixes, direct index high/low forms, indexed memory update/load and
  self-pair addition.  It passes on x64 and x86.
- The complete MyRaidenII unit label passes 12/12 on both widths.
- The bounded selected-set Coin/Start probe completes identically at
  2,000,000 and 8,000,000 instructions on x64 and x86, with evolving,
  non-black frame samples.
- A current 26,000,000-instruction probe on both widths passes the former
  Sound receiver and reaches the same later, independent Sound receiver after
  25,416,482 instructions. That later receiver is not interpreted or changed
  here.

Temporary external staging is empty after each probe.  This closes the finite
Sound receiver only; it is neither gameplay nor release qualification.
