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
- The current provider facts are limited to their stored callback/context and
  the observed call sites: `core_machine` retains an execution provider through
  destroy and invokes its reset/refresh/advance-time callbacks; port, media,
  trace, and display contracts retain their documented context. This audit does
  not establish a general current non-reentry property. T297 must specify and
  prove that property for its new firmware callback boundary.

## Migration Matrix

| Surface (definition) | Production consumers; test consumers | Owner/lifetime and raw exposure | T295--T299 disposition; core responsibility | Regression / gate |
| --- | --- | --- | --- | --- |
| CPU profile, copied state, execution point, and fault diagnostic: `cpu_interface.h` | Production: `vm/composition/session/session_factory.c` formats the CPU profile; `vm/composition/session/fault.c` consumes copied diagnostics. Tests: lifecycle/instance/entry-plan corpus plus CPU, protected-mode, real-mode, and fault diagnostic smokes use copied state/diagnostics. | Core owns CPU and diagnostic-ring storage for the machine lifetime. `core_machine_cpu_state`, execution points, and diagnostics are copied values returned through machine operations; no CPU pointer is exposed by this header. | **Keep.** T295 preserves the profile and copied observation boundary while moving initialization authority into core. T298 may consume these copied values for debugger replacement. | Retain machine-instance/lifecycle/entry-plan, CPU fault diagnostic, and protected/real-mode corpus gates. |
| FPU profile, operation metadata, and copied FPU state: `fpu_interface.h` | Production: `session_factory.c` formats FPU profile; CPU instruction implementation consumes operation metadata. Tests: `core_machine_fpu_8087_smoke` and FPU/profile corpus validate metadata and copied FPU state. | Core owns FPU state for the machine lifetime; metadata is immutable value data and state is copied through `core_machine_get_fpu_state`. No raw FPU storage is exported. | **Keep.** No T295--T299 replacement is required; T295 preserves profile configuration, and T299's raw-borrow closure must not mistake this copied contract for a borrow. | Retain FPU profile/escape/8087 smoke and CPU profile gate. |
| Lifecycle enumeration: `lifecycle_interface.h` | Production: core debug implementation and VM session/control/fault paths interpret lifecycle results. Tests: core lifecycle/stopped-lifecycle and VM fault-outcome corpus. | `core_machine` owns lifecycle transitions from create through destroy; the enum is a copied status value. | **Keep.** T295 retains core lifecycle authority; no raw-state replacement or deferred work. | Retain lifecycle, stopped-lifecycle, boot-failure, and fault-outcome smokes. |
| Value presentation helpers: keyboard queue and text snapshot: `presentation_interface.h` | Production: retained non-runnable `src/vdm/machine/dos_minimal.c` initializes its own text snapshot. Tests: `tests/core/presentation_smoke.c`. | Caller owns queue/snapshot value storage; helpers neither name a machine nor borrow guest state. | **Keep.** Outside the T295--T299 raw-borrow migration; no deferred runtime work. | Retain presentation smoke and VDM fixture build coverage. |
| Machine lifecycle/configuration: `core_machine_create/reset/reconfigure_memory/run/request_stop/destroy`, observations and copied CPU/FPU/memory-size/time/diagnostic state (`machine_interface.h`, `machine.c`) | VM session/control/lifecycle/runner/fault/machine-info and VDM fixture; `tests/core/*machine*`, `tests/machine/core_machine_*`, `tests/products/nxvm_session_*` | Core owns all guest state from create to destroy. Only copied states and bounded run results cross. | **Keep.** T295 moves CPU/PIC initialization behind this surface; cold RAM reconfigure remains the explicit exception. | Retain lifecycle, instance, scheduler, timing, stopped-lifecycle, reconfigure, and core-contract smokes. |
| Checked memory and A20: `memory_interface.h` (`read/write/query/set_a20`) | VM session/debug target/fault; entry-plan/ROM and core corpus | Core-owned RAM/mappings; no pointer exposed. Mutating writes are execution-boundary operations. | **Keep.** T297 may whitelist checked real/physical guest memory operation only; no universal RAM accessor. | Retain checked-memory, immutable-ROM, entry-plan and A20 corpus; T299 static gate forbids public `t_ram *`. |
| Entry plan and immutable ROM mapping: `entry_plan_interface.h`, `rom_mapping_interface.h` | VM profile BIOS materialization; core entry-plan/ROM tests | Core copies/prevalidates plan and owns mappings; ROM bytes are immutable borrowed input during registration. | **Keep.** Composition supplies profile asset/mapping before freeze; core owns validation/materialization order. | Retain entry-plan and immutable-ROM smokes. |
| Port providers and bus operations: `port_interface.h` | VM device setup; core port/device tests | Core owns port table and routes. Provider/owner context is narrow borrowed registration state, not raw core state. | **Keep typed provider.** T296 S2 stages display/port registration, then S3 DMA/RTC/CMOS/NMI, S4 FDC/HDC; T299 confirms no public `t_port *`. | Existing VADP/port, DMA, RTC/CMOS/NMI, FDC, and HDC port corpus; add per-stage source-closure check. |
| Media registry/provider: `media_interface.h` | VM session/media and FDD/HDD backing; core FDC/HDC and media tests | Composition owns backing/provider context; registry owns frozen bindings; controllers own no backing pointer. | **Keep.** T296 S4 consumes only frozen typed media topology. | Retain media-provider plus VM-free FDC/HDC controller corpus. |
| Execution provider registration/freeze: `execution_provider.h`, `core_machine_bind_execution_provider`, `core_machine_freeze_execution_providers` | VM lifecycle provider; core scheduler/clock corpus and mantle-shape fixture | Core retains provider/context through teardown and calls reset/refresh/advance. No CPU/RAM pointer in this contract. | **Keep but narrow audit.** T295 makes built-in CPU/PIC lifecycle core-owned; provider remains only optional external time/service provider, not a scheduler. | Scheduler/rational-clock/timing checkpoint and mantle-shape tests; static gate forbids VM calls to CPU/PIC init/bind helpers. |
| Trace provider: `trace_interface.h` | VM composition debug target; trace smoke | Core trace state owns the callback slot; supplied context is borrowed until replace/destroy. | **Keep.** No raw state crosses. | Retain trace smoke and non-reentry contract review. |
| Display snapshot and presentation helpers: `display_interface.h`, `core_machine_capture_display_snapshot` | VM display composition and DOS/video system tests | Core VADP/RAM owns mutable display state; capture returns copied snapshot only. | **Keep.** T296 S2 moves VADP initialization/port registration into core while VM retains profile timing/configuration. | Retain display snapshot and CGA/EGA system corpus. |
| Keyboard/mouse/NMI/fault operations (`machine_interface.h`) | VM request bridge and VM/default-profile device setup; keyboard/mouse/CMOS tests | Core owns KBC/PIC/NMI and fault state; inputs are copied scalars. | **Keep.** T296 S3 places DMA plus RTC/CMOS/NMI construction/order in core; VM supplies typed routes/defaults only. | Retain keyboard/mouse, CMOS/RTC port, DMA-channel, and fault-outcome tests. |
| Configuration raw borrows: CPU, instructions, execution, RAM, port, PIC master/slave, PIT, DMA latch/primary/secondary, FDC, HDC, KBC (`machine_interface.h:177-191`; `machine.c:306-436`) | Production call map is below: session storage/profile binding, control debug initialization, and device setup. Test families are below. | Embedded core-owned storage, valid only `INITIALIZED`/unfrozen, yet leaks every listed raw pointer. | **Replace/remove.** T295: CPU/instructions/execution/PIC; T296 S2: port/VADP implied setup; S3: PIT/DMA/KBC/PIC-related routes; S4: FDC/HDC. T299 removes remaining borrows; tests move to corpus or a clearly test-only adapter. | Preserve each named corpus; replace storage-identity assertions with behavior/ownership closure. T299 grep rejects public raw types/accessors. |
| Profile binding object and raw accessors (`core_machine_profile_binding`, `*_memory`, `*_execution`) | VM session creates binding; default-profile firmware context/qdcga reaches memory/execution | Binding wraps `core_machine *` but exports `t_ram *` and executor pointer; its lifetime is not independently closed on freeze. | **Replace/remove in T297.** Firmware capability must be opaque, core-invoked, available only in its stated lifecycle, and permit only S1-whitelisted checked guest-memory, port, and copied CPU-state operations. | Retain ROM/INT10/keyboard/FDC/HDC firmware corpus. T297 adds lifecycle, failure atomicity, re-entry and nested-call focused gates before implementation. |
| Profile binding checked operations and VADP configuration (`read_real`, `write_real`, `write_port`, `configure_text_raster`, `configure_ega_*`) | VM session and profile firmware | First three use core state without pointer return; VADP calls target core-owned mutable VADP/RAM. | **Split.** T296 S2 absorbs typed frozen display configuration; T297 considers only checked memory/port operations for firmware whitelist. No BIOS/DOS meaning, CRx/mode setter, controller/executor pointer, or raw state backdoor. | Existing VADP configuration and ROM/video corpus; new T297 whitelist static review. |
| Debug copied operations (`debug_read_cpu`, `debug_read_memory`, `debug_step`, `debug_continue`) | VM debug target; CPU diagnostic and NXVM/DOS/video/timer/Windows probes | Core owns execution/RAM; results are copied or bounded run results. | **Keep.** T298 verifies they satisfy retained Console/debugger UX without prompt/startup change. | Retain debug smoke and debugger/product acceptance corpus. |
| Debug raw borrows: CPU, instructions, execution, RAM, port (`debug_interface.h`; `debug.c`) | **Production:** `vm/composition/session/debug_target.c`: `vm_debug_cpu` (35), `vm_debug_instructions` (39), `vm_debug_execution` (44), `vm_debug_memory` (48), and `vm_debug_port` (52). These are the VM composition debugger-adapter helpers. Tests: `cpu_fault_diagnostic`, `vm_cpu_stop`, `vm_dos_video_port`, `vm_ega_*`, `vm_fault_outcome_runner`, `vm_no_media_video_port`, `vm_timer_firmware`, `vm_windows31_setup_probe`. | Core owns the storage; debug borrow definitions permit it only after the execution thread returns a paused boundary. VM composition owns the adapter/session lifetime and currently passes these pointers into retained debugger paths. | **Replace/remove in T298/T299.** T298 replaces each helper with copied CPU/instruction/memory/port observation or a named operation-specific debug capability; no general snapshot/getter and no Console/debugger UX change. | Preserve the named debugger adapter behavior and all listed test intent; T299 grep rejects public debug raw borrows. |
| Test-only probe/seam: `tests/support/core_machine_executor_fixture.h`, adapter `vm_cpu_probe`, and direct private-header tests | Test-only, including core CPU/protected-mode corpus and mantle-shape fixture | Test-owned setup may directly exercise same-module implementation, but cannot become a product route or mirror mutable state. | **Deferred to T299 test migration.** Retain a narrow test-only adapter only where a public corpus cannot set an already-owned CPU state; it must be outside product headers and never be used by `src/`. | Add static closure: no `tests/support` seam included from `src/`; focused corpus names its ownership proof. |

### Public-interface coverage index

This index makes the `src/core/machine/*_interface.h` audit mechanically
checkable. `cpu.h`, `fpu.h`, and device implementation headers are private
implementation headers, not public product contracts; they enter T299 only
when a listed raw-borrow surface exposes one of their storage types.

| Header | Matrix coverage |
| --- | --- |
| `cpu_interface.h` | CPU profile, copied state, execution point, and fault diagnostic |
| `debug_interface.h` | Debug copied operations; Debug raw borrows |
| `display_interface.h` | Display snapshot and presentation helpers |
| `entry_plan_interface.h` | Entry plan and immutable ROM mapping |
| `fpu_interface.h` | FPU profile, operation metadata, and copied FPU state |
| `lifecycle_interface.h` | Lifecycle enumeration |
| `machine_interface.h` | Machine lifecycle/configuration; Keyboard/mouse/NMI/fault; Configuration raw borrows; Profile binding; Execution provider registration/freeze |
| `media_interface.h` | Media registry/provider |
| `memory_interface.h` | Checked memory and A20 |
| `port_interface.h` | Port providers and bus operations |
| `presentation_interface.h` | Value presentation helpers: keyboard queue and text snapshot |
| `rom_mapping_interface.h` | Entry plan and immutable ROM mapping |
| `trace_interface.h` | Trace provider |
| `execution_provider.h` (public non-interface provider header) | Execution provider registration/freeze |

### Product-path raw-borrow call map

These are production calls, not fixture-only evidence. All borrowed targets are
embedded in `core_machine`; each is currently consumed while composition is
constructing or servicing the retained VM session.

| File and symbol | Exact current borrow/use | Current owner/lifecycle | Planned removal |
| --- | --- | --- | --- |
| `src/vm/composition/session/session.c`, `vm_session_storage_initialize` (131--155) | RAM (131), execution context (132), PIC master/slave (133--134) feed `core_machine_cpu_execution_context_bind_pic` (135); profile binding initializes at 137 and configures text/EGA at 143/149/155. | Core owns embedded targets; VM composition owns setup during `INITIALIZED` before provider freeze. | T295 absorbs CPU/PIC wiring; T296 S2 absorbs VADP configuration; T297 replaces profile binding. |
| `src/vm/composition/session/control.c`, session initialization (204--205) | CPU and instruction borrows feed `vm_machine_debug_initialize`. | Core owns the CPU/instructions; VM composition creates its debug object during session initialization. | T295/T298 remove this direct debug initialization borrow; retained debugger behavior is regression-protected. |
| `src/vm/composition/session/session.c`, storage setup | Profile data is converted to typed DMA and RTC/CMOS/NMI declarations. | Core owns DMA, RTC, PIC, and ports; composition submits copied neutral values while configuration is open. | Completed by T296 S3; raw RTC/DMA binding is absent from VM composition. |
| `src/vm/composition/session/machine_devices.c`, controller topology setup | FDC/ATA profile topology is converted to `core_machine_fdc_topology` and `core_machine_hdc_topology`; the existing typed media registry and opaque DMA request are submitted. | Core owns controller/DMA/PIC/port storage and lifecycle; VM retains FDD/HDD backing and provider policy. | Completed by T296 S4; raw controller/PIC/port borrows and direct connect/initialize/port registration are absent from VM composition. |
| `src/vm/composition/session/debug_target.c`, `vm_debug_{cpu,instructions,execution,memory,port}` (35/39/44/48/52) | Calls all five `core_machine_debug_*_borrow` functions directly. | Core owns targets; VM composition owns the debugger adapter and its session-bound use after a paused execution boundary. | T298 supplies copied/operation-specific debug capability; T299 removes raw debug exports. |
| `src/vm/profile/default_profile/firmware/context.h`, `vm_profile_default_context_{memory,execution}` (37/42) | Directly returns profile-binding RAM/execution raw pointers; used by default-profile firmware, including `qdcga.c`. | Core owns targets; VM profile firmware retains binding through current session/provider lifetime. | T297 opaque capability replaces both; firmware no longer receives direct pointers after freeze. |

### Test raw-borrow families for T299

`tests/machine/vm_core_executor_storage_smoke.c` and
`tests/machine/vm_two_session_isolation_smoke.c` assert storage identity and
directly mutate/check embedded storage; they require behavior/ownership
replacements, not a new public getter. `tests/core/machine_{lifecycle,memory_reconfigure}.c`
also assert configuration/debug borrow availability or identity and must become
lifecycle/operation tests. The `core_machine_{80286,80386,arpl,call_gate,
fpu_8087,protected_*,task_switch,tss_iomap,*real_mode*}`, `cpu_*`, and
`fpu_escape` families set prepared CPU/executor state; retain their CPU corpus
intent through a narrow test-only adapter only where a public entry plan cannot
express it. `core_machine_{fdc*,hdc*}` and core PIT/DMA tests use raw controller
setup and must migrate to typed frozen topology/provider corpus. Debug-borrow
tests remain the explicitly named debug family in the matrix row above.

## Required Subsequent Contracts

### T296 stage boundaries

T296 S1 is this matrix. S2 is display/port only (VADP configuration and port
registration); S3 is DMA plus RTC/CMOS/NMI; S4 is FDC/HDC. Each stage must
move core's actual initialization/reset/finalization order without moving a
PC/AT default, media path, boot policy, or controller mirror into core.

### T297 firmware capability admission

Before code, T297 S1 must freeze an operation enum and the following adjudicated
future lifecycle; it is not a statement of current implementation behavior.
Composition binds an opaque firmware provider/context only while configuring;
freeze makes topology immutable. Core alone calls the capability at an explicit
firmware-service callback boundary, so VM/profile has no direct invocation
right after freeze. Reset retains the capability and invokes the frozen
provider's reset semantics; callback return ends that operation's borrow;
destroy/provider teardown invalidates the capability. Mutable operations are
non-reentrant; nested callback is rejected by default unless a future contract
explicitly admits and proves one read-only operation safe. The only candidate
operations are checked guest-memory read/write, checked port write/read if
needed by existing firmware, and copied CPU-state read. Every operation returns
factual status, validates fully before mutation where possible, and states its
partial-effect rule. No raw CPU/RAM/port/controller/executor access, DOS/BIOS
semantic operation, mode/segment/CRx setter, or decoder hook is admissible.

T300 and T302 are **conditional admission only**: neither has a recorded
first-party use case in this audit, so neither is implemented, enabled, or
treated as a default follow-on.

## T294 S1 Verification Record

The correction pass ran `git diff --check` successfully and then ran:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .
```

Its actual marker was `Documentation governance checks passed for vm-0-5-0293.`
The subsequent public-interface completeness pass re-ran the same two commands
and produced the same marker.
No build graph changed, so this documentation-only correction did not require a
build gate.

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
check but its later accessor calls do not re-check lifecycle; the T297 lifecycle
above is the required future replacement, not a claim about those accessors.
The current `session.c` direct CPU/PIC bind is concrete evidence that T295 must
preserve ordering before its borrow removal. The numerous direct CPU corpus
fixtures show that T299 needs a deliberate test migration plan rather than a
bulk getter replacement.
