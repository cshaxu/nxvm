# M5 T213: HDD Controller Profile And Port Contract

## Goal

Replace the ambiguous "HDD" claim with a bounded, profile-declared primary
ATA PIO controller contract. T213 does not yet claim IDE compatibility merely
because NXVM can boot through the existing in-memory CHS image and F4/F5
firmware portals.

## Contract

The default PC/AT profile declares one primary, master-only ATA PIO channel:

| Register group | Ports | Width | S1 status |
| --- | --- | --- | --- |
| data | `1F0h` | 16-bit | declared, not mapped |
| error/features through status/command | `1F1h`--`1F7h` | 8-bit | declared, not mapped |
| alternate status/device control | `3F6h` | 8-bit | declared, not mapped |
| interrupt | IRQ14 | n/a | declared, not asserted |
| DMA | none | n/a | explicitly unsupported |

`vm/machine` will own the controller's task-file registers, transfer phase,
status/error bits, data buffer, pending IRQ14, and reset behavior. The existing
`vm_machine_hdd` owns only the private image backing until a later bounded
backend refactor; it is not a guest-visible controller. `core/machine` retains
the port bus and PIC. `vm/composition` is the only layer that may create the
controller and bind its core port/PIC providers. The profile is immutable
topology only.

Default-ROM disk firmware owns the guest `INT 13h` calling convention, FLAGS,
AH error result, and BDA-facing BIOS behavior. It must reach storage through
the same composition-bound controller capability as port I/O, never by direct
image access or a second transfer state. The existing F4/F5 portals and
`qddisk` helpers remain temporary historical behavior through S1; removing
them is outside this subtask and requires the later ROM regression.

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

Make the admitted ROM `INT 13h` subset use the composition-bound HDC capability
and retire only the corresponding F4/F5 shortcut when a generated guest probe
has demonstrated the same controller state owner. The probe must issue direct
ATA port transactions and validate one DOS/system-image path with bounded
instruction, wall-time, and no-progress budgets. Run the retained FDD/HDD
boot, DOS prompt, Console, debugger/pause, and two-session matrix. Only S3 may
produce `nxvm_0_5_0213.exe`.

## S1 Verification

Applicable rules: immutable profile topology; core owns bus/PIC; VM owns its
machine-only controller and image backend; composition is the only integration
layer; firmware does not bypass controller state; no user-visible NXVM change.

Run `vm-hdc-port-smoke` for the focused contract and `current-gates-gcc` for
the retained matrix. S1 produces no artifact. Optional Bochs comparison is not
needed: this is project-owned declarative topology, not live device behavior.

## S1 Result

`vm-hdc-port-smoke` passes with
`M5:T213:S1:HDC:PORT:OK data=01F0 status=01F7 alt=03F6 irq=14 dma=none`.
It verifies that the frozen default profile declares the primary task file,
alternate-status/device-control port, 16-bit data path, 8-bit register path,
IRQ14, and explicit absence of DMA. No HDC port provider was added, so guest
HDD, ROM, and Console behavior remain unchanged.

`current-gates-gcc` rebuilds successfully and the retained current CTest matrix
passes 49/49, including the FDD/HDD session, DOS prompt/keyboard, Console,
debugger, and multi-session coverage. One preceding matrix attempt observed an
unmodified `vm-dos-keyboard-smoke` timeout; the identical executable then
passed directly with the same fixture and in the recorded 49/49 rerun. Treat
that as a test-stability observation, not evidence of a T213 controller change.
