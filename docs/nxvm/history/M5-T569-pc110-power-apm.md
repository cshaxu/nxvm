# M5 T569 — PC110 Power Management And APM

## Closure

T569 is closed as the source-gated PC110 power-management receiver following
T567's explicit disposition.  Its audit confirms that the existing VL82C420
indexed configuration route is correct; it must not be replaced by the stale
external-emulator interpretation of `0ECh/0EDh` as a power-MCU window.

The real BIOS owns APM `INT 15h` status.  The observed Block2 suspend write
and `HLT` establish a register sequence, but not the complete wake source,
resume image/flag transaction, error behavior or lifecycle boundary.  No
production power endpoint is therefore installed.  The later real-firmware
boot receiver owns the required guest execution trace and any resulting
App-local power mechanism.
