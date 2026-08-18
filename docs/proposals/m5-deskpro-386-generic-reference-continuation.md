# DeskPro 386 Generic-Reference Functional And Deterministic-Timing Continuation

## Purpose

Owner-authorized successor to T399. Where exact DeskPro-specific behavior or
primary material is unavailable, use a traceable PCjs, 86Box, MAME, QEMU or
Bochs implementation as a **generic reference baseline** to improve NXVM’s
logical device behavior and deterministic event order before CPU audit. This is
not physical Compaq hardware proof.

## Boundaries

A generic reference may establish only register/state, command/result, IRQ/DRQ,
reset/cancellation or deterministic ordering requirements that can be replayed
in a project-owned asset-free probe. It may not establish a Compaq board
identity, physical clock scalar, DCLK/BWAIT conversion, media mechanics, raster
signal, electrical phase, firmware execution result or physical L3 readiness.
No third-party source, firmware, media, trace, configuration or binary is
imported; source is read-only behavior research.

## Initial Universe

1. Compaq HDC route: compare the existing normal PIO/IRQ14 state machine with
   generic AT HDC behavior; repair only an observable shared-owner defect.
2. CECG: compare applicable EGA register/reset/raster ordering with generic
   reference models while retaining CECG-specific unknowns as TODO.
3. CPU/DMA/BWAIT: inventory generic emulator abstractions and join any usable
   deterministic transaction ordering to the CPU audit; do not invent a board
   conversion.

Each accepted row names reference revision, exact generic component/config,
normalized project-owned probe, reset/cancellation replay, disagreement mask,
and physical receiver. The task closes only after every initial row is accepted
as generic-derived logical behavior or transferred explicitly. A later L3 audit
may decide only a generic-reference-derived deterministic tier.