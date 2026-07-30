# Project Status

## M0 Closure

M0 is complete. Its governance, source and redistribution boundaries, roadmap,
lightweight C11 toolchain contract, and deterministic probe laboratory have all
been documented and verified. M1 is eligible to begin, but has not started.

## Active Work

None. M1 T1 S1 is the next eligible subtask, not an active subtask.

## Completed

- M0 T1 S1 P1: repository governance and architecture foundation.
- M0 T1 S1 P2: recorded the project MIT license after integrating the remote
  initial commit.
- M0 T1 S2: recorded the initial CMake/Visual Studio 2022 contract and added a
  deterministic generated M1 DOS probe; the default toolchain was later
  replaced by M0 T5.
- M0 T2 S1: pinned the local NXVM reference and LGPL license identity without
  importing source.
- M0 T2 S2: recorded NTVDMx64 behavioral-only and WineVDM process-only
  boundaries.
- M0 T3 S1: reviewed M0 closure conditions.
- M0 T4 S1: adopted the initial NXVM-foundation architecture correction, source
  policy, redistributability rules, and directory plan.
- M0 T4 S2: downgraded Microsoft component work from a planned backend to an
  M6 feasibility study after M5 host-integration research; added M4 real-program
  compatibility and M7 Win16 research; and moved Microsoft material to the
  research boundary.
- M0 T5 S1: corrected milestone/snapshot governance and selected MinGW-w64 GCC
  with CMake and Ninja as the default lightweight C11 toolchain.
- M0 T5 S2: verified the x64 GCC/CMake/Ninja C11 configuration and deterministic
  probe build; adopted the Windows 7 through Windows 11 support policy.
- M0 T6 S1: recorded the future `ntvdm64 run` CLI, display, debugger, host-drive
  visibility, cancellation, and Console-ownership contract without beginning
  implementation.

## Next Eligible Work

- M1 T1 S1: import and baseline the smallest NXVM machine slice required for a
  real-mode probe. Follow [the subtask record](subtasks/m1-t1-s1.md). This task
  does not implement DOS services or alter NXVM source in place.

## Delivery State

No NXVM runtime code, other third-party source, guest media, or DOS
compatibility claim has been added. The generated M1 COM probe is test input
only. The MinGW-w64 GCC CMake smoke gate passed. Project-owned material is MIT;
later NXVM-derived units retain applicable LGPL-3.0-or-later obligations.
