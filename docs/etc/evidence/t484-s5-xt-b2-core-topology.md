# T484 S5 P3 5160 B2 Core Topology

`M5:T484:S5:XT-B2-PLAN:OK`

`M5:T484:S5:XT-NO-AT-TOPOLOGY:OK`

## Result

The immutable Core configuration now owns two construction-only selections:
`pic_topology` and `dma_controller_count`.  Every product profile materializes
one of those selections explicitly: 5160-268 copies single-PIC/one-DMA;
5170/default-at and Model-40 copy cascaded/two-DMA before a machine is
created.  Zero remains only a direct-Core-fixture compatibility value and is
not a product-profile choice.  No VM runtime setter, controller mirror or
profile-owned mutable state was added.

`core_machine_pic_initialize` omits `A0h`/`A1h` decode for the
single-PIC selection.  `core_machine_dma_initialize` omits the
secondary DMA register decode.  Every Core and focused controller-test call
now supplies its topology explicitly; the PC/AT, IBM 5170 and Model-40 profile
materializers likewise copy explicit cascaded/two-DMA values.  Core remains
the sole PIC and DMA state owner.

The plan validator requires a selected DMA topology to agree with the copied
configuration.  The focused plan smoke proves 256 KiB/8088, primary PIC/DMA
ports present, and slave PIC/secondary-DMA ports absent.  The 5160 resolver
smoke proves those values are copied and cannot be changed through a session
request.

## P3: Refresh Is Not An FDC Claim

The former `core_machine_dma_wiring` incorrectly required an FDC request
channel whenever a board selected DMA, although the refresh route is
independent and the 5160 B2 scope selects no FDC.  `fdc_channel` may now be
the explicit `CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND` value.  Core still owns
the refresh channel and all DMA state; it signs an FDC request binding only
when a later copied FDC topology selects a channel.  A plan that does select
an FDC rejects the unbound value.

The resolved 5160 snapshot copies exactly that one-controller, no-cascade,
unbound-FDC topology.  It can therefore construct the selected PIC/PIT/DMA
board facts without inventing an AT FDC route, RTC/CMOS device or second DMA
controller.  B3 remains the sole receiver for an evidence-qualified XT FDC
binding.

## Limits And Next Receiver

This B2 work does not make the 5160 session runnable.  It does not bind a ROM,
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
- 5170/default-at/Model-40 resolver smoke asserts explicit cascaded/two-DMA
  materialization.
- The XT profile smoke constructs the copied B2 topology and proves that no
  FDC request binding exists before B3 selects one.
