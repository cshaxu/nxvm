# Project Status

## M0 Closure

M0 is complete. Its governance, source and redistribution boundaries, roadmap,
lightweight C11 toolchain contract, deterministic probe laboratory, and
dual-product NXVM-successor direction have all been documented and verified.

## M1 Closure

M1 is complete. The full NXVM `src/` baseline is provenance-recorded, builds on
Windows with the default GCC toolchain, and has bounded FDD/HDD execution
evidence. Its immutable snapshot branch is created from the verified M1 commit
before M2 begins.

## M2 Closure

M2 is complete. Shared Core Architecture Requirements V2 defines the
`nxvm.exe` and `ntvdm64.exe` product split, `nxvm.full_pc` and
`ntvdm64.dos_minimal` profiles, lifecycle, ownership, registries, abstract
host-service and trace boundaries, and M3 regression strategy. The bounded M3
Task/subtask breakdown is approved; M3 has not started.

## Active Work

None. M3 T1 S2 is the next eligible subtask and remains inactive.

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
- M0 T4 S2: downgraded Microsoft component work from a planned backend to
  then-current research milestones and moved Microsoft material to the research
  boundary. This numbering was superseded by later roadmap revisions.
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
  beginning implementation. Its milestone placement was superseded by M0 T13.
- M0 T8 S1: recorded the NXVM copyright-holder authorization for MIT imports
  into ntvdm64, while retaining upstream-license capture as historical evidence.
- M0 T9 S1: defined optional differential verification interfaces for machine
  and DOS traces, external-reference boundaries, watchdogs, and cleanup without
  beginning M1 or M2.
- M0 T10 S1: resequenced the then-current delivery plan into baseline,
  architecture, machine refactor, DOS, Platform/CLI, and product milestones;
  clarified M1 fixtures and CLI debug ownership without beginning M1. This
  sequence was superseded by M0 T13.
- M0 T11 S1: closed remaining planning ambiguities around implementation
  dependencies, milestone authority, M1 experiment freezing, containment
  ownership, M3 planning, corpus policy, toolchain identity, and local BYOB
  research. Its old milestone numbering was superseded by M0 T13.
- M0 T12 S1: consolidated the then-current optional-research milestones,
  deferred all product CLI work, and merged redundant short planning,
  requirement, and research documents without beginning M1. This numbering was
  superseded by M0 T13.
- M0 T12 S2: clarified the then-future project-owned pre-product Console
  `load` command, required it to enter through the owned DOS loader, and made
  GCC migration design the mandatory first M1 task without beginning M1.
- M0 T12 S3: constrained debugger `load` to an empty paused debug session and
  aligned its reset, first-instruction, and rejection semantics without
  beginning implementation.
- M0 T12 S4: removed premature implementation subtasks and required each
  preceding design milestone to produce the bounded Task/subtask breakdown for
  its implementation successor without beginning M1.
- M0 T12 S5: defined the ignored local executable-artifact convention for
  completed runnable subtasks and recorded the M1 baseline developer artifact.
- M0 T12 S6: defined runtime identity, pre-cutover task version suffixes, the
  post-cutover DOS Machine banner, and post-cutover version encoding.
- M0 T13 S1: reset governance to the dual-product shared-core direction:
  `nxvm.exe` remains a bootable whole-machine VM, `ntvdm64.exe` becomes the
  non-bootable DOS app runner, and both share `core`, `firmware`, `platform`,
  `runtime`, and product-specific composition.
- M1 T1 S1: approved the bounded GCC migration design after a zero-patch GCC
  compile and warning inventory.
- M1 T2 S1: imported and built the full provenance-recorded NXVM baseline;
  recorded bounded FDD/HDD instruction-trace evidence, the raw M1 snapshot,
  and the post-snapshot M1 T2 build identity banner.
- M2 T1 S1: approved Machine/DOS Architecture Requirements V1, including
  explicit boot and DOS-minimal profiles, lifecycle and ownership rules,
  versioned interface contracts, M1 regression mapping, and the bounded M3
  Task/subtask breakdown. No runtime behavior changed. This was superseded by
  M2 T1 S2.
- M2 T1 S2: revised M2 to Shared Core Architecture Requirements V2, promoting
  `nxvm.full_pc` to a first-class product profile, adding explicit registry
  boundaries, and resequencing firmware, DOS, platform, product, compatibility,
  and research milestones without beginning M3.
- M2 T1 S3: adopted parity-first module headers for M3: contract headers live
  beside C implementations in `src/core/`, private headers use `_impl.h`, and a
  top-level `include/` tree is deferred until a future packaging/SDK need.
- M3 T1 S1: defined the Shared Core V1 contract skeleton with module-local
  `src/core/*.h` headers, minimal `src/core/*.c` implementation, CMake target,
  compile/link smoke test, and forbidden-dependency scan. No baseline behavior
  moved.

## Next Eligible Work

- M3 T1 S2: create the Machine instance shell after M3 T1 S1 closes.

## Delivery State

The full NXVM `src/` baseline is now present only in `src/nxvm-baseline/` under
MIT authorization, preserved copyright attribution, and exact provenance. The
repository direction is dual-product: future `nxvm.exe` preserves bootable
whole-machine VM value, and future `ntvdm64.exe` provides the DOS app runner.
No guest media, Microsoft binary, or DOS compatibility claim has been added.
The generated M1 COM probe remains test input only.
