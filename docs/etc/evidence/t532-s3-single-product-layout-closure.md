# T532 S3 Single-Product Layout Closure

This evidence records the final closure sweep for the approved
[single-product layout](../../history/M5-T532-nxvm-single-product-layout-proposal.md).
The current [Architecture](../../design/ARCHITECTURE.md) and
[Source Layout](../../design/CODING.md) remain the current authorities.

## Final Source And Test Layout

The tracked source roots are `src/app`, `src/core/{core,machine,profile}` and
the unchanged shared `src/{lib,common,x86}` components.  There is no tracked
or filesystem `src/vm` or `src/vdm` component.

Repository-only tests mirror those owners:

- `test/app` covers App;
- `test/core/core`, `test/core/machine`, and `test/core/profile` cover the
  three NXVM Core owners; and
- `test/{lib,common,x86,support}` retain their shared/generic owners.

`test/integration` remains the sole independent external-ROM/media boundary.
The final tracked-file sweep has no path below `src/vm`, `src/vdm`, `test/vm`,
or `test/vdm`.  Direct integration fixture includes were mechanically changed
to their moved Core owners; no test semantics or guest behavior changed.

The root `src/type.*` facade and `src/banner.h` are deliberately retained as
project-wide entry/foundation files.  They are neither a VM/VDM component nor
a second App/Core implementation route.

## Release Artifacts

Both artifact copies are byte-identical between `build/output` and
`assets/sessions`.

| Artifact | PE architecture | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `nxvm_0_5_0532_x64.exe` | x86-64 | 1,265,350 | `0D18EC577487A9A549AE8E4190EA84065D67B214A32B2AD6EECF0790C1DC6E9A` |
| `nxvm_0_5_0532_x86.exe` | i386 | 1,026,048 | `5F6285A45A84C67CB252776F0C551305BECAACC39486F40EC9035FB489A9160A` |

Both are Release outputs linked with the existing `--strip-debug` policy.  PE
inspection reports an empty Debug Directory for each output; the product
debugger remains compiled as a product feature, not development trace data.

## Verification

| Check | Result |
| --- | --- |
| Final repository-only unit suite | 336/336 passed; 20.21 s wall time |
| Independent integration suite | 42/42 passed; 171.21 s wall time |
| Current-artifact target | `vm-0-5-0532`: passed |
| Specialized gates | Passed, including their intentional negative self-test |
| Documentation governance | Passed for `vm-0-5-0532` |
| JSON and diff hygiene | `CMakePresets.json` parses; `git diff --check` is clean |

The source-layout move itself remains the accepted S2 implementation
`66bba6f3`, accepted by `e9c5a55f`.  S3 contains only final artifact version
selection, four direct owner-path include repairs, and closure evidence; it
introduces no alternate compatibility tree, platform route, or product
behavior.
