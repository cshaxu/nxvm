# M5 Naming Remediation

## Scope

T97 removed active `vmachine*` composition names. This plan finishes the
remaining active old-style API vocabulary without changing NXVM Console,
debugger, boot, media, or platform behavior. A task changes direct callers and
private helpers in its owned files in the same commit; it never leaves a
compatibility alias behind.

## Retained Vocabulary

The `type.*` C-runtime wrappers are intentional and use the collision-safe
`STD_*` spelling, including `STD_LOCALTIME`, `STD_STRCAT`, `STD_STRCPY`,
`STD_STRTOK`, `STD_STRCMP`, `STD_STRLEN`, `STD_PRINTF`, `STD_FPRINTF`,
`STD_SPRINTF`, `STD_FOPEN`, `STD_FCLOSE`, `STD_FREAD`, `STD_FWRITE`,
`STD_FGETS`, `STD_MALLOC`, `STD_FREE`, `STD_MEMSET`, `STD_MEMCPY`, and
`STD_MEMCMP`.

Keep the approved compact families: `kbc`, `vadp`, `win32app`, `win32con`,
`linuxapp`, `linuxcon`, `w32*`, `xasm32`, `aasm`, `dasm`, and `debug`. Keep
`qdx`, `qdcga`, and `qdkeyb` while they remain documented default-profile
firmware hacks. These are lexical retentions, not an exemption from owner
prefixes on enclosing public APIs.

## Task Order

| Task | Owner and rename boundary | Required result |
| --- | --- | --- |
| T108 | Root C-library facade audit | Define the complete `type.*` standard C facade and its collision-safe uppercase aliases before any call-site migration. Planning only. |
| T99 | Root foundation include boundary | Replace every active `core/machine/vglobal.h` include with `type.h`; delete `vglobal.h`. |
| T100 | Root foundation helper APIs | Rename `utilsLowerStr` and `utilsTrace*` to `ntvdm64_type_*`; leave uppercase wrappers unchanged. |
| T101 | Root status/type vocabulary | Rename `nxvm_core_status` and its status constants to root-owned `ntvdm64_*` names; inventory remaining `t_*` aliases and legacy macros for the final type task. |
| T102 | `core/product` | Rename `debugMain`, `utilsSleep`, assembler/disassembler wrappers, and `nxvm_execution_context_*` / `nxvm_runtime_registry_*` to `core_product_*`. Preserve the `debug`, `aasm`, `dasm`, and `xasm32` lexical terms. |
| T103 | `vm/` composition and machine | Rename `machine*`, `vmCompositionBindBlock`, `vhdc*`, and `nxvm_cpu_probe_*` to `vm_composition_*` or `vm_machine_*`. |
| T104 | `vm/platform` | Rename generic `platform*` and unscoped `linux*` / `win32*` public APIs to `vm_platform_*`; retain backend lexical tags such as `win32app`, `win32con`, `linuxapp`, `linuxcon`, and `w32*`. |
| T105 | `vm/product` and `vm/profile` | Rename `consoleMain` and remaining `nxvm_product_nxvm_*` / `nxvm_vm_*` public APIs to `vm_product_*` / `vm_profile_*`. |
| T106 | Implemented `vdm/` skeleton | Rename `nxvm_runtime_dos_minimal_*`, `nxvm_dos_minimal_*`, and `ntvdm64_vdm_minimal_*` to `vdm_machine_*` or `vdm_composition_*`. No new VDM behavior. |
| T107 | Root type completion and closure | Resolve the T101 inventory of active `t_*` public aliases and legacy non-retained macros, then run an owner-prefix and retained-vocabulary closure scan. |
| T111 | C scalar aliases | Adopt `C_*` aliases in active project code. |
| T112 | ISO C type/object/atomic aliases | Adopt `STD_*` types, objects, constants, and atomics without changing memory order. |
| T113 | ISO C header ownership | Make `type.*` the ISO C header boundary; remove `memory.h` and add a scan. |
| T114 | Win32 adapter vocabulary | Adopt `WIN32_*` at the project-facing edge of Win32 adapters. |
| T115 | Input flush capability | Replace `fflush(stdin)` with an explicit platform capability. |
| T116 | Closure audit | Prove static naming, dependency, GCC, DOS, Console, and documentation closure. |

Each code-changing task produces `nxvm_0_5_0NNN.exe` when it changes the
runnable NXVM path. T98 and T108 are planning-only and produce no artifacts.
T111--T116 follow the same artifact and regression policy.

## Mapping Rules

- `src/core/<module>` exports `core_<module>_*`; `src/vm/<module>` exports
  `vm_<module>_*`; `src/vdm/<module>` exports `vdm_<module>_*`.
- Root composition exports `vm_composition_*` or `vdm_composition_*`.
- Root `type.*` has no product owner, so its new non-wrapper symbols use
  `ntvdm64_type_*`; repository-wide status vocabulary uses `ntvdm64_status`.
- A source file's `static` helpers adopt the same owner prefix when its task
  touches that file. This prevents a public cleanup from leaving misleading
  internal names in the same implementation.
- Historical documents and `src/nxvm-baseline/` retain original spellings as
  provenance and are excluded from closure scans.

## Gates And Stop Conditions

For every runnable task: build its artifact and `nxvm-current-gates-gcc`, run
the FDD DOS-prompt smoke with `D:\\fdd.img`, and run the retained Console
script (`help`, `device fdd insert D:\\fdd.img`, `info`, `exit`). A task stops
on any Console output/grammar difference, debugger behavior change, boot or
prompt failure, unapproved rename of retained vocabulary, or a required
compatibility alias. T107 additionally proves no active forbidden old-style
public symbol remains outside the retention list.
