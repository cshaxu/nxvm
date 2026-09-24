# M6 Executable-Directory Base Capability

## Outcome

Give Lib Base one bounded, caller-buffer executable-directory query. MyNES and
NXVM use that capability to locate their adjacent INI, instead of respectively
calling Win32 directly or inferring a directory from `argv[0]`.

## Scope

- Shared Base owns the public contract and Win32/Linux implementations.
- MyNES and NXVM App startup code consumes the same contract.
- Tests cover success, short-buffer rejection and each product's adjacent-INI
  route on Windows x64/x86.

## Exclusions

No Common change, no SoftPC repository change, no command-line INI override,
no working-directory fallback, and no product artifact change until the final
product adoption P.

## Completion Standard

Both products obtain an INI path from the actual executable directory. Lib has
one neutral public contract with platform-private implementations, both product
routes have focused regressions, and every target-scoped P is verified and
pushed.
