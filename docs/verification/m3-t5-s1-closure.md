# M3 T5 S1 Closure Verification

## Composition

- `runtime/session.c` is the sole session owner for `nxvm.full_pc` and
  `ntvdm64.dos_minimal` profile lifecycle. Its smoke printed
  `M3:T5:S1:SESSION:OK` using the local FDD/HDD fixture inputs.
- The full-PC path remains a deliberate single-session legacy adapter. The
  DOS-minimal path is an independent core-backed no-media session.

## Complete Matrix

The GCC build and smoke matrix printed all expected markers:

```text
M3:T1:S1:CORE-CONTRACT:OK
M3:T1:S2:MACHINE-INSTANCE:OK
M3:T2:S1:LIFECYCLE:OK
M3:T2:S2:TRACE:OK
M3:T3:S1:FULL-PC-PROFILE:OK
M3:T3:S2:DOS-MINIMAL-PROFILE:OK
M3:T4:S1:PRESENTATION:OK
M3:T4:S2:DEBUG:OK
M3:T5:S1:SESSION:OK
```

M1 `nxvm-baseline` also built successfully.

## Fixture And Baseline Evidence

- Local fixture hashes matched `docs/fixtures/m1-local-images.md`:
  - FDD: `fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5`.
  - HDD: `f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688`.
- Full-PC adapter tests separately configured both fixtures and observed
  `F000:FFF0` after reset.
- `git diff --exit-code -- src/nxvm-baseline` passed. The M1 FDD/HDD
  ten-second watchdog traces, insertion/connection markers, and DOS `INT 21h`
  checkpoints therefore remain applicable to the unchanged legacy path.

## Boundary Audit

- Forbidden core dependency scan returned no matches for Win32, DOS INT 21h,
  product CLI, product source, or baseline source.
- Platform source scan found no core Machine or private-core-header access.
- `git diff --check` passed. No guest media, Microsoft binary, trace output, or
  product artifact was committed.

## Result

M3 is complete. M4 starts from `docs/planning/m4-design-inputs.md` and the
bounded `docs/planning/m4-firmware-nxvm-design-breakdown.md`; it assigns
firmware and nxvm product ownership without reopening M3's shared-core or
product-CLI boundary.
