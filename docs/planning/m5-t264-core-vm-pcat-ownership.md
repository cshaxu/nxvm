# M5 T264: Core/VM PC/AT Ownership Closure

## Scope And Rules

T264 closes ownership and execution-path debt in the default PC/AT assembly.
It does not add hardware features, profiles, mantle, DOS, or VDM behavior, and
it must retain the NXVM Console, debugger, boot flow, FDD/HDD behavior, and
product-facing display/input policy. `core_machine_run()` remains the only
guest CPU executor. `core` cannot include or depend on `vm`.

S1 is an evidence-only audit. S2 may repair only the confirmed violations in
this record. If a repair needs a second machine/session, a host shortcut, a
second device-state copy, a VM instruction loop, or a user-visible behavior
change, stop and split the work.

## S1: Current Runtime Path

The real NXVM path is one composition-owned `vm_session`:

`vm/main.c` -> session manager -> `vm_session_create()` -> one
`core_machine_create()` -> profile/device/provider binding -> provider freeze
-> `vm_session_runner_run()` -> `core_machine_run()`.

The runner is a product control loop, not a CPU executor: it submits a bounded
budget only to `core_machine_run()` at
`src/vm/composition/session/runner.c:63`. Core owns the instruction loop,
elapsed ticks, and dispatch order at `src/core/machine/machine.c:221` and
`src/core/machine/machine.c:725`. No `core -> vm` include was found by the S1
source scan.

| Concern | Mutable owner | Create/bind/freeze | Reset/run/finalize path | Boundary | S1 classification |
| --- | --- | --- | --- | --- | --- |
| CPU, FPU, RAM, port bus | `core_machine` (`machine.h`) | `core_machine_create()` | `core_machine_reset/run/destroy()` | core interfaces | retain |
| PIC, PIT, DMA, KBC, VADP | `core_machine` (`machine.h`) | core create; profile supplies frozen parameters | core reset/scheduler/finalize | provider/port contracts | retain |
| Core clock domains | `core_machine` (`machine.h`) | profile clock plan at create | core scheduler only | execution-provider time callback | retain |
| CMOS/RTC, FDD/FDC, HDD/HDC | `vm_session` VM-only fields (`session.h`) | `machine_devices.c` during configuration | provider lifecycle callbacks | core DMA/PIC/port bindings | retain, with CMOS repair below |
| ROM/BIOS code and firmware service list | default profile plus `t_bios` in `vm_session` | `profile_firmware.c` | provider reset/refresh/finalize | ordinary core memory/port route | retain path, but mapping/materialization debt deferred below |
| Host input, copied display, run handle | VM platform/session | `lifecycle.c` | session start/stop/join/finalize | copied transport/provider boundary | retain |
| Console/debugger UX and controls | VM product/session | `main.c`, session debug target | command boundary and runner | adapted `core_product_debug_target` | retain; transitional paused-boundary borrows are tracked debt |
| Boot-failure outcome | profile firmware writes owned BDA report; session runner consumes it | profile firmware reset | `runner.c:16` | copied/ordinary guest-memory observation | retain |

The execution provider owns the VM-only lifecycle order in
`src/vm/composition/session/provider_lifecycle.c`; it does not own CPU
execution. Core dispatches DMA, PIT, VADP, KBC, the frozen provider time
callback, then PIC visibility in that order. Platform only paces or reports
control events.

## S1 Findings

### Confirmed S2 Repairs

1. **CMOS directly mutates core CPU state.** `src/vm/machine/cmos.c:158`
   writes `t_cpu.flagMaskNMI` through a retained raw CPU pointer. This is a
   runtime VM-to-core ownership violation. S2 replaces it with a narrow,
   core-owned NMI-mask operation bound through the CMOS device contract.
   Probe: write CMOS index port bit 7 and observe the core-owned NMI mask via
   the existing CPU/CMOS port corpus. Stop if the repair requires VM access to
   CPU storage or changes interrupt delivery semantics.

2. **Default firmware writes VADP through a profile-binding implementation
   shortcut.** `src/vm/profile/default_profile/firmware/qdcga.c:73-90` writes
   cursor, display-start, and mode state with profile-binding VADP setters.
   S2 replaces the runtime portion with the same CRTC/mode port path the guest
   uses; BDA remains firmware-owned guest RAM. Probe: reset and INT 10h text
   cursor/page behavior must produce the same copied text snapshot. Stop if a
   required behavior has no admitted VADP port representation.

3. **Generic firmware registry is test-only historical scaffolding.**
   `core_machine_firmware` and `vm_profile_default_firmware_compose/apply_image`
   have no production consumer; only `tests/firmware/*` and their CMake targets
   use them. The real path is `session/profile_firmware.c`. S2 deletes the
   orphan registry, its default-profile wrapper, tests, and build targets.
   Probe: CMake has no remaining references; default-PC/AT firmware closure
   and boot regressions continue to use the real profile path.

### Retained, Bounded Interfaces

* `core_machine_configuration_*_borrow()` calls in
  `machine_devices.c` occur only while `core_machine` is `INITIALIZED`; core
  retains child storage until provider teardown. They bind the VM-only CMOS,
  FDC, and HDC to core-owned PIC/DMA/port services and create no mirror.
  This is the documented configuration-borrow exception, not a second owner.
* `debug_target.c` uses `core_machine_debug_*_borrow()` only at a returned
  paused/stopped command boundary. It is an adapted product target and no
  second CPU, RAM, port, or execution loop exists. The raw debug borrow API is
  explicitly transitional (`debug_interface.h:20-29`); it remains a named
  follow-up rather than being silently treated as a clean final contract.
* `core_machine_profile_binding_configure_*` is configuration-only profile
  setup for text-raster/EGA controller defaults. It is not used for guest
  execution after S2 removes the QDCGA runtime setters.

### Deferred, Explicit Debt

* The real default BIOS materializes generated ROM/BDA data during reset via
  ordinary core memory calls (`bios.c`, `profile_firmware.c:156,229`). It is
  one path and one owner, but it is not yet the immutable ROM mapping admitted
  by T245. Converting generated default BIOS assembly into a frozen ROM image
  requires splitting ROM construction from BDA reset writes and is outside
  T264's minimal repair scope. Record it in `TODO.md`; do not claim that the
  current generated BIOS is immutable ROM.
* The paused-boundary debug raw-borrow API needs a separately bounded migration
  to copied/operation-specific core debug access. It is not an ownership
  mirror or alternate executor, but it is not the desired final public shape.

No firmware portal registration, host-to-DOS/BDA shortcut, VM-side instruction
loop, second `core_machine`, or VM device-state mirror was found in the real
NXVM path.

## S2/S3 Plan And Gates

S2 performs only the three confirmed repairs. It may touch core/machine,
VM machine/default-profile/session firmware, their focused tests, CMake, and
the documentation/ledger. It must not change product UX.

S3 adds a structural gate that rejects core-to-VM source dependencies, runtime
CMOS raw CPU state access, QDCGA direct VADP setters, and orphan firmware
registry sources/targets. It runs focused ownership/topology probes plus
`current-gates-gcc`, then rebuilds `build/output/nxvm_0_5_0264.exe` and records
the commit, SHA-256, and banner. The retained DOS prompt, MEM, FDD/HDD boot,
Console/debugger, multi-session, keyboard, display, ATA, and FDC matrix must
remain green.

## Closure

### S1

The owner/path table above is the completed S1 audit. It found one
composition-owned `vm_session`, one `core_machine`, and one CPU executor; it
also distinguished configuration borrows from runtime mutation. No production
firmware portal, host-to-guest shortcut, VM instruction loop, second machine,
or device-state mirror was found.

### S2

CMOS no longer stores a `t_cpu *`: the 0x70 index-port NMI bit reports through
a session-bound narrow callback to `core_machine_set_nmi_mask()`. QDCGA reset
and cursor setup now use core profile-binding memory operations and the same
3D4h/3D5h/3D8h port route used by the guest; it no longer writes VADP storage
through profile setters. The unused generic firmware registry, its
default-profile wrapper, two smoke tests, and associated CMake targets were
removed after confirming that only tests consumed them.

### S3

`verify-t264-core-vm-pcat-ownership` rejects core-to-VM includes, CMOS raw CPU
state, QDCGA VADP setters, retained registry files/targets, and every VM
`core_machine_run()` caller except the session runner. The focused
`vm-pcat-ownership-smoke` verifies the session-owned CMOS callback and both
NMI mask transitions through port 70h. `current-gates-gcc` passed all 100 CTest
cases and its structural gates. The rebuilt
`build/output/nxvm_0_5_0264.exe` has banner `0.5.0264` and SHA-256
`2B7AB0288DF7F3CDE5622496082D86CF6C6A11D288398853CE892C807ECAD705`.

The two deliberately deferred non-violating debts are recorded in `TODO.md`:
generated-ROM materialization and the paused-boundary debug-borrow migration.
