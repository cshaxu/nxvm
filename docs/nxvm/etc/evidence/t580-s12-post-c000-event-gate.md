# T580 S12 — Post-C000 event source gate

## Question and method

S11 establishes that the selected real TFT BIOS returns from the admitted C000
video path to `F000:EA84`, but does not execute its F65535 panel table there.
This S temporarily used the existing Core trace provider to record the first
port event after that return. The diagnostic stopped only when such an event
appeared; the existing 13,000,000-instruction value was a containment ceiling,
not a success condition.

No source or firmware bytes were changed. The tracer was removed after the
probe, along with its temporary CTest.

## Two-width result

On both x64 and x86 the result was identical:

- the established C000 path returned through `F000:EA84`;
- no port read or write followed before the containment ceiling;
- execution remained in ROM at `F000:3F6C` (`8B C5 5D 89`); and
- no Core fault or delivered exception occurred.

The source-qualified next I/O owner is therefore still unknown. This is not a
missing ATA, F65535, panel, keyboard, power or PCMCIA receiver demonstrated by
the admitted path.

## Disposition

No production or persistent test behavior changes. Extending the ceiling,
inventing a timer event, or injecting a panel table would create an arbitrary
path and is not eligible as a PC110 qualification receiver. The later
qualification ledger must select a new semantic checkpoint or external
source-defined event before implementation resumes in these domains.
