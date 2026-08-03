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

**Latest task:** M5 T146 S1 implemented the generic core-product manager and
made NXVM create/select session `0` through the VM composition provider before
entering the retained Console. Its evidence is
[`M5-T146.md`](../tracking/M5-T146.md).
The authoritative core/product and composition ownership, UX, source shape,
and T145--T150 breakdown are in
[M5 Shared Product Session Management Design](m5-product-session-management.md).

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
now has session-owned, execution-thread-owned, caller-owned, or explicitly
leased mutable state; the remaining VDM platform/product layers are deliberately
absent. Evidence is in
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

**M5 T101 complete:** all active source/tests use the root-owned
`ntvdm64_status` and `NTVDM64_STATUS_*` vocabulary without aliases.

**M5 T102 complete:** shared product debug/runtime APIs use
`core_product_*` while retaining approved debug/assembler lexical names.

**M5 T103 complete:** VM composition lifecycle/block/CPU-probe and VM HDC
APIs now use their source-owner prefixes without compatibility aliases.

**M5 T104 complete:** active VM platform hub and OS-backend APIs now use
`vm_platform_*`, retaining approved backend lexical names.

**M5 T105 complete:** VM product/profile and adjacent composition/platform
legacy APIs now use their actual source-owner prefixes.

**M5 T106 complete:** the implemented VDM composition skeleton now uses
`vdm_composition_*` names without adding a VDM runtime path.

**M5 T107 complete:** root `type.*` aliases and non-retained macros use
`ntvdm64_type_*` / `NTVDM64_TYPE_*`; module-owned structs remain separate.

**M5 T109 complete:** `C_*`/`STD_*` facade surfaces exist and former root
wrappers/callers now use `STD_*`; scalar aliases and Win32 remain separate.

**M5 T110 complete:** active direct allocation, memory/string, file, time,
ctype, and formatting calls outside `type.*` use `STD_*`. The C facade is a
base target rather than a core-machine dependency; `fflush(stdin)` remains for
the later platform input-flush design.

**M5 T111 complete:** active project declarations and definitions use `C_*`
scalar aliases; public headers now declare their `type.h` dependency directly.
Guest firmware strings retain their native assembly vocabulary.

**M5 T112 complete:** active project ISO C types, objects, constants, and
atomic operations use `STD_*`; explicit atomic memory orders are unchanged.

**M5 T113 complete:** `type.h` is the only active ISO C header boundary;
`memory.h` is removed and the static ownership gate passes.

**M5 T114 complete:** Win32 adapter public signatures use `C_VOID` and
`WIN32_*`; SDK calls and private message-loop state remain inside
`platform/win32`.

**M5 T115 complete:** debugger input clearing routes through a core product
provider and VM platform capability; `fflush(stdin)` is absent.

**M5 T116 complete:** final static vocabulary, dependency, documentation,
GCC, DOS boot, and retained Console closure passed. The T111--T116 naming
sequence is complete.

**M5 T116 S2 complete:** the source vocabulary and build-tree cleanup policy
are permanent governance rules; only `build/output/` remains after cleanup.

**M5 T117 complete:** VM and VDM composition implementation/private headers
now live in their product-root `composition/` directories. GCC gates, FDD DOS
boot, and retained Console behavior passed without a behavioral change.

**M5 T118 complete:** VM composition has one real `vm_session` object. The
historical `full_pc`, default-profile forwarding, runtime-profile session, and
minimal session-model wrappers are deleted; their coverage now exercises the
real session. Control and runner are separate, and VDM composition file names
match the same directory rule.

**M5 T119 complete:** the T120--T127 unique-owner convergence baseline is
recorded in [`m5-unique-owner-convergence.md`](m5-unique-owner-convergence.md).

**M5 T120 complete:** an active `vm_session` cannot be initialized twice.
Console lifecycle, debugger, session, and DOS-prompt gates passed.

**M5 T121 complete:** custom core machines have one production executor;
test-minimal state is explicitly non-executing contract-fixture state. Core
and DOS-prompt gates passed.

**M5 T122 complete:** CPU-probe and compact alternate Console/debugger models
are test support, not production VM targets.

**M5 T123 complete:** debugger target access is explicit and no longer uses a
TLS target scope. Retained parser-local TLS is recorded for T127 review; T124
is active to converge the VDM skeleton boundary.

**M5 T124 complete:** VDM minimal machine APIs are VDM-owned and presentation
uses the session as its sole entry.

**M5 T125 complete:** unconsumed descriptor/registry wrappers are removed;
the full focused GCC gate suite passes after the state-inventory update.

**M5 T126 complete:** no-op VM platform/HDC lifecycle facades are removed;
the retained hard-disk interrupt firmware belongs to the default VM profile.
The two host Console leases remain explicit process-exclusive resources.

**M5 T127 audit complete:** VM has one formal session and execution path, and
the unconsumed profile wrappers are gone. The audit records three P1 remaining
TLS implementation dependencies (debug, Console, wait), plus VDM's intentional
non-runnable skeleton boundary and host-lease verification gap. These are
post-audit work, not evidence that the unique-owner objective has fully closed.

**M5 T128 complete:** [`m5-t128-t136-convergence.md`](m5-t128-t136-convergence.md)
defines the explicit-invocation and unified-executor sequence. T129 is the
sole active subtask.

**M5 T130 complete:** NXVM Console command helpers receive their explicit
caller-owned context; `consoleContext` TLS is removed. T129 remains active for
the shared debugger invocation migration.

**M5 T133 complete:** core exposes ordered executor CPU/bus/memory preparation;
VM retains its proven provider order through that common contract. T129 remains
active for debugger TLS removal.

**M5 T135 complete:** VDM-minimal now borrows the same custom executor CPU,
RAM, and port path as NXVM, while retaining only VDM-owned providers and
snapshot state. It remains deliberately non-runnable pending the DOS design
milestone. T129 remains the sole active subtask.

**M5 T134/T136 complete:** core contract smoke uses the real executor and the
test-minimal profile/state has been removed. T129 remains the sole active
subtask.

**M5 T131 complete:** wait selection is explicit through platform, profile,
and debugger contexts; no production wait TLS or enter/leave facade remains.

**M5 T129 complete:** debugger command dispatch and xasm execution now pass
their caller-owned contexts explicitly; no debugger or xasm TLS selection
remains. The retained Console/debugger and DOS-prompt gates pass with
`nxvm_0_5_0129.exe`.

**M5 T131 S2 complete:** the full build revealed and corrected a stale
wait-scope smoke test that still named the removed ambient API. It now proves
explicit scope isolation.

**M5 T132 complete:** production source has no TLS/current-object selection
facade, core has no VM/VDM include, and NXVM retains one product entry/run
path. Windows GCC plus all 54 smoke executables pass; Linux remains a native
POSIX follow-up gate and VDM intentionally remains non-runnable.

**M5 T137 complete:** VM-only execution control is now named
`vm_session_execution_context` and compiled directly in VM composition; the
generic `nxvm-runtime` target is removed. Focused execution, Console,
two-session, and FDD DOS-prompt gates pass with `nxvm_0_5_0137.exe`.

**M5 T138 complete:** the run-handle owner, creator, worker reporters,
cancellation/join/destroy sequence, and current Win32/Linux owner map are
recorded. Workers may report stop/completion but may not release shared
platform/session state.

**M5 T139 complete:** Win32 Console and window workers now run through one
session-owned handle; task evidence records the join/finalize and DOS-prompt
gates with `nxvm_0_5_0139.exe`.

**M5 T140 complete:** Linux Console now has the same explicit handle and
sole-destroyer source structure. Native POSIX compilation and runtime evidence
remain a designated POSIX-environment handoff.

**M5 T141 complete:** core now owns one standard executor and the full shared
device lifecycle. Windows GCC core, VDM, debugger, session, Console, and FDD
DOS-prompt gates pass with `nxvm_0_5_0141.exe`.

**M5 T142 complete:** the VM raw-alias map, forwarding borrow facade, and
execution-context CPU/RAM/port aliases are removed. Root composition directly
borrows core-owned capabilities only for actual binding/debug-target assembly;
the execution context holds its explicit `vm_session*` callback owner.

**M5 T143 complete:** isolated VM product models and their fixture-only CMake
targets are deleted; the VDM-only input event is correctly owned by
`vdm/composition`; source TODOs are priority-tagged and ledgered; current
artifact/gates are `0.5.0143`; generated build products are removed outside
`build/output/`.

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
