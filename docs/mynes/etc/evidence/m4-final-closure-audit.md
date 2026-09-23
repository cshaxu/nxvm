# M4 Final Closure Audit

## Decision

M4 is closed. Its finite support boundary was two owner-local NTSC iNES inputs:
Dr. Mario through MMC1 and Super Mario Bros. 2 through MMC3. The closure does
not extend to arbitrary ROMs, mappers, regions, persistence, expansion audio,
additional controllers or save states.

## Exit Ledger

| M4 exit obligation | Evidence and disposition |
| --- | --- |
| MMC1 Dr. Mario machine path | [T21 profile closure](../../history/M4-T21-mmc1-profile.md) proves the bounded mapper-1 behavior and owner-local load observation. Accepted. |
| MMC3 Super Mario Bros. 2 machine path | [T22 profile closure](../../history/M4-T22-mmc3-profile.md) proves bounded banking, mirroring, A12-qualified IRQ and title-suffix normalization. Accepted. |
| RP2A03 audio generation | [T23 hardware closure](../../history/M4-T23-apu-hardware.md) proves five-channel state, MMIO, IRQ/DMC arbitration and bounded PCM. Accepted. |
| Host audio and one-times pacing | [T24 adoption and pacing](../../history/M4-T24-audio-adoption-pacing.md) proves the original product path; T25's named-input runs record zero dropped PCM in bounded execution. Accepted. |
| Window and Core-produced Console presentation | [T25 automated acceptance](m4-t25-automated-acceptance.md) records both representations on x64 and x86. Accepted. |
| Default controller, lifecycle and management experience | [T25 acceptance](../../history/M4-T25-supported-game-acceptance.md) records the owner's direct Window/Console experience approval, including input, pause/resume and cartridge workflow. Accepted. |
| Additional NROM owner experience | The owner also completed full acceptance for `smario1.nes`. It is an accepted supplementary Mapper-0/NROM experience, not a third member of M4's declared two-input compatibility boundary. |
| Shared Lib Audio corpus readiness | [T29 closure](m4-t29-lib-audio-corpus-closure.md) removes the dead interface, makes policy private and defines finite PCM delivery with dual-architecture 112-test evidence. Accepted. |
| No unclassified residual implementation work | `docs/states/TODO.md` has no open M4 debt; M4 queue entries are closed. Accepted. |

## Verification

The audit re-read the M4 plan, roadmap exit, all M4 closure rows, T25 automated
and owner-visible acceptance, T29 evidence, Queue and TODO. The audited scope
contains no unproved M4 obligation. The documentation governance gate and its
self-test pass. This document contains no local ROM bytes or paths.

## Transfer

M5 inherits only its roadmap scope: a newly declared finite compatibility and
release baseline. It does not inherit an open M4 defect, and it must perform
its own planning before any numeric implementation task is admitted.
