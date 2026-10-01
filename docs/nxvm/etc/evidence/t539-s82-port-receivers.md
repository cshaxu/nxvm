# T539 S82 Port Receiver Evidence

## Delivered Boundary

S82 makes the Shared CPU test corpus the sole owner of scalar and string port
instruction semantics. `cpu_port_io_smoke.c` and
`cpu_port_strings_smoke.c`, together with their one 68-line CPU-bus fixture,
now link only `x86-cpu-shared`. The fixture supplies only copied memory and
the CPU bus's `transfer_port`/`complete_port` callbacks; it has no board port
map, PIC, profile, or host state.

NXVM removes those duplicate tests and fixture, and its static test/gate lists
now name the same two Shared targets. Existing `machine_port_io_board_smoke`
and `machine_port_strings_board_smoke` paths remain the sole receivers for
actual board routing, permission, and IRQ behavior.

## Delivered Commits

- `95bc28324` — Shared P1 adds the two canonical CPU receivers, their one
  fixture, registrations, and manifest records.
- `2b81828fa` — NXVM P2 removes the three duplicate App paths and switches
  registrations and #UD inventory to the Shared targets.

The tracked implementation paths add 716 and remove 724 lines (net -8),
excluding generated manifest records and this evidence/state documentation.
The retained owner is the existing Shared CPU execution path; no production
path, API, callback, or state owner changed.

## Verification

- Each Shared port receiver passes independently on x64 and x86.
- Complete repository-only unit suites pass **429/429** on x64 and x86.
- CPU/PIC authority, Shared manifest/corpus verification, documentation
  governance, and `git diff --check` pass.

This is a test/CMake-only migration. Firmware, assets, INI, and executable
inputs are unchanged, so no NXVM product binary is rebuilt.
