# M5 T13 S8 Session Composition Split

`core/product/runtime` now retains only generic registry infrastructure and a
product-neutral profile descriptor shape. It has no product profile ID, static
product descriptor, session implementation, VM include, or VDM include.

`vm/profile/full_pc_profile.*` owns the unchanged `nxvm.full_pc` descriptor,
and `vm/product/full_pc_session.*` owns its FDD/HDD create, reset, and destroy
path. `vdm/profile/dos_minimal_profile.*` owns the unchanged
`ntvdm64.dos_minimal` descriptor, and `vdm/product/minimal_session.*` owns the
no-media create, reset, and destroy path. The previous mixed session smoke is
split into focused VM and VDM session smokes.

GCC configured and built all targets. The VM and VDM session smokes, the two
profile smokes, finite CPU probe, FDD execution-context lifecycle, FDD/HDD
reset-vector, retained Console, and delayed debugger gates passed. No raw
recorder ran. `build/output/nxvm-m5_t13.exe` is the ignored developer artifact:
SHA-256 `9B2189A1F7413CBB8768ECEC5D67968C833737D61655B53AEAE7B14E8865B61C`,
emitting the retained `Neko's x86 Virtual Machine [0.4.015d.m5t13]` identity.
