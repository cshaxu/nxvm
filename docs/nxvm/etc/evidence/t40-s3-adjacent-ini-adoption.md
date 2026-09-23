# Shared M6 T40 S3 NXVM Adjacent-INI Adoption

## Result

NXVM now asks Lib Base for the current executable directory, then appends the
product-owned `NXVM.ini` filename in a small App startup module. Its entry point
no longer accepts or inspects `argv`; it reports an explicit startup failure if
the adjacent INI path cannot be formed. There is no current-working-directory
fallback.

The App module has a direct `base` dependency. The focused NXVM unit test runs
against the actual test process path, verifies the `NXVM.ini` suffix, and proves
that an undersized caller buffer is rejected without overwriting its prior text.

## Verification

On 2026-09-23:

- `vm-app-ini-smoke`, `lib-test-process_directory`, and current
  `vm-0-5-0535` linked with MinGW-w64 GCC for x64 and x86.
- Focused CTest passed 2/2 on both architectures:
  `unit.vm-app-ini-smoke` and `library.process_directory`.
- Current NXVM product links reported `x64: OK` and `x86: OK`.
- The existing x86 cache held an obsolete profile-archive path and a separate
  MSYS compiler that failed ABI detection. An ignored T40 x86 build directory
  used the repository's existing `profiles-nxvm` archive and the working
  MinGW-w64 GCC 16.1.0 toolchain; no source or BYOB asset was changed to solve
  that local build-environment issue.
- Static sweep found no `argv[0]`, `return "NXVM.ini"`, or
  `GetModuleFileName` startup-path inference in `src/app-nxvm/product`.
- NXVM and MyNES documentation governance checks, plus `git diff --check`,
  passed.
