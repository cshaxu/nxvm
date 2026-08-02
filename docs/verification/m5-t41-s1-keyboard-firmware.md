# M5 T41 S1 Keyboard Firmware Boundary

Default-profile `qdkeyb` has no separate mutable service object. Its immutable
provider table is bound by VM composition to core keyboard ingress; flags and
buffering remain in the existing BIOS RAM authority, while IRQ delivery uses
the bound PIC/KBC providers.

Windows GCC, `M5:T41:S1:KEYBOARD-FIRMWARE:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t41.exe`.
SHA-256: `584176A69FBF740D28EFFDB9997032840F630E4BEF5C758CDA29CEB2AA6A5C7B`.
