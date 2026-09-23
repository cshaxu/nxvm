# T437 S3 x87 CPU/FPU interface contract

## Scope

This record owns the CPU-side interface for the 8086/8087, 80186/8087,
80286/80287, and 80386/(80287 or 80387) pairings.  It deliberately does not
claim complete arithmetic semantics for an x87 implementation.

## Contract

- `ESC` (`D8` through `DF`) is a processor-extension command, not a scalar
  80386 CPU instruction-time entry.  The CPU decodes it, applies the existing
  `EM`/`TS` availability checks, and records a CPU-owned
  `CPU_FPU_COMMAND` transaction containing its opcode, ModR/M, and selected
  FPU profile.
- A configured `none` profile consumes ESC without a device handoff, retaining
  the pre-existing no-coprocessor compatibility behavior.  A configured FPU
  accepts only its physical CPU pairing: 8087 with 8086/80186; 80287 with
  80286/80386; 80387 with 80386.  An incompatible configured pair is diagnosed
  as the existing internal unsupported-model condition, never as `#UD`.
- BUSY and unmasked ERROR are separate states. `ESC` registers one Core-owned
  completion interval; every ordinary elapsed-tick publication advances that
  interval, and `WAIT` first retains the existing `#MF` path for ERROR before
  consuming only its remaining ticks. A `WAIT` instruction never clears BUSY
  merely because it decoded. BUSY is therefore never converted into a pending
  floating-point exception.
- 80287/80387 operand movement remains CPU-owned.  The MCP requests transfer
  through PEREQ; it is not a HOLD/HLDA bus owner.  This task records only the
  command handoff, leaving a later physical-cycle owner to publish requested
  PEREQ transfers against the ordinary CPU memory rules.
- A selected 80387 operation records its published MCP-clock interval without
  adding that interval to ESC retirement. Its current projection onto the
  unqualified Core elapsed axis is External-L2. Unknown or not-yet-semantic
  valid ESC forms use the named External-L2 proportional completion quantum;
  they no longer have an implicit L1 immediate-completion path.

## Derived 80387 interval inputs

The following are direct inputs for the selected semantic subset.  They are
MCP clock-count ranges, so they are External-L2 timing evidence under the
CPU-tier rule; they assume the data-sheet conditions (prefetched/decoded, no
bus wait states, no HOLD delay, no detected exception), and are not CPU
retirement ticks.

| ESC operation | range |
| --- | --- |
| `FNINIT` | 33 |
| `FLD m32real` | 9-18 |
| `FSTP m32real` | 25-43 |
| `FLDCW m16` | 19 |
| `FADD ST(0),ST(i)` | 12-26 |
| `FMUL ST(0),ST(i)` | 17-50 |
| `FSUB ST(0),ST(i)` | 15-29 |
| `FDIV ST(0),ST(i)` | 77-80 |

## Sources

- Intel, *Intel387 DX Math Coprocessor*, order 240448-005: sections 3.3.4 and 3.4.4
  (CPU/MCP synchronization, PEREQ, BUSY); section 5, pages 36-39 (operation ranges).
- Intel, *Intel 80286 and 80287 Programmer's Reference Manual*, section 3.12.1
  (ESC/WAIT interface).
- Intel, *Using the 80287 Numeric Processor Extension*, order 210760-002,
  chapter 6 (processor-extension data channel and PEREQ priority).

The source material was consulted as behavioral evidence only; no third-party
source, firmware, or guest media was imported into this repository.

## Completion model

The private FPU owner selects 85 Core ticks for pre-387 unclassified commands
and 28 for 80387 unclassified commands. These are not claims of hardware clock
identity: they are the 86Box-style External-L2 concurrent macro quanta, chosen
from the corresponding FADD class while a full FPU arithmetic/timing owner is
outside this CPU-interface scope. Selected 8087/80287 values use the 80287
table's typical values; selected 80387 ranges use their midpoint. Exact MCP
rows remain source facts, but their unverified conversion to the Core elapsed
axis remains L2. The sole Core scheduler publishes completion and exposes its
deadline; VM profiles neither clear BUSY nor supply a second clock.

## Closure record

S3's 809 ordinary 80386DX keys, including the ordinary r/m8 SETcc forms, and the separate CPU/FPU handoff proof passed
before S4 admission.  This retained record is the committed S3 evidence used
by the continuation-identifier verifier; it does not enlarge the x87 scope.
