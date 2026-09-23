# T484 S9 IBM 5160-268 Diskette Source Contract

`M5:T484:S9:XT-FDC-SOURCE:OK`

`M5:T484:S9:XT-NO-AT-FDC-ALIAS:OK`

## Scope And Method

This record qualifies only the selected IBM 5160-268 diskette attachment
needed by T484 B3.  It re-reads the IBM sources in rendered form where OCR is
ambiguous and compares their result with the read-only PCjs implementation.
IBM material is normative.  PCjs is `Other L3` corroboration only; no source,
ROM, media, code, path, or service-time value is imported.

Only PCjs was available under `O:\repos.external` for this review; no absent
86Box, MAME, Bochs, or QEMU checkout is represented as a comparison.

## Source-Qualified Immutable Selection

| Required relation | Source-quality disposition | Evidence and resulting value |
| --- | --- | --- |
| Selected model and drive count | Manual L3 | IBM *Personal Computer Product Reference*, fourth edition (April 1987), lists `5160-268` as XT-256KB with one half-height double-sided diskette drive. |
| Selected drive class and capacity | Manual L3 composite | IBM *5160 Service Manual* describes the normal PC/XT as one 5.25-inch, 360KB diskette drive.  Read together with the model-specific one-half-height-double-sided selection, this selects one 5.25-inch 360KiB drive; neither document asserts a 1.2MB or 1.44MB drive for this selected configuration. |
| Logical geometry | Manual L3 | IBM *5155/5160 Technical Reference*, Diskette Adapter BIOS listing dated 1986-01-10, defines a 320/360K drive as tracks `0-39`, heads `0-1`, sectors `1-6/9`, and its 360K path uses the nine-sector form.  The established 360KiB declaration is therefore `40 x 2 x 9 x 512 = 368,640` bytes. |
| Transfer rate | Manual L3 | The same IBM listing defines `RATE_250` as 250 Kb/s and its 360K path selects that rate. |
| FDC control and reset | Manual L3 | The IBM listing names `03F2h` the adapter/FDC control port and its diskette-system reset holds then releases the NEC controller through that port. |
| Interrupt and DMA route | Manual L3 | The IBM 5155/5160 Technical Reference I/O-channel material identifies IRQ6 and DRQ2 for the diskette controller path. |
| FDC command/data route | Manual L3 | The IBM I/O map reserves `3F0h-3F7h` for the diskette controller; the existing Intel-8272A Core owner consumes its selected control, main-status, and data ports as `3F2h`, `3F4h`, and `3F5h`. |

PCjs independently configures its IBM 5160 examples with a 40-cylinder,
two-head, nine-sector, 512-byte drive.  Its FDC implementation also labels
`3F7h` digital-input/control as Model-5170-only.  Those observations support,
but do not establish, the primary-source result.

## Deliberate `3F7h` Boundary

The later 1986 IBM Diskette Adapter BIOS listing uses `3F7h` to program a data
rate and to read disk-change state for its 360K/1.2MB-capable adapter.  That is
not evidence that the selected 5160-268's base attachment has the PC/AT
`3F7h` direction/control route.  PCjs makes the same Model-5170 distinction,
but an emulator cannot settle the hardware question.

Accordingly, T484's selected declaration binds only the source-required
`3F2h` control, `3F4h` main-status, `3F5h` data, IRQ6, and DMA2 route.
`direction_port` and `control_port` remain absent in its immutable Core FDC
config.  This corrects the earlier S6 shorthand: the rule is not that an IBM
document can never mention `3F7h`; it is that no PC/AT `3F7h` behavior is
inherited by this selected XT path without an XT-specific source.

## Current Owner And One Receiver

Current Core already has the sole Intel 8272A state owner and accepts one
copied `core_machine_fdc_config` plus immutable per-drive media IDs through
`core_machine_plan_configure_fdc()`.  Its media contract already carries
geometry; it does not need a second XT geometry or FDC state owner.

The next implementation S may add exactly one selected-profile construction
route with:

```text
drive A: removable 40 x 2 x 9 x 512 media declaration
FDC:     3F2h, 3F4h, 3F5h; IRQ6; DMA2; no 3F7h route
```

The profile supplies only frozen configuration.  Core retains controller
registers, command phase, reset, IRQ, DRQ, deadline, and media access; the
VM-side image provider retains only host bytes and persistence.  No generic
XT profile, AT alias, parallel FDC, media cache, firmware image, or default
guest disk is admissible.

## Explicit Transfers

The selected logical media geometry and functional routing are ready for the
next bounded implementation receiver.  Physical spindle, seek, head-load,
index, and board service-duration data have not been selected from a source
that applies to this specific mechanism.  They remain outside this functional
binding and cannot become an estimated FDC deadline or host pacing input.
