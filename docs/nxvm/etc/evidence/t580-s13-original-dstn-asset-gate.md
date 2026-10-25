# M5 T580 S13 — original DSTN asset gate

## Scope

This is an external-asset identity check. It does not import, modify, derive,
or embed firmware.

## Candidates checked

| Repository path | SHA-256 / disposition |
| --- | --- |
| `PC110-EMU/Roms/pc110_bios.bin` | `232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312`; identical to the admitted TFT BIOS. |
| `Open-Source-PC110/Mods/LBA-BIOS-Patch/PC110_BIOS_lba.BIN` | `6c859ad263ef18e6eb4568d3ff426f4e0ec1dd624039e74c0acb5b54bd2bec1d`; explicitly an LBA-modified full image, not a stock input. |
| `PC110-QEMU` | No full PC110 BIOS image is present. |

Open-Source-PC110's `Mods/LBA-BIOS-Patch/README.md` says the LBA image has
never been run on hardware or an emulator. Its `Mods/TFT/bios-patch/README.md`
states that a correct TFT-to-DSTN reverse requires a confirmed unpatched DSTN
dump and that the archived BIOS already contains TFT values.

## Disposition

No candidate is a verified original-DSTN BIOS/font pair. The original-DSTN
profile and its two artifacts remain an external BYOB gate. The TFT evidence
and source remain unchanged; no guessed reverse patch or derived firmware is
admitted.
