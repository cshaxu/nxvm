# T560 S2 Base System Work

## Source Decisions

Baseline 755496841. Original manuals and hashes are pinned by T558; no
third-party implementation is imported. PRM Figure 10-2 and Table 10-1 were
rendered and visually checked at PDF pages 252--253 (printed 10-2--10-3).
They give CR0 CD/NW/ET set, DR6 FFFF0FF0h, DR7 zero, EFLAGS 2, CS F000h
with hidden base FFFF0000h and EIP FFF0h. The developer manual Table 9-8
independently gives CR0 60000010h and DR7 zero. Undefined register values
are not promoted to documented reset guarantees. SX software initialization
recommendations for EM/MP/NE are not hardware reset values.

Developer manual 6.4.3/Table 6-4 confirms supervisor writes to readonly pages
fault with WP set, independent of the page's U/S bit. PDE/PTE permissions
combine by the most restrictive value. The existing translation owner now
includes the 486 WP condition and preserves supervisor PF error-code U/S=0.
The old profiles keep their previous permission contract.

## Implementation And Verification Sequence

Reset state and reset-fetch provenance are changed at the existing owners;
the existing CPUID/reset test now checks documented reset fields after dirtying
them. CR0 bit names remain owned by Core CPU. This does not implement cache,
alignment or VME effects merely because their control bits have names.

Developer manual Tables 4-2/4-3 explicitly require GP(0) for PG without PE
and NW without CD on a MOV CR0 load. Section 4.4.1 assigns CR3 PWT/PCD
and its page-aligned directory base. The existing CR writers now distinguish
486 from retained 386 behavior without a second dispatch path. PRM 4.1.3
states ET is one; the writer now preserves that fixed value. Developer manual
4.8 treats reserved bits as fully undefined, specifically forbidding reliance
on their retaining written data. The model normalizes them to zero rather
than inventing a UD fault. This reserved-bit choice is deterministic modeling,
not a claim about undocumented silicon readback. Tests explicitly clear ET and
write the reserved-bit pattern. Cache and TLB effects are not claimed by these
register changes.

The existing control-state test now executes guest MOV CR0/CR3, checks four
legal CR0 patterns and all four PWT/PCD combinations, and proves both illegal
CR0 combinations deliver GP through a valid real-mode vector without changing
the control state. It also checks LMSW/CLTS preserve the upper 486 controls.
The original x64 Ninja/ccache target builds and the registered
core.chips/cpu/cpu_control_state test passes (0.11 seconds). An initial build
caught an incorrectly abbreviated profile identifier, corrected to the existing
80486SX name before that run.

PRM 5.2.5 locates GDT/IDT in linear address space; 5.2.3 and 21.3.6
distinguish 486 conditional Accessed writes from 386 unconditional writes.
The existing table helpers now use supervisor linear reads/writes for 486;
earlier profiles retain their current routes. The existing descriptor write
owner sets Accessed for user descriptors and skips already-set 486 writes;
system descriptor updates, including TSS busy, are not skipped. Prepared
segment caches carry Accessed=true while the descriptor retains its original
bit until publication, so preflight can avoid an unnecessary write fault.

The actual-guest control-state test maps GDT and IDT away from their linear
addresses, leaving those physical addresses empty. MOV DS proves translation
and Accessed writeback; INT3 proves paged gate lookup. Making the GDT readonly
then proves already-accessed data/code loads succeed with WP=1, while clearing
the data descriptor's bit delivers PF(3) through a valid handler, reports the
descriptor linear address in CR2 and leaves the segment/descriptor unchanged.

Focused grouped qualification passes 5/5 on each host width in the original
Ninja/ccache graphs: control_state, cpuid_flags, execution_paging,
task_switch32_state and protected_iret_state. This includes retained-family
regressions. At that intermediate point no full-suite, integration or new
artifact result was claimed; the later batch results are recorded below.

The control-state test additionally covers 256 actual MOV byte accesses:
all PDE/PTE R/W and U/S combinations, both WP values, CPL 0--3 and read/write.
The data lives in a separate page directory from code, stack and exception
tables. Each fault must reach a valid same-level handler with the exact PF
error, CR2 and stacked error code; denied writes preserve the data byte and
denied reads preserve AL. The first fixture run correctly exposed its null DS
selector; the protected fixture now supplies an explicit nonnull selector.
The complete matrix passes on x64 and x86.

The reset test now also executes a far jump fetched at FFFFFFF0h through a
recording bus provider, checks the reset-fetch provenance and hidden-CS
transition, then executes a low-address NOP without reset provenance. The
updated reset/CPUID and control tests pass on both widths. These are focused
checks in the reused build graphs, not a new full-suite qualification.

The descriptor test also splits a GDT or LDT entry across two noncontiguous
physical pages. It checks the loaded base, selector and Accessed update, then
makes both pages readonly and repeats the successful already-accessed load.
The LDT cache is explicitly valid with a nonnull selector; a first fixture
attempt omitted the latter and was corrected, not treated as a CPU defect.
The expanded control test passes on x64 and x86 in the original build graphs.

Additional actual-guest tests cover VM86 reads/writes against all four page
RW/US values with WP clear/set. Faults use a valid 32-bit TSS to enter the
ring-zero handler and check PF(5/7), CR2, CS/SS and cleared VM. Successful
operations remain in VM86. Split-dword tests use two noncontiguous pages and
cover successful reads/writes, second-page absence and second-page readonly
protection; failures preserve both data fragments and EAX. Successful accesses
check both PTE Accessed bits and write-only Dirty bits. Undefined fault-time
Accessed side effects are not asserted as a hardware guarantee.

The CR writer sweep found task-switch CR3 publication bypassed the new masks.
486 task loads now use the same existing CR3 owner as MOV; earlier profiles
retain their previous path. Existing five task-completion routes now exercise
all PWT/PCD combinations and reserved low-bit normalization alongside their
flags matrix. Control-state and task-switch tests pass on both host widths.
Debugger register patches remain explicit paused-state edits, not guest MOV
instructions; they are not made to inject guest exceptions. Reset, LMSW,
CLTS, page-fault CR2 publication and task TS updates keep their distinct owners.

The VM86 fixture also checks eight privileged forms: reads/writes of CR0,
CR2 and CR3, CLTS and LMSW. All deliver GP(0) through the valid TSS/handler
without changing CR0/CR3 or GPR results. This extension passes in both complete
unit runs below.

Both Core manifests are regenerated and verify successfully. The final owner
sweep confirms one Accessed publication site and one guest CR3 writer for 486
MOV/task loads. Both original Ninja/ccache complete unit builds have finished.
The x64 complete unit run
passes 597/597 in 224.03 seconds, including all five Apps, Core and public
components, with three desktop-labelled unit cases. Its detailed native CTest
log is preserved at build/t560-s2-x64-unit-details.log. The relative requested
output-log file was not created; the retained LastTest.log is the authoritative
per-test record. The x86 complete unit run passes 597/597 in 45.51 seconds;
its detailed log is preserved at build/t560-s2-x86-unit-details.log.
NXVM documentation governance also passes. These automated desktop-labelled
tests are not manual RDP or external guest-boot acceptance.
The three Shared root preset target-version updates were initially held for
review. On 2026-10-14 the owner approved them and permanently authorized
equivalent routine task-version synchronization of build targets, presets and
release configuration. This does not authorize unrelated Shared functionality,
APIs or MyNES changes. The three NXVM targets now name vm-0-5-0560.

Executor review is complete; delivery remains subject to commit/push and
coordinator acceptance. No completed CPU claim accompanies this base batch.

Read-only manual extraction/renderings are ignored in build/t560-s2-research
and retained for this active S only. They are not sources or release inputs.

## Release Qualification

NXVM-owned version and artifact targets are updated to 0.5.0560. Both default
EXEs have been built and deployed through the original Ninja/ccache graphs;
their deployment PE architecture checks pass. The complete default integration
aggregate passes 35/35 on each width, x64 in 52.78 seconds and x86 in 62.12
seconds. These include DOS/Windows checkpoints and the real product INI route.
Logs are build/t560-s2-<architecture>-default-integration.log and the adjacent
integration-details.log copies. The aggregate includes twelve MyNES tests;
it does not rebuild or change the published MyNES pair.

XT, AT and DeskPro publication and integration also pass. No unchanged old
binary or pre-repair test result substitutes for their final S2 qualification.
All eight old 0559 EXEs were removed after replacement verification; Git
history retains them. Adjacent INIs, media and the MyNES pair are unchanged.

| Profile | Cases per width | x64 seconds | x86 seconds |
| --- | --- | --- | --- |
| Default NXVM | 35 | 52.78 | 62.12 |
| IBM 5160 | 15 | 27.15 | 34.42 |
| IBM 5170 | 17 | 41.24 | 52.06 |
| DeskPro Model 40 | 17 | 114.61 | 154.72 |

Other-profile logs use build/t560-s2-<architecture>-<profile>-integration.log,
with adjacent detailed CTest copies. Each aggregate contains the same twelve
MyNES cases; repeated runs are not counted as distinct coverage. Both original
build graphs are restored to default NXVM. Post-version Core Product units
pass 2/2 per width. Existing integration predicates and budgets are unchanged;
no manual desktop/RDP acceptance is claimed.

Actual production review retains one CPU dispatch and page-translation owner,
one user-descriptor Accessed writer, and one 486 guest/task CR3 writer. No new
production state, public function, thread or fallback path was added. The only
public vocabulary addition is five existing CR0 bit constants. The reset bus
fixture is test-local; it distinguishes FFFFFFF0 reset fetch from the far-jump
target and checks the actual bus provenance, rather than merely reading state.

Counted with git diff --numstat against 755496841: the five changed Core
production C/H files add 105 and remove 49 lines (net +56, including the
one-line version replacement). Three CPU test files add 543 and remove 2
(net +541). The NXVM artifact CMake changes add/remove 2 lines. Documentation,
manifests and binaries are excluded from those code counts. The positive test
delta covers actual instructions, valid fault delivery and permission matrices;
it does not add a framework or duplicate the interpreter.

## Artifact Identity

All eight 0560 Release binaries pass PE-width and embedded-version checks.
The corresponding real-product INI test passes in each integration aggregate.

| Path relative to assets | SHA-256 |
| --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 6CF2F130746F914E36F99EA03F51E0F33342427250734B84AF2EAAE92373B28C |
| my5160/my5160_0_5_0560_x86.exe | F75BB83C5639E0BA50274415880913634430BF3D178CE896D066DE40339A3801 |
| my5170/my5170_0_5_0560_x64.exe | D9C70484FBA287C2AAE16F697D0322DC7DEA3E984F6DF406CDACA88C42796D5C |
| my5170/my5170_0_5_0560_x86.exe | F6D9A9A33719C18B5016778EDC8D60B2CD057E1A622AA5F228BB3A9B9C746361 |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | FCE0E0E8DBEE175DF43CD6F7D07CA7EB44B3A39D8EE6DDD61798D1E0FEC2118A |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | CB2C86525A42C627D9D1328B7079D88BE87D226A85C1C41CF64F084A4F985665 |
| nxvm/nxvm_0_5_0560_x64.exe | CC47FB6D155AA5242ECFBE8BEEBDF09E5CF3F727E32810EFC19F38B2B8D51EF9 |
| nxvm/nxvm_0_5_0560_x86.exe | F59C988CA43B443EC11590AA23140D69CE70AD305D0534FF7199E70CE5C46F98 |

## Actual-Change Review And Delivery Boundary

The executor reviewed all four CPU implementation/header diffs, the version
replacement, all three test diffs and the manifests; the retained control and
descriptor callers were searched again for bypasses. Reset, MOV, LMSW/CLTS,
task CR3 and debugger edits have explicit distinct publication semantics.
Debugger mutation remains an intentional paused-state edit; real translation
invalidation effects remain with S4, not a second guest exception route.

The three root preset replacements add/remove 3 lines and do not change a
compiler, launcher, cache directory, configure preset or MyNES target.
cmake --list-presets=build parses them successfully. Their target matches
the already-built NXVM vm-0-5-0560 target. Runtime tests are not repeated for
this target-name-only change. Both Core manifests, documentation governance
and git diff --check pass. All eight artifact hashes were rechecked against
this record after deployment. Shared source/test trees and MyNES are unchanged.

The C02--04/C08 base subcase dispositions are recorded in the convergence
ledger. Cache/TLB/control side effects, alignment/RMW, selected feature/SMM,
inherited complete instruction semantics, timing and production admission
remain within T560. No applicable CPU obligation transfers out of this T.
