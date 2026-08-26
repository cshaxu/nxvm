# T484 S5 P1 5160 B2 Core Topology

`M5:T484:S5:XT-B2-PLAN:OK`

`M5:T484:S5:XT-NO-AT-TOPOLOGY:OK`

## Result

The immutable Core configuration now owns two construction-only selections:
`pic_topology` and `dma_controller_count`.  Their zero values retain existing
PC/AT cascaded defaults; the resolved 5160-268 declaration copies
single-PIC/one-DMA values before a machine is created.  No VM runtime setter,
controller mirror or profile-owned mutable state was added.

`core_machine_pic_initialize_with_topology` omits `A0h`/`A1h` decode for the
single-PIC selection.  `core_machine_dma_initialize_with_topology` omits the
secondary DMA register decode; existing controller-level initialization
functions retain their old PC/AT-default entry points so individual controller
tests do not acquire machine-profile knowledge.  Core remains the sole PIC and
DMA state owner.

The plan validator requires a selected DMA topology to agree with the copied
configuration.  The focused plan smoke proves 256 KiB/8088, primary PIC/DMA
ports present, and slave PIC/secondary-DMA ports absent.  The 5160 resolver
smoke proves those values are copied and cannot be changed through a session
request.

## Limits And Next Receiver

This P1 does not make the 5160 session runnable.  It does not bind a ROM,
keyboard/PPI, FDC drive/media, CGA session provider, Xebec, or an XT parity
port; those remain the admitted later B2/B3--B6 batches.  The private secondary
DMA storage remains an implementation collaborator for the existing Core DMA
owner, but it has no guest-visible XT port decode or binding.  Replacing that
owner-local implementation with a new DMA hierarchy is neither needed nor
admitted by this functional B2 step.

The prior S4 memory observation is corrected: 256 KiB is already accepted by
Core creation; the 2 MiB constant governs stopped-state reconfiguration.  The
remaining low-memory/ROM work is profile materialization and external-BYOB
validation, not a fabricated Core memory limit.

## Verification

- `cmake --build build/mingw-gcc-x64 --target core-machine -j 4` passed.
- `core-machine-plan-smoke.exe` passed, including both S5 markers.
- `vm-xt-5160-268-profile-smoke.exe` passed.
