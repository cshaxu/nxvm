# MyArcade: RAIDEN II

## Goal And Admission

Create an independent MyArcade App whose primary goal is running a real,
lawfully supplied RAIDEN II ROM set with playable graphics, input and sound.
This is an unnumbered NXVM queue-tail candidate after PC110, not an admitted
implementation T or a claim of existing support. Metal Slug 2 belongs to the
separate MyNeoGeo proposal, not this App.

Target the original NEC V30-based Seibu board, not an IBM PC or automatically
the later V33-based Raiden II/DX revision. Admission freezes the board revision,
region and exact ROM set against the owner's actual assets.

## Architecture And Reuse

- Candidate roots: `src/app-myarcade`, `test/app-myarcade`, `assets/myarcade`.
  Planning uses NXVM's queue/MTSP; implementation admission first establishes
  new-App documentation, tools, version and commit-target ownership.
- Product owns identity, configuration, board selection, controls and injected
  commands. Reuse Emulator Product/Session/UI for host composition, monitor and
  lifecycle, and Lib services for storage, audio and platform I/O. Do not copy
  MyNES's runner or introduce a second Session/provider path.
- Machine adapts the selected board to Emulator Machine with copied input,
  frames/audio, execution boundaries and lifecycle. Board logic owns one guest
  timeline, device state, interrupt wiring and mapped ROM/RAM.
- Current CPU/device implementation lives in `core/chips` and `core/x86`.
  Audit V30 instruction, interrupt and timing differences before proposing
  reuse; never assume the Intel 386 implementation is a drop-in substitute.
  PC/AT BIOS/PIC/PIT wiring must not be inherited by an arcade board.
- Shared `src/x86` currently owns x86 Debug/product/tooling, not the CPU engine.
  Reuse only qualified matching capabilities. Lib/Emulator must not learn game
  names or board behavior. No general arcade framework or plugin registry is
  justified by this first board.

## Evidence And Assets

Initial reference checked on 2026-10-13:
[MAME Seibu driver](https://github.com/mamedev/mame/blob/master/src/mame/seibu/raiden2.cpp).
It supplies board/ROM-map, protection and graphics/sound research leads.
Admission pins exact revisions, seeks primary manuals/board observations and
labels reference-based models L2. It is not imported source or proof of hardware
accuracy. Copying or deriving third-party code needs a separate license review;
MAME code is not presumed MIT.

Game, sound and protection ROMs remain lawful external owner inputs. Inventory
roles, sizes, hashes and missing files. Do not acquire game downloads, commit
payload bytes or embed game ROMs in releases under the PC firmware exception.
Loader validation must reject missing/wrong input without substitute ROMs,
protection bypasses, game-specific success patches or a second execution path.

## Planned Batches

These are planning batches, not allocated S identifiers:

1. Freeze the board/set inventory and convergence ledger; audit CPU/device
   gaps, implementation provenance, reuse and new-App governance/build owners.
2. Establish Product/Machine composition, asset validation and ROM-free tests.
   A scaffold is not game compatibility.
3. Qualify V30 execution, bus/banking and interrupts; implement the board's
   protection behavior, graphics decoding/layers/sprites, sound CPU/devices
   and controls. Split these into bounded reviewable S batches at admission.
4. Prove real-ROM boot, attract, coin/start, gameplay and completion checkpoints.
5. Complete lifecycle/error/regression qualification and publish MyArcade's
   current x64/x86 pair for owner gameplay acceptance.

## Convergence And Exit

Before implementation, freeze a finite ledger for the selected ROM set:
reset/boot, attract, coin/start, all controls, video orientation/geometry/
priority, sound, interrupts/timing, stage transitions, boss/end-of-game,
game-over/restart and host lifecycle/cleanup. Each entry names inputs, semantic
checkpoints, budgets, source grade and test owner. A title screen, compilation
or surviving a timeout is not completion.

Use ROM-free unit tests and separate owner-ROM integration tests. Qualify both
host widths in reusable Ninja/ccache graphs, affected existing-product
regressions, types/dependency/manifest gates and deployed identities. Distinguish
authentic game slowdown from host inefficiency. Audio tests do not depend on
physical speaker volume. Unsupported optional Debug/snapshot capabilities use
the common honest failure contract, not fake success.

Close only after the complete playable-game ledger, published dual binaries,
actual-diff/license review and owner acceptance. Cycle accuracy and other ROM
revisions are separate claims. Missing CPU/protection/sound evidence, lawful
assets or a necessary Shared change requires explicit review, not silent
scope reduction or a scripted substitute. This proposal authorizes no code.
