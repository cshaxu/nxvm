# T2 S51 Test Convergence And Final Qualification

## Admission And Before-Code Disposition

Clean baseline d033af600. The coordinator admits the original final S51 under
automatic S authority, incorporating the owner's test-growth review before
final qualification. No final CPU obligation is removed or shifted to another
S. Current owns the complete16-field packet. No production/API/model change,
new test file/target or Lib/Emulator/MyNES edit is planned.

The last read-only inventory counts physical C/H lines, including comments
and blank lines: X86 Core source39460, owned tests84328. Current V30-named C/H
is32 files/11625 lines. Since T2 baseline7f7d23938, source C/H is+2198/-1143
and tests+11905/-8. Ratios alone do not prove redundant coverage; actual
responsibility/input/assertion comparison controls removal.

| Class | Actual duplication and retained owner | Before-code disposition |
| --- | --- | --- |
| Q1 protected setup | enter_leave, gpr_push_pop, pusha_popa and push_immediate have identical normalized protected bootstrap bodies, including GDT bytes, ten real guest instructions and final CS/IP check. | Move one unchanged mechanism into existing cpu_instruction_fixture.h; replace all four local definitions/callers. Preserve every instruction-specific scenario, expected state and failure assertion. |
| Q2 segment comparison | Those four plus imul_immediate, legacy_lock, legacy_sreg_stack, prefix_attributes and pushf_popf repeat the exact six segment byte comparisons. | One CPU fixture comparison for ES/CS/SS/DS/FS/GS. Preserve byte comparison semantics, all call sites and distinct non-target segment/register predicates. |
| Q3 Machine registry | S50 execution_boundaries repeats null/duplicate/frozen attachment and duplicate/frozen port checks already owned by machine_attachment_phases_smoke and neutral_link. | Remove only repeated rejection assertions and unnecessary port setup from that V30 scenario. Preserve real construction/bind/freeze, one-tick pending instruction, refused debugger mutation, reset/stop, clean continuation and finalize-once. |
| Q4 transfer matrices | transfer proves exact CPU/data/accepted-prefix failure effects; nominal_transfer independently covers a broader ModR/M/parity/prefix timing domain. | Retain both now: shared opcode names do not prove identical input domains or interchangeable assertions. Do not delete378 timing lines without a complete union mapping. |
| Q5 chip versus receiver | CPU tests prove instruction/mode/bus details; genuine Machine tests prove copied facts, configured time, attachment routing and public lifecycle boundaries. | Retain distinct owner proof. A CPU fixture cannot substitute for real Machine construction, and Machine does not repeat the full instruction matrix. |

Q1/Q2 only add two small static helpers to an existing CPU-owned fixture;
Machine never includes them. No options/configuration object/second runner or
mirrored state is introduced. Estimated tests+60--110/-240--350 with a net
reduction near180--250; exact tracked diff and unchanged cases decide the
result, not an arbitrary line or executable count.

Other normalized-body families (register lookup/seeds, register-subset compare,
lightweight protected-state setup, Machine patch/capture helpers) are audit
candidates, not automatically safe equivalents. They are not deleted by a
whitespace/name heuristic. Semantic distinctions and complete task reconciliation
will be recorded before S51 acceptance. No claim of a complete84k-line manual
test audit follows from this bounded normalization scan.

## Implemented Convergence Checkpoint

Test C/H actual delta is+107/-320/net-213 across eleven existing paths:
nine CPU callers, their existing CPU fixture and the V30 Machine scenario.
No CMake registration or production path changes. There is no new test file,
target, framework, public API or runtime state. The test manifest replaces
only the eleven changed owned hashes. New predicates use Lib bool vocabulary.

An independent readback from committed d033af600 compares every old helper
body with its new owner after whitespace normalization and the equivalent
return0/LIB_FALSE spelling. All thirteen removed definitions match their two
owners. Every one of the56 old calls is retained:48 segment comparisons and
eight real protected bootstraps. Removing only those old definitions and
renaming those calls reproduces all nine resulting source files exactly.
Thus their scenario bodies, case tables/loops, independent expected values,
failed-transfer assertions and main dispatch remain unchanged. This is static
preservation evidence; actual full execution is still required separately.

Machine removes a net eight repeated rejection/route-setup lines, while its actual
pending V30 INC, debugger mutation refusal, reset/requested-stop, reset state,
fresh INC and finalize-once remain unchanged. The attachment and port registry
implementations have no profile branch; dedicated existing owner tests retain
their rejection coverage. Real V30 construction/bind/freeze is still executed.
The initial+60--110/-240--350 estimate contains the actual result.

The retained legacy_sreg_stack protected bootstrap is not Q1 duplication: its
GDT contains extra writable/code/nonpresent descriptors used by its own limit/
access tests. The existing protected-exception fixture also retains its actual
GP/VM86 handler and ring-stack responsibility. Neither is replaced by the
four-entry stack bootstrap merely because all enter protected mode.

Prevention is review at the existing owner: establish the capability/input/
assertion delta before adding a file, extend an existing case where possible,
and keep source semantics versus receiver wiring separate. No new permanent
duplicate checker, test-on-test framework or mandatory count target is added.

## Original CPU Requirement Reconciliation

The original seventeen rows are not replaced with the three cleanup classes.
Their initial/past-stage statuses remain historical evidence. This current
map consumes accepted S47 semantic R01--R14, S48 nominal N01--N10, S49 elapsed
E/F/G/H/J/K and S50 real Machine L01--L12. Runtime final qualification below
must pass before acceptance. File stems below identify existing registered
X86 Core tests; they are proof owners, not a new focused-test manifest.

| Original row | Current sole implementation and direct existing proof | Disposition and boundary |
| --- | --- | --- |
| V01 family/admission | CPU profile/reset/format owner and real Machine config validation; cpu_v30_native, cpu_v30_form_admission, machine_cpu_profile_gate and neutral_link. | V30 enum7 preserves0--6; no FPU/deterministic constructor is real. Invalid/FPU/verified-physical configurations remain rejected, not masqueraded as Intel. |
| V02 reset | CPU reset/entry owner; native, mode_transition, emulation and machine_v30_execution plus retained non_pc_board. | Native MD, FFFF:0000, segments, dirty mode/write-enable abort and actual Machine reset/stop have proof; unspecified hardware register contents are not invented as silicon facts. |
| V03 primary map | Existing profile-aware metadata/scanner and original table executor; form_admission plus native, transfer, segment, stack, arithmetic and control family units. | All256 primary slots and legal constrained forms have source/explicit-model or diagnostic undefined disposition. Bounded lexeme storage is not a fabricated instruction-length maximum or second decoder. |
| V04 NEC suffix map | Same NEC metadata/handler owner; bit_nibble, bit_field, packed_bcd, mode_transition and escape. |26 documented suffixes,230 undefined suffixes and legal register/memory forms are reconciled. Undefined forms retain diagnostic refusal rather than native fallback. |
| V05 prefixes/restart | Existing prefix/repeat/lock owner; conditional_repeat, block_transfer, port_transfer, bus_lock and elapsed. | Effective/order/ignored/redundant/long/wrapped prefixes, entry restart and lock cancellation are covered. Prefix combination cost retains approved L2, not electrical pipeline accuracy. |
| V06 transfer/stack | Original shared instruction handlers; transfer, nominal_transfer, memory_transfer, segment, stack and bus_lock. | Register/memory/width/alias/segment/wrap/lane/frame and accepted-refusal proof is retained. Distinct complete timing and data-effect input domains are not dropped to shrink tests. |
| V07 arithmetic/decimal/shift | Original ALU/decimal/shift handlers; alu, decimal, multiply_divide, shift_rotate and packed_bcd. | Independent full defined results/flags, non-ten conversion literal rejection, defined counts and success/error paths remain. Source-unknown nibble/alias/dynamic details keep reviewed explicit models/undefined masks. |
| V08 control flow | Sole control/entry/IRET path; control_flow, software_entry, software_arbitration and interrupt. | Actual targets/frames/returns/branch outcomes, legal memory forms, alias/wrap and accepted failures have proof; no target-PC shortcut or scripted execution is introduced. |
| V09 strings | Sole repeat executor/phase owner; block_transfer, conditional_repeat and port_transfer. | Count0/first/continuation/terminal, carry/zero predicates, DF, lanes, ports, failure prefixes and actual entry/return restart retain complete independent expectations. |
| V10 PSW/MD | CPU register/flag/entry owner; flags, mode_transition, software_entry, emulation and copied Machine observations. | Native fixed/writable bits, loaded BRK, MD restoration/lock and all PSW-word contexts remain, without an App mode mirror. |
| V11 priority/entry | Existing successful entry and pending-source owner; interrupt, software_entry, software_arbitration, segment and emulation. | NMI/INT/BRK/divide/step, shadows, callback arrivals, native/emulation nesting/return and each accepted failure retain proof. Signed CHKIND and store-BRK distinctions remain explicit approved L2. |
| V12 ordinary8080 | MD selects the existing CPU executor; emulation's independent operation table/semantic oracle and actual mixed-mode program. |244 ordinary slots plus constrained ED escape, reserved slots/suffixes, mappings/ALU/PSW/stack/memory/ports/conditions/refusals are covered; nominal AF compatibility clocks are explicit L2, not V30 silicon timing. |
| V13 mode transitions | Sole CPU BRKEM/CALLN/RETEM/native IRET and reset owners; mode_transition, emulation and actual machine_v30_execution. | Real guest mode entry, native handler return, reset abort and forbidden nesting are covered through both owner and receiver boundaries; no debug-only latch mutation substitutes for Machine proof. |
| V14 bus/wait/lock | CPU bus/fetch/segment/lock and existing attached Core timeline; memory_transfer, port_transfer, bus_lock, elapsed, native and machine_v30_time/execution. |20-bit wrap, split/odd lanes, real versus observational access, failed accepted prefix, POLL/HLT cadence, lock/reset/code replacement and actual attachment wake are qualified at their declared models. Electrical waveforms/background overlap are not claimed. |
| V15 time/bounds | Sole V30 nominal selector plus CPU accepted fetch/entry publisher and Core extra-wait/pending owner; existing family timing seams, elapsed and machine_v30_time. | Source nominal plus approved serialized-fetch/ordinary8080/dynamic L2 is complete; no duplicate operand baseline/wait charge. Successful/failed entry and idle remain distinct. Maximum0 honestly reports no finite universal prefix/wait bound; bounded runtime still completes actual work. |
| V16 genuine Machine | Real neutral factory, one Core time/route/attachment owner; machine_v30_time, machine_v30_execution and parameterized non_pc_board. | Actual configured V30, public paused debug, native/emulation/IRQ/NMI/vector30/deadline/failure/lifetime proof is complete in S50 and rerun here. Verified-physical time remains intentionally ineligible. Real game ROM/COP checkpoints retain their explicitly owner-approved board receiver. |
| V17 Intel/consumers | Existing Intel/FPU/Machine units and four actual fixed-profile PC products. | Full repository unit qualification and all registered PC receiving integrations per width are required below, with current PE/deployment/media guards. MyNES has no X86 Core dependency; its releases are preserved, not refreshed for a CPU-test edit. |

The A--L accounting inventory is also fully consumed, not hidden inside V15:
A/B/C/D/I nominal/form obligations use the same source/explicit-L2 selector
and family oracles qualified by S46--S48; E fetch and F extra waits use the
separate actual CPU/Core owners proved in S49; G never adds operand baseline
twice; H distinguishes combined completed entry from asynchronous or failed
accepted event work; J separates initial POLL/HALT from later idle/samples;
K retains pure bounded lexical preview and honest max0; L is real configured
Machine proof, not a profile flag or partial fixture.

Accepted model limits remain explicit: signed CHKIND, source-order undefined
aliases/decimal details, native contiguous data tails/reserved-port provider,
store-BRK suppression, dynamic upper-endpoint clocks, C6/C7 register/extra REP
costs, AF-compatible8080 clocks and serialized fetch/overlap. Source nominal
rows are not silently upgraded into electrical accuracy. No newly invented
model or timing downgrade is introduced by this S. V30 physical time rejects
admission rather than hiding these limits. Original board/COP/video/sound and
playable M1 completion remain separate required receivers after T2.

## Final Executed Qualification

Both original stable Release Ninja/ccache graphs build and pass685/685 complete
repository units, with zero failures/skips: x64 in140.22 seconds, x86 in150.58.
Both sorted-name hashes remain S50's
7E752320B3700CDA734E53870104614C831CE282B84335590A00ED993C085662.
No final covered source/test input changes during either suite. There is no
new unit registration, assertion-budget relaxation or count-based success.

The final receiving run builds each actual PC profile and executes every one
of its currently registered integration.* rows, not the earlier14-case smoke
subset: My5160 three, My5170 five, DeskPro five, default NXVM23, for36 per width
and72 total. All pass with zero skipped cases. Both graph configurations finish
at default. DeskPro suites take132.54 seconds x64 and177.89 x86 within unchanged
budgets. Root Lib native-device/MyNES integration labels are not PC receiving
requirements; listing them is not execution or failure evidence. Their aggregate
ownership mismatch is explicitly recorded in TODO, not fixed in this CPU S.

All eight built/deployed PC EXE hashes match and PE widths/stripped sections
are checked. Final no-change product builds both report no work to do. Repeated
profile builds compile zero CPU objects. Relinking changes only PE timestamp/
checksum bytes: independent normalization of those eight header bytes makes
each entire binary identical to its committed baseline. New receiving hashes
are delivered under the owner's standing timestamp-publication approval at
unchanged revision0564; no new runtime logic or Built-on refresh is claimed.
The MyNES0044 pair and all33 protected tracked assets remain byte-identical.
Six actual INI-selected media roles retain their full hashes and overlay modes.

Both builds retain498 strict rows,44 CPU compile-command checks,536 unique
declarations,150 deferred ownership rows and1402 Types inputs. Production/test
manifests, inward dependency, unchanged Machine chip boundary and documentation/
whitespace checks pass. Existing legacy warning waivers remain explicit; this
is not an all-source waiver-free or all-platform claim.

The [results](t2-s51-results.json) retain exact unit/integration name and JUnit
hashes, artifact metadata-only comparison, helper preservation proof and input
guards. Original17-row and A--L proof is reconciled above. The clock-budget
proof is actual guest-time publication/pending completion, not a claim of
universal wall-clock preemption within an unterminated instruction; max0 does
not acquire a fabricated finite decode or physical bound.

No runnable MyRaidenII, physical V30/electrical waveform, manual desktop/RDP or
complete game is qualified. The named Windows31 checkpoint actually proves
DOS C-drive presence/directory access, not Windows GUI execution. The original
real-set board/COP/video/sound and host/gameplay receivers remain required.
No CPU capability is silently transferred into those game tasks.

## Tool Attempts And Retained Limits

No final unit or PC integration execution fails. Two initial read-only guard
commands have harness errors: one incorrectly renames fixed_disk0 to harddisk0
when matching an existing receipt, and a diagnostic foreach pipeline is invalid
PowerShell syntax. Reading the actual stored role and using valid syntax fixes
the collector; every real medium hash/mode matches before and after. No input
is modified and no production defect is inferred.

The first byte-comparison collector outlives its observation interval and its
handle is not retained by the orchestrating expression. Native process inspection
then confirms the specific collector is no longer running before any replacement.
A new bounded readback zeros only PE timestamp/checksum bytes in in-memory
copies, hashes the complete remaining binary and retains its generated receipt.
This is missing output recovery, not restarting a live job or rerunning tests.

Remaining test maintenance candidates are separately bounded in TODO; no
whole84k-line manual audit or zero-redundancy claim follows from the thirteen
helper extractions. Independent oracles, source/receiver boundaries and distinct
input matrices deliberately remain. The original full gameplay goal stays open.

## Delivery And Temporary Output

Shared P1 `2255ca3f6` contains only the owned test consolidation and manifest.
NXVM P2 `3d47ddec7` contains the eight verified receiving EXEs; their differences
are confined to PE timestamp/checksum metadata, not executable payload changes.
The result receipt retains hashes for all42 generated proof files before cleanup.
After every owned process exited, the formal cleanup tool previewed and removed
only build/myraidenii-t2-s51:42 files,758673 bytes. Both persistent caches remain.
No unexecuted desktop, RDP, physical-device or MyNES integration is called passed.

## Archived S51 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S51, next numeric S after accepted S50 at d033af600. One session switches coordinator/executor roles; no delegation. |
| Admission And Approval | Owner's full MyRaidenII goal, standing automatic numeric S admission, approved original S51 final qualification and2026-10-19 instruction to reuse/refine existing tests rather than continually grow them. Shared owns only X86 Core tests/manifest; MyRaidenII owns records. NXVM receiving artifacts change only if executable inputs require it. No Lib/Emulator/MyNES/peer production change is authorized. |
| Objective | Reconcile the original V01--V17/A--L CPU scope to current source and proof, first consolidating confirmed redundant test setup/registration checks without losing distinct coverage, then complete final full-unit/gate and receiving qualification. |
| Non-goals | No new CPU/model/API, board/COP/game implementation, production refactor, test framework, arbitrary case-count/line target or deletion of semantically distinct tests. Do not turn a few shared helper removals into an audit claim over every line of every component. |
| Reference Baseline | Clean pushed d033af600; S47--S50 actual implementation/proof, current CPU and Machine source, original task convergence ledger and the latest read-only test inventory. Existing test names/registrations and ten artifact identities form preservation guards. |
| Candidate Proposal | ../proposals/m1-v30.md#proposed-s51-complete-t2-qualification-and-receiving-publication; ../etc/t2-v30-ledger.md and S46 accounting A--L; evidence ../etc/evidence/t2-s51-final-qualification.md. |
| Files And ABI Surface | Existing test/x86-core/cpu/support/cpu_instruction_fixture.h and nine existing CPU callers; test/x86-core/machine/machine_v30_execution_smoke.c and test manifest. MyRaidenII Current/proposal/history/evidence/ledger as required. Expected tests+60--110/-240--350, net reduction around180--250; production0 and no ABI change. No new test file or target. |
| Applicable Rules | Execution complete ledger, before-code mapping, per-target complete pushed P and actual-change role review; Architecture inward CPU fixture ownership and sole runtime route; Coding stable owner-local reuse without framework/packing; Document current/history boundaries. Architecture/coding governance skills apply. Source policy admits only existing project-owned synthetic inputs. |
| Verification | Original stable x64/x86 Ninja/ccache graphs, -j8 builds, complete repository unit suites with four test workers; source/test manifests, Types, strict/dependency/chip and docs gates. Compare all test names/counts and retained case loops, normalized helper bodies/call sites, Machine generic-owner assertions. Final original receiving configuration/boot/lifecycle integrations on both widths, current eight PC PE/version/hash/deployment identity and unchanged MyNES pair; preserve user assets/media. |
| Expected Markers | Existing markers and all685 unit names remain unchanged unless evidence first justifies a different count. Original V30 public Machine and chip proofs still pass; no removed unique assertion or downgraded source/model claim. All required integrations pass without relaxing assertions/budgets. |
| Asset Needs | Existing synthetic unit bytes and previously qualified read-only/overlay PC integration inputs; no game acquisition or ROM change. Temporary build/myraidenii-t2-s51 owned by executor, two-hour phase/five-minute no-progress/128MiB budgets, proof/hash retention then preview/apply Clear-Build after actual process exit. Preserve both persistent caches. |
| Reporting Requirements | Report audit corrections, exact removed-to-retained mapping and estimated deltas before implementation; then actual code size, full qualification and boundaries. Preserve failed attempts and unresolved requirements honestly. No partial P before assigned scope/proof and records are complete. |
| Stop Conditions | Stop for public/model/production expansion, protected input change, lost unique coverage, unsupported source claim, timing downgrade or excluded target change. Do not repair a new unrelated production finding under the guise of test cleanup; inventory its whole class and seek correct admission. |
| Exit Criteria | Confirmed duplicate setup/comparisons have one CPU fixture owner, generic registration duplication has dedicated retained proof, all unique semantic/bus/time/real-route tests remain, original V01--V17/A--L complete dispositions have adequate direct evidence, full units/gates and required receiving proof pass both widths, artifacts/inputs current and preserved, actual diff reviewed, complete scoped commits pushed and owned temporary output safely cleared. T2 closure still requires its own original-scope audit; gameplay remains later. |
| Original Owner Request | Implement MyRaidenII fully under single-session dual-role governance, and stop to audit test inflation: reuse or improve existing coverage instead of repeatedly adding hundreds of redundant test lines. |
| Similar-Issue Sweep | Normalize static helper bodies across test/x86-core to find actual shared setup/comparison responsibility, inspect all matching callers and distinct variants; map Machine duplicate assertions to existing owner tests. Record other duplicate families and retained independent timing/mode/bus boundaries with reasons, not a blanket whole-repo no-duplication claim. No new generic duplicate-detector framework. |
