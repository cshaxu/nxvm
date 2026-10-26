# M5 T580 S63 — PC110 Font-ROM window receiver

## Source and boundary

`O:/repos.external/Open-Source-PC110/Discovery/Chipset/readme.md`, section
13e, records the live, post-initialization PC110 Font ROM window: an 8 KiB
aperture at `DE000h`, a 7-bit bank register at `1160h`, `FFh` from unused
`1161h`, fixed segment value `DEh` at `1162h`, bit-0 enable/readback at
`1163h`, `80h`/`C0h` bank aliases, and `FFh` reads while deselected. This is
external live-dump evidence (L2). It does not establish a power-on reset
value, persistence behavior, or a writable `1161h` register.

The receiver remains wholly in `app-mypc110`. It has one immutable Font-ROM
object, one four-port provider installation, and one existing memory-provider
route. The route owns disabled aperture reads and returns the observed `FFh`;
declining the access would incorrectly fall through to ordinary RAM. No Shared
component, firmware, INI, ROM/font byte, asset, panel path, or generic window
framework changed.

## Model

- `1160h` selects and reads a bank with bit 7 masked.
- `1161h` always reads `FFh`; writes retain no invented scratch state.
- `1162h` always reads `DEh`; writes neither relocate nor otherwise alter the
  physical aperture.
- `1163h` exposes only bit 0. Disabled aperture reads are `FFh`; enabled reads
  expose the selected immutable 8 KiB bank at `DE000h`.

The initial enabled state follows the cited live observation only. This S does
not claim a documented reset contract.

## Verification

The existing `mypc110.unit/font-rom` test now covers all four readbacks, bank
aliases, immutable writes, ignored unknown-control writes, disable/open-bus
reads, re-enable, and ordinary-RAM ownership immediately outside the aperture.

After rebuilding the unit graph, complete repository unit suites passed:

| Host width | Result |
| --- | --- |
| x64 | 714 / 714 |
| x86 | 714 / 714 |

The two PC110 preview executables were relinked in the reusable Ninja trees
but are deliberately unpublished: PC110 remains unqualified for product
release. This is not proof of DOS/V font use, panel scanout, persisted setup,
guest boot, PCIC/media operation, desktop/RDP behavior, or the four-artifact
release gate.
