# M6 T39 S9 Controlled-Play Verification

## Purpose

Verify the repaired MyNES TMNT3 path through the production Driver without
Computer Use or a protected-ROM dependency in tracked material.

## Method

An ignored, bounded local probe created one production Driver, inserted the
owner-managed cartridge as read-only media, and captured published RGB frames
at four checkpoints: initial execution, a Start press/release, then two later
checkpoints while Right and A remained held through the normal Driver input
route. Each checkpoint also observed frame revision, program counter, stack,
cartridge IRQ and trap state. The probe emitted only ignored local images.

## Probe Self-Check

The first image variant allocated several complete frame records on the probe
stack; it failed before collecting an observation. The probe was reduced to
one temporary frame at each checkpoint. A second probe-only error inverted the
successful `core_driver_set_media()` result. Correcting that boolean contract
made the same production path run successfully. Neither finding changed
tracked MyNES production code.

## Result

All four checkpoints advanced the published frame revision, retained a valid
ROM program counter, reported no trap and no asserted cartridge IRQ, and had
stable/recovered stack state. Start altered the published state. With Right and
A held, the next image became a nonblank rendered game scene; the later image
changed again while remaining healthy. This proves the requested bounded
controlled-play receiver, rather than merely controller serial delivery or a
title-frame transition.

## Boundary

The evidence validates the owner-reported cartridge through its real Driver
input route. It does not claim universal Mapper-004, revision, or game
compatibility. Local media, paths, bytes, digests, raw output and images remain
ignored.

## Automated-Gate Classification

The x86 native-window smoke exposed that its direct GDI pixel sampling creates
and depends on a real desktop Window, but the test lacked the repository's
`desktop` label. The same headless run showed that native-console smoke samples
a real Console buffer and has the same omitted dependency. Neither failure uses
the TMNT3 Driver probe or indicates a Core execution failure. S9 classifies both
owned presenter checks as `desktop`, matching the existing Lib desktop-test
policy, so the complete non-desktop automatic suite is reproducible on both
architectures. Both checks remain available for a desktop-capable integration
run.
