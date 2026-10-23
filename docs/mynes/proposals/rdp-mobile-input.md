# RDP mobile keyboard input

Candidate, not admitted. T556 S10's diagnostic observed repeated reads
`08,00` for a character pulse versus `08,08` for held Enter; no production fix
or actual mobile native-event capture has been completed. Software-keyboard
pulses can be lost during repeated polling, and sustained movement depends on
whether the client sends held keys. See the
[investigation](../etc/evidence/t556-s10-rdp-controller-investigation.md).

Review a bounded guest-time pulse expiry with held-key overlap, pause/reset and
source cleanup, and snapshot compatibility. Keep one input path and no ROM-specific
rules; review any necessary Shared change before implementation. Exit requires
dual-width regression and actual Window/mobile RDP qualification; diagnostics
alone do not establish a fix.
