# M5 T35 - Six-Input Acceptance Design

T35 consumes the ignored, owner-local inputs already present under
`assets/roms`: Dr. Mario (MMC1), Super Mario Bros. (NROM), Super Mario Bros. 2
(MMC3), Jackal (UxROM), Super Mario Bros. 3 (MMC3) and TMNT III (MMC3). Paths
and bytes remain absent from committed evidence.

S1 audits header/profile availability and freezes this plan. S2 extends the
existing owner probe to NROM and runs all six in graphics and Core-generated
text modes on x64/x86. Each run must load, publish a frame, execute its bounded
slice budget without a Core trap, and drain audio on pause. Existing unit and
integration fixtures remain the proof for mapper register behavior and the
Common pause/stop/eject lifecycle. S3 combines those outputs with a fresh
dual-architecture regression, release-manifest verification and the explicit
support/exclusion matrix to decide M5 closure.

Commercial game progress beyond this bounded execution protocol is not claimed.
