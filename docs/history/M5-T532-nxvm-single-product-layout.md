# M5 T532 NXVM Single-Product Layout

Owner admitted this task on 2026-09-20 to reorganize NXVM as one VM product.
The durable candidate scope is retained in
[the active proposal](../proposals/m5-nxvm-single-product-layout.md).

S1 freezes a complete production/test/build path ledger before any structural
move. Its completed ledger is
[indexed evidence](../etc/evidence/t532-s1-single-product-layout-ledger.md):
App owns the root entry and parsed request; generic Core becomes `core/core`;
the NXVM adapter becomes `core/machine`; profiles become `core/profile`; every
repository-only test mirrors that owner; and VDM has only a finite deletion
set. The next S may perform the relocation batch.

## S2: Source And Test Relocation

Implementation `66bba6f3` executes that frozen map: App is `src/app`, generic
Core is `src/core/core`, the NXVM adapter is `src/core/machine`, profiles are
`src/core/profile`, and repository-only tests mirror those owners. It deletes
the VDM skeleton, its tests and its dedicated forwarding gate. Integration
remains independent, with only direct include repairs. The complete evidence,
including the 336/336 unit and static/documentation-gate result, is
[indexed here](../etc/evidence/t532-s2-single-product-layout-relocation.md).

## S3: Closure Sweep

The final sweep retains the requested source-mirroring test layout: App,
generic Core, Core adapter and profiles are covered by `test/app`,
`test/core/core`, `test/core/machine` and `test/core/profile`; only
`test/integration` remains an independent external-asset boundary.  It finds
no former VM/VDM root, builds byte-identical stripped x64/x86 0532 artifacts
in both required locations, and passes 336/336 repository-only unit plus
42/42 independent integration checks.  The complete closure record is
[indexed here](../etc/evidence/t532-s3-single-product-layout-closure.md).
