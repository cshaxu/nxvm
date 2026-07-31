# Firmware And NXVM Product Requirements

## Status

M4 design record. This document assigns the bootable whole-PC path before M5
implementation. It does not change the retained baseline or add a process CLI
to `nxvm.exe`.

## T1: Ownership And Migration

`nxvm.full_pc` remains the M3 supported adapter descriptor. Its M5 target is
the canonical `nxvm.machine.pc_at_builtin` machine profile defined in
`profiles.md`. Its source of behavior is the M1 baseline, but its target
ownership is the table below. A target owner may use a module-local adapter
during M5; no target module may reach baseline globals after its migration task
closes.

| Baseline units | Target owner | Responsibility | M5 order and regression |
| --- | --- | --- | --- |
| `vcpu`, `vram`, `vport` | `core` | CPU execution, RAM, port/memory dispatch and A20 behavior. | Already represented by M3 contracts; preserve core smoke tests and reset-vector path. |
| `vpic`, `vpit`, `vdma`, `vkbc`, `qdx` | `core` optional devices | Machine-neutral PC-compatible interrupt, timer, DMA, keyboard-controller and text/CGA device behavior. They expose ports, IRQ/DRQ, and immutable snapshots only. | M5 T3 establishes the device contracts after CPU/profile gates; M1 FDD/HDD boot and presentation regressions remain required. |
| `vbios`, `vcmos`, `vvadp` | `firmware/pc_at` | POST, ROM/BDA construction, CMOS-backed firmware configuration, and BIOS interrupt services including display routing. | M5 T2 replaces string-assembled registration with typed firmware registrations and proves reset `F000:FFF0`. |
| `vfdc`, `vhdc`, `vfdd`, `vhdd` | `products/nxvm/pc_at` | Full-PC controller/device composition plus removable and fixed disk policy. Media selection is a product concern, not a firmware or core concern. | M5 T3 proves FDD insertion and HDD connection against the existing fixture identities. |
| `console`, `machine.c`, `debug`, `vdebug`, `xasm32` | `products/nxvm` plus debug adapter | Interactive NXVM Console, command parsing, debugger presentation, and product lifecycle policy. Debug inspection uses the M3 synchronized debug boundary. | M5 T4 preserves the documented Console commands and does not add a process CLI. |
| `platform/win32/*`, retained `platform/linux/*` | `platform` | Host threads, console/window presentation, keyboard event collection, clocks, block-file I/O, and logging. Host callbacks never mutate guest state directly. | M5 T4 uses queued commands/events and copied snapshots; Windows whole-PC regression is mandatory. |
| `device.c`, `vmachine.c`, baseline global connection helpers | temporary baseline adapter | Evidence-preserving bridge only while a target owner is migrated. | M5 T5 removes each migrated path from the active composition; the adapter remains only for uncovered baseline behavior and is deleted only after equivalent regression coverage exists. |

## Composition Rules

`runtime` creates the Machine and selects `nxvm.machine.pc_at_builtin`. The
NXVM product then selects its media policy, requests the PC/AT device package,
and composes the built-in firmware provider. Firmware registers typed POST,
ROM, and interrupt services; it does not open host files, create windows, or
parse Console input.

```text
products/nxvm Console and media policy
                 |
runtime -> core Machine <- pc_at optional devices
                 |
             firmware/pc_at
                 |
       abstract host capabilities
                 |
              platform
```

`ntvdm64.dos_minimal` neither selects the PC/AT device package nor imports
NXVM firmware. M6 may select a separately specified firmware subset only by a
new profile decision.

## M1 Regression Ownership

| M1 observation | Future owner | Required M5 evidence |
| --- | --- | --- |
| Reset reaches `F000:FFF0` | `firmware/pc_at` plus core reset | reset-vector trace and focused firmware reset test |
| FDD boot reaches the recorded DOS checkpoint | `products/nxvm/pc_at` plus firmware INT 13h | fixture hash, insertion marker, bounded trace |
| HDD boot reaches the recorded DOS checkpoint | `products/nxvm/pc_at` plus firmware INT 13h | fixture hash, connection marker, bounded trace |
| Console `exit` returns 0 | `products/nxvm` | scripted Console lifecycle test |
| Console/window guest presentation receives input | platform adapter plus core device snapshots/events | deterministic adapter test and Windows manual smoke |

## Prohibited Shortcuts

- Firmware cannot call Win32, Linux, Console, or block-file APIs directly.
- Product Console cannot write Machine state, device globals, or display memory
  directly; it submits typed commands at the runtime command boundary.
- Core cannot name PC/AT firmware, disk images, DOS, or a host UI.
- M5 cannot absorb a DOS ABI, `ntvdm64 run`, drive mapping, or any M8 policy.

## T2: Firmware V1 Contract

Firmware is a session-owned package composed by `runtime`; it is neither a
global singleton nor a core dependency. The M5 implementation creates the
following module-local contract headers in `src/firmware/` and keeps their
implementation private to the package.

```c
typedef struct nxvm_firmware nxvm_firmware;
typedef struct nxvm_host_capabilities nxvm_host_capabilities;

typedef enum {
    NXVM_FIRMWARE_SERVICE_POST,
    NXVM_FIRMWARE_SERVICE_ROM,
    NXVM_FIRMWARE_SERVICE_INTERRUPT
} nxvm_firmware_service_kind;

typedef struct {
    const char *id;
    nxvm_firmware_service_kind kind;
    unsigned int order;
    unsigned int vector; /* 0 through 255 for INTERRUPT; otherwise unused */
} nxvm_firmware_service_descriptor;

typedef enum {
    NXVM_FIRMWARE_OK,
    NXVM_FIRMWARE_INVALID,
    NXVM_FIRMWARE_DUPLICATE,
    NXVM_FIRMWARE_UNSUPPORTED,
    NXVM_FIRMWARE_MACHINE_ERROR,
    NXVM_FIRMWARE_HOST_ERROR
} nxvm_firmware_status;
```

`firmware_register_service_v1` is available only while the session is being
composed. It rejects a duplicate service id, an interrupt vector claimed by a
different enabled firmware service, an invalid ordering key, or a service that
is unavailable in the selected profile. `runtime` freezes the registry before
the first reset and destroys it after the Machine has stopped and all product
adapters have detached.

The service entries are typed metadata plus an implementation-private callback.
The callback receives the Machine contract, its owning firmware context, and
only the declared abstract host capabilities. It cannot receive a Win32 handle,
host pathname, Console handle, window handle, or a mutable core pointer.

### Lifecycle

1. `runtime` creates the core Machine and optional core devices.
2. `products/nxvm` selects boot target and media policy, then composes the
   PC/AT firmware package and its declared services.
3. At reset, core devices reset first; firmware builds ROM/BDA state, installs
   interrupt vectors, runs ordered POST services, and transfers the reset path
   to `F000:FFF0`.
4. During execution, an interrupt service runs only at the Machine execution
   boundary. A service may enqueue a declared device operation but cannot start
   a host thread or re-enter `machine_run_v1`.
5. Stop/fault detaches product presentation before firmware teardown. Firmware
   then unregisters services, releases its session allocations, and emits its
   final trace event.

The M5 PC/AT package provides at least these entries: `pc_at.rom`,
`pc_at.post`, `bios.int10.video`, `bios.int13.disk`, `bios.int16.keyboard`,
and `bios.int1a.clock`. Unsupported vectors use the Machine's unhandled-vector
result; they never fall through to a host API or DOS service.

### ROM And Interrupt Semantics

ROM construction is deterministic for a fixed profile, device inventory, and
boot selection. The implementation may use preassembled bytes or a bounded
project-owned assembler during composition, but it must not retain the
baseline's mutable string pointer tables as the public registry. Firmware owns
the BDA/ROM layout, service vector installation, POST order, and firmware
status codes. Core owns RAM range checks, vector installation mechanics, port
dispatch, and CPU execution.

Each interrupt entry declares whether it is implemented by a guest-ROM stub or
a synchronized host-side handler. Both forms have the same registry identity,
trace schema, error model, and register-preservation contract. M5 initially
preserves observed behavior; expanding an interrupt's compatibility requires a
later corpus requirement.

### Failure, Host Capability, And Trace Rules

- A composition failure occurs before reset and reports a stable firmware
  status; it leaves no partially installed service.
- A POST or ROM build failure faults the session before guest execution and
  records service id, stable detail code, and reset phase.
- A runtime service failure returns either a documented BIOS status/register
  result or a Machine fault. The choice is part of that service's contract.
- Clock, block-device, and display/input use only declared host capabilities.
  Product code selects the providers; firmware never opens a host image or UI.
- Trace emits `firmware.compose`, `firmware.reset`, `firmware.post`,
  `firmware.interrupt`, and `firmware.fault` records with copied scalar data.
  Trace observers cannot retain a firmware or Machine pointer.

## T3: NXVM Console Product Contract

### No Process CLI

Launching `nxvm.exe` enters the interactive NXVM Console. It retains the
whole-machine command workflow rather than defining `nxvm run`, program-path
arguments, host-drive mapping, or any ntvdm64 option. Automation is outside
the M4/M5 compatibility contract.

### Console Commands

Commands are case-insensitive and whitespace-tokenized as in the baseline.
M5 preserves the following documented surface; an invalid form prints command
help and changes no session state.

| Command | Ready/stopped state | Running state | Owner |
| --- | --- | --- | --- |
| `help [command]` | show product help | unavailable while Console display owns input; otherwise read-only | `products/nxvm` |
| `info` | show profile, device, firmware, media, display, and lifecycle summary | read-only only at a synchronized command boundary | `products/nxvm` queries runtime snapshots |
| `device ram <KB>` | configure before the next reset | reject | product configuration |
| `device display console|window` | select next guest presentation mode | reject | product plus platform capability selection |
| `device fdd create|insert <file>|remove [file]` | change removable-media configuration | reject | nxvm media policy and block capability |
| `device hdd create [cyl <n>]|connect <file>|disconnect [file]` | change fixed-media configuration | reject | nxvm media policy and block capability |
| `set boot fdd|hdd` | select next firmware boot target | reject | nxvm product configuration |
| `start` | reset then run selected profile | reject | runtime lifecycle |
| `reset` | reset selected profile and remain ready | reject | runtime lifecycle plus firmware |
| `stop` | report no active guest | request an asynchronous stop and return only after a stopped boundary | runtime lifecycle |
| `resume` | resume only a debugger-paused session | reject otherwise | runtime lifecycle |
| `debug` | enter the developer debugger only at a paused boundary | request stop, then enter only after paused | debug adapter |
| `record start <file>|stop` | configure or close trace recording | reject | debug adapter and logging capability |
| `exit` | finalize session and return status 0 | reject until stopped | `products/nxvm` |

The baseline's undocumented `test` and `mode` aliases may remain developer
compatibility aliases during M5, but are not part of the stable product
contract. `debug32` is not a required M5 command because it is not currently
dispatched by the documented Console path.

### Console State And Presentation

```text
console-ready -> guest-running-console -> console-ready
      |                 |
      |                 +-> stopped/faulted -> console-ready
      +-> guest-running-window
      +-> debugger-paused -> guest-running-console|guest-running-window
      +-> finalized
```

The diagram names command availability, not a second Machine lifecycle. The
runtime remains the sole lifecycle owner. In `console` display mode, guest text
and keyboard own the Console until the guest stops; product commands do not
compete with guest input. In `window` display mode, the window owns guest
presentation/input and the NXVM Console can receive only lifecycle-safe
commands through the synchronized command registry. Closing the guest window
requests the same controlled stop path as `stop`; it does not destroy Machine
state from the platform thread.

Platform code supplies queued keyboard events and consumes copied display
snapshots. It cannot inspect Console parsing state or mutate the Machine. The
Console never calls a baseline `deviceConnect*` function in the M5 target; it
submits a typed product command to runtime and reports the resulting stable
status.

### Debug And Artifact Identity

The developer debugger is an NXVM product adapter over the M3 debug/command
registry. It owns presentation and command parsing but not guest state. At a
paused boundary it must support the existing required inspection path: register
view, disassembly, memory inspection, breakpoints, finite step/continue, trace
control, reset, and quit. A debugger failure is reported to the Console and
does not fault an otherwise healthy guest.

M5 produces `nxvm.exe` as the first-class whole-machine artifact. Until the
existing version-cutover rule is explicitly revised, developer task artifacts
retain the pre-cutover Virtual Machine banner with task suffixes. The artifact
contains no guest media or Microsoft binary and its verification record names
the exact source commit, SHA-256, Console banner, and FDD/HDD regression.
