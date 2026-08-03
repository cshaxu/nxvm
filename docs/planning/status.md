# Project Status

## Closed Milestones

- **M0:** governance, MIT source boundary, lightweight C11/GCC toolchain, and
  project direction are defined.
- **M1:** the NXVM baseline is provenance-recorded, GCC-runnable on Windows,
  and has bounded FDD/HDD evidence.
- **M2:** shared-core architecture and the dual-product split are defined.
- **M3:** the shared-core source shape and no-media VDM profile tests are
  complete.
- **M4:** firmware, profiles, CPU verification, and retained NXVM Console
  contracts were designed.

## M5 State

M5 has preserved the bootable NXVM Console, debugger, boot sequence, and FDD
fixture behavior while moving the whole-PC execution path into the documented
`core/` and `vm/` ownership model.

- T64 defined the explicit session and provider model.
- T65 through T73 removed mutable selected-session state from the VM execution
  graph. T73 proves two simultaneously constructed sessions are isolated and
  records a clean legacy-selector scan.
- T68's original artifact is revoked because it could fault after FDD boot;
  T68 S2 repaired its live-machine binding before T70 continued.
- T70's original artifact is revoked because it failed to prove guest boot;
  T70 S2 repaired FDC initialization order and added the three-second
  boot-and-display gate.
- T71's original artifact is revoked because it did not prove the host display
  frame; T71 S2 corrected the snapshot binding. The manually reviewed
  replacement is `build/output/nxvm_0_5_0071a.exe`.

The completed authority plan and its evidence index are in
[the archived plan](../history/m5/planning/m5-instance-authority-plan.md) and
[the authority-closure index](../verification/m5-t64-t73-authority-closure-index.md).

T75 recorded the residual process-global facade baseline and added the CMake
`verify-facade-ownership` gate. Its GCC artifact is
`build/output/nxvm_0_5_0075.exe`.

T76 moved full-PC host keyboard-state requests to their own frozen keyboard
provider slot. Its GCC artifact is `build/output/nxvm_0_5_0076.exe`.

T77 moved QDCGA display notifications and display generation to their owning
profile/session context. Its GCC artifact is `build/output/nxvm_0_5_0077.exe`.

T88 made CPU trace workspaces execution-context-owned and VM control state
atomic. Its GCC artifact is `build/output/nxvm_0_5_0088.exe`.

T89 confirmed VDM-minimal state is instance-owned and added the shared
host-surface context/lease contract. It changes no runnable product path.

T90 made VM Win32 renderer resources context-owned and modeled shared Console
and Linux terminal ownership as explicit leases. Its GCC artifact is
`build/output/nxvm_0_5_0090.exe`.

T91 made core debugger command state caller-owned and assembler/disassembler
workspaces invocation-owned. Its GCC artifact is
`build/output/nxvm_0_5_0091.exe`.

T92 made NXVM Console state caller-owned. Its GCC artifact is
`build/output/nxvm_0_5_0092.exe`.

## Active Work

**M5 T93 S1: VM composition context integration.** Move current caller-owned
machine, platform, debugger, and Console context ownership into VM composition
lifecycle without changing the retained NXVM user surface.

## Short-Term M5 Plan

The completed executor sequence is defined in
[Facade And Executor Convergence](m5-facade-executor-convergence.md). The
active session-readiness sequence is defined in
[M5 Session Readiness](m5-session-readiness.md).

M6 T1 remains deferred until this M5 sequence closes. M7 implementation remains
ineligible until M6 defines and closes its implementation breakdown.

## Delivery Boundary

The repository builds `nxvm.exe` and the future `ntvdm64.exe` over one shared
core. No guest media, Microsoft binary, DOS compatibility claim, invasive
integration, or Win16 route is part of the default product.
