# M5 T1 S1 CPU Capability Probe Verification

## Evidence

- Entry commit: `c6358c120620c98cdde28c59b4aa0aef2ce87bbc`.
- Scope: `docs/planning/subtasks/m5-t1-s1.md`.
- The retained baseline source is unchanged; new test code is confined to the
  project-owned core and baseline-adapter boundary.

## Commands And Result

```text
cmake --preset mingw-gcc-x64
cmake --build --preset m5-cpu-probes-gcc
build/mingw-gcc-x64/nxvm-core-cpu-capability-smoke.exe
build/mingw-gcc-x64/nxvm-baseline-cpu-probe-smoke.exe
git diff --check
git diff --exit-code -- src/nxvm-baseline
```

Both smokes passed on Windows x64 GCC. The capability smoke emitted
`M5:T1:S1:CPU-CAPABILITY:OK`. The baseline probe smoke emitted the intentional
baseline `#UD` diagnostic for `0F 0B`, then
`M5:T1:S1:CPU-PROBES:OK`.

The probe covers one-step real-mode immediate MOV, immediate ADD, relative
jump, segment-prefix NOP, operand-size-prefixed immediate MOV, and invalid
opcode capture. It records the prescribed bounded state but makes no complete
8086, 286, or 386 profile claim. `docs/compatibility/cpu-capability-matrix.md`
records the remaining gates.

Build-tree SHA-256 values:

- `nxvm-core-cpu-capability-smoke.exe`:
  `a6805f676a21bda6df895f832db813cbf2dd2a264b0855b997a45b4abf887d3b`.
- `nxvm-baseline-cpu-probe-smoke.exe`:
  `f339304ef53ad5d24329082987effbe3e2f35f0f3420606ce911caf6b8c8ac9f`.

No task artifact is created until the whole M5 T1 task, including S2, is
complete.
