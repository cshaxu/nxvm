# M5 Current-Product Device-Capability L3 Closure

## Purpose

Close source-backed, profile-local L3 timing for every device capability that
the current NXVM product exposes. This follows the three baseline-machine
closures and precedes every Windows 3.1 candidate and the final M5 audit. It
does not broaden the product merely to make an unimplemented historical
peripheral eligible for M5.

## Required scope

First refresh the current-capability inventory and map every supported device
surface to one exact baseline profile or a separately declared current-product
configuration. At minimum, reconcile the bounded ATA PIO/HDD and FDD boot
paths, the currently exposed CGA/EGA digital display paths, and the bounded
KBC/AUX path with their port/memory/DMA/IRQ/reset and host-copy boundaries.
For each supported capability, establish the source contract, command/service
and availability timing, event order, cancellation/reset behavior, trace, and
project-owned firmware/DOS consumer. Reuse the 5170, DeskPro Model 40, and
PC/XT 5160-268
ledgers only after proving profile, device, and timing-accounting equivalence.

Every entry in the refreshed inventory must be either L3-closed or removed from
the supported product surface before this candidate closes. A compatibility
label is not a transfer for a capability the product still exposes.

## Non-goals and stop conditions

Do not add advanced AUX protocols, unimplemented CGA/EGA/VGA breadth,
composite video, speaker/PPI, serial, parallel, game-port, generic host-device
passthrough, broad ATA/IDE extensions, analog/pin waveforms, or host-time
pacing. Those remain unplanned debt unless a later owner request expands the
supported product surface. Stop a capability if no profile-local primary
contract and project-owned corpus can establish its timing; remove it from the
supported surface or block M5 closure rather than assign a synthetic delay.

## Evidence standard

Require a product-device inventory with support status and profile assignment;
one source-to-model/service ledger per supported capability; command-to-
IRQ/DRQ/frame trace evidence; reset, cancellation, and host-boundary proof;
focused regressions plus the current gate; and an explicit no-supported-device-
left-behind decision. The resulting evidence is a prerequisite for Windows and
the final M5 audit, not a physical cycle-exact or broader-device claim.
