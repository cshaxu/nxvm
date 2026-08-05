# M5 T213: HDD Controller Profile And Port Contract

## Goal

Replace the ambiguous "HDD" claim with a bounded, profile-declared primary
ATA PIO controller contract. T213 does not claim full IDE compatibility: it
admits a real port-and-IRQ path for the retained CHS image backend.

## Contract

The default PC/AT profile declares one primary, master-only ATA PIO channel:

| Register group | Ports | Width | S1 status |
| --- | --- | --- | --- |
| data | `1F0h` | 16-bit | mapped |
| error/features through status/command | `1F1h`--`1F7h` | 8-bit | mapped |
| alternate status/device control | `3F6h` | 8-bit | mapped |
| interrupt | IRQ14 | n/a | asserted through core PIC |
| DMA | none | n/a | explicitly unsupported |

`vm/machine` will own the controller's task-file registers, transfer phase,
status/error bits, data buffer, pending IRQ14, and reset behavior. The existing
`vm_machine_hdd` owns only the private image backing until a later bounded
backend refactor; it is not a guest-visible controller. `core/machine` retains
the port bus and PIC. `vm/composition` is the only layer that may create the
controller and bind its core port/PIC providers. The profile is immutable
topology only.

Default-ROM disk firmware owns the guest `INT 13h` calling convention, FLAGS,
AH error result, and BDA-facing BIOS behavior. Its read/write service programs
the same HDC ports as guest code; it never accesses the image directly. The
generic block slot is geometry-only and borrows the HDC, so BDA geometry has
the same device owner. T213 deletes the former F4/F5 portal and `qddisk`
helpers.

S2 is limited to reset, register reads/writes, `IDENTIFY DEVICE`, and CHS PIO
read/write with defined `BSY`, `DRQ`, `DRDY`, `ERR`, error, IRQ14, and
status-versus-alternate-status acknowledgement semantics. It excludes a
secondary channel, slave device, LBA, bus-master DMA, ATAPI, timing emulation,
write cache, and host-path policy. The supported image backend remains
session-owned and file-loaded under existing NXVM media policy.

## Breakdown

### S1: Profile Contract And Port Probe

Add the immutable primary-channel declaration and a profile-owned probe. The
probe checks every task-file port, data/register widths, IRQ14, and the explicit
no-DMA boundary. It proves only topology: no HDC port provider is installed and
no guest I/O behavior changes in S1.

Success marker: `M5:T213:S1:HDC:PORT:OK`.

### S2: VM HDC State Machine And Core Binding

Introduce one `vm_machine_hdc` object owned by the VM session. Composition
constructs it from the frozen profile declaration, binds its single port
provider to the core bus, connects IRQ14 through the core PIC contract, and
uses `vm_machine_hdd` only as its backend. Implement exactly the S2 command
subset and extend the port smoke with reset, task-file, DRQ/data-transfer,
status/alternate-status, IRQ acknowledgement, and no-media/unsupported-command
cases. Do not make profile firmware or platform code mutate controller state.

Stop for a second image or transfer state, direct firmware image access, a
second IRQ path, a core-to-VM dependency, unbounded tracing, or a change to
retained Console/debugger/startup UX.

### S3: BIOS Boundary And System-Image Regression

Make the admitted ROM `INT 13h` subset program the composition-bound HDC ports
and retire the corresponding F4/F5 shortcut. `vm-hdc-hdd-boot-smoke` runs the
owner-provided HDD image with a 500,000-instruction budget, observes two `20h`
reads, and verifies that the MBR reaches the active partition VBR at `0000:7C00`.
The probe uses the VBR jump/OEM identity and signature, because normal VBR
execution may update writable BPB fields. The retained FDD/HDD boot, DOS
prompt, Console, debugger/pause, and two-session matrix must pass. Only S3 may
produce `nxvm_0_5_0213.exe`.

## S1 Verification

Applicable rules: immutable profile topology; core owns bus/PIC; VM owns its
machine-only controller and image backend; composition is the only integration
layer; firmware does not bypass controller state; no user-visible NXVM change.

Run `vm-hdc-port-smoke` for the focused contract and `current-gates-gcc` for
the retained matrix. S1 produces no artifact. Optional Bochs comparison is not
needed: this is project-owned declarative topology, not live device behavior.

## Result

S1 verified the frozen profile declaration. S2 owns task-file registers,
status/error, 512-byte transfer buffer, reset, and pending IRQ14 in one
session-owned `vm_machine_hdc`; core owns only the port bus and PIC. Its port
probe passes with `M5:T213:S2:HDC:PORT:OK identify=0040 pio=A55A irq=14` and
covers `ECh`, `20h`, `30h`, invalid command, no-media, status acknowledgement,
alternate status, and SRST.

S3 removes `firmware_portal.*` and `qddisk.*`. The ROM `INT 13h` implementation
uses the declared ATA task file and PIO data port. Its HDD-image regression
passes with `M5:T213:S3:HDC:SYSTEM:OK command=20 reads=2 instructions=1241`.
`current-gates-gcc` passes its architecture gates and the current CTest matrix
passes 50/50. The task artifact is `build/output/nxvm_0_5_0213.exe`, SHA-256
`335466E503226796AE80499A73D79C2D0718E74907AD18511164C988E77A4158`.
