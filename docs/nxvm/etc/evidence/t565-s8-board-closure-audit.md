# M5 T565 S8 — PC110 board mapping closure audit

## Closure universe

T565 consumes the board/chipset/firmware **mapping foundation** that can be
established without fabricating an AT board: the PC110 VL82C420 configuration
mechanism, its physical configuration windows and gate, and a typed TFT BIOS
mapping.  The table below reconciles every remaining broad-board item against
the earliest correct receiver rather than treating port presence as a complete
device model.

| Remaining item | Disposition | Receiver / reason |
| --- | --- | --- |
| Shared PC110 construction root; PIC, PIT, DMA and RTC substrate | Transferred | `m5-pc110-base-composition.md`. Existing `x86-nxvm-board-base` creates VADP, 8042, A20, reset-alias and AT-profile semantics together, so it cannot be used as a neutral controller bundle. |
| VL82C420 SCAMP/Block2/EC-ED physical decode | Accepted | T565 S3/S5/S7. One Core protocol owner and one App port decoder now exist. |
| TFT immutable BIOS C0000h--FFFFFh mapping | Accepted | T565 S6. It is a copied, typed Core firmware provider, not runtime asset I/O or a boot claim. |
| Cold/warm reset image, RAM decode, shadow and cross-window transforms | Transferred | Base-composition task and real-firmware boot task; no documented reset table authorizes a model now. |
| Keyboard, pointer, A20 and power/suspend | Transferred | Existing PC110 input/power candidate. PC110's MCU/Pluto behavior is not an ordinary AT 8042 contract. |
| F65535, panel timing and font ROM | Transferred | Existing display candidate. |
| FDC, IDE/PCMCIA, flash and boot storage | Transferred | Existing storage candidate. |
| Audio, serial/dock and remaining peripheral routes | Transferred | Existing audio/aux candidate. |
| Original DSTN firmware and all executable/guest claims | Blocked / transferred | Original pairing remains unqualified; build embedding and POST/guest proof belong to firmware-boot and qualification tasks. |

## Why direct controller reuse is rejected

The Open-Source-PC110 live dump proves standard PIC, PIT, DMA and RTC port
families are present.  It does not turn the current all-in-one
`x86-nxvm-board-base` constructor into a PC110 substrate: that constructor
also installs VADP, an ordinary 8042, A20 decoding, reset aliases and profile
callbacks.  Those are precisely the separate PC110 contracts still unresolved.
Using it unchanged would create a second, implicit PC110 model and violate the
one-owner rule.  The successor therefore owns an explicit extraction/reuse
decision and must prove atomic construction before any controller becomes live.

## S8 verification

No production behavior changed in S8.  The closure audit re-read T565 S1--S7,
the current `app-mypc110`, `x86-core` VL82C420 and `x86-nxvm-board-base`
construction paths, and the pinned Open-Source-PC110 evidence.  S7's direct
dual-width route/chip regressions remain the latest applicable executable
evidence.  No EXE, external ROM, media, INI or Shared component is changed.

T565 closes only as the mapping-foundation task described above.  It does not
claim a complete board, POST, desktop, guest boot or hardware qualification.
