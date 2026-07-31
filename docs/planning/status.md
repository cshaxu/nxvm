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
- M0 T2 S1: pinned the local NXVM reference and its then-current upstream
  license identity without importing source.
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
- M0 T6 S2: changed M1 to a full NXVM baseline-first, subtractive-refactoring
  strategy; recorded its local image-fixture boundary without beginning M1.
- M0 T6 S3: retained the existing Linux platform source in the M1 baseline and
  recorded the future shared `machine + dos` portability direction without
  adding a Linux delivery commitment.
- M0 T7 S1: fixed the owned-DOS profile, service allowlist, memory and
  filesystem boundaries, acceptance probes, and escalation rule without
  beginning implementation; the profile is now scheduled for M5 after the
  required M2/M3/M4 design and refactor work.
- M0 T8 S1: recorded the NXVM copyright-holder authorization for MIT imports
  into ntvdm64, while retaining upstream-license capture as historical evidence.
- M0 T9 S1: defined optional differential verification interfaces for machine
  and DOS traces, external-reference boundaries, watchdogs, and cleanup without
  beginning M1 or M2.
- M0 T10 S1: resequenced delivery into M1 baseline, M2 architecture design, M3
  machine refactor, M4 DOS design, M5 DOS implementation, M6 Platform/CLI
  design, and M7 product implementation; clarified M1 fixtures and CLI debug
  ownership without beginning M1.
- M0 T11 S1: closed remaining planning ambiguities around M5 dependencies,
  milestone authority, M1 experiment freezing, M2/M6 containment ownership,
  M3 planning, corpus policy, toolchain identity, and local BYOB research.

## Next Eligible Work

- M1 T1 S1: import and baseline the full NXVM machine before subtractive
  refactoring. Follow [the subtask record](subtasks/m1-t1-s1.md). This task
  does not implement an owned DOS backend or alter the sibling NXVM checkout.

## Delivery State

No NXVM runtime code, other third-party source, guest media, or DOS
compatibility claim has been added. The generated M1 COM probe is test input
only. The MinGW-w64 GCC CMake smoke gate passed. Project-owned material is MIT;
authorized later NXVM-derived units use MIT with source provenance and copyright
attribution.
