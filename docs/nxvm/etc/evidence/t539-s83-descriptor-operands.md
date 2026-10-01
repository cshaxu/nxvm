# T539 S83 Descriptor-Operand Receiver Evidence

## Delivered Boundary

S83 moves the CPU-only protected descriptor-operand suite to the Shared CPU
owner: `ARPL`, `BOUND`, `LAR`/`LSL`, and `VERR`/`VERW`. Their sole 121-line
descriptor-query fixture is also Shared. It supplies copied CPU state, memory,
descriptor bytes and terminal CPU faults only; it does not construct a Core
machine, route an IRQ, or own profile state.

NXVM deletes all five duplicate paths and changes its CMake inventories to the
same Shared targets. `machine_arpl_board_smoke` and
`machine_bound_board_smoke` remain the explicit public-board receivers for
their distinct descriptor/fault/IRQ observations.

## Delivered Commits

- `133ffd748` — Shared P1 adds the four receivers, one fixture, registrations
  and manifest records.
- `7ed9dfd4f` — NXVM P2 deletes the duplicate paths and switches unit, T317,
  and T337 inventories to the Shared targets.

The tracked implementation paths add 1,280 and remove 1,290 lines (net -10),
excluding the generated manifest and this evidence/state documentation. No
production implementation, ABI, callback, firmware, asset, INI, or profile
changed.

## Verification

- Shared successors and the retained ARPL/BOUND board receivers pass on x64
  and x86.
- Complete repository-only unit suites pass **433/433** on x64 and x86.
- CPU/PIC authority, Shared manifest/corpus verification, documentation
  governance, and `git diff --check` pass.

No executable input changed, so the existing product artifacts remain current.
