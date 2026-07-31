# M3 T1 S2 Machine Instance Shell Provenance

## Source Basis

The implementation is project-owned C11 code. No NXVM source text, declaration,
or macro was copied into the new core modules.

The reset-state values and transition boundary were verified against the
copyright-holder-authorized NXVM M1 snapshot
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`:

| Baseline source | Observed behavior | Project-owned destination |
| --- | --- | --- |
| `device/vcpu.c` | reset sets CS `F000`, base `FFFF0000`, EIP `FFF0`, and EFLAGS bit 1 | `src/core/cpu.c` reset snapshot |
| `device/vram.c`, `device/vram.h` | 16 MiB default allocation and A20-off reset behavior | `src/core/memory.c`, `src/core/profile.h` checked instance memory |
| `device/vport.c`, `device/vport.h` | per-port callback dispatch | `src/core/port.c`, `src/core/port.h` session-local typed dispatch |

## Changes From The Baseline Shape

The baseline modules remain untouched, process-global, and single-instance.
The new core uses private per-Machine storage, checked copying rather than raw
RAM pointers, typed callback operations rather than function-address casts,
and no platform, DOS, firmware, or product dependency. The M1 baseline remains
the temporary full-PC regression adapter until M3 T3 composition.

## Authorization And Verification

NXVM code imports and derivations are authorized under the repository MIT
license as stated in `docs/source-policy.md`. This task did not import code;
the M1 source is an observed behavioral reference. Verification is recorded in
`docs/verification/m3-t1-s2-machine-instance.md`.
