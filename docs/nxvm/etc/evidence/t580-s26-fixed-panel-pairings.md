# M5 T580 S26 — fixed PC110 panel build pairings

## Decision

MyPC110 exposes two separate preview build identities, not a runtime model
selector.  Each generated firmware object embeds exactly one immutable external
ROM input:

| Variant | Machine identity | Product identity | BIOS SHA-256 |
| --- | --- | --- | --- |
| TFT | `ibm-pc110-tft` | `MyPC110 TFT` | `232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312` |
| DSTN | `ibm-pc110-dstn-derived` | `MyPC110 DSTN` | `aa2ab27a8cc4fee059238fdbd4bd869f209f5f1c3a67096fc8704ca795857121` |

Both variants reuse the App-owned board and Kanji font route and retain the
same adjacent `mypc110.ini`.  The executable never selects a panel or ROM from
INI.  The ROMs remain external BYOB build inputs and are not repository files.

## Source naming

The firmware map is now named `mypc110_firmware`: it maps a selected immutable
PC110 image and contains no TFT-specific behavior.  Panel-specific terms are
limited to the two CMake build inputs and their fixed product identities.

## Verification

The TFT and DSTN product-composition units each assert their immutable machine
identity and a panel-table byte at full-ROM offset `0x20AF6`.  Direct App units
and the existing real-firmware diagnostics remain separate: the latter run the
qualified TFT image only and do not claim DSTN POST or panel qualification.

This S establishes build identity only.  It does not qualify real POST, Setup,
DOS, power/MCU/PCIC behavior, panel output, lifecycle UX, or release-artifact
publication.
