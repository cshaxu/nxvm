# Project Status

## Completed

- M0 T1 S1 P1: repository governance and architecture foundation.
- M0 T1 S1 P2: recorded the project MIT license after integrating the remote
  initial commit.
- M0 T1 S2: selected the CMake/Visual Studio 2022 contract and added a
  deterministic generated M1 DOS probe.
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

## Next Eligible Work

- M1 T1 S1: first run the MinGW-w64 GCC compiler/probe smoke gate, then import
  and baseline the smallest NXVM machine slice required for a real-mode probe.
  Follow [the subtask record](subtasks/m1-t1-s1.md). This task does not
  implement DOS services or alter NXVM source in place.

## Delivery State

No NXVM runtime code, other third-party source, guest media, or DOS
compatibility claim has been added. The generated M1 COM probe is test input
only. The MinGW-w64 GCC CMake smoke gate is an M1 entry condition and has not
yet run on this authoring host. Project-owned material is MIT; later
NXVM-derived units retain applicable LGPL-3.0-or-later obligations.
