# T580 S14 — PC110 post-C000 I/O boundary

The prior diagnostic reported that no port operation followed the admitted
C000-to-port-61 return. That conclusion was invalid: its trace callback first
filtered events to a whitelist of already known ports, so it could not observe
the very next unknown receiver it was intended to discover.

The selected TFT image maps `F000:3F6C` to the continuation after the real
`C000:0003` call. It then invokes ROM helpers for PC110 configuration state.
The Open-Source-PC110 live SCAMP tail (`7Ch..7Fh = 00 10 15 EE`) is useful
research evidence for a later mailbox receiver, but it is a runtime capture,
not enough evidence to seed a cold chip image. No production SCAMP state is
changed by S14.

S14 temporarily widened the existing real-firmware diagnostic after the
established C000-to-port-61 return. Within the bounded 13 million-retirement
run, the firmware returned to the existing port-61 wait loop and issued no
later port operation. The probe is deliberately not retained as a failing
CTest: it identified no next receiver and therefore cannot justify production
hardware changes. The next investigation must begin from the C000 option-ROM
return semantics and the modeled F65535 device state, not from a guessed
SCAMP cold image.
