# M5 T253: ATA/IDE PIO Compatibility Extension

**Status:** S1 active.

## Goal

Extend the existing primary-master ATA PIO controller only through one declared
feature matrix entry and a failing corpus. IDE DMA, ATAPI, LBA48, cache,
secondary channels, slave devices, extra topology, and host-clock timing stay
out of scope.

## S1: Task-File Progress Contract

### Baseline

`vm/machine/hdc` already owns primary-master CHS/LBA28 PIO read/write,
`count=0 -> 256`, IDENTIFY DEVICE, SRST, status/error, and IRQ14. Its image
backend remains VM-owned; core owns only PIC and guest memory.

### Admitted Gap

For multi-sector READ SECTORS (`20h`) and WRITE SECTORS (`30h`), the controller
advances CHS/LBA address fields but leaves the task-file sector-count register
at its original value. Guest software polling the task file cannot observe
remaining-sector progress. T253 admits only decrementing that register after
each successfully completed sector, including the final transition to zero.

### Contract

- Owner: `vm/machine/hdc`; no BIOS, firmware, host-file, product, or core
  shortcut.
- On each completed 512-byte PIO sector, decrement both controller-private
  remaining count and visible sector-count register exactly once.
- A failed next-sector load/store retains the count representing unfinished
  work, reports the existing error/status/IRQ behavior, and does not advance a
  second time.
- Port probe covers CHS and LBA multi-sector read/write progression, final zero,
  status-read IRQ acknowledgement, and an out-of-range next-sector failure.
- DOS fixture performs an owner-built two-sector PIO round-trip through ports;
  it does not receive a host RAM or BIOS shortcut.

**S1 marker:** `M5:T253:S1:ATA-PIO-PROGRESS-CONTRACT:OK`.

## S2: Implement

Implement this task-file progression solely in the controller owner and add the
focused port probe. Do not admit any other command or transport.

## S3: Verify

Add the DOS/system-image fixture, retain Console/debugger, FDD/HDD boot and
current GCC/CTest coverage, then produce `nxvm_0_5_0252.exe` and its SHA-256.

## Stop Conditions

Stop if this needs direct guest-RAM mutation, a second controller/execution
path, a core-to-VM dependency, undefined guest-visible timing, or a product
experience change.
