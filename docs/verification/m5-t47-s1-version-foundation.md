# M5 T47 S1: Version Foundation Verification

## Result

`src/version.*` is now the only source of retained NXVM identity and build-time
data. `vm/main.c` supplies only its CMake task suffix to the formatter.

## Verification

- GCC CMake configure and full build passed.
- `ntvdm64-version-smoke.exe` returned zero and printed
  `M5:T47:S1:VERSION-FOUNDATION:OK`.
- `nxvm-m5-t47.exe` printed the unchanged retained identity:
  `Neko's x86 Virtual Machine [0.4.015d.m5t47]`, entered the retained Console,
  and accepted piped `EXIT` with exit status zero.
- `nxvm-product-console-smoke.exe`, `verify-dependency-dag`, and
  `verify-live-machine-authority` passed.

## Artifact

- Path: `build/output/nxvm-m5_t47.exe`
- SHA-256: `C7C1BC358A9223F7F649C7B437EC293C581153AEF5D5C6FAAAB00BFEAAD90742`
- Classification: local developer artifact, not a release artifact.
