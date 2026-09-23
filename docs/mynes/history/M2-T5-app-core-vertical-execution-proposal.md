# App/Core Vertical Execution

Uses [shared context](../etc/m2-execution-management-context.md) and the accepted T4 contracts.
Entry: unchanged tested Lib/Common foundation, flat source/test design, no App/Core
runtime. This is the first implementation candidate, without an assigned T number.

## Outcome

Deliver a real Windows MyNes executable that starts its cooked monitor, inserts an
original NROM fixture, runs it through Common/Core, pauses, reports copied progress,
resets, ejects and exits. The first source S must deliver this vertical behavior,
including the necessary product build/gates; no dummy main or library-only exit.

## Frozen Scope

- App entry/defaults/parser/composition; help, status, config, rom insert/eject,
  run, pause, stop, reset and quit. Remaining designed commands explicitly report
  unavailable. Startup --rom uses the same media helper and never autoruns.
- Common driver callbacks, finite slices, pre-instruction callback, guest-stop
  reporting, serialized removable media and ordered shutdown. Minimal OBSERVE
  supports copied paused status; the full debugger belongs to its receiver.
- Full strict basic NROM header/size policy, 16/32 KiB PRG, CHR ownership, 2 KiB
  mirrored RAM, open bus and explicit unavailable-device traps. No permissive
  temporary parser. Failed candidate preparation preserves accepted hardware.
- One resumable cycle engine. Initial opcode set is exactly 78 SEI, EA NOP,
  A9 LDA immediate, A5 LDA zero page, 85 STA zero page, 8D STA absolute,
  A2 LDX immediate, E8 INX, CA DEX, E0 CPX immediate, D0 BNE and 4C JMP absolute.
  Other bytes trap; this is explicitly an incomplete execution profile, not
  all documented CPU support. Reset loads the real vector through the bus.
- Real x64/x86 product targets, strict C11, source include boundaries and flat
  component tests. Imported shared roots remain byte-identical.

## Acceptance And Transfer

An original program loads A=$2A, stores it to $0200, counts X from 0 to 10 with
INX/CPX/BNE, then enters a named JMP checkpoint. Production machine tests assert
the copied PC/X and RAM checkpoint; repeated loop iterations alone never pass.
Real Common integration proves insert -> reset completion with zero retirement
-> run -> pause -> copied checkpoint -> reset -> eject -> quit, plus failed
replacement preserving the previous machine. Run the actual executable's monitor
flow as well as deterministic integrated tests; no test-only command dispatcher.

Prove every initial opcode result/flags/base timing, both branch outcomes, each
construction stage's cleanup and basic parser/state rejection. Admit all these
regressions permanently. A finite run budget contains failures only. Record
artifact identity, architecture and hashes; no external game ROM required.

Transfer remaining 139 documented opcodes and broad functional cases to CPU/bus
completion; full debug to debugger workflow; exhaustive bus/interrupt timing to
cycle refinement; full failure/state cross-products to reliability. New features
must be accurately marked unavailable until their receiver delivers them. No
PPU/APU/controller, graphics/audio or gameplay claim. Stop under shared-context
conditions; no unsafe implementation is deferred merely to meet this early exit.
