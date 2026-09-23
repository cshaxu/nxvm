# T498 S5 80286 Boot Predecessor Ledger

`M5:T498:S5:I286-BOOT-PREDECESSOR:RECORDED`

## Authority

Intel's original 80286 instruction/exception specification remains normative:
an operand-size prefix is not a valid 80286 instruction prefix and must deliver
the invalid-opcode exception.  External emulator behavior is corroboration
only and cannot broaden that architectural boundary.

## Same-Input Variant Result

The same owner-managed 1.2-MB DOS control input was replayed in Release Turbo
mode on both IBM 5170 Model 339/80286 and `default-at`/80286.  Each reaches
the same active driver component, after successful floppy command completion,
and delivers the same first invalid-opcode exception at its 80386-only
operand-size-prefix instruction.  The corresponding `default-at`/80386 replay
reaches a DOS prompt.

This rules out the selected FDD geometry, FDC/DMA result lifecycle, media-byte
provider, reset mapping, and 80286 exception delivery as the cause of the
control input's terminal result.  The input's active component, not MS-DOS as
a whole, requires an 80386.

## Owner Disposition

No Core repair is justified: accepting that prefix on a 80286 would violate
the Intel boundary and break CPU identity.  The current input remains an
explicit external-input disposition for the IBM 5170 native 1.2-MB row.  A
future owner-supplied 80286-compatible 1.2-MB boot input may be replayed
through the existing sole FDD/FDC/Core path without a new profile branch.
