# M5 T498 S4 Reset-ROM And Build Closure

`M5:T498:S4:RESET-ROM-BUILD:OK`

## Accepted boundary

S4 accepts only the source-graph repair, CPU-family reset-ROM fetch, Core
immutable alias route, and the existing PC/AT multi-window/zero-parity board
topology needed to compile that route.  It does not accept the co-located
worktree candidates for DMA, scheduler deadlines, timeline representation,
XT timing, firmware assembly, session pacing, CPU diagnostics, generated
timing data, or broad fixture updates.

The retained candidates remain unstaged in the primary worktree and require
their own admitted receiver.  They were neither discarded nor used as proof.

## Clean proof

From a detached clean worktree at commit `5a9fe9ed`:

| Check | Result |
| --- | --- |
| Fresh Release configure | Passed with `cmake --preset mingw-gcc-x64-release`. |
| Complete Release product | Passed with `cmake --build --preset current-gcc --parallel 12`; emitted `nxvm_0_5_0495.exe`. |
| Reset and firmware route | Passed: `M5:T496:S7:RESET-ROM-ALIAS:OK`, firmware capability, ROM-alias lifecycle, and firmware-failure propagation markers. |
| Plan and PC/AT topology | Passed: plan declarations/validation/copy/rollback, controller-plan, IBM-5170 resolver, PC/AT composition, and PC/AT topology smokes. |
| Artifact identity | Stripped Release `nxvm_0_5_0495.exe`, SHA-256 `437994843CDBE7581ABF7948AF469256B6C54C91897F7EC0DF118F61ED830267`. |

The runtime debugger remains a product feature.  Stripped here means no
compiler debug information, not debugger removal.

## Ownership result

Core alone derives the CPU reset alias and serves reset-only ROM bytes;
firmware/profile construction contributes immutable mappings only.  The same
Core plan owns all declared absent-memory windows and the PC/AT Port-B parity
state, including the zero-parity-RAM case.  This removes the former split
between profile materialization and a missing Core multi-window owner without
adding a second reset, ROM, memory, or board-state path.

## Limits and transfer

This proves a buildable prerequisite, not Model-40 DOS boot.  The S3-selected
receiver remains firmware-to-board startup progression through the first FDD
request; no FDC/DMA/PIC conclusion follows from S4.
