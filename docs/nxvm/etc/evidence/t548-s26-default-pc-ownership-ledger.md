# T548 S26 Default-PC Unit Ownership Ledger

## Scope and decision rule

This record consumes the Default-PC App unit batch frozen by the S1 ledger.
It records behavior rather than include paths: a test remains App-owned only
when it proves a Default-PC firmware, descriptor, selected composition, or a
full-assembly route that a Core-only fixture cannot prove.  A generic
construction or executor assertion moves only when the Core receiver executes
the same API, state transition and failure predicate.

| Entry at S26 admission | Behavior and required context | Canonical owner / disposition |
| --- | --- | --- |
| `firmware/floppy_smoke.c` | Default firmware image exposes its floppy service contract. | Retain App: firmware bytes and service selection are profile facts. |
| `machine/nxvm_boot_failure_lifecycle_smoke.c` | Only stopped executor state; it has no boot-failure predicate. | Retire: `test/core/machine/executor_state_smoke.c` owns start/reset/stop state transitions. |
| `machine/nxvm_cga_graphics_system_smoke.c` | Guest execution through the selected Default composition produces CGA pixels. | Retain App: it proves executor-to-selected-board wiring, not the CGA algorithm alone. |
| `machine/nxvm_cmos_rtc_port_smoke.c` | Default seed, firmware and RTC port route are jointly visible. | Retain App: selected CMOS/ROM binding increment. |
| `machine/nxvm_console_pause_resume_smoke.c` | Default application console request reaches its composed session. | Retain App: product-to-composition integration. |
| `machine/nxvm_core_executor_storage_smoke.c` | Default composed executor preserves storage/debug route. | Retain App: selected composed-machine increment. |
| `machine/nxvm_display_composition_smoke.c` | Default video composition publishes the expected display contract. | Retain App: selected video binding. |
| `machine/nxvm_ega_controller_system_smoke.c` | Full Default machine connects EGA controller behavior to display output. | Retain App: Core owns controller mechanics; this owns selected composition. |
| `machine/nxvm_ega_sequencer_system_smoke.c` | Full Default machine connects EGA sequencer behavior to display output. | Retain App: selected composition increment. |
| `machine/nxvm_fault_outcome_runner_smoke.c` | App runner publishes a fault outcome from the Default machine. | Retain App: runner/product boundary. |
| `machine/nxvm_fdc_authority_smoke.c` | App media registry binds the Default FDC/DMA route. | Retain App: selected registry and board binding. |
| `machine/nxvm_fdc_port_smoke.c` | Default FDC port registration is reachable through selected composition. | Retain App: selected route increment. |
| `machine/nxvm_fdc_read_track_smoke.c` | Default drive/media path completes a read-track route. | Retain App: App asset/media binding. |
| `machine/nxvm_hdc_port_smoke.c` | Default HDC port registration is reachable through selected composition. | Retain App: selected route increment. |
| `machine/nxvm_host_cancellation_smoke.c` | Host cancellation travels through Default runner/session assembly. | Retain App: runner integration rather than executor primitive. |
| `machine/nxvm_kbc_aux_guest_smoke.c` | Guest-visible auxiliary KBC route on Default composition. | Retain App: AT-board selection increment. |
| `machine/nxvm_keyboard_host_ingress_smoke.c` | Host key ingress reaches the Default guest mapping. | Retain App: application ingress and selected keyboard binding. |
| `machine/nxvm_machine_media_lifecycle_smoke.c` | Default factory/media slots admit/eject selected media. | Retain App: selected factory/media combination. |
| `machine/nxvm_machine_reconfigure_smoke.c` | Reconfigure memory, report memory info and reset vector. | Retire: `test/core/machine/construction_smoke.c` now asserts the same API results on a generic reconfigurable machine. |
| `machine/nxvm_machine_smoke.c` | Default factory constructs its fixed machine identity. | Retain App: fixed descriptor/firmware composition. |
| `machine/nxvm_machine_speed_policy_smoke.c` | Default App speed policy controls its runner. | Retain App: product policy. |
| `machine/nxvm_pcat_composition_smoke.c` | Reset re-arms Default selected machine, clearing timeline and NMI mask. | Retain App after removing duplicate topology matrix; reset behavior is observed on the actual selected machine. |
| `machine/nxvm_pcat_ownership_smoke.c` | Two independently assembled Default machines isolate FDC/NMI ownership. | Retain App: selected composition isolation. |
| `machine/nxvm_pcat_topology_smoke.c` | Default descriptor materializes exact ports, IRQ/DMA and board routes. | Retain App: sole Default topology receiver. |
| `machine/nxvm_runner_display_cadence_smoke.c` | Default runner publishes display cadence. | Retain App: runner/display integration. |
| `machine/nxvm_runner_error_propagation_smoke.c` | Default runner exposes machine errors to product outcome. | Retain App: product error boundary. |
| `machine/nxvm_two_session_isolation_smoke.c` | Two Default App sessions remain isolated. | Retain App: App composition and multi-session context. |
| `machine/nxvm_x86_debug_mapping_smoke.c` | Default debugging mapping reaches selected x86 machine. | Retain App: adapter/composition mapping. |
| `profiles/default_pc_at_plan_smoke.c` | Default plan values are internally coherent. | Retain App: fixed profile descriptor. |
| `profiles/default_pc_at_profile_smoke.c` | Default profile declares selected device/firmware facts. | Retain App: fixed profile descriptor. |
| `profiles/nxvm_default_pc_at_apply_smoke.c` | Default profile applies its planned board configuration. | Retain App: fixed profile application. |
| `profiles/rom/default_pc_at_rom_materialization_smoke.c` | Default external ROM materializes at the documented addresses. | Retain App: firmware asset binding. |

## Receiver changes

- `construction_smoke.c` now checks the public machine-information result and
  reset vector after a generic reconfigurable construction changes memory.  It
  retains the same 32 MiB input and `F000:FFF0` predicate that the retired App
  test used, without importing a Default profile fixture.
- `executor_state_smoke.c` already directly covers the only behavior the
  retired misnamed boot test asserted: stopped state clears active/reset work.
- `nxvm_pcat_topology_smoke.c` remains the sole Default receiver for the full
  port/IRQ/DMA/KBC/RTC/FDC/HDC topology matrix.  S26 removes that same matrix
  from `nxvm_pcat_composition_smoke.c`, which now owns only reset re-arming.

No App fixture is moved to Core merely because it includes a Core private
header.  Default's ROM/session helper stays local, and the former cross-profile
qualification helper is retired rather than retained as a hidden fourth test
layer.

## Complete-suite gate corrections

The required x64 complete run exposed two pre-existing static ownership
violations before any runtime assertion failed:

- `machine_competition_smoke.c` used raw `printf` despite the test Types rule.
  It now uses the existing `lib_c_printf` wrapper from `lib/types/file.h`; no
  output marker or competition assertion changes.
- The selected IBM 5170 clock/typematic contract was physically stored in
  `test/core` while directly including `app-my5170` profile headers.  It is
  moved to `test/app-my5170/unit/profiles`, uses that App's own session-assets
  fixture, and no longer imports Default-PC profile data merely to contrast
  constants.  Its CTest identity and assertions for the selected 5170 machine
  remain intact.  Core's manifest no longer claims it.

The latter is one My5170 receiver correction, not permission to move the
remaining Core qualification corpus mechanically; S29 reviews those entries
against their individual fixture and profile contexts.

## Blocking mixed-receiver correction

The same required complete-suite run exposed one remaining mixed test whose
placement in Core was structurally impossible: it assembled Default-PC state,
validated a Model 40 observation contract and exercised all four profile
timing configurations.

- The Default-specific initialization, firmware rejection, configuration
  materialization and invalid-media assertions now live in
  `test/app-nxvm/unit/machine/nxvm_initialization_atomicity_smoke.c`.
- Core's `construction_smoke.c` owns the generic partial-media finalization
  predicate.  The separate option-ROM argument predicate is already owned by
  `board-base/construction_helpers_smoke.c`, so it is not duplicated.
- The Model 40 observation argument predicate is now in its existing Model 40
  BYOB receiver.
- The former four-profile DMA deadline table is replaced by four App-owned
  configuration receivers.  They share only
  `test/core/machine/support/dma_deadline_fixture.h`, which owns the generic
  DMA plan/create/reset/deadline assertion and imports no App source.

This removes every `test/core` include of an App header.  The eight changed
receivers plus Core Types, ownership, test-manifest and x86-boundary gates pass
on x64 and x86.

## Final verification

The full repository-only `unit` selection is 515 routes per architecture.  It
was run by exact CTest-name partitions rather than unstable ordinal filters:
App/profile routes (93), Lib (36), Emulator (15), Product (15), Core static
gates (4), CPU manifest runners (5), Core/x86 receiver partitions (347).
All **515/515** pass on x64 and all **515/515** pass on x86.

The first x86 complete attempt correctly failed as a build-precondition issue:
the directory contained only changed targets.  A complete Ninja build then
exposed two test-only compile defects and no product defect: this probe still
called the retired qualification helper, and the 80386 timing JSON writer used
its `lib_c_file` type name in place of its `file` local.  The probe now passes
`&session->construction` to its existing Model 40 predicate; the writer uses
the single local handle consistently.  After rebuilding, the complete x86 unit
selection passed.  Core manifests pass on both widths after the corresponding
hash update.
