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

Commit `af006a19` makes `vm-0-5-0330` the sole current-artifact target. Its
locally retained developer output is `build/output/nxvm_0_5_0330.exe`,
SHA-256 `F0AEC7780F2BC0A0AEF6451B91CCD4EE25DF6A9D1EE4A5AFA8D1AD3E58F9707A`.
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
