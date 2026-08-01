# Canonical Breakdown

The roadmap is the single source of milestone truth. This index only lists the
current sequence.

- M0: governance reset for the dual-product NXVM successor repository.
- M1: preserve the current NXVM baseline with GCC/CMake/Ninja evidence.
- M2: design the shared core architecture.
- M3: refactor the shared core.
- M4: design firmware and the `nxvm.exe` product surface.
- M5: implement firmware and the retained NXVM Console according to
  `docs/history/m5/planning/m5-firmware-nxvm-implementation-breakdown.md`.
- M6: design the owned DOS module.
- M7: implement the owned DOS backend.
- M8: design ntvdm64 platform integration, process CLI, and window-mode control
  Console.
- M9: implement `ntvdm64.exe`.
- M10: expand compatibility and extended devices through corpus increments.
- M11: optional integration, Microsoft NTVDM, and Win16 research.

Tasks and subtasks add bounded scope and acceptance criteria under these fixed
milestones. Each design milestone produces the Task/subtask breakdown for its
immediate implementation successor; M10 produces one for each corpus increment.
Do not predeclare implementation tasks or reorder milestones without an
owner-approved direction record.
