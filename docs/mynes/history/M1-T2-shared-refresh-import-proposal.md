# Refresh The Shared Four-Root Import

## Goal And Approval

The owner approved this M1 candidate on 2026-09-20 after an independent audit
of SoftPC commit `1fe946a242df9356f84a0b4e5961ee4266930810`. Refresh the
shared corpus from immutable Git blobs at that pin. This is a source-refresh
task, not a new NES capability or a neutral-contract implementation.

## Scope

Transfer exactly these source roots with their manifests and tests:

| SoftPC source | MyNes destination | Frozen file count |
| --- | --- | ---: |
| `src/lib` | `src/lib` | 91 |
| `src/common` | `src/common` | 23 |
| `test/lib` | `test/lib` | 46 |
| `test/common` | `test/common` | 20 |

MyNes uses a root `test/` tree. Relocate the prior `src/test/lib` and
`src/test/common` transfer roots to `test/lib` and `test/common`; no source
test tree remains. Update only MyNes-owned root build, preset, verification,
provenance and task evidence needed to compose that unchanged four-root corpus.
The current upstream delta removes the x86 debug and xasm units from Common;
the refresh retains that removal rather than restoring them locally.

## Exclusions And Acceptance

No NES Core/App code, ROM, guest media, product debug policy, sibling edit, or
new optional x86 component is admitted. Lib and Common files are copied from
the frozen pin without MyNes-specific source edits. The task inventories every
unit and license disposition, verifies source/destination hashes, validates
manifests and dependency checks, and runs the complete shared suite and each
standalone root on Windows x64 and x86 under strict C11. The task stops if an
upstream unit requires a product-local change, an unreviewed third-party notice,
or a failure that cannot be resolved within build composition and test-location
ownership.
