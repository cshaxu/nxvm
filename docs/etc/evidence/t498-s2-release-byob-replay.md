# T498 S2 Release BYOB Replay

`M5:T498:S2:REPLAY:OWNER-BATCH`

## Replayed Universe

The frozen matrix contains nine rows.  The Release `vm-byob-dos-boot-probe`
was rebuilt, then used in Turbo with owner-supplied external firmware and DOS
media.  A terminal is semantic only: `dos-prompt`, `date-input`, or
`installer-ready`; time limits are diagnostic containment, never success.

| Profile | CPU | Format | Release result | Disposition |
| --- | --- | --- | --- | --- |
| IBM 5160 Model 268 | 8088 | 360 KB | `installer-ready` | Accepted. |
| IBM 5170 Model 339 | 80286 | 360 KB | `installer-ready` | Accepted. |
| IBM 5170 Model 339 | 80286 | 1.2 MB | No supplied 80286-compatible 1.2-MB DOS boot image. | External-input gap; the supplied DOS 6.22 image actively loads `CD1.SYS`, whose observed PCI probe reaches an operand-size prefix on every pre-80386 row. It remains an 80386 control, not evidence that DOS itself requires an 80386. |
| DeskPro 386 Model 40 | 80386 | 1.2 MB | No semantic terminal. | Full first-failure batch transfers to S3. |
| `default-at` | 8086 | 360 KB | `installer-ready` | Accepted. |
| `default-at` | 80186 | 720 KB | `date-input` | Accepted. |
| `default-at` | 80286 | 360 KB | `installer-ready` | Accepted. |
| `default-at` | 80386 | 1.2 MB | `dos-prompt` | Accepted. |
| `default-at` | 80386 | 1.44 MB | `dos-prompt` | Accepted. |

## Shared Observation Repair

The Core display API reports `TYPE_STATUS_UNSUPPORTED` while a configured EGA
adapter has not yet enabled output.  That is a valid unavailable copied
snapshot, not an execution or display failure.  The shared BYOB probe now
continues only for that exact status, reschedules its next capture either way,
and still fails for every other status.  It owns no guest state and has no
profile branch; all seven accepted rows above retain their terminal after the
change.

## Model 40 First-Failure Boundary

With a valid external even/odd 16-KiB firmware pair, the existing retirement
capture reaches 18,255 classified instructions, including the firmware's
protected-mode transition and return.  Therefore ROM load, interleaving,
reset aliasing, and the initial CPU path are not the failure.

After the observation repair, the Model 40 probe executes 9,412,999
instructions in its bounded diagnostic run but reaches no FDC command or
semantic display terminal.  Its final boot-loader diagnostic names predecessor
`07CEBh`; this is no longer an unavailable-display result.  S3 must audit the
complete Model 40 firmware-to-FDD startup chain (resolved plan, ROM/reset
mapping, firmware-visible memory and ports, FDC/DMA/PIC signal path, and
VADP publication) before selecting any production repair.  It must not add a
firmware shortcut, profile-side controller, media inference, or timeout pass.

No external path, hash, firmware byte, or guest-media byte is retained here.
