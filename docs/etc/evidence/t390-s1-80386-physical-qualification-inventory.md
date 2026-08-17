# T390 S1: 80386 Physical-Qualification Inventory

`M5:T390:S1:80386-PHYSICAL-QUALIFICATION-INVENTORY:OK`

## Decision

T390 S1 confirms that the current 80386 source classifier contains a mixture
of Intel-primary exact successful-retirement rows and explicit one-tick
nonphysical fallbacks. T388's physical contract correctly rejects the latter
before publication, but Model 40 selects deterministic time and has no
committed or approved external-ROM execution corpus. There is therefore no
basis to enable physical retirement for the Model-40 profile, and no board
clock, DCLK or PIT ratio may be published.

## Publisher And Classification Matrix

`core_machine_instruction_cost()` is the one post-refresh successful-retirement
publisher. It resets `source_timing_unallocated` and calls the following
private owners in order:

| Owner | Current 80386 exact scope | Physical-qualification disposition |
| --- | --- | --- |
| string/I-O classifier | Selected `REP` strings and ordinary `IN`/`OUT`, including source-backed port permission forms. | Exact only where repeat/prefix/permission form is captured; provider latency, denied/fault paths and residual string forms do not qualify. |
| dynamic multiply classifier | Selected 80386 `IMUL` source formula. | Exact only for its captured operands and register/memory forms. |
| secondary `0F` classifier | Near `Jcc`, bit, double-shift, two-operand `IMUL`, scan and MOVX rows from T359 S5. | Exact where lexical/operand form is captured; unavailable target lexeme stays nonphysical. |
| privileged `0F` classifier | T359 S6 fixed system/control/debug/test/segment rows and T388 S8 `LSL` byte/page capture. | Exact only for the named successful mode/privilege/prefix/ModR/M rows; delivery, task, fault and unselected contexts do not qualify. |
| primary and control/stack classifiers | T359/T360/T363 selected data, control, stack and repeat rows. | Exact only within each ledger's stated mode/prefix/outcome assumptions. |
| final 80386 fallback | No-prefix short `Jcc`, `MOV`/moffs/immediate and fixed flag forms are selected; every other successful default, and any nonzero-prefix fallback, calls `core_machine_source_timing_mark_unallocated()`. | Explicit nonphysical prohibition. It remains one deterministic tick and physical mode faults before elapsed/timeline/device publication. |

The relevant production sweeps were:

```text
rg -n -C 8 "core_machine_80386_source_instruction_cost|core_machine_80386_privileged_source_instruction_cost|core_machine_source_timing_mark_unallocated|core_machine_instruction_cost|CORE_MACHINE_CPU_PROFILE_80386" src/core/machine/machine.c
rg -n "source_timing_unallocated|core_machine_source_timing_mark_unallocated|CORE_MACHINE_RETIREMENT_TIME_PHYSICAL" src/core src/vm tests
rg -n -C 3 "80386|source_timing_unallocated|retirement_time_contract|physical" tests CMakeLists.txt cmake docs/etc/evidence
```

No second elapsed publisher, profile-local timing classifier or physical-mode
Model-40 construction was found.

## Intel-Authority Boundary

T359 S5/S6 and T360 S4 identify the authority as the original Intel 80386 PRM
section 17.2.2.3 and applicable instruction pages. Their exact values are core
clock rows under the manual's prefetched, no-wait and no-HOLD assumptions.
They do not establish board waits, DMA arbitration, device service, cache,
prefetch, pin phase, fault/delivery or an unknown prefix/default form. T388
S4/S8 preserves the same distinction: a missing capture or source row is a
physical prohibition, not a one-clock cycle claim.

## Model-40 Corpus Status

The Model-40 profile accepts owner-supplied BYOB ROM chips through a local
manifest, but no ROM byte, hash, local path or real-ROM execution trace is in
the repository or current gate. The existing Model-40 smokes prove composition,
ROM carrier geometry, selected controllers and logical device routes; they do
not execute an external firmware corpus. Existing generic 80386 DOS/Windows
smokes use non-Model-40 session configurations and cannot stand in for the
selected board/firmware corpus.

Consequently, S1 can inventory static eligibility but cannot identify every
successful instruction form needed by the selected Model-40 boot or DOS
checkpoint. A complete profile qualification must be tied to a bounded,
owner-managed external-ROM and guest-media corpus without importing any of
those assets.

## Next Receiver

The earliest T390 implementation/design receiver is a bounded BYOB
physical-qualification observation contract. It must declare owner-managed ROM
and guest-media identity outside the repository; an exact Model-40 profile;
finite reset/boot/checkpoint/stop budgets; an ignored bounded trace; and a
normalised sequence of retired instruction bytes, mode/prefix/form context,
classifier disposition and terminal reason. It must use the existing Core trace
and lifecycle boundaries where sufficient, and propose a narrow shared capture
only if the current trace cannot distinguish an unallocated successful form.

Only after that corpus is frozen may later T390 slices assign an Intel-primary
row, implement missing private capture, add regressions and prove that every
success through the selected checkpoint is exact-or-stops before physical
publication. This is not a universal 80386 claim and still does not authorize
Model-40 board timing.