# M5 Dual-Architecture Developer Artifacts

## Purpose

Make every runnable implementation-task closure reproducible as both a native
64-bit and a 32-bit Windows NXVM executable.  The current source graph already
accepts either host pointer width, but the supported release process currently
publishes only the x64 artifact.

## Design

One CMake product-target definition continues to own executable sources,
version/banner, optimization checks and deployment.  Two configure presets
select the x64 or x86 MinGW toolchain.  The artifact copier derives its suffix
from the configured target architecture, producing exactly:

- `nxvm_0_5_NNNN_x64.exe`
- `nxvm_0_5_NNNN_x86.exe`

No architecture-specific product code, second source target, copied source
list, YAML mutation, or asset route is permitted.  The build must verify the
emitted PE architecture and stripped Release condition before publishing each
artifact.

## Subtask Plan

1. **S1 - build-contract and x86 toolchain admission.** Add explicit x64/x86
   configure/build presets and an architecture-aware artifact contract.  Fail
   clearly if the configured compiler architecture does not match the selected
   preset or required artifact suffix.
2. **S2 - one product artifact route.** Replace the unsuffixed copy route with
   one architecture-derived output implementation.  Advance the current task
   artifact identity to T517 and update the static ownership verification.
3. **S3 - dual-architecture proof and closure.** Build stripped Release x64
   and x86 T517 artifacts, verify both PE formats and hashes, run the complete
   repository-only unit suite on both, and run the complete external-ROM/media
   integration suite on both when executable under the declared host.

## Exit Criteria

- Both T517 artifact files exist in `build/output/`, have the same version
  banner and respectively PE32+ x64 / PE32 i386 identity.
- The product remains one CMake target/source route; only configured toolchain
  and artifact suffix differ.
- Both architecture test suites pass, all relevant static gates and
  documentation governance pass, and future task closure has an enforceable
  dual-artifact rule.
