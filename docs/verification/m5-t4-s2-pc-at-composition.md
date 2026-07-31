# M5 T4 S2 PC/AT Composition Verification

GCC built and ran `nxvm-pc-at-firmware-smoke`; it emitted
`M5:T4:S2:PC-AT-FIRMWARE:OK`.
It verifies ordered ROM/POST/INT 10h/13h/16h/1Ah identity composition,
interrupt-vector lookup, deterministic CMOS defaults, BDA bytes, ROM signature,
and the `F000:FFF0` far-jump reset stub. This is a project-owned reset package,
not a complete BIOS implementation; executing its services through the
first-class full-PC path remains T7 work.

The T4 task artifact is the retained bootable NXVM Console at
`build/output/nxvm-m5_t4.exe`, built from the T4 completion source state. Its
SHA-256 is `66b60bd610d7d670e4bcb22049042b18f83a0fa05037e2947fcf79e09c1512d6`;
its banner is `0.4.015d.m5t4`, and a piped `exit` Console smoke returned zero.
