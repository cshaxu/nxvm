# M4 T25 Automated Acceptance Record

## Recorded Evidence

The two owner-local support inputs were exercised without copying their bytes
into the repository. The bounded owner probe reports mapper identity, frame
count, PCM queue state, dropped sample count and Core trap state.

| Input | Architecture | Observation | Result |
| --- | --- | --- | --- |
| Dr. Mario / MMC1 | x64 | 1,280,000 paced driver slices | 325,071,828 instructions, 34,484 frames, zero Core trap and zero dropped PCM samples in 574.48 seconds. |
| Super Mario Bros. 2 / MMC3 | x64 | 1,280,000 paced driver slices | 325,692,166 instructions, 33,683 frames, zero Core trap and zero dropped PCM samples in 561.11 seconds. |
| Dr. Mario / MMC1 | x86 | 1,280,000 paced driver slices | 325,071,828 instructions, 34,484 frames, zero Core trap and zero dropped PCM samples in 574.50 seconds. |
| Super Mario Bros. 2 / MMC3 | x86 Release | 1,280,000 paced driver slices | 325,692,166 instructions, 33,683 frames, zero Core trap and zero dropped PCM samples in 561.19 seconds. |

The x64 and x86 Release double-representation probe then ran each input for
20,000 slices in each mode. Each produced a valid Window graphics frame and a
valid Core-generated 80x25 Console text frame, with zero traps and zero dropped
PCM samples. These are representation facts; the Console still receives text
only and does not render NES graphics itself.

After the x86 Release rebuild, the 39 M4-relevant tests passed: Core, mapper,
APU, audio stream, App composition, controller ROM, native Window and native
Console. The native Console acceptance was repeated after a real x86 raw-input
cutover race was repaired; the fix reissues a physical key cycle until a guest
frame consumes it. The same native Console scenario passed twenty consecutive
times on x64 and at least nineteen consecutive times on x86 before the former
flaky assertion was removed by the repaired protocol.

## Remaining Direct Observation

Automated evidence proves named-ROM execution, frame publication, Core text
conversion, PCM submission/bounds, controller routing and native KVM lifecycle.
It cannot honestly prove a human saw a particular named title/gameplay state or
heard the host speaker. The available desktop automation surface exposes no
native application window or speaker observation. T25 therefore remains open
for the owner-visible Window and Console play observations required by the M4
complete-experience contract: title/gameplay transition after the mapped
controls, audible pause/resume, and the configured media-management flow.

## Owner Acceptance

On 2026-09-21 the owner tested the current MyNes executable and reported it
satisfactory, then explicitly authorized S and T closure. This supplies the
previously unavailable direct game-experience acceptance for the configured
Window/Console product route. No ROM bytes or local file paths were recorded.
