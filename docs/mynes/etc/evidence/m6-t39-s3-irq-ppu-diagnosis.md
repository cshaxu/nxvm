# M6 T39 S3 MMC3 IRQ/PPU Diagnosis

## Scope And Method

S3 used build-ignored C probes with the owner-local Mapper-004 input.  They
run the production Driver/Core path only; the input's path, bytes, digest,
screenshots and raw traces remain outside the repository.  The probes compare
finite same-build states and emit only derived observations.

## Disposition Matrix

| Candidate | Observation | Disposition |
| --- | --- | --- |
| S2 `$2006` A12 publication | Rebuilding the current probe with the pre-S2 `ppu.c` produced the same bounded PPU register, palette and IRQ state. | No bounded difference; do not select this patch for reversion. |
| Controller and guest execution | Start alters guest RAM, while a matched no-input/Start pair retains identical black published frames. | Input reaches the guest but does not establish a display transition. |
| PPU publication | Rendering is enabled and palette state changes during initialization; the emitted frame at the examined checkpoint remains black. | Frame publication is not independently selected. |
| MMC3 IRQ delivery | Normal execution reaches a persistent cartridge IRQ state and runs an IRQ-heavy path.  An ignored A/B run that suppresses only the cartridge IRQ line restores a normal bounded frame cadence. | Repair receiver: correct the demonstrable counter/reload contract before drawing further visual conclusions. |

The existing implementation sets the MMC3 reload flag on `$C001`, but leaves
the live counter intact.  The authoritative Mapper-004 contract requires that
`$C001` clear the counter and request reload at the next qualified A12 rising
edge.  This is a project-owned Cartridge correction, not a shared-component
change.  The hardware reference also confirms that an asserted MMC3 IRQ is a
level signal and must be acknowledged at `$E000` by the guest.

## Transfer To S4

S4 must update the Cartridge's `$C001` state transition and retain an owned
Mapper-004 regression for counter clearing, next-edge reload and IRQ
acknowledgement.  It must re-run the finite owner-local frame probe, but may
claim a visual transition only when that probe observes one directly.

Reference: [NESdev MMC3 register and IRQ contract](https://www.nesdev.org/wiki/MMC3).
