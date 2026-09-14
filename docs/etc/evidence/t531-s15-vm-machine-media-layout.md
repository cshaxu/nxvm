# M5 T531 S15 VM-Machine Media Layout

## Result

The existing NXVM FDD/HDD Core-media-provider adapter was moved with `git mv`
from `src/vm/media/` to `src/vm/machine/media/`.  The retained owner is still
the VM-machine assembly layer:

- `src/lib/storage` owns file access, lease and direct/readonly/overlay
  semantics;
- Core owns FDC/HDC controller behavior and the generic media-provider
  boundary;
- `vm-media` translates frozen NXVM profile media geometry and Lib storage
  mediums into that Core boundary.

The `vm-media` CMake target and all public `vm_machine_*` symbol names are
unchanged.  This is a path-only relocation: no new Common component, storage
path, media state, compatibility include, ABI or behavioral branch exists.

## Sweep and review

The sweep searched tracked CMake, `cmake/`, `tools/`, `src/` and `test/` for
`src/vm/media` and `vm/media/`: no live hit remains.  The five former
VM-machine-to-`vm/media` dependency exceptions were deleted from the DAG
allowlist because the moved files are now owned by the same `vm/machine`
module.  CMake source ownership rows and static ownership checks use the new
path.

Tracked code/test changes are seven pure renames plus direct
include/source-list substitutions: 43 additions, 48 removals, net minus five
(the retired dependency exceptions). Documentation and governance records are
excluded from that count. Actual-diff review confirms
that only include paths, CMake locations, source layout, the obsolete DAG
exceptions and this evidence changed.

## Verification

| Check | Result |
| --- | --- |
| Focused FDD/HDD media smokes | 5/5 passed |
| Repository-only unit suite | 325/325 passed serially; the suite's current non-parallel invocation is retained |
| Dependency DAG and old-path sweep | passed; 11 remaining migration edges, zero old media paths |
| Documentation governance and `git diff --check` | passed |
| x64 stripped Release 0531 | PE x64; SHA-256 `E4346FEF1BC8ED2186335EF81851AA0466F3F15136FF1BCC2EBB87E9D6662384` in both required locations |
| x86 stripped Release 0531 | PE x86; SHA-256 `C7539E3446343F4C5E78714C48336F0E32BD10D350CA9778DEB54DDEE9E68151` in both required locations |

## Disposition

S15 is ready for coordinator acceptance and its implementation commit. T531
remains open for the owner's requested manual test and later whole-task
acceptance.
