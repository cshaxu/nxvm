# M5 PC/AT 8042 And AUX Completeness

Extend the retained 8042/KBC owner only through its hardware protocol:
controller command/status sequencing, keyboard response/error timing, and the
bounded PS/2 AUX features needed by a selected corpus (including, if selected,
sample-rate identification, wheel packets, remote/read-data, and resend).
Retain core-owned guest time and IRQ1/IRQ12; do not add host-input passthrough
or guest-memory shortcuts.

