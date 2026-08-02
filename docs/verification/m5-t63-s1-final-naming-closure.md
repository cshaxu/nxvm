# M5 T63 S1 Final Second-Pass Naming Closure

## Closure

M5 second-pass naming and ownership cleanup is complete through T63.

The current source tree has only the approved product-shape roots:

- `src/core`
- `src/vm`
- `src/vdm`

The T59/T60 platform audit preserves symmetric host-surface names and records
that no current VM platform file is safe to migrate wholesale to
`core/platform`. The T61/T62 alias audit records retained current-object aliases
as accepted compatibility accessors, not second storage or synchronization
bridges.

## Static Audit

- Current source roots: `core`, `vm`, `vdm`.
- Old public symbols absent from source, tests, and CMake:
  `nxvm_firmware_default_profile`, `nxvm_product_nxvm_session`, and
  `nxvm_vm_full_pc_session`.
- Old retained source filenames absent from `src`: `vcpu.c/h`,
  `vcpuins.c/h`, `vram.c/h`, `vport.c/h`, `vpic.c/h`, `vpit.c/h`,
  `vdma.c/h`, `vkbc.c/h`, `vvadp.c/h`, `vcmos.c/h`, `vdebug.c/h`,
  `vfdd.c/h`, `vfdc.c/h`, `vhdd.c/h`, `vhdc.c/h`, and `vbios.c/h`.
- Current architecture/planning front-door docs do not reference old top-level
  source roots such as `src/machine`, `src/platform`, `src/product`,
  `src/firmware`, or `src/runtime`.
- Historical, provenance, and old verification records may still mention old
  paths and old symbols as evidence of earlier migrations.

## Retained Exceptions

- Runtime aliases such as `vcpu`, `vcpuins`, `vram`, `vport`, PIC/PIT/DMA
  aliases, VM media aliases, `vdebug`, `vbios`, and QD firmware names remain
  recorded T61/T62 exceptions.
- Platform files named `win32app`, `win32con`, `linuxcon`, and future
  `linuxapp` remain accepted symmetric host-surface names. Ownership continues
  to be determined by policy dependencies.

## Verification

- Built `build/output/nxvm_0_5_0063.exe`.
- SHA256:
  `E9A7488ABFE80DB706713ACFC0169758FBE9246DA9E748B51A2BA76A622B5010`.
- `nxvm-product-console-smoke`: passed.
- `verify-dependency-dag`: passed.
- Old public-symbol scan over `src`, `tests`, and `CMakeLists.txt`: no match.
- Old retained source filename scan under `src`: no match.
- Active-work scan before closure showed T63 as the only active subtask.
