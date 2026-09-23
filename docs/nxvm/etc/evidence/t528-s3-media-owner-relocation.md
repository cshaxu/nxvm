# M5 T528 S3 VM Media Owner Relocation

Baseline: `67902bf6` after accepted T528 S2.  This is a narrow NXVM-local
ownership correction.  It does not alter image modes, profile topology, Core
media ABI, Common contracts, executor/lifecycle ownership or firmware mapping.

## One Owner And One Route

| Former location | Final owner | Disposition |
| --- | --- | --- |
| `src/vm/machine/fdd.{c,h,private.h}` | `src/vm/media/fdd.{c,h,private.h}` | Moved unchanged apart from the owner path.  It owns FDD geometry, the Lib-storage medium and the FDD Core-media provider. |
| `src/vm/machine/hdd.{c,h,private.h}` | `src/vm/media/hdd.{c,h,private.h}` | Moved unchanged apart from the owner path.  It owns HDD geometry, the Lib-storage medium and the HDD Core-media provider. |
| `src/vm/machine/runtime/media.c` | none | Deleted.  Its registry binding depended on the complete `vm_machine` composition and was therefore not a media-owner implementation. |
| Core registry binding/freeze | `vm/machine/runtime/machine_devices.c` | Retained with device assembly: it binds each owner-provided FDD/HDD provider exactly once to the already-created Core registry, then freezes it. |

`vm-media` is the sole target compiling the FDD/HDD owner sources.  `vm-machine`
depends on that target; it does not compile any media-owner source.  The legacy
`vm_machine_fdd_*` and `vm_machine_hdd_*` symbol prefixes remain only to avoid
an unrelated public ABI rename; their source path and state ownership are now
`vm/media`.

The source sweep found exactly one production
`core_machine_media_registry_bind`/`freeze` producer:
`vm_machine_devices_bind_media()`.  It is Core composition, not a second
media state or provider.  The moved FDD/HDD provider definitions remain the
sole FDD/HDD implementations.

## Verification

- Focused x64 build: `vm-media-provider-smoke`,
  `vm-media-direct-readonly-smoke`, `vm-fdc-port-smoke`, and
  `vm-hdc-port-smoke`.
- Focused CTest: all four tests passed.
- Full repository-only unit suite: **299/299 passed** with `ctest --parallel 8`.
- `verify-current-specialized-gates` passed, including dependency DAG, FDC/HDD
  sole-route, provider-composition, deadline-scheduler and direct-compilation
  ownership gates.
- `git diff --check` passed.  The exit sweep found no current CMake, source,
  test, tool or verifier reference to the former `vm/machine/fdd`,
  `vm/machine/hdd`, or `runtime/media.c` paths.

`git diff --cached --numstat -- src test CMakeLists.txt cmake tools` records
**86 added, 88 removed, net -2 lines** across the 38 tracked implementation,
test and verification paths.  The only added logic is the pre-existing registry
binding moved into its real device-composition owner; the reverse-dependent
`runtime/media.c` route is deleted.

## Boundary Retained

The unresolved executor disposition is not consumed here.  It remains outside
this S; this relocation adds no Common executor and no second runtime loop.
