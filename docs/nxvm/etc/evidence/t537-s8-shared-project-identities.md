# M5 T537 S8: Shared Standalone Project Identities

## Result

| Corpus | Standalone CMake project name |
| --- | --- |
| Lib | `nxvm_shared_library` |
| Common | `nxvm_shared_common` |
| X86 | `nxvm_shared_x86` |

The Common and X86 declarations were renamed from `common_corpus` and
`x86_corpus`. Targets, source layout, public APIs, and consumer links are
unchanged.

## Verification

- Common standalone configuration, `common-verify`, and 18/18 Common tests pass.
- X86 standalone configuration, `x86-verify`, and 10/10 X86 tests pass.
- Fresh NXVM root configuration builds. Full direct CTest unit execution completes
  with no failed-test record; the existing bounded wrapper instead times out at
  300 seconds while serially rebuilding/running the same 336 labelled unit rows.
- Fresh MyNES root configuration, build, and 53/53 MyNES tests pass.
- Lib/Common/X86 manifests and the shared dependency verifier pass; the current
  source, test, and documentation sweep has no retired project-name use.
