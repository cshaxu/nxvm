# M5 T117 S1: Composition Directory Convergence

## Scope

Moved all VM and VDM root-composition sources and private headers with `git mv`:

- `src/vm/composition*` to `src/vm/composition/`
- `src/vdm/composition*` to `src/vdm/composition/`

Direct include paths and CMake source paths now target those directories. The
dependency-DAG gate recognizes composition as the documented product-root
integration point; it does not weaken module-to-module dependency rules.

## Verification

- MinGW-w64 GCC 16.1.0: configure, `nxvm-current-gcc`, and
  `nxvm-current-gates-gcc` passed.
- Dependency-DAG gate: `0` migration edges; passed.
- `nxvm-vm-dos-prompt-smoke.exe D:\\fdd.img`:
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0117.exe` retained Console `help`, `info`, `exit` script: passed.
- Artifact: `build/output/nxvm_0_5_0117.exe`
  SHA-256 `4D928B11B5D966865F3D603B8CFF813685A6C6CB08697AAAE817A6B73E8182D2`.

No Console, debugger, boot, device, or session logic changed.
