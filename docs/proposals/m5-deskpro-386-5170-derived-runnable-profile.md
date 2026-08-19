# M5 DeskPro 386 5170-Derived Runnable Profile

## Purpose

Make the existing Compaq DeskPro 386 Model 40 run as a narrow 80386DX/Compaq
delta over the accepted IBM PC/AT 5170 profile detail and the Core L3 contract
catalog. This is a runnable-profile convergence task, not a new physical
DeskPro reconstruction or a fresh device/timing program.

## Required Scope

Start from the frozen 5170 composition and list every retained 5170 field that
DeskPro inherits. Add only the declared 80386DX, memory/ROM map, Compaq
controller/display, firmware-policy and board-route deltas already present in
current code or accepted evidence. Reuse a Core contract only by its neutral
ID and retained evidence tier; do not create a DeskPro-only Core mechanism.

The resulting selected profile must create, reset and run through the existing
bounded startup and smoke corpus. It must preserve explicit L2 fallbacks and
reference-derived labels. The later VM profile foundation normalizes this
interim direct derivation into the general resolver; this candidate does not
attempt that framework.

## Dependencies

Consumes the six ordered Core L3 candidates and the accepted 5170 baseline. It
precedes VM PC profile foundation and makes no claim that DeskPro is a complete
L3 physical machine.

## Evidence And Completion Standard

Require a finite 5170-inherited-versus-DeskPro-delta ledger, reset/ROM/route/
device/timing-contract parity checks, immutable configuration proof, and the
current startup/smoke regressions. Every difference is inherited, explicit
DeskPro delta, accepted L2 fallback, or transferred to the VM foundation; no
new generic or physical behavior is inferred.

## Non-goals And Stop Conditions

Do not reopen physical-cycle, CECG monitor, controller waveform or firmware
research; add ROM/media; implement profile inheritance generally; or pursue
L4. Stop if running requires an unproved machine fact or a profile-local Core
branch; transfer it to the appropriate Core or VM receiver.
