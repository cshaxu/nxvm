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

## Boundary

Analog monitor/composite waveform and host presentation are outside L3. It
cannot add a display-owned memory transaction or time publisher.
