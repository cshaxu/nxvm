# Core Machine Public-Surface Inventory

## M5 T294 S1 Decision Record

This is the single migration inventory for T295--T299.  It records the source
as audited on the T293 baseline, not a new API.  `core_machine` is the mutable
guest-state owner for its complete lifetime: create initializes its embedded
CPU/executor/RAM/port/PIC/PIT/DMA/KBC/VADP/FDC/HDC state, `INITIALIZED` admits
binding, freeze closes that window, reset/run operate the same state, and
destroy finalizes it.  VM composition owns only profile selection, frozen
topology/defaults and provider lifetime.  It must not wire a CPU/PIC pair or
retain a second controller/executor.

Audit evidence was the definition/caller sweep below (including headers,
production, and tests), followed by direct inspection of `machine.c`,
`vm/composition/session/session.c`, `profile_firmware.c`, and device setup.

```powershell
rg -n "core_machine_(configuration|profile_binding|debug)_" src tests --glob '*.[ch]'
rg -n "core_machine_(bind_execution_provider|freeze_execution_providers|install_port_provider|register_immutable_rom_mapping|set_trace_provider|apply_entry_plan)" src tests --glob '*.[ch]'
rg -n "^type_status core_machine_|^t_.*core_machine_" src/core/machine --glob '*.[ch]'
```

The first query found definitions in `src/core/machine/machine.c` and every
current raw-borrow use listed in the tables.  The second found the configuration
registrations in `src/vm/composition/session/{lifecycle,machine_devices}.c`,
the public corpus, and no mantle/DOS runtime.  `src/vdm/machine/dos_minimal.c`
is the retained non-runnable fixture and is not a second product path.

### Disposition key

- **Keep** means a typed/copying public operation already meets the boundary;
  its regression remains as a closure gate.
- **Replace** describes a future minimal capability, not an S1 declaration or
  implementation.  It cannot be generalized before the named task supplies a
  real consumer and contract.
- All listed callbacks/providers are borrowed only through core-controlled
  teardown. They cannot re-enter mutable operations on their originating
  machine.

## Migration Matrix

| Surface (definition) | Production consumers; test consumers | Owner/lifetime and raw exposure | T295--T299 disposition; core responsibility | Regression / gate |
| --- | --- | --- | --- | --- |
| Machine lifecycle/configuration: `core_machine_create/reset/reconfigure_memory/run/request_stop/destroy`, observations and copied CPU/FPU/memory-size/time/diagnostic state (`machine_interface.h`, `machine.c`) | VM session/control/lifecycle/runner/fault/machine-info and VDM fixture; `tests/core/*machine*`, `tests/machine/core_machine_*`, `tests/products/nxvm_session_*` | Core owns all guest state from create to destroy. Only copied states and bounded run results cross. | **Keep.** T295 moves CPU/PIC initialization behind this surface; cold RAM reconfigure remains the explicit exception. | Retain lifecycle, instance, scheduler, timing, stopped-lifecycle, reconfigure, and core-contract smokes. |
| Checked memory and A20: `memory_interface.h` (`read/write/query/set_a20`) | VM session/debug target/fault; entry-plan/ROM and core corpus | Core-owned RAM/mappings; no pointer exposed. Mutating writes are execution-boundary operations. | **Keep.** T297 may whitelist checked real/physical guest memory operation only; no universal RAM accessor. | Retain checked-memory, immutable-ROM, entry-plan and A20 corpus; T299 static gate forbids public `t_ram *`. |
| Entry plan and immutable ROM mapping: `entry_plan_interface.h`, `rom_mapping_interface.h` | VM profile BIOS materialization; core entry-plan/ROM tests | Core copies/prevalidates plan and owns mappings; ROM bytes are immutable borrowed input during registration. | **Keep.** Composition supplies profile asset/mapping before freeze; core owns validation/materialization order. | Retain entry-plan and immutable-ROM smokes. |
| Port providers and bus operations: `port_interface.h` | VM device setup; core port/device tests | Core owns port table and routes. Provider/owner context is narrow borrowed registration state, not raw core state. | **Keep typed provider.** T296 S2 stages display/port registration, then S3 DMA/RTC/CMOS/NMI, S4 FDC/HDC; T299 confirms no public `t_port *`. | Existing VADP/port, DMA, RTC/CMOS/NMI, FDC, and HDC port corpus; add per-stage source-closure check. |
| Media registry/provider: `media_interface.h` | VM session/media and FDD/HDD backing; core FDC/HDC and media tests | Composition owns backing/provider context; registry owns frozen bindings; controllers own no backing pointer. | **Keep.** T296 S4 consumes only frozen typed media topology. | Retain media-provider plus VM-free FDC/HDC controller corpus. |
| Execution provider registration/freeze: `execution_provider.h`, `core_machine_bind_execution_provider`, `core_machine_freeze_execution_providers` | VM lifecycle provider; core scheduler/clock corpus and mantle-shape fixture | Core retains provider/context through teardown and calls reset/refresh/advance. No CPU/RAM pointer in this contract. | **Keep but narrow audit.** T295 makes built-in CPU/PIC lifecycle core-owned; provider remains only optional external time/service provider, not a scheduler. | Scheduler/rational-clock/timing checkpoint and mantle-shape tests; static gate forbids VM calls to CPU/PIC init/bind helpers. |
| Trace provider: `trace_interface.h` | VM composition debug target; trace smoke | Core trace state owns the callback slot; supplied context is borrowed until replace/destroy. | **Keep.** No raw state crosses. | Retain trace smoke and non-reentry contract review. |
| Display snapshot and presentation helpers: `display_interface.h`, `core_machine_capture_display_snapshot` | VM display composition and DOS/video system tests | Core VADP/RAM owns mutable display state; capture returns copied snapshot only. | **Keep.** T296 S2 moves VADP initialization/port registration into core while VM retains profile timing/configuration. | Retain display snapshot and CGA/EGA system corpus. |
| Keyboard/mouse/NMI/fault operations (`machine_interface.h`) | VM request bridge and VM/default-profile device setup; keyboard/mouse/CMOS tests | Core owns KBC/PIC/NMI and fault state; inputs are copied scalars. | **Keep.** T296 S3 places DMA plus RTC/CMOS/NMI construction/order in core; VM supplies typed routes/defaults only. | Retain keyboard/mouse, CMOS/RTC port, DMA-channel, and fault-outcome tests. |
| Configuration raw borrows: CPU, instructions, execution, RAM, port, PIC master/slave, PIT, DMA latch/primary/secondary, FDC, HDC, KBC (`machine_interface.h:177-191`; `machine.c:306-436`) | Production: `vm/composition/session/session.c` (RAM mapping, CPU/PIC bind). Test-only: `tests/adapters/support/vm_cpu_probe.c`; `tests/core/machine_*`; `tests/machine/core_machine_{80286,80386,arpl,call_gate,fpu_8087,protected_*,task_switch,tss_iomap,*real_mode*,fdc*,hdc*}`, `cpu_*`, `fpu_escape`, `vm_core_executor_storage`, `vm_two_session_isolation`. | Embedded core-owned storage, valid only `INITIALIZED`/unfrozen, yet leaks every listed raw pointer. | **Replace/remove.** T295: CPU/instructions/execution/PIC; T296 S2: port/VADP implied setup; S3: PIT/DMA/KBC/PIC-related routes; S4: FDC/HDC. T299 removes remaining borrows; tests move to corpus or a clearly test-only adapter. | Preserve each named corpus; replace storage-identity assertions with behavior/ownership closure. T299 grep rejects public raw types/accessors. |
| Profile binding object and raw accessors (`core_machine_profile_binding`, `*_memory`, `*_execution`) | VM session creates binding; default-profile firmware context/qdcga reaches memory/execution | Binding wraps `core_machine *` but exports `t_ram *` and executor pointer; its lifetime is not independently closed on freeze. | **Replace/remove in T297.** Firmware capability must be opaque, core-invoked, available only in its stated lifecycle, and permit only S1-whitelisted checked guest-memory, port, and copied CPU-state operations. | Retain ROM/INT10/keyboard/FDC/HDC firmware corpus. T297 adds lifecycle, failure atomicity, re-entry and nested-call focused gates before implementation. |
| Profile binding checked operations and VADP configuration (`read_real`, `write_real`, `write_port`, `configure_text_raster`, `configure_ega_*`) | VM session and profile firmware | First three use core state without pointer return; VADP calls target core-owned mutable VADP/RAM. | **Split.** T296 S2 absorbs typed frozen display configuration; T297 considers only checked memory/port operations for firmware whitelist. No BIOS/DOS meaning, CRx/mode setter, controller/executor pointer, or raw state backdoor. | Existing VADP configuration and ROM/video corpus; new T297 whitelist static review. |
| Debug copied operations (`debug_read_cpu`, `debug_read_memory`, `debug_step`, `debug_continue`) | VM debug target; CPU diagnostic and NXVM/DOS/video/timer/Windows probes | Core owns execution/RAM; results are copied or bounded run results. | **Keep.** T298 verifies they satisfy retained Console/debugger UX without prompt/startup change. | Retain debug smoke and debugger/product acceptance corpus. |
| Debug raw borrows: CPU, instructions, execution, RAM, port (`debug_interface.h`; `debug.c`) | No production caller outside test-facing composition adapter. Tests: `cpu_fault_diagnostic`, `vm_cpu_stop`, `vm_dos_video_port`, `vm_ega_*`, `vm_fault_outcome_runner`, `vm_no_media_video_port`, `vm_timer_firmware`, `vm_windows31_setup_probe`. | Core-owned storage, nominally paused-only, leaks raw state. | **Replace/remove in T298/T299.** Use copied read state, checked memory/port operation, or operation-specific debug capability; do not make a general snapshot/getter. | Preserve all listed test intent; T299 grep rejects public debug raw borrows. |
| Test-only probe/seam: `tests/support/core_machine_executor_fixture.h`, adapter `vm_cpu_probe`, and direct private-header tests | Test-only, including core CPU/protected-mode corpus and mantle-shape fixture | Test-owned setup may directly exercise same-module implementation, but cannot become a product route or mirror mutable state. | **Deferred to T299 test migration.** Retain a narrow test-only adapter only where a public corpus cannot set an already-owned CPU state; it must be outside product headers and never be used by `src/`. | Add static closure: no `tests/support` seam included from `src/`; focused corpus names its ownership proof. |

## Required Subsequent Contracts

### T296 stage boundaries

T296 S1 is this matrix. S2 is display/port only (VADP configuration and port
registration); S3 is DMA plus RTC/CMOS/NMI; S4 is FDC/HDC. Each stage must
move core's actual initialization/reset/finalization order without moving a
PC/AT default, media path, boot policy, or controller mirror into core.

### T297 firmware capability admission

Before code, T297 S1 must freeze an operation enum and exact lifecycle. The
only candidate operations are checked guest-memory read/write, checked port
write/read if needed by existing firmware, and copied CPU-state read. Every
operation returns factual status, validates fully before mutation where that is
possible, states its partial-effect rule, and cannot re-enter a mutable machine
operation. Nested firmware calls are rejected unless an explicitly named
read-only operation proves safe. Capability destruction/freeze/reset behavior
must be defined; no raw CPU/RAM/port/controller/executor access, DOS/BIOS
semantic operation, mode/segment/CRx setter, or decoder hook is admissible.

T300 and T302 are **conditional admission only**: neither has a recorded
first-party use case in this audit, so neither is implemented, enabled, or
treated as a default follow-on.

## Sustainable Static Closure Gate (design only)

T299 should add one source-shape inventory script, run by its focused CTest or
documented static target. It must: (1) enumerate declarations in the listed
public `core/machine/*_interface.h` headers; (2) fail when a product-facing
header exports `t_cpu *`, `t_cpuins *`, `t_ram *`, `t_port *`, `t_pic *`,
`t_pit *`, `t_dma *`, `t_latch *`, `t_kbc *`, `core_machine_fdc *`,
`core_machine_hdc *`, or `core_machine_cpu_execution_context *`; (3) fail on
`core_machine_configuration_*_borrow`, `core_machine_profile_binding_*` raw
accessors, or `core_machine_debug_*_borrow` outside a documented test-only
adapter; (4) enumerate `src/` and tests separately and require every exception
to be listed with owning task and focused regression; and (5) verify that
`src/` does not include a test seam. It is an inventory/closure gate, not a
future API generator and not a runtime claim.

## Risks Requiring Owner Attention

`core_machine_profile_binding` presently has a freeze-window initialization
check but its later accessor calls do not re-check lifecycle. T297 must decide
whether an opaque capability expires at freeze, reset, callback completion, or
provider teardown before implementation. The current `session.c` direct
CPU/PIC bind is concrete evidence that T295 must preserve ordering before its
borrow removal. The numerous direct CPU corpus fixtures show that T299 needs a
deliberate test migration plan rather than a bulk getter replacement.
