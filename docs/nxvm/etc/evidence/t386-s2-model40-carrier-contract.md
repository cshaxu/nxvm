# T386 S2: Model-40 Profile Carrier And Publication Contract

`M5:T386:S2:MODEL40-CARRIER-CONTRACT:OK`

`M5:T386:S2:NONRUNNABLE-PUBLICATION-GUARD:OK`

## Current Carrier Is Not A General PC/AT Contract

The only carrier type is `vm_profile_default_pc_at_descriptor`. Its validity
function accepts either the hard-coded Model-339 identity or a descriptor whose
ports, routes, firmware slot, ATA PIO register block, IRQ14 and EGA state are
the `default-pc-at` shape. The session factory selects only those two identities.
Consequently, adding a Model-40 value to the current enum or YAML parser would
publish a false machine: its lifecycle always binds the generated default BIOS,
the ATA `core_machine_hdc` path when `hdc_present`, and default-profile EGA
firmware/configuration when `ega_present`.

| Model-40 requirement | Current carrier/consumer disposition | Later implementation receiver | Publication condition |
| --- | --- | --- | --- |
| 80386DX-16, no x87, one MiB | Descriptor fields and shared CPU contract can express the semantic CPU/no-FPU and memory selection; current generic overrides must remain unavailable. | Profile carrier plus session CPU/materialization validation. | May be carried privately; no public profile yet. |
| 128 KiB external ROM geometry | ROM mapping field can express geometry, but the sole firmware slot/context binds generated default BIOS. | New product-local external-firmware provider/binding contract, separately admitted after the carrier. | No catalog/YAML entry until a real BYOB provider is bound. |
| PIC/DMA, primary PIT, RTC/CMOS, keyboard IRQ1 and FDC IRQ6/DMA2 | Port leaves/routes and session storage configure these common mechanisms. Current routes are hard-coded to the shared default array and include AUX/IRQ12. | Carrier split/validation; then S3/S4/S5 source owners. | The carrier must select keyboard-only and reject AUX before publication. |
| Second PIT at `48h-4Bh` | No carrier role/route exists and core owns only one configured PIT. | S3 shared PIT instance/port/reset owner, followed by Model-40 composition. | Blocks runnable publication. |
| D4 NMI/reset/A20 and port `61h` latches | Not represented; current generic KBC and CMOS NMI mask do not model this board-local state. | S4 product/platform composition around the earliest shared state owner. | Blocks runnable publication. |
| Compaq Multipurpose Fixed Disk Controller and 40 MB disk | `hdc_present` means the ATA PIO struct, `core_machine_hdc`, generic HDD media and generated INT 13h path. This is expressly not the selected Compaq controller. | S5 first sources its controller contract, then gives it its own composition/provider/media route. | Blocks runnable publication; no ATA or IBM-MFM fallback. |
| Selected 1.2 MB FDC/drive | Generic 765 topology uses profile port leaves and FDD media, but its firmware and drive policy are default-profile behavior. | S5 validates selected controller/drive, then binds it through the carrier. | Blocks runnable publication until selected behavior is proven. |
| Compaq EGA plus color monitor | `ega_present` exposes default VADP configuration, generic ports and generated video firmware; it is not a Compaq personality. | S6 small Compaq VADP personality with its own monitor/firmware-facing contract. | Blocks runnable publication; IBM EGA remains excluded. |

## Required Carrier Shape And Guard

Do not add a third `vm_session_profile_kind`, YAML spelling, session-factory
mapping or console catalog entry yet. The first source S must instead separate
an immutable product-local Model-40 profile specification from the current
`default-pc-at` descriptor validation, with explicit unavailable receivers for
firmware, second PIT, D4 platform state, fixed-disk controller and Compaq EGA.
It may not be passed to `vm_session_create`, `vm_session_storage_initialize`,
`vm_session_provider_lifecycle_initialize` or the default firmware provider.

After S3--S6 each supplies its complete owner contract and focused regression,
a later S may add a single selection/publication gate. That gate must require
all selected receivers available, reject generic overrides and selected-device
substitutes, and prove both direct `--profile` and YAML catalog paths reject an
incomplete Model-40 identity. The existing Model-339/default identities stay
unchanged.

## Receiver And Test Plan

- Profile source receiver: `src/vm/profile/` gains the Model-40 specification
  and its availability contract without making `default_profile` another truth
  source.
- Product composition receiver: `src/vm/composition/session/session.c`,
  `machine_devices.c`, `provider_lifecycle.c`, `profile_firmware.c` and media
  binding gain selected providers only after their hardware S closes.
- Public selection receiver: `session_interface.h`, `session_factory.c`, YAML
  catalog-to-Console transport and product profile smokes change only in the
  later publication S.
- Regressions: a private specification smoke proves selected facts and
  unavailable receivers; the future publication smoke must prove rejected
  direct/YAML selection before completeness and accepted selection only after
  S3--S6. Existing Model-339/default composition and catalog smokes remain
  required negative controls.

## Sweep Conclusion

The profile descriptor, session selector, session factory, lifecycle, media,
machine-device and default-firmware paths are all production hits. None can
currently host a truthful Model-40 profile; each is a named later receiver or
an unchanged control. No source change is made by this S, and this is neither
a runnable-model, firmware-binding, device-completeness nor L3 claim.