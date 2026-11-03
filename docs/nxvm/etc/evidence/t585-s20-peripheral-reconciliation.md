# M5 T585 S20 — current power and peripheral reconciliation

This review uses current source after S19 and references Open-Source-PC110
`02ccf2e8e19b298a2cf04c5d430278eb087cc368`, PC110-EMU
`81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`, and PC110-QEMU
`bae390ebf21665ec97860d28e26ec46f1fe8f4e9`. These are research sources; no
third-party implementation is imported.

| Receiver | Present evidence | Remaining distinction |
| --- | --- | --- |
| Suspend/wake | Chipset suspend refresh, F12 and PC Card wake use the same guest chipset; T580 S71 exercised a real firmware suspend/resume and RSM. | The selected BIOS still has an explicit boot compatibility patch; battery telemetry and complete guest APM are not established. Emulator pause is not guest suspend. |
| Power inputs | `pc110_ec_block` is an intentionally static capture; `pc110_ec_mailbox` implements selected firmware-visible commands. | The reference reports AC-present changes in EC block indices 06h and 09h and a command-driven battery gauge. Current code has neither a live AC input nor a complete battery command model. The PC110-EMU diagnostic metadata map expressly is not a literal hardware register contract. |
| Display/font | F65535 and external font are connected; the two real firmware variants and CMOS product tests passed in S19. | Physical panel color, FRC and timing remain unverified; this run does not establish interactive display fidelity. |
| PC Card | Both sockets, memory cards, window decode, IRQ and wake have real chip/board tests. | A populated external I/O card has no production receiver. The internal ATA controller is independent, as corrected in S13. |
| Serial | Current PC110 construction installs no COM1 UART route; no 16550 implementation was found in the product or X86-NXVM trees. | Open-Source-PC110 live dump records a FIFO-capable 16550-class COM1 at 3F8h and IRQ4. This is an actionable missing guest-visible device, not evidence of an existing host serial connection. Design a single reusable UART chip and a PC110 board binding, including reset, interrupt and loopback tests; state clearly whether an external endpoint exists. |
| Infrared/modem/dock | The references identify physical devices and the docking connector. | Physical presence alone does not define a usable byte protocol or prove current software needs it. Keep these separately identified; do not claim that COM1 alone implements them. |
| Audio | S16–S19 implement guest DMA, FM and automatic block playback. | FM is L1 melodic synthesis, without rhythm and modulation; full ES488 and native audibility are not established. |

The PC110-EMU power MCU notes describe a diagnostic map and explicitly disclaim
traced battery, charging, lid and suspend protocols. PC110-QEMU uses simplified
indexed register files. Neither justifies silently substituting those models as
the real PC110 power contract. User-authorized L1 estimates remain possible,
but must have a defined receiver and observable behavior.

S20 is a source/reference review, not a new dynamic qualification run. Its
follow-up must cover the COM1 receiver and continue to track the other explicit
model gaps rather than counting the S19 tests as complete peripheral coverage.
