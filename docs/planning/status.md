# Project Status

## Current Work

**M5 T220 S3 active:** record the verified text-raster artifact for `3DAh`
display-enable and vertical-retrace observations derived only from elapsed
ticks. See [the task record](m5-t220-cga-text-raster.md) and the authoritative
[PC/AT hardware queue](m5-pcat-hardware-convergence.md).

**M5 T219 complete:** `core_machine_run()` is the sole elapsed-tick scheduler;
after each completed instruction it advances DMA, PIT, VADP, then PIC visibility
in a fixed order. No budget-exhausted or faulting instruction advances time.
Focused probes and the 55/55 current CTest matrix pass. See [the task
record](m5-t219-elapsed-tick-scheduler.md).

**M5 T217 complete:** `core_machine` solely owns elapsed guest ticks and the
frozen per-instruction clock parameter; run budgets, reset origin, result, and
observation semantics are covered by a focused smoke plus 53/53 current CTest
coverage. See [the task record](m5-t217-core-machine-time.md).

**M5 T218 complete:** the retained real-mode 8086/80186 CPU corpus now has
actual reset-vector instruction/tick probes for MOV, port I/O, INT, segment
prefix, HLT, and rejected 386 prefix behavior; 54/54 current CTest coverage
passes. See [the task record](m5-t218-real-mode-cpu-ticks.md).

**M5 T216 complete:** PIT, KBC, FDC, and HDC signal IRQs through the core PIC
source boundary. The PIC alone owns edge/level delivery, IRR/ISR, mask, EOI,
priority, and cascade; S4 also locks level-mode IRQ14 re-presentation through
slave/master double EOI. S5 additionally repairs rapid keyboard delivery, the
ROM `INT 16h` return-FLAGS frame, deterministic CGA text-status retrace, and
the normal-path `NEW CODE PATH` Console leak; real FDD `EDIT.COM` launch now
passes. The 52-test current matrix passes. See [the task
record](m5-t216-pic-irq-lifecycle.md).

**M5 T215 complete:** the unused generic firmware-interrupt portal API,
executor storage, dispatch branch, portal-only smoke, and closure gate are
deleted. Ordinary CPU `INT` followed by IVT delivery is the sole interrupt
path; 8086/80386 focused coverage and all 51 current CTest smokes pass. See
[the task record](m5-t215-remove-firmware-interrupt-portal.md).

**M5 T214 complete:** CPU/machine faults now return stable `STOP_FAULT` with
detail and diagnostic before the normal reset path. The VM session owns one
copied outcome for retained Console/debugger inspection; only explicit reset
clears it. The runner-level smoke and all 51 current CTest smokes pass. See
[the task record](m5-t214-cpu-fault-outcome.md).

**M5 T213 complete:** the default profile now owns a primary master-only ATA
PIO HDC at `1F0h`--`1F7h` and `3F6h` with IRQ14. ROM `INT 13h` performs PIO
through that controller, not the old F4/F5 helpers; the bounded HDD-image
smoke reaches the active partition VBR after two reads. See [the task
record](m5-t213-hdd-controller-profile.md).

**M5 T211 complete:** the default profile's F0
boot-failure portal is retired. The ROM reports an acknowledged failure through
its BDA POST status; the VM session runner consumes it and requests the sole
core stop. See [the task record](m5-t211-boot-failure-portal-retirement.md).

**M5 T210 complete:** the default profile's F1/F3
keyboard portals are retired. The retained input path is KBC -> IRQ1 -> ROM
`INT 09h` -> BDA -> ROM `INT 16h`; platform input still submits only host
events. See [the task record](m5-t210-keyboard-portal-retirement.md).

**M5 T209 complete:** the default profile's QDX
opcode override is replaced with frozen, ROM-origin-limited private `INT`
portals. Standard `INT`/IVT semantics remain intact outside a matching frozen
portal. The default machine remains `80386 + no FPU`; its focused strict-8086
diagnostic stops at a genuine 80186 instruction in the DOS image. See [the
task record](m5-t209-firmware-interrupt-portal.md).

**M5 T208 complete:** the default PC/AT profile is an immutable declaration
interpreted by composition. Its ROM mapping, firmware service order,
CMOS/FDC port configuration, and FDC IRQ/DMA route no longer reside in session
wiring or VM device implementation. The retained startup, Console, debugger,
and DOS-prompt paths are unchanged. See
[`pc-at-profile.md`](../architecture/pc-at-profile.md).

M5 remains open. `M5 Td S2` clarified documentation entry points only; it did
not close any M5 technical or verification item. The earlier documentation-only
commit labels `T208` and `T209` are reclassified as `M5 Td S1` and `M5 Td S2`;
they do not reserve numeric implementation task identifiers.
`M5 Td S3` records the ROI-ordered NXVM PC/AT workstream in `TODO.md`; it does
not activate an implementation task or change M5 closure state.
`M5 Td S4` establishes the required hardware-device verification template:
every admitted device task has S1 contract/port probe, S2 owner-local
implementation, and S3 DOS/system-image regression, with a fixed retained
NXVM matrix and bounded optional Bochs differential rules. It changes no guest
behavior and produces no artifact.
`M5 Td S5` closes the T212 governance record and reconciles stale active-work
and TODO wording with its completed artifact and current-gate evidence. It
changes no source or build output.
`M5 Td S6` establishes the authoritative ROI-ordered PC/AT hardware queue,
corrects the baseline to T216 S5, and reserves T217--T233 for time, devices,
storage, and display work; deferred CPU/FPU tasks are renumbered T234--T237.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M11 | Not started | [Roadmap](roadmap.md) |

## Latest Technical Baseline

M5 T216 retains NXVM behavior while making core PIC state the sole owner of
device IRQ delivery. T217 S1 now defines the next design boundary: one
core-owned guest-time model before device timing evolves. The recorded T216
developer artifact is `nxvm_0_5_0216.exe`; its historical evidence is
summarized in [M5 History](../history/m5.md).

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)

Completed task-level records are historical context, not current authority.
