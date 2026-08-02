# M5 T75 S1 Verification

Date: 2026-08-02

## Static Gate

`tools/VerifyFacadeOwnership.ps1 -RepositoryRoot .` completed successfully.
The CMake `verify-facade-ownership` target emitted
`M5:T75:FACADE-OWNERSHIP:OK` after reporting every ledgered facade family.

## GCC Artifact

The existing WinGet GCC 16.1.0 and CMake 4.4.0 toolchain configured the
`mingw-gcc-x64` preset and built `nxvm-0-5-0075` successfully.

Artifact: `build/output/nxvm_0_5_0075.exe`

SHA-256: `29750F84A35734FF713D4FF5364A28059C9FBBAD6A1BE3094C802EE7C74CA9AA`

T75 changes only governance, a static verification tool, and build targets;
it does not modify NXVM runtime source or user-visible behavior.
