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

T93 moved VM context lifetime into composition. Its GCC artifact is
`build/output/nxvm_0_5_0093.exe`.

T96 repaired a retained Console lifecycle regression introduced by the T92/T93
context work and added a real command lifecycle gate. Artifacts `0.5.0092`
through `0.5.0095` are revoked because they can fault on the first non-`exit`
Console command. Use `build/output/nxvm_0_5_0096.exe`.

**M5 T97 complete:** composition lifecycle APIs now use
`vm_composition_providers_*`; the retained bootable VM behavior and the
historical QDX/approved compact-name exceptions are unchanged.

## M5 Closure

**M5 T87--T95 session-readiness closure: complete.** Every implemented module
now has session-owned, thread-local, or explicitly leased mutable state; the
remaining VDM platform/product layers are deliberately absent. Evidence is in
[`m5-t95-s1-session-readiness-closure.md`](../verification/m5-t95-s1-session-readiness-closure.md).

**M5 T96 Console regression repair: complete.** The retained Console has a
single live-machine lifetime and a command-level lifecycle gate.

**M5 T97 composition API naming: complete.** The active full-PC lifecycle
surface no longer uses `vmachine*` or `devicePrintMachine` names. Evidence is
[`m5-t97-s1-composition-api-naming.md`](../verification/m5-t97-s1-composition-api-naming.md).

**M5 T98 naming-remediation breakdown: complete.** The remaining active
legacy symbols are scheduled in owner-bounded tasks; uppercase `type.*`
C-runtime wrappers and approved compact names are explicit retentions.

**M5 T108 C-library facade audit: complete.** The standard C vocabulary,
collision-safe uppercase facade, direct-call inventory, and two behavior risks
are recorded before the `type.*` call-site migrations.

**M5 T99 complete:** all active former `vglobal.h` callers include the root
`type.h` foundation directly; the forwarding header is deleted.

**M5 T100 complete:** root string/trace helper APIs now use
`ntvdm64_type_*`; retained wrapper behavior is unchanged.

## Short-Term M5 Plan

The completed executor sequence is defined in
[Facade And Executor Convergence](m5-facade-executor-convergence.md). The
active session-readiness sequence is defined in
[M5 Session Readiness](m5-session-readiness.md): machine, platform, product,
then root composition in the fixed core/VM/VDM order.

[M5 Naming Remediation](m5-naming-remediation.md) is the approved execution
order for the remaining active old-style APIs. Its T108 C-library facade audit
precedes `vglobal.h` cleanup and does not start M6.

M6 has not started. M7 implementation remains ineligible until M6 defines and
closes its implementation breakdown.

## Delivery Boundary

The repository builds `nxvm.exe` and the future `ntvdm64.exe` over one shared
core. No guest media, Microsoft binary, DOS compatibility claim, invasive
integration, or Win16 route is part of the default product.
