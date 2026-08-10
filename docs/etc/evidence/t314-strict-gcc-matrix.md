# T314 Strict GCC Target Matrix

This S4/S6 matrix inventories production libraries and the current NXVM artifact
under the GCC preset. `Strict` means the target's own compile commands contain
`-Wall -Wextra -Wpedantic -Werror`; it does not assert that transitive
libraries or the resulting product have full strict coverage. Source
provenance is classified from the retained NXVM import index and each unit's
first-path history.

| Target | Source list | Provenance | Effective GCC policy before S4 | S4/S6 disposition and admission trigger |
| --- | --- | --- | --- | --- |
| `type-facade` | `src/type.c` | Mixed: extracted from the retained root runtime foundation. | None. | Deferred: strict admission needs a retained-source diagnostic baseline and an owned change. |
| `core-utils` | `src/core/utils/wait.c` | ntvdm64: T234 owned utility boundary. | None. | **Strict selected**: one owned source and independent static-library build. |
| `core-platform` | `backing_resource_interface.c`, `input.c`, `presentation_mailbox.c`, `wait.c`, and platform `sleep.c`. | Mixed: moved/migrated platform units include inherited host code. | Strict. | **Retained target-local strict**: applies only to this library's sources; it makes no transitive coverage claim. |
| `vm-platform-requests` | `src/vm/platform/request_bridge.c` | NXVM: retained request bridge move. | None. | Deferred: substantive owned change or safe source separation. |
| `vm-request-transport` | `src/vm/platform/vm_request_transport.c` | ntvdm64: owned request-transport boundary. | None. | **Strict selected**: one owned source and independent static-library build. |
| `core-product-debug` | `debug_access.c`, `debug.c`, `xasm32/aasm32.c`, `xasm32/dasm32.c`. | Mixed: retained xasm is NXVM-derived. | None. | Deferred: approved xasm capacity/failure-semantics admission and caller migration. |
| `core-machine-executor` | `display.c`, `vadp.c`, `port.c`, `memory.c`, `cpu.c`, `fpu.c`, `cpu_instructions.c`, `pic.c`, `pit.c`, `dma.c`, `kbc.c`, `rtc.c`. | Mixed: core device work plus inherited CPU/FPU/instruction execution. | None. | Deferred: do not split in S4; inherited-unit diagnostic baseline or approved separation is required. |
| `core-machine` | `clock.c`, `debug.c`, `entry_plan_interface.c`, `fdc.c`, `hdc.c`, `machine.c`, `memory_interface.c`, `media_interface.c`, `port_interface.c`, `presentation_interface.c`, `rom_mapping_interface.c`, `trace_interface.c`; links `core-machine-executor`. | Mixed: retained NXVM machine path and mixed executor dependency. | Strict. | **Retained target-local strict**: it covers only this target's direct sources, never the mixed executor dependency. |
| `core-product-utils` | `src/core/product/utils.c`; links `core-product-debug`. | Mixed: retained xasm utility facade and mixed debug dependency. | None. | Deferred: xasm capacity/failure-semantics admission must establish the API and caller inventory. |
| `core-product-session` | `src/core/product/session/command.c`, `manager.c`. | ntvdm64: owned shared-session boundary. | None. | **Strict selected**: owned sources and independent static-library build. |
| `vm-profile` | `pc_at_profile.c`, `keyboard_mapper.c`, `mouse_mapper.c`, `firmware/bios.c`, `firmware/qdcga.c`. | Mixed: retained NXVM firmware/profile material. | Strict. | **Retained target-local strict**: it covers only these profile sources and makes no product-wide claim. |
| `vm-platform` | `platform.c`, `host_surface.c`, `execution.c`, `input_flush.c`, and host-specific VM platform sources. | Mixed: retained Win32/Linux host adapters. | None. | Deferred: host-specific inherited diagnostic baseline or safely separated platform target. |
| `vm-composition` | `session_factory.c`, `console_machine_adapter.c`, `debug_target.c`, `media.c`, `display.c`, `lifecycle.c`, `session.c`, `control.c`, `fault.c`, `execution.c`, `runner.c`, `provider_lifecycle.c`, `machine_devices.c`, `profile_firmware.c`, `machine_info.c`. | Mixed: retained NXVM composition binds mixed product and machine dependencies. | None. | Deferred: do not treat a local target flag as transitive coverage; require a complete composition diagnostic baseline. |
| `vm-machine` | `src/vm/machine/fdd.c`, `hdd.c`, `media_save.c`, `debug.c`. | NXVM: retained VM devices/debugger. | None. | Deferred: inherited-unit diagnostic baseline and substantive owned change. |
| `vm-product` | `src/vm/product/console.c`. | NXVM: retained Console. | None. | Deferred: retained Console diagnostic baseline and owned change. |
| `vdm-machine` | `src/vdm/machine/dos_minimal.c`. | NXVM: moved non-runnable VDM scaffold. | Strict. | **Retained target-local strict**: it covers this scaffold source only, with no provenance or transitive-coverage claim. |
| `vdm-composition` | `src/vdm/composition/session.c`, `presentation.c`. | NXVM: moved non-runnable VDM scaffold composition. | Strict. | **Retained target-local strict**: it covers these scaffold sources only, with no provenance or transitive-coverage claim. |
| `vm-0-5-0314` | `src/vm/main.c`, `src/vm/composition/session/machine_info.c`; links the libraries above. | Mixed: retained NXVM entry and mixed product graph. | Strict on artifact sources only. | **Retained target-local strict**: it covers only the two artifact sources and makes no linked-graph coverage claim. |

## Verification Record

Configure `mingw-gcc-x64`, build `core-utils`, `core-product-session`, and
`vm-request-transport`, then inspect their Ninja compile commands. Each selected
command must contain all four strict flags. Build `current-gates-gcc` and retain
the configured documentation and diff gates. No global C flags, target split,
runtime source, or artifact identity changes are part of S4.

S6 restores the six historical target-local strict sets for `core-machine`,
`vm-profile`, the current artifact, `core-platform`, `vdm-machine`, and
`vdm-composition`. They remain explicitly local; the inherited NXVM debt stays
open because none is evidence of transitive or whole-product strict coverage.
