# M5 T253: ATA/IDE PIO Compatibility Extension

**Status:** complete.

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
- The existing final-address behavior remains unchanged: after the last sector,
  task-file address fields identify the final loaded sector rather than a new
  next-sector address.
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

**S2 result:** `vm_machine_hdc_complete_data_sector()` is the sole completed-
sector transition. It decrements both private and visible counts after a
successful PIO read or write sector. The retained port smoke verifies LBA
two-sector read/write progression, count zero, intermediate IRQ14, status
acknowledgement, and data round-trip.

**S2 marker:** `M5:T253:S2:ATA-PIO-PROGRESS:PORT:OK`.

## S3: Verify

`vm-ata-pio-dos-smoke` clones both owner-provided system images, installs an
owner-built `ATA253.COM` into the temporary FDD, boots DOS normally, submits the
command through KBC ingress, and lets the guest perform the two-sector PIO
write/read round-trip through `1F0h`--`1F7h`. The COM fixture compares both
guest-read words and writes its own text-mode success marker; it receives no
host RAM, BIOS, or firmware shortcut. The source HDD image is never modified.

The current media classification now models all dual-image smokes as one list,
so this fixture is registered once with FDD and HDD arguments. The retained
HDC port smoke covers the controller-level count/IRQ progression and existing
failure paths. `current-gates-gcc` passed all static gates and **87/87** CTest
cases, including FDD/HDD boot, Console/debugger regressions, and the new DOS
fixture. `current-gcc` produced `build/output/nxvm_0_5_0252.exe`, SHA-256:

`B565B73D1C1E5EB77C04549944AB28EA2CF6DE59AB2FD2B087D957CAEB020A87`.

**S3 marker:** `M5:T253:S3:ATA-PIO:DOS:OK`.

## Stop Conditions

Stop if this needs direct guest-RAM mutation, a second controller/execution
path, a core-to-VM dependency, undefined guest-visible timing, or a product
experience change.
