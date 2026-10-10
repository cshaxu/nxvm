# T548 S27 — My5160/My5170 Unit Ownership Ledger

## Scope and method

This ledger covers every current My5160 and My5170 unit entry.  Each entry was
reviewed against the Core test corpus by asserted predicate, not by its include
set.  Core owns reusable chip, Board and Machine algorithms.  An App retains a
test when its assertion selects a fixed board identity, profile descriptor,
firmware role, topology, clock ratio, memory population, or real composed
route that Core deliberately does not choose.

| App entry | Asserted behavior | Core comparison | Disposition |
| --- | --- | --- | --- |
| `app-my5160/unit/profiles/profile_smoke.c` | Model 268 selects 8088, 256 KiB, XT PPI, single PIC/DMA, CGA, Xebec Type 2, XT FDC geometry and ports; constructed route exposes that exact topology; its BYOB asset roles map into the selected XT route. | Core independently tests generic Xebec wiring, FDC routing, CGA mechanics, plan validation and media lifecycle, but no Core test selects the Model 268 descriptor or its firmware/media identity. | Retain as one App profile contract.  Do not split generic port assertions out: their value here is the selected combination. |
| `app-my5160/unit/profiles/ibm_5160_dma_deadline_smoke.c` | The Model 268 configuration supplies a valid concrete deadline schedule to the generic DMA fixture. | `test/core/machine/support/dma_deadline_fixture.h` owns the generic deadline algorithm; this entry supplies only the XT configuration. | Retain as the narrow App receiver. |
| `app-my5170/unit/profiles/ibm_5170_plan_smoke.c` | Model 339 plan selects 80286, 512 KiB/default expansion range, cascaded PIC/DMA, physical time axis and firmware/media policy. | Core validates plans and construction mechanics but cannot choose Model 339 values or memory admissibility. | Retain. |
| `app-my5170/unit/profiles/ibm_5170_clock_contract_smoke.c` | Model 339 L2/L3 clock ratios, RTC/KBC cadence, timing dispositions and observed selected keyboard path. | Core tests KBC/PIT/RTC mechanics and timing policy independently; it does not own the 5170 source values or the composed cadence. | Retain. |
| `app-my5170/unit/profiles/ibm_5170_composition_smoke.c` | Model 339 font/ROM assets, 80286/planar parity/KBC/HDC/FDC choices, media admissibility, refresh polling and POST-adjacent DMA page I/O execute together. | Core has independent Port B, DMA, parity, HDC, FDC and refresh tests.  None selects the 5170 board plus its BIOS-visible values and firmware route. | Retain as a composed-profile regression; no generic mechanism receiver is removed. |
| `app-my5170/unit/profiles/ibm_5170_dma_deadline_smoke.c` | Model 339 configuration supplies a valid concrete deadline schedule. | Same generic Core fixture as the XT receiver. | Retain as the narrow App receiver. |
| `app-my5170/unit/profiles/ibm_5170_video_topology_smoke.c` | Model 339 exposes CGA ports/aperture and rejects the EGA aperture in the selected composed machine. | Core tests generic video ports/apertures but cannot establish the selected 5170 video personality. | Retain. |
| `app-my5170/unit/profiles/rom/ibm_5170_model_339_firmware_fdc_topology_smoke.c` | Model 339 firmware slot, CMOS 1.2 MB drive field and FDC IRQ6/DMA2 route agree with its composed machine. | Core tests generic FDC topology; firmware slot and CMOS profile values are App-owned. | Retain. |

## Resulting S27 change boundary

There is no equal-or-stronger Core receiver for any of these App contracts.
S27 may make only mechanical test-boundary repairs discovered during the
review (for example use the Lib C-runtime vocabulary instead of raw `stdio`),
remove duplicate includes, or narrow a test helper if that does not change its
assertion set.  It must not rehome, delete, or weaken any of the listed
profile/composition assertions.

## Remaining T548 route

The finite semantic batches remaining after S27 are:

1. **S27 — My5160/My5170:** finish mechanical boundary cleanup, affected dual-width tests and manifest evidence.
2. **S28 — MyDeskPro386:** apply the same behavior-level review to Model 40/D4 receivers, retaining its D4/CMOS/firmware/topology increments and moving only a proven generic predicate.
3. **S29 — Shared monitor receivers:** audit Lib, Emulator and Product entries for duplicate shared versus consumer assertions; retain consumer delegation/extension contracts.
4. **S30 — MyNES:** audit App-owned CPU/PPU/APU/mapper and product tests separately from shared monitor behavior; keep NES bus and snapshot increments.
5. **S31 — Closure:** re-audit every frozen ledger disposition, verify registration/manifests/dependency gates, then run the complete repository-only unit suite once on x64 and x86.  Integration/desktop checks are reported separately and are never relabelled as units.

No batch above authorizes production, firmware, media, INI, snapshot or deployed-artifact changes.  An unexpected gap remains in its original owner until separately admitted.
