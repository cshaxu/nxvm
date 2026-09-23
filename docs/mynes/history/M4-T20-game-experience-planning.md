# M4 T20: Supported-Game Experience Planning

T20 converts M4's approved boundary into a finite implementation program.

The owner-local, ignored support inputs are Dr. Mario (mapper 1/MMC1) and
Super Mario Bros. 2 (mapper 4/MMC3). Their iNES headers were inspected locally;
they are not copied, hashed, distributed or made build dependencies. The plan
requires a controllable in-game transition, sound, one-times pacing and safe
management behavior through both Window and Core-text Console paths on x64/x86.

The plan also confirms a hard architectural dependency: the accepted shared Lib
snapshot has no neutral audio stream. M4 must adopt a reviewed upstream generic
audio revision before native sound or pacing integration begins. This is not
permission for a MyNes-local Win32 audio backend. MMC1/MMC3 and deterministic
APU work can proceed against project-authored fixtures while that prerequisite
is resolved.

Five outcome-bearing candidates are queued: MMC1 Dr. Mario, MMC3 Super Mario
Bros. 2, RP2A03 APU hardware, neutral audio/pacing adoption, and combined
supported-game acceptance. The complete boundary, observations and exclusions
are in the [M4 game-experience plan](../etc/m4-game-experience-plan.md).
