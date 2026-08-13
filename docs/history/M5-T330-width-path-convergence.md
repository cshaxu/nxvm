# M5 T330: Width-Path Construction Convergence

## Scope And Result

T330 performed a whole-file mechanism audit of
`src/core/machine/cpu_instructions.c`. It classified width-parallel paths by
validation, materialization, preflight, commit, and fault boundary, rather
than attempting a syntactic removal of all 16/32-bit branches. The durable
[inventory](../etc/evidence/t330-width-path-inventory.md) retains the
classification and transfers.

S1 replaces the previous outgoing-TR-selected task-switch construction with
one private transition plan. The plan independently selects outgoing and
incoming 286/386 TSS formats, so all four pairs are supported while their real
layouts remain explicit. The 286 outgoing span is fully preflighted and
written through LDTR at `0x2a`; null non-stack data selectors materialize an
invalid cache, while null SS remains `#TS(0)`.

S2 corrects the 32-bit protected CALL-gate same-CPL route. Equal target DPL is
accepted, a same-CPL transfer needs no TSS, and it writes only the 32-bit
return EIP/CS image on the current stack. Lower-CPL transfers retain the
separate TSS, replacement-stack, parameter-copy, and outer-frame path.

## Evidence And Verification

`core-machine-task-switch-smoke` proves direct far JMP, nested far CALL,
task-gate entry, and bounded nested IRET return for every 286/386 source and
target TSS combination. It retains descriptor, TSS-size, LDT, stack, paging,
and T-bit fault coverage, and adds T330 marker
`M5:T330:S1:TASK-TRANSITION:OK`.

`core-machine-call-gate-privilege-entry-smoke` now proves a CPL0-to-CPL0
32-bit gate with an invalid TR: target entry succeeds, the two-dword return
frame is exact, and adjacent stack storage, EFLAGS, and non-stack GPRs remain
unchanged. Retained vectors prove lower-CPL TSS and parameter behavior plus
descriptor, TSS, and stack fault boundaries. Its T330 marker is
`M5:T330:S2:CALL-GATE-SAME-CPL:OK`.

Fresh GCC configuration, documentation governance, and `git diff --check`
passed. The full current-gate CTest selection passed 211/211 tests for both
accepted implementation commits: `82bea776` (S1) and `263766f0` (S2).

## Developer Artifact

Commit `f8116f99` makes `vm-0-5-0330` the sole current-artifact target. Its
locally retained developer output is `build/output/nxvm_0_5_0330.exe`,
SHA-256 `926D0F4A2CDB48367C522AD799FF4E9C2DE7AD6BF6C75EDB7889E2D8B9B62CAA`.
The executable embeds version `0.5.0330`; `verify-current-artifact-target`
selects only that target. Historical task artifacts remain evidence and are
not rebuilt or relabeled.

## Transfers

T330 intentionally retains Intel-required 16/32-bit differences in TSS, gate,
interrupt, IRET, descriptor-table, control-state, and frame layouts. It does
not close generic exception/IRQ redesign, VM86/VME/PVI, paging/TLB, public
CPU interfaces, or x87 execution. The inventory records these as retained
shared mechanisms, true architectural layouts, or separate named packages;
they must not be reopened through isolated symptom patches.

## Corrective S4: Retired Parallel 32-bit Construction

The post-closure whole-source audit identified a residual unreachable legacy
32-bit task-transition plan/commit/wrapper chain. S4 removed it and added a
current specialized verifier that proves the accepted T330 private transition
plan is the sole constructor reached by task entry and backlink return. The
correction preserves the separate Intel 286/386 TSS layouts and the accepted
task-switch test corpus; it does not broaden task-switch semantics.

## Continuation S5: Media-Provider Backing Construction

The whole-codebase follow-up corrected the only shared-media construction
drift: FDD could publish a present medium after allocation had left its backing
null, while FDD direct callbacks and HDD formatting could dereference a null
backing in a malformed present context. The two provider implementations now
classify that state as a permanent media failure before dereference; FDD create
publishes presence and generation only when backing exists. The focused media
provider smoke proves direct and registry-facing read/write/format rejection,
FDD failed-create nonpublication, and all retained normal replacement, remove,
insert, formatting, capacity, and atomic-save paths. This is provider-local
contract convergence, not a media-interface, controller, or persistence-policy
change.

## Continuation S6: CALL-Gate Preflight Convergence

S6 reconciles the accidental preflight-order drift between 16-bit and 32-bit
outer-CPL CALL-gate serializers. Both now validate the TSS-provided replacement
stack and its full target frame before reading old-stack parameters, as required
by the Intel 80386 CALL sequence. The width-specific TSS, parameter, and frame
layouts remain explicit. Paired dual-fault vectors make an old parameter unreadable
and the new SS invalid; both serializers deliver the width-specific `#TS` code
through vector 10, proving that the new-stack validation wins before any target
publication. No
shared exception, descriptor, paging, or ABI behavior was changed.

## Historical S7: State-Machine Matrix Discipline

T330 S7 documented a state-machine matrix practice for its own review: before
P1, the task recorded callers, state/width/mode dimensions, validation and
competing-fault order, source and target preflight, materialization, writes,
commit/rollback, delivery dependency, and named transfers. The practice
preserved Intel-defined layouts and ordinary shared decode/read/write paths.

This was subsequently withdrawn as a project-wide Execution Rule. It remains
historical T330 evidence only; current packet and review requirements are
defined exclusively by `docs/rules/EXECUTION.md`.

## Final Closure Audit

The owner reopened T330 to resolve four audited construction-drift items. S4
removed the unreachable parallel 32-bit task-transition constructor and added
its static closure gate. S5 reconciled the FDD/HDD provider backing and failed-
construction contract. S6 corrected the CALL-gate preflight sequence in both
widths and proved the competing-fault result. S7 retained the resulting full
mechanism matrix as T330 review evidence. These corrections retain Intel-defined layouts and do not replace shared
memory, descriptor, exception, paging, media-interface, or controller owners.

The runnable S6 source commit is `f8116f99`; its T330 developer artifact is
`build/output/nxvm_0_5_0330.exe` with SHA-256
`926D0F4A2CDB48367C522AD799FF4E9C2DE7AD6BF6C75EDB7889E2D8B9B62CAA`.
The S7 documentation-only continuation did not rebuild that artifact. S4--S6
focused checks and current-gate CTest 211/211 passed; S7 documentation
self-test, default governance check, and diff check passed. The resolved
task-switch debt is not retained as current Queue or TODO work.
