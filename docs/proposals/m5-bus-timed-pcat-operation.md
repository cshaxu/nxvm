# Bus-Timed PC/AT Operation

## Purpose

Extend the accepted T354 transaction owner from synchronous selected
publication to the IBM PC/AT 5170 80286 baseline's bounded bus-availability
model. This candidate begins only after instruction-timed execution supplies a
guest-time contract. T366 first locks the exact baseline and corpus, then
consumes T365's non-admission finding by implementing the selected board NMI
source lifecycle, before it allocates bus availability.

## Required scope

**Profile-lock stage.** Before any timing allocation, record the exact IBM
PC/AT 5170 system-board revision and manual edition, CPU clock, motherboard
RAM, ROM configuration, selected diskette-drive/FDC DMA-channel-2 path, every
selected ISA adapter, and a project-owned firmware/DOS probe corpus. The
locked configuration is IBM PC/AT 5170 Model 339 with the Type 3 system board:
an 8 MHz 80286, Rev.3 BIOS slots dated 1985-11-15, 512 KB planar RAM, 101-key
enhanced keyboard, IBM CGA, and one TEAC FD-235HF-A529 3.5-inch 1.44 MB
field-upgrade drive. It has no fixed disk and no 1 MB expansion. The TEAC
drive is a compatible post-market upgrade, not an IBM factory Model 339
component or option. An unselected adapter cannot supply a timing value.

**NMI-source stage.** After that lock, select one documented board parity or
I/O-channel-check source and define its producer, latch/clear operation,
CMOS-mask interaction, reset/finalize ordering, deterministic visibility, and
real/protected/ordinary-VM86 delivery proof. Do not allocate availability until
this truthful interrupt-source set is established; RTC, CMOS, PIC and CPU
delivery state remain non-producers.

**Bus stage.** Define one owner
for CPU memory/I/O availability, named memory and port wait states, DMA
HOLD/HLDA-style exclusion, and selected device ready/DRQ/IRQ visibility.
Use the existing transaction begin/commit/cancel lifecycle rather than a
parallel scheduler or test-only mirror state.  Map validation, reservation,
commit, cancellation, reset and trace behavior across CPU, DMA, PIC, and the
selected FDC; distinguish logical interrupt acknowledgement from physical
INTA. The current ATA/HDC is not a selected 5170 component. A future IBM
fixed-disk route requires its own MFM/ST-506 controller-and-drive admission;
it must not reuse ATA ports, media, timing, or terminology.

## Non-goals and stop conditions

No universal chipset, electrical waveform, speculative latency, arbitrary
device timing, cache/prefetch, host-time coupling, or broad peripheral
implementation. Stop if a selected device requires a separate command/model
contract, the exact baseline cannot be locked, the NMI source lacks one
producer/clear path, or the PC/AT source cannot identify the required
wait/ownership behavior.

## Evidence standard

Require exact profile/corpus provenance; a source/caller/write and timing-state
ledger; NMI producer/latch/clear/mask/reset/delivery proof; contention and
reset proof; transaction trace proof; project-owned firmware/DOS consumers;
direct current-gate regression; and precise transfers. It consumes instruction
timing and is the prerequisite for the 5170 phase and closure candidates, not
a cycle claim.
