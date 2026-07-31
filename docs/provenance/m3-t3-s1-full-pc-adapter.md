# M3 T3 S1 Full-PC Adapter Provenance

## Source Basis

The runtime descriptor and adapter are project-owned C11 code. They copy no
NXVM implementation text. They bind the MIT-authorized M1 snapshot
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f` through its retained public
interfaces only.

| Baseline source | Observed interface | Adapter use |
| --- | --- | --- |
| `machine.c`, `machine.h` | initialize/finalize whole machine | explicit legacy session lifecycle |
| `device/device.c`, `device/device.h` | reset/start/stop and CPU references | reset observation and bounded-harness control |
| `device/vfdd.c` | insert returns zero on success | FDD fixture configuration |
| `device/vhdd.c` | insert returns zero on success | HDD fixture configuration |
| `device/vbios.c` | boot-device selector | explicit FDD/HDD boot configuration |

## Changes From The Baseline Shape

The legacy graph remains a single global session. The adapter prevents a
second session, hides its globals, makes fixture loading and teardown explicit,
and leaves all imported source unchanged. `runtime/profile.c` records it as the
temporary implementation of `nxvm.full_pc`; it is not a reusable core Machine.

## Authorization And Verification

NXVM imports and derivations are authorized under the root MIT policy. This
task adds no imported source. Verification is recorded in
`docs/verification/m3-t3-s1-full-pc-profile.md`.
