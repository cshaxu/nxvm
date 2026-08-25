# M5 Core VADP Phase Contract

## Purpose

Define selected digital VADP CRTC/raster/status/aperture phases without making
host presentation part of guest time.

## Admission And Dependencies

Consumes T449 and the selected CGA/EGA/CECG adapter documentation. Any
character/pixel clock selection requires a named source and board contract.

## Scope And Completion

Freeze reset/default, CRTC mode, raster/status transition, aperture route,
declared contention and copied-frame boundary rows. Prove mode/reset/status
and raster-wrap behavior under the existing Core clock and memory owners.

## Ownership Constraint

VADP is the sole owner of guest video state, ports, memory mapping and copied
display snapshots: firmware or guest writes flow into VADP, VADP publishes a
copied snapshot, and VM only presents it. Common CRTC/text, CGA, EGA and later
VGA code may have separate internal implementation boundaries, but cannot own
a second mode, VRAM or frame state. EGA keeps CPU planar-window eligibility,
sourced planar-frame geometry, and text/output-disabled fallback distinct.
VGA extends this same owner with its DAC, chain-4, 256-colour and timing
rules; it cannot introduce a parallel video path.

## Boundary

Analog monitor/composite waveform and host presentation are outside L3. It
cannot add a display-owned memory transaction or time publisher.
