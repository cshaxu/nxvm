# M2 T1 S1 Machine And DOS Architecture Verification

## Evidence Reviewed

- M1 source provenance pins the full 75-file NXVM baseline at
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- M1 FDD and HDD traces reached the recorded reset and DOS `INT 21h`
  checkpoints under ten-second watchdogs; neither is represented as a natural
  guest exit.
- The baseline audit confirms the motivating coupling: `machine.c` includes and
  starts `platform`, `device.c` owns global `flagRun`/`flagFlip`, and Win32
  presentation loops access those globals directly.

## Result

Architecture Requirements V1 defined explicit boot and DOS-minimal profiles;
Machine ownership and thread/lifecycle rules; V1 Machine, host-service
and trace contracts; DOS interrupt/loader direction without a Machine-to-DOS
dependency; and concrete M1 regression checkpoints. The separate M3 breakdown
contains nine bounded subtasks, each with an approved decision, acceptance, and
stop condition.

The review found no current-governance conflict: M4 retains DOS ABI ownership,
M6 retains concrete Windows/CLI ownership, and M3 has no DOS implementation
scope. Historical M0 wording was clarified to state its superseded planning
role. `git diff --check` passed. This documentation-only subtask produces no
runnable executable and therefore no `build/output` artifact.

M2 T1 S2 supersedes this V1 record's profile names and milestone sequence with
Shared Core Architecture Requirements V2.
