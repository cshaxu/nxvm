# M5 T14 S1 Dependency Governance

The source include inventory confirms the recorded debt: `core/product/utils.c`
includes `vm/platform`; `core/machine/vcpuins.c` includes `vm/machine/device`;
and core PIC, PIT, DMA, and keyboard-controller code include default-profile
firmware. The retained VM also has both `vm/machine -> vm/platform` and
`vm/platform -> vm/machine` source crossings. VDM minimal presentation has a
`vdm/platform -> vdm/machine` crossing.

The CMake inventory confirms that `nxvm-baseline-runtime` is a mixed aggregate
target. These facts match `docs/planning/m5-dependency-governance.md` and
justify superseding the isolated T13 S9-S11 plans. No source, CMake, runtime,
or user-visible behavior changed in this design-only subtask.
