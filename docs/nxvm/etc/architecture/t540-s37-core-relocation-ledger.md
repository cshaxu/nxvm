# T540 S37 Neutral Core Relocation Ledger

S37 audits the accepted S31-S36 changes against the actual source after
`7fabc990a`. This is a source-only cut: it neither moves code nor edits
Shared/MyNES. The present `core-machine` CMake target still combines 28
mixed NXVM sources and the four primitive sources in `core-machine-executor`.
Passing boot tests does not make every source neutral.

## Accepted path audit

| Packet | Actual surviving single route | Receiver still needed |
| --- | --- | --- |
| S31 | VM no longer mirrors the frozen construction inputs; board plan validates before allocation and copies only on success. | Keep `machine_plan.c` with board composition. |
| S32 | One neutral preflight/create stage owns CPU/FPU, time, transaction, RAM, port and bus; all post-initialization failure branches call the public destroy route. | Move board clock instances and the high-ROM RAM fallback out of this stage. |
| S33 | One board-create stage retains port checkpoint and controller order; `create_from_plan` remains with topology apply and one rollback. | Keep controller creation and topology application with the board. |
| S34 | One cold reset calls a board-device stage at the old point; CPU-only reset leaves RAM, devices and scheduled time intact. | Move board clock reset and the D4 shutdown choice out of neutral Core control flow. |
| S35 | One destroy delegates board chip release; ROM rollback releases only `owns_image` mappings. | Split private state; board callbacks must be revoked before Core storage disappears. |
| S36 | Entry, ROM and trace implementations read Core fields only; an unused board helper include was removed. | Replace the present mixed private `machine.h` include during physical separation. |

No constructor, reset or destruction parallel path was found. Current
partial failures before a complete Core allocation retain their small local
cleanup; later failures use `core_machine_destroy`. Firmware configuration
failure rolls ROM mapping count back. The four profile boot checks and full
units recorded in S31-S36 evidence cover the accepted behavior. Those tests
do not prove that the current private header is independently compilable.

## Concrete source ownership blockers

1. `machine.c` still contains IBM-PC F0000h-to-high-reset ROM derivation
   (`core_machine_register_reset_rom_alias`, around lines 155-251), and
   `machine_firmware.c` calls it after a provider supplies the F0000h role.
   The generic immutable ROM table and firmware operation guard can be Core;
   the chosen ROM role and alias composition are board/firmware policy.
2. `core_machine_neutral_create` still registers a firmware-less high reset
   RAM fallback at F0000h (around lines 580-589). This is board address
   composition, even though the memory route operation itself is Core.
3. `machine.c` still checks planar parity while resizing RAM (around line
   928), chooses the DeskPro D4 shutdown-to-processor-reset path in the run
   loop (around line 1055), and dispatches XT-versus-8042 native keyboard
   input near lines 1412-1479. Those are board choices; Core should consume
   bounded policy/results, never read a named board field.
4. `core_machine` in `machine.h` interleaves Core CPU/time/bus/ROM state with
   D4, topology, board latches, board clocks and chip instances. Six named
   device clock domains are initialized/reset in `machine.c` but consumed by
   `board_deadline.c` and `board_advance.c`; the provider clock remains Core.
   A blind file move would carry the second owner into `x86/core`.
5. `machine_scheduler.c` and `cpu_bus.c` now avoid direct named-chip fields,
   but still compile through the mixed private header. Their typed board
   callback uses are the intended seam. The same header dependency applies
   to entry/ROM/trace and the neutral memory/port interfaces.

These are relocation preconditions, not newly discovered guest failures.
They remain inside T540 with explicit receivers; none is silently declared
complete by this source-only audit.

## Finite owner/file ledger

- Neutral Core candidates after the private-state split: `clock.c`,
  `cpu_bus.c`, `debug.c`, `entry_plan_interface.c`, the generic portion of
  `machine_firmware.c`, the neutral portion of `machine.c`,
  `machine_scheduler.c`, `memory_interface.c`, `port_interface.c`,
  `rom_mapping_interface.c`, `trace_interface.c`,
  `retirement_observation_interface.c`, `timeline.c`, and the primitive
  `port.c`, `memory.c`, `transaction.c` with their required headers. Source
  filenames are candidates, not authorization to copy their mixed header.
- Board/App retainers: `board_advance.c`, `board_deadline.c`, `d4_memory.c`,
  `dma_bus.c`, `fdc.c`, `hdc.c`, `kbc.c`, `machine_board.c`,
  `machine_display.c`, `machine_plan.c`, `media_interface.c`, `pic_bus.c`,
  `pit_bus.c`, `xt_ppi_keyboard.c`, `vadp.c`, and the display provider slot
  `display.c`. `machine_firmware.c`'s F0000h binding and `machine.c`'s
  board-specific parts also remain with board composition.
- Direct test receivers start with the synthetic Core memory/port/transaction/
  timeline, prepared-entry, immutable-ROM and trace tests under
  `test/app-nxvm/unit/core/devices`. Existing board fixtures and four-profile
  tests stay product-local. The move S must list exact test files after the
  private header is split; filename prefixes alone are not ownership proof.

## Bounded next receivers

- S38: move F0000h firmware role/reset-alias composition out of neutral Core
  functions, preserving one provider binding and the current rollback.
- S39: move the high-reset RAM fallback and board memory-reconfigure veto to
  board composition, preserving firmware-less fixtures and failure order.
- S40: move the D4 shutdown choice and XT/8042 input dispatch behind bounded
  board policy/results while preserving CPU-only reset priority.
- S41: put named device clock-domain initialization/reset with the board
  advance/deadline owner; Core retains the sole guest timeline and provider
  clock.
- S42: split the mixed private state/header and remaining mixed `machine.c`
  functions into one neutral Core owner and one board attachment, without a
  mirrored fact, second constructor or second executor.
- S43: physically move only the now-proven neutral files and exact direct
  test receivers into `src/x86/core` and `test/x86/core`; delete the App copy,
  reconnect NXVM and pass independent Shared builds.
- S44 onward: move only S2-proven IBM-PC common, AT and XT mechanisms in
  owner-sized batches. Each proposed receiver is rechecked against its actual
  diff before admission; no suffix S numbers are used.

The old S30/S7 numerical rows are historical prospective plans. This ledger
is the current post-S36 allocation; no physical Core move has occurred.
