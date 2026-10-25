# M5 T580 S25 — DSTN Panel-Derived Firmware Input

## Decision

The external PC110 asset archive now contains one **derived** DSTN-panel
candidate, not a claimed recovered factory DSTN ROM:

```text
profiles-nxvm/ibm-pc110-dstn-derived/firmware/
  pc110-dstn-derived-2026-07-27.rom
```

It is derived from the admitted 256 KiB TFT BIOS by replacing exactly the 18
documented F65535 video-BIOS panel-table bytes with the stock-DSTN values
captured from one unmodified PC110. The TFT master remains unchanged.

| Input / output | SHA-256 |
| --- | --- |
| TFT parent | `232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312` |
| DSTN-derived candidate | `aa2ab27a8cc4fee059238fdbd4bd869f209f5f1c3a67096fc8704ca795857121` |

The external asset manifest records both identities and the derivation. The
candidate is not tracked in this repository.

## Provenance

At Open-Source-PC110 revision `02ccf2e8e19b298a2cf04c5d430278eb087cc368`:

| Input | SHA-256 | Relevant fact |
| --- | --- | --- |
| `Discovery/65535/readme.md` | `557850d64f004d75031e2821a2349934be9f439ea80428cf0ac65b4d4629f65c` | Section 6c.1 records all 18 stock-DSTN bytes as a live capture from one unmodified unit and distinguishes it from the TFT table. |
| `Mods/BIOS-Multi-Patcher/PCPATCH.ASM` | `a4b5dbb8a98acf72e8a4fa1307f270cc60d6dabe30ec1e192787a8de1cb3201` | `dstn_val` supplies the same 18 bytes for its TFT-to-DSTN recovery path. |
| `Mods/TFT/bios-patch/tft_patch.txt` | `a7a4d6a4bbee784e625488e9ecd2bb0df755a64e6856a2f23837e3d36bf222e7` | Names the 18 offsets relative to the video-BIOS bank at chip offset `20000h`. |

The two independent external records agree on the exact offsets and values.

## Mechanical verification

The transform has these measured facts:

- parent and candidate are both 262,144 bytes;
- exactly 18 bytes differ, all at `20000h +` a listed video-BIOS offset;
- the changed file offsets are `201CBh`, `20A9Ah`, `20A9Ch`, `20AA0h`,
  `20AA4h`, `20AA6h`, `20AB4h`, `20AB8h`, `20AE6h`, `20AE8h`, `20AEAh`,
  `20AF6h`, `20AF7h`, `20AFAh`, `20AFCh`, `20AFEh`, `20B00h`, and `20B04h`;
- every other parent byte is retained byte-for-byte.

## Boundary

This only establishes an external input candidate for a later, separately
admitted build pairing. It does not recover a vendor-original full DSTN ROM,
prove that the 18 values apply to every BIOS revision, implement a DSTN panel,
model power/MCU/PCIC hardware, establish POST/Setup/guest boot, or authorize
MyPC110 artifact publication. The existing App remains TFT-only until a later
task admits both fixed build pairings and independently qualifies them.
