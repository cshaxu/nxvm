# T563 Final Closure Audit

## Decision And Fixed Qualification Scope

On 2026-10-17 the owner accepts S10 and authorizes T closure if the complete
486 CPU assignment is finished. Coordinator review at `80bc1ac92` confirms
the selected embedded 80486SX C01--C17 universe, twenty C15 families and all
R1--R10 receivers have their implemented/proven dispositions. Accept S10's
Shared `a936e554b`, MyNES `621b98d15` and NXVM `80bc1ac92`; close S10 and T563.
This is final acceptance of the original selected-SX task, not another bounded
stage transfer or a claim to implement every 486 variant or PC110 board.
MyNES's acceptance-only P4 `43f0d8164` is pushed separately; the final NXVM
governance P archives this proposal and records task closure without code changes.

## Original Requirement Reconciliation

The [original ledger](t560-s1-cpu-convergence.md), [S1 receiver census](t563-s1-rebase.md)
and [S8 complete current-source matrix](t563-s8-whole-qualification.md#original-requirement-to-current-proof-audit)
remain the detailed evidence rather than being replaced by green test counts.

| Original scope | Implemented owner and accepted proof |
| --- | --- |
| C01 identity/features | Selected CPU reset/CPUID/flags and VME owner; direct CPU/profile cases. |
| C02 reset/fetch/modes | CPU dirty-state reset, hidden reset fetch and real Core/Board ROM/RAM aliases. |
| C03 control registers | Guest CR validation/publication and common invalidation; S8 repairs Debug candidate preparation/cache commit. |
| C04 paging/access | Single translation/A-D/permission owner; actual successful and failing physical accesses, completed by S3 timing. |
| C05 alignment/order | Shared logical preflight and selected-family segment/AC/page precedence; real exception/restart cases. |
| C06 TLB/test commands | One TLB, real test commands, invalidation/dirty transition and passive inspection; S3 costs. |
| C07 cache/bus commands | One 8-KiB cache, TR3--5, INVD/WBINVD, physical coherence and S4 bus/write FIFO. |
| C08 descriptor effects | Validated descriptor owner, Accessed/busy writes and failure prefixes; S8 live-cache repair. |
| C09 inherited RMW | Original handlers use common preflight/commit with accepted-prefix failure proof across all admitted groups. |
| C10 VME/VM86 | CR4/flags/TSS bitmap/interrupt/IRET owner and mode/privilege/failure matrices; no advertised PVI/PSE/APIC. |
| C11 SMM/RSM | Real save/restore, relocation, restart, priority, failed entry/restore and Core system receivers. |
| C12 signals/power | CPU system/bus signals feed the sole Core scheduler; event elapsed time is not fake retirement. |
| C13 Debug/exceptions | DR/fault/shadow/entry and public paused Core operations; S8 B1--B4 closes candidate/cache and prepared-entry gaps. |
| C14 absent FPU | Selected SX ESC/WAIT exceptions and actual no-FPU construction, including unsupported binding rejection. |
| C15 all inherited forms | All F01--F20, original semantic/context/fault witnesses and reserved/invalid encoding cases retained by S8. |
| C16 complete timing | S2 nominal, S3 memory/translation, S4 fetch/bus/decode/REP, S5 system/event and S6 sole runtime/preview/budget aggregation. |
| C17 production admission | S7 real Core constructor/reset/run/Debug/DMA/clock predicates and removal of nineteen private profile rebindings. |

T558 CPU and T559 instruction handoffs are mapped in S8's inherited-receiver
section. R2 maps to S2; R3/R4 to S3; R5/R6 to S4; R7 to S5; R1/R8 to S6;
R9/R10 to S7. No applicable missing CPU operation is newly transferred or
silently marked non-applicable at closure.

## Current Source And Failure-Boundary Review

`git diff 16e7e3a97 HEAD -- src/x86-core test/x86-core` is empty: the repaired
S8 CPU/Core implementation and tests survive both imports unchanged. The
post-S9 X86 NXVM source delta is only the reviewed key-symbol mapper adaptation.
Inspect current `cpu_timing.c`: the selected SX arm invokes only
`x86_core_cpu_80486_source_instruction_cost`, not compatibility fallback.
Inspect its implementation: checked nominal/L2, memory, decode and bus costs
join one result with accurate source/model origin. The zero static maximum
retains bounded pending completion for variable service, not fabricated time.
Core admits real SX/NONE and rejects SX/FPU and the 386-only MOV-CR quirk;
Board retains the source-qualified high reset alias and AT capability.

Architecture/coding review preserves CPU state/cache/TLB/FIFO ownership,
Core guest time/pending completion and Board reset/wiring. S8's actual code
review and C03/C13 regression remain essential; the all-green baseline that
missed that defect is not substituted for the repaired proof. No duplicate
execution route, decoder, clock, compatibility wrapper or new API is added
by this governance-only closure.

## Final Verification And Residual Boundaries

Recheck S10's actual final XML: both widths execute 655 repository units,
including all 166 X86 Core tests, plus 48 product and four shared native
integrations, with zero final failures/skips. Both full builds, specialized
aggregates and 498 strict rows passed. The ten current artifact hashes still
match [the S10 inventory](t563-s10-artifacts.json). Recheck current Core source/test
manifests and inward-boundary gate successfully. No executable input changes
at closure, so no timestamp-only rebuild or repeated whole suite is needed.

S8's sole transferred native MIDI gate is now actually successful in both
S9/S10 native integrations, not merely reclassified. Initial S10 x64 negative
test timeout and native Console pause failure remain recorded alongside
unchanged successful full reruns. The existing NXVM test-runtime and MyNES
native-input TODOs remain; this closure does not claim their intermittency fixed.

Approved L2 arithmetic/access/TLB/decode/write-buffer/system models remain L2;
undefined outputs and raw administrative Debug policy retain their explicit
contracts. Broader x87, other CPU variants, analog/pin timing, fitted PC110
silicon/board/firmware, manual RDP, physical audio and full guest GUI are not
qualified by this task. These are existing distinct boundaries, not newly
deferred applicable CPU requirements.

The archived proposal, T history, Queue and Current are reconciled. No new T
is admitted; PC110 Board remains first queued. Owner INIs, ROMs, media,
snapshots and all ten EXEs are untouched by closure. Documentation gates,
link checks, target-isolated actual diff/commit review and immediate push
complete the governance delivery; shared source/test bytes remain unchanged.
