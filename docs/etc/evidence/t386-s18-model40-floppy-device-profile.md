# T386 S18: Reusable Floppy Device Profile

`M5:T386:S18:MODEL40-FDD-GEOMETRY:OK`

`M5:T386:S18:MODEL40-FDD-MEDIA:OK`

`M5:T386:S18:MODEL40-FDD-RESET-BINDING:OK`

## Scope And Ownership

T386 S5 identifies the selected DeskPro 386 Model 40 1.2 MB diskette drive,
using 500 kbit/s with the retained 765A, IRQ6 and DMA2 path. This S closes only
the raw-IMG geometry and selected-drive functional binding: 80 cylinders, two
heads, 15 sectors per track, 512 bytes per sector and 1,228,800 bytes.

Core continues to own the profile-neutral media geometry contract plus the
FDC, DMA and IRQ mechanics. `vm/profile/device/floppy` owns reusable floppy
specifications: 3.5-inch 1.44 MB and 5.25-inch 1.2 MB. The VM FDD provider
adapts an admitted geometry to allocated raw-IMG media, sidecar validation,
reset and media observation. Model-40 private composition selects the 1.2 MB
profile; zero-initialized default and Model-339 sessions retain the 1.44 MB
profile. No Model-40 detail enters Core, and profile code has no VM-machine
dependency.

This layout permits a later permitted 5170 variant to select the same 1.2 MB
device profile. Public YAML backbone and variant selection is deliberately not
introduced here; it requires its own schema/catalog/Console admission.

## Functional Proof

- `vm-model40-fdd-s18-smoke` creates the private Model-40 route with synthetic
  ROM bytes only. It proves selected 80x2x15x512 geometry, exact 1,228,800-byte
  media acceptance, wrong-size rejection, Core media observation, retained
  FDC IRQ6/DMA2 binding and reset persistence.
- The same smoke creates default and Model-339 sessions as controls and proves
  both retain 18 sectors per track (the 1.44 MB device profile).
- `vm-media-provider-smoke` and `vm-model40-private-composition-s7-smoke`
  remain focused controls. The FDD provider continues to expose only the
  existing Core media contract; no test-only production API is added.
- Dependency-DAG verification passes with zero known migration edges. T345
  ownership verification reports 220 rows: 160 owner tests, seven embedded
  production entries, one type foundation, two safely separable production,
  50 mixed/inherited and 57 residual-production entries.
- T344 now classifies the discovered existing
  `core_machine_compaq_cecg_s9_smoke.c` direct constructor; it was not added by
  S18, but the newly exercised fixture-shape gate correctly required its
  explicit retained classification.
- The rebuilt developer artifact is `vm-0-5-0389`,
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `29A174DBC889A1220B0F4BD17805AAA78B326F01B31E5D68D5688F6B5DEC09E4`.

## Similar-Issue Sweep And Transfer

The S18 sweep covered every VM FDD initializer, profile/session construction
route, raw image replacement/insertion/reset/sidecar path, media registry and
FDC binding caller, hard-coded form-factor/capacity string and focused FDD
consumer. The only shared profile route is `vm/profile/device/floppy`; no
second device-specific FDD implementation remains.

S18 does not claim real BIOS execution, public profile/catalog selection,
physical flux/CRC/rotation/index/CHRN behavior, 500-kbit/s service duration,
controller/ISA waits, or DeskPro board timing/L3. Those remain T386 functional
or board-timing receivers. The generalized backbone/allowed-variant YAML
surface transfers to a separate following S so the Console schema does not
silently change during a storage-functional proof.