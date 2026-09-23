# T484 S5 P5 5160 B2 Core Topology

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

## P5: One Resolved Board-Plan Path

The remaining session-local PC/AT board materialization is removed.  A resolved
PC/AT profile now copies its `core_machine_plan_topology` alongside the copied
Core configuration and timing rules.  Its profile-local materializer is the
sole reader of PC/AT port leaves, routes, CMOS defaults, display selection,
DMA/FDC binding, planar-parity selection, and RTC timing provenance.  Session
composition retains dynamic media/display-provider binding and passes only the
immutable copied topology to the Core plan.

This puts current `default-at` and IBM 5170 on the same
`resolved profile -> copied topology -> Core plan` direction as the existing
5160 B2 snapshot.  It introduces neither an XT session route nor an AT alias:
the fixed 5160 declaration remains unavailable until its later B3--B6
receivers select their own sourced additions.

Historical white-box fixtures that deliberately supply a raw descriptor (also
including malformed-descriptor failure cases) use that same profile
materializer; they do not restore session-local leaf or route interpretation.

The profile validator now identifies IBM 5170 by its immutable firmware
personality rather than by the selected CPU enum.  Consequently a generic
`default-at` request may select 80286/80287 without becoming a false 5170
profile; the focused session regression creates that selected pair through the
same copied topology.

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

## P6: Source-Gated B2 Boundary

The selected `256 KiB` is already the copied Core construction value and the
focused plan creates it.  The IBM baseline proves only an abstract external
pair of 32 KiB ROM devices; it deliberately selects no ROM revision, bytes,
digest, local path, or requested physical mapping.  The source policy requires
all of those owner-supplied fields at a later VM-composition construction
boundary.  Thus B2 retains the immutable `BYOB` policy but cannot create a
default ROM mapping or a second reset route.  That finite mapping/manifest work
transfers to B6, where it must reach Core's existing immutable-ROM owner before
machine creation.

The IBM reference also describes `RESET DRV` as a system-board output to reset
or initialize expansion logic.  Core's ordinary cold reset already owns the
selected machine-state reset; this B2 baseline selects no expansion controller
whose reset state needs another owner.  The remaining program-visible parity/
NMI controls are inseparable from the XT 8255/PPI board attachment at `60h`--
`63h` and its source-qualified port semantics.  They transfer to B3 with that
one attachment.  In particular, B2 does not reinterpret the PC/AT-only
planar-parity controller at `61h` as an XT controller.

The focused Core-plan regression now proves the negative construction facts
directly: a single-PIC/one-DMA XT plan exposes master/primary ports, omits
slave/secondary and RTC/CMOS ports, has no FDC DMA binding, and retains those
choices across the Core-owned cold reset.

## Verification

- `cmake --build build/mingw-gcc-x64 --target core-machine -j 4` passed.
- `core-machine-plan-smoke.exe` passed, including both S5 markers.
- `vm-xt-5160-268-profile-smoke.exe` passed.
- 5170/default-at/Model-40 resolver smoke asserts explicit cascaded/two-DMA
  materialization.
- The XT profile smoke constructs the copied B2 topology and proves that no
  FDC request binding exists before B3 selects one.
- The Core-plan smoke proves `70h`/`71h` RTC/CMOS and the unselected
  slave/secondary controller ports remain absent before and after cold reset.
- `vm-ibm-5170-root-resolver-smoke.exe` proves copied 5170 and default-AT
  topology values can be given directly to `core_machine_plan_set_topology`,
  including the generic 80286/80287 selection.
- `vm-product-session-smoke.exe` creates a default-AT 80286/80287 session
  through that resolved topology.
- A Debug full build and the 297-target `current-gate` pass.  Release target
  `vm-0-5-0484` rebuilds the stripped
  `build/output/nxvm_0_5_0484.exe` (1,216,262 bytes, SHA-256
  `F577B321CAFB0B9B5C566C5507C05F663978531F388B3F18D2A45294730F5E66`).
