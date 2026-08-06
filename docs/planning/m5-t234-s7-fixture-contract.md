# M5 T234 S7: Fixture Contract Closure

## Objective

Remove raw mutable session and device borrows from the test fixture without
weakening the existing system and device smoke coverage.

## Contract

The fixture may expose only behavior-level operations, immutable snapshots,
and boolean/assertion helpers. It must not return `core_machine`, device,
transport, control, BIOS, platform-run, or other session-owned object pointers.
It must not cast away `const`.

## Subtasks

### P2: Machine Contract Design

Inventory every `core_machine` fixture borrow and define the smallest action
and immutable-observation API needed for execution, memory, display, timing,
and diagnostics. No test migration occurs in P2.

### P3: Machine Contract Migration

Implement P2 actions/snapshots and migrate every machine-facing smoke test.
The fixture must no longer return `core_machine *`.

### P4: Device Contract Migration

Replace FDD, FDC, HDD, HDC, CMOS, BIOS, block-provider, and debug raw borrows
with explicit media/device actions, snapshots, and assertions. This removes all
device and firmware pointer returns.

### P5: Platform And Control Contract Migration

Replace request/mouse transport, presentation mailbox, run context/handle, and
control raw borrows with event injection, display-mode, lifecycle, and
execution-control actions plus immutable snapshots.

### P6: Closure Gate

Remove allocation and legacy storage helpers, delete all remaining raw-borrow
APIs, and extend the static gate to reject session-owned pointer returns and
const-removing casts. Run the complete GCC gate.

## Exit Conditions

- `vm_session_fixture.h` contains no raw session-owned object pointer return.
- No test includes composition `session.h`.
- The fixture implementation contains no const-removing cast.
- The static boundary gate and all current GCC smoke tests pass.

## Active Subtask

**P2 active.** P3--P6 remain pending and may not begin until the P2 machine
contract is reviewed in this packet.
