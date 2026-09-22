# T535 S2: Canonical Six-Component Import

## Imported Corpus

NXVM copies exactly these committed SoftPC trees from
`1c5a47146dd4fd87b09423b7a7b960becb50cd67`:

```text
src/lib      src/common      src/x86
test/lib     test/common     test/x86
```

The post-copy relative-path/SHA-256 comparison is exact for all six roots:
97, 23, 15, 49, 20 and 10 files respectively. SoftPC's dirty media files were
not read or copied. The source is project-owned MIT material and carries no
independent third-party notice in this corpus.

## Exact Changes

- Lib gains its canonical neutral bounded PCM `audio` stream, one Win32
  `winmm` backend, one Linux unsupported/backend leaf, Types declarations and
  three owner-local tests.
- Common gains the canonical state reducer condition that only a running
  machine may synthesize a missing Window; the state-matrix regression enters
  `test/common`.
- x86 source and tests remain an exact no-change copy.
- No NXVM application/core caller references `lib_audio_stream_*`; importing
  the capability does not add a guest-audio route or product behavior.

## Fresh-Rebuild Test Repair

A first clean full rebuild exposed pre-existing test migration debt outside the
six imported roots: Profile public headers now correctly expose opaque types,
but six white-box smoke paths still expected their old layouts; a second group
still read the retired `vm_machine.profile` mirror; two VM tests used a VADP
private constant through transitive includes. S2 repairs those test-only paths:

- White-box Profile tests explicitly include their existing same-owner private
  Profile headers.
- Tests assert the sole frozen `profile_plan`/Core configuration rather than
  resurrecting the deleted `session->profile` mirror.
- VADP-address tests explicitly include their existing owner header.

The full repository search for `session->profile` now finds only
`session->profile_plan`; production already used that sole owner. This repair
does not change App, Core, profile, device or shared-corpus production logic.

## Verification

- `lib-verify-manifest`: pass.
- `common-verify`: manifest and dependency-boundary pass.
- `x86-verify`: manifest and dependency-boundary pass.
- Fresh root build: pass, including Lib audio and Common state-matrix targets.
- `ctest --test-dir build/t534-s37-x64 -L unit -j 8 --output-on-failure`:
  **336/336 pass** in 195.96 seconds.

The source-only Lib/Common/x86 test executables are registered in that complete
unit gate and passed, including `library.audio_stream`, `library.audio_native`,
`library.audio_win32_platform` and `common.control_state_matrix`.

## S3 Disposition

No NXVM adaptation is required: the canonical Lib target owns its `winmm`
linkage transitively, the root build discovers it without caller changes, and
there are no stale shared-target/public-contract consumers. T535 therefore
proceeds directly to final integration/artifact convergence; S3 is not
admitted as empty work.
