# T576 S1: PC110 C-segment video shadow

## Evidence and model

The admitted TFT firmware remains an external BYOB input.  No ROM bytes or
third-party implementation source are committed by this task.

Open-Source-PC110 is used only as L2 behavioural research:

- `Discovery/BIOS-Flash/readme.md` identifies bank 2 as the video BIOS source
  and records the runtime C000 shadow.
- `Discovery/65535/readme.md` records that the E-bank video source is copied
  to C000 at runtime.
- `Discovery/Chipset/readme.md` records the observed EC/ED sequence: ROMSET
  (`0Ch`) open, CAXS (`0Fh`) and CCBL (`15h`) low six bits `2Ah`, then ROMSET
  relock.  Its three-16-KiB CAXS interpretation bounds the copy at
  `C0000–CBFFF`; `CC000–EFFFF` remains open UMA.

The App therefore owns one 48 KiB backing store.  Reset clears it and leaves
the immutable ROM route visible.  The observed EC/ED operations alone open
writes and, after the two C-shadow values followed by relock, expose that
backing store.  Core only dispatches its existing replacement routes and has
no PC110 branch.

## Verification

`mypc110.unit/config-route` proves:

1. C000 initially falls through to immutable ROM.
2. The observed open/CAXS/CCBL/relock sequence admits a write and exposes it.
3. `CC000` remains outside the 48 KiB replacement route and keeps its immutable
   raw owner.
4. Reset discards the C shadow and restores the raw C000 route.

The App's existing F000/reset overlay tests remain unchanged.  The retained
`mypc110.diagnostic/real-firmware-post` still reaches T575's real-firmware
checkpoint after `F000:6044` on both host widths.

| Architecture | Unit/config route | Firmware binding | Base composition | Real-firmware diagnostic |
| --- | --- | --- | --- | --- |
| x64 | pass | pass | pass | pass |
| x86 | pass | pass | pass | pass |

This does **not** prove that the real firmware has executed C000.  A later
receiver must handle the next independently reproduced protected-segment
reload at `F000:605F`; no invented descriptor, BIOS interception, PC/AT
fallback or artificial C000 jump is introduced here.
