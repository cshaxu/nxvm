# M6 Product Layout Reconciliation

## Outcome

Adopt the owner-selected MyNes product tree without changing emulator behavior:
`src/app-mynes/product` owns startup and commands, `src/app-mynes/core` owns
emulation, and `test/app-mynes` mirrors those components with unit and
integration coverage.

## Scope

- Reconnect root and component CMake traversal, include roots, and test
  registration to the moved product directories.
- Reconnect the sole packaged configuration, binaries, and release-manifest
  tooling to `assets/binary-mynes`.
- Update active architecture, coding, verification, and build documentation to
  name the new layout.
- Rebuild and test both supported architectures sequentially.

## Exclusions

This task does not alter Lib or Common sources, emulator behavior, ROM content,
command semantics, or historical evidence.

## Completion Standard

The new tree builds directly, all active documents name it accurately, the
sole packaged configuration and versioned executables are under
`assets/binary-mynes`, and the x64 and x86 CTest suites both pass.
