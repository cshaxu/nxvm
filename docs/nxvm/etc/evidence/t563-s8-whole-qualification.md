# T563 S8 Whole 486SX Qualification

## Frozen Scope And Completion Predicate

Admit from clean S7 acceptance cd523e0a0. The original
[C01--C17 ledger](t560-s1-cpu-convergence.md),
[T558 receivers](t558-s2-receiver-checklists.md),
[T559 instruction inventory](t559-s1-instruction-inventory.md),
[post-refactor R1--R10](t563-s1-rebase.md) and current T563 proposal define the
universe. The S7 constructor/boot success does not replace that universe.

Initially this batch changes no production/test/build code, API, state or
manifest. Estimated documentation is +220--380/-45--80 lines; profile-induced
relinks may update the eight current 0563 PC EXEs. Existing source/model limits
are retained, not downgraded or silently removed. A confirmed incomplete class
requires a bounded owner/file/diff decision before repair, not an opportunistic
first-failure patch or a transfer that pretends the CPU is complete.

Acceptance requires each original CPU row, inherited instruction family and
receiver to have current source ownership, applicable primary/L2 authority and
direct proof. Reusing an unchanged mechanism is allowed only after checking
its current callers and actual tests. Original undefined behavior, explicit
accepted models and board-specific non-goals must remain distinct from missing
implementation. Exact fitted PC110 package/stepping and board/firmware work
are not inferred from the selected embedded SX model.

## Initial Whole-Suite Inventory

The current default x64 CTest JSON contains 667 registrations: 632 unit and
35 integration, zero disabled or unclassified tests. Five are desktop-labelled:
three unit and two integration. `setup` support is compiled fixture machinery,
not an unregistered runtime suite; diagnostic-only programs are not relabelled
as passing integration. Capture the other width and each real profile before
whole qualification; current registration, not an old expected count, owns
that inventory.

Use the two original Ninja/ccache graphs. Build the complete unit aggregate
and specialized gates, then run complete units once per width. For integration,
run all registered Default cases, including MyNES once; XT/AT/DeskPro run all
their registered non-MyNES integrations, avoiding repeated execution of the
independent MyNES App. Serialize native qualification across architectures and
profiles. Capture inventory/name parity and reject disabled/skipped/missing
cases. Restore Default, verify final eight deployed CLI/INI identities,
Release PE widths/debug sections and hashes, and preserve owner input bytes.

This admission inventory and plan are not final qualification or acceptance.
The complete C01--C17/current-source audit and runtime results follow below.

## Original Requirement To Current Proof Audit

The table records the current-source/direct-proof reconciliation, not a claim
that a test name or green manifest proves a CPU feature. CPU paths below are
under `src/x86-core/cpu`; CPU tests are under `test/x86-core/cpu`. Their real
instruction fixtures execute the selected SX and check actual effects,
exception frames or physical callbacks. Core/Board tests retain their own
owners. Final frozen suite execution remains required after this audit.

| Original row | Current implementation and direct proof | Disposition and retained boundary |
| --- | --- | --- |
| C01 identity/features | `cpu.c` reset/profile and instruction CPUID/flags owners; `cpu_cpuid_flags_smoke.c`, `cpu_vme_smoke.c`, real Core profile gate | Candidate signature 042B, leaves/ID, VME and absent FPU are implemented and directly exercised. Selected embedded SX is source-defined; exact fitted PC110 package is not claimed. |
| C02 reset/fetch/mode | `x86_core_cpu_state_apply_reset`, reset-code-base and `_kma_is_reset_vector_fetch`; CPUID/reset-fetch test plus real Core/Board alias execution | Dirty-state reset, CR0 60000010, DR6 FFFF0FF0, hidden CS/high fetch and low far-transfer transition have direct proof. Cache/TLB/SMM/reset lifetime is also covered by those state-owner tests. Undefined reset fields are not asserted as silicon constants. |
| C03 control registers | `_s_write_cr0`, `_s_write_cr3`, selected CR4 validator, task/reset/Debug writers; control-state, task and VME tests | Guest illegal combinations, fixed ET, reserved normalization, CR3 PWT/PCD and invalidations are implemented. Paused raw Debug edits have their existing distinct publication contract, not guest MOV exceptions; its broader policy hardening remains the pre-existing cross-family TODO, not a missing guest opcode. |
| C04 paging/access | `_kma_prepare_physical_linear` and single commit/update owner; control-state permission matrix, Cache second-read/update/redirect tests and TLB dirty transition | Actual WP/CPL/PDE/PTE, A/D, cache controls, split access and accepted failure prefixes have proof. C16 adds actual physical-role/timing allocation; a TLB hit rechecks current WP. |
| C05 alignment/order | `_kma_check_alignment` and logical-access preflight; alignment-access and execution-fault-event tests | Scalar/stack/string/pair, CPL/AM/AC, supervisor-table exemption and real AC frames are covered. Original 486 Table 9-2 orders operand segment, AC, then operand page faults; do not import later-family precedence. |
| C06 TLB/test commands | Existing translation TLB, `_kma_tlb_test` and INVLPG/control/reset owners; `cpu_tlb_smoke.c`, control/task and Core inspection tests | All sets/ways, attributes, lookup/write, invalidation, first-dirty transition, replacement and passive preview have real proof. Internal replacement correspondence and dirty rewalk retain their explicit L2 grades. |
| C07 cache/bus commands | `_kma_cache_test`, physical cache/read/write/flush owners and CPU bus unit; `cpu_cache_smoke.c`, bus and Core inspection tests | Ordinary fetch/data and TR3--5 share 8-KiB cache state. INVD/WBINVD, 128x4 geometry, CD/NW, PCD/PWT, failed fills, snoop and external maintenance are implemented, not no-ops. SX write-through and explicit L2 temporal buffering remain distinct. |
| C08 descriptor effects | `_ksa_write_xdt` and single validated descriptor-read owner; control-state, descriptor, task and Cache role tests | Already-accessed user descriptors avoid SX writeback; clear bits and system/TSS busy effects keep required writes. GDT/LDT/IDT paging, readonly WP and bus/fault outcomes have direct proof. |
| C09 inherited RMW | Existing handlers preflight at the shared operand owner; `cpu_rmw_access_smoke.c`, XADD/CMPXCHG and LOCK tests | All admitted modifying groups, widths, zero counts, page/segment/provider failures and accepted write prefixes are covered. Arbitrary provider side effects cannot be rolled back or replayed as a transaction fiction. |
| C10 VME/VM86 | One CR4/flags/TSS bitmap/entry owner; `cpu_vme_smoke.c` plus task and IRET tests | Actual CLI/STI, virtual flags, INT redirection, protected/hardware routes, map/page/provider failures, IRET and pending/shadow contexts are implemented. No PVI/PSE/APIC is advertised or inferred. |
| C11 SMM/RSM | `_smm_enter`, image/restore and existing physical/SMM binding; `cpu_smm_smoke.c` and real Core executor SMM/failure cases | Save/restore, guest edits, relocation, nonrecursive priority, I/O/HALT restart, accepted prefixes, failed RSM and SMIACT lifetime have direct proof. Public image fields are source-backed; hidden image and timing envelopes retain their accepted L2 limits. |
| C12 signals/power | CPU system/bus owners plus sole Core scheduler; SMM, execution-fault-event, executor, transaction and system-power tests | IRQ/NMI/SMI, HLT/HOLD/READY, shutdown, reset distinction, qualified clock/Stop Grant and FLUSH are connected. CPU consumes qualified levels; raw board electrical/PLL qualification is not manufactured by host sleep. |
| C13 Debug/exceptions | Existing DR/DB/fault/shadow/entry owners; `cpu_debug_state_smoke.c`, exception/IRQ/task, `cpu_cache_smoke.c` and public Core `machine_debug_smoke.c` | GD/aliases, DR6/7, RF/TF, local task clearing, fetch/privilege priority, nested entry and restart have selected-family proof. S8 B1--B4 closes the subsequently confirmed register-candidate/cache gap, including prepared entry. Copied diagnosis is passive; pending execution cannot be mutated through Debug. No raw CPU API was added. |
| C14 no-FPU | ESC/WAIT CPU predicates, FPU binding and real Core constructor; `cpu_fpu_interface_state_smoke.c`, executor and neutral-link tests | All ESC classes trap NM on selected SX, WAIT has its separate MP/TS behavior, prefixes/fetch/LOCK and real frame delivery are tested. No integrated/external x87 is fabricated; SX/FPU mismatch is rejected by Core. Broader coprocessor arithmetic remains a distinct product capability. |
| C15 all forms | Both dispatcher spaces, metadata/scanner and original table handlers; all twenty family owners below | The original full legal/reserved/prefix/group universe is retained, not a BIOS subset. Independent positive semantics compose with contextual faults and 170 unadmitted escaped-slot/nineteen invalid-ModRM classes. Undefined/historical forms retain explicit source/model dispositions. |
| C16 timing | Sole SX selector calls nominal/L2, memory, decode and CPU bus aggregation; CPU timing/cache/bus/entry/SMM tests and Core trace/budget/observation tests | No compatibility fallback qualifies SX. Original source-form and aggregate-model origins remain distinct; actual waits/event prefixes are charged once. Zero static maximum means bounded pending completion, not a one-tick bound. Stable completed observations preserve branch-target lexical facts across entry. |
| C17 production | Core config validation/constructor, sole Board reset-alias helper, AT capability list and generic adapter; accepted S7 A1--A7 | Actual SX construction/reset/run/preview/Debug/system/DMA routes are tested without staged rebinding. Invalid CPU/FPU/386-quirk inputs fail atomically. The existing four Apps retain their fixed CPUs; PC110 is not implied. |

### Twenty C15 Families And Current Test Owners

The accepted [T560 S8 F01--F20 disposition](t560-s8-inherited-semantics.md#c15-final-functional-disposition-candidate)
retains source and undefined-output rationale. These current owner files still
execute their selected-family forms; context and shared helper proof is reused
only where semantics and implementation ownership remain the same.

| Family | Current CPU test witnesses (filename stems) |
| --- | --- |
| F01 binary/Group 1 | `cpu_legacy_alu`, `cpu_rmw_access`, `cpu_486_nominal_timing` |
| F02 decimal/ASCII/XLAT | `cpu_legacy_alu`, `cpu_address_span`, `cpu_execution_fault_event` |
| F03 unary/multiply/divide | `cpu_integer_multiply_divide`, `cpu_imul_immediate`, `cpu_imul2`, RMW/fault tests |
| F04 INC/DEC | Three `cpu_inc_dec_*_group` tests, RMW and reserved-ModRM tests |
| F05 rotate/shift | `cpu_rotate`, `cpu_rmw_access`, actual nominal/model count matrices |
| F06 double shift | `cpu_double_shift`, RMW, decode and timing tests |
| F07 bit/scan | `cpu_bit_test`, `cpu_bit_scan`, RMW and operand-span tests |
| F08 MOV/extend/LEA/XCHG | `cpu_gpr_mov`, `cpu_moffs`, `cpu_movx`, `cpu_lea`, `cpu_xchg`, segment and span tests |
| F09 stack | `cpu_gpr_push_pop`, `cpu_push_immediate`, `cpu_pusha_popa`, `cpu_enter_leave`, segment/FS/GS stack and fault tests |
| F10 flags/sign/NOP | `cpu_sign_extend`, `cpu_lahf_sahf`, `cpu_direct_flags`, `cpu_eflags_local`, decode and nominal tests |
| F11 near/conditional | `cpu_control_transfer_near`, `cpu_control_transfer_branch`, `cpu_setcc`, transfer-boundary tests |
| F12 far/task/returns | `cpu_control_transfer_far`, `cpu_protected_far`, `cpu_outer_return`, task16/task32/cross-width tests |
| F13 memory strings | `cpu_movs`, `cpu_cmps`, `cpu_scas`, `cpu_lods`, `cpu_stos`, guest fault/IRET restart and REP timing tests |
| F14 ports | `cpu_port_io`, `cpu_port_strings`, RMW/permission/provider and Core transaction tests |
| F15 queries/data loads | `cpu_bound`, `cpu_arpl`, `cpu_lar_lsl`, `cpu_verr_verw`, `cpu_les_lds`, `cpu_lss_lfs_lgs` |
| F16 descriptor/task tables | `cpu_descriptor_system`, `cpu_descriptor_table_register`, `cpu_lgdt_lidt`, `cpu_sgdt_sidt`, task tests |
| F17 flags/interrupt/HLT | `cpu_pushf_popf`, `cpu_iret_state`, `cpu_software_int_state`, protected/VM86/VME entry and system tests |
| F18 control/debug/test/cache/SMM | Control, debug, test-register, TLB, Cache, CPUID and SMM owner tests |
| F19 486 integer delta | `cpu_bswap`, `cpu_xadd`, `cpu_cmpxchg`, LOCK and nominal-model tests |
| F20 syntax/reserved/absent FPU | Decode, prefix/operand-address, fault-event reserved ranges, FPU and nominal allocation tests |

### Inherited Receiver Reconciliation

T558 CPU-1 and T559 I1--I3 map to C15/F19. CPU-2--3 and I4--I6 map to
the now-real C06/C07 state owners, not the original unavailable-state stubs.
CPU-4--6 and I11 map to C03--C05/C08/C09. CPU-7 and I7/I8/I11 map to
C01/C10--C14; CPU-8 maps to C16; CPU-9 and I10/I11 map to C02/C11/C12/C17.
Inherited I9 LOCK and the RMW debt map to C09 and F01/F04--F07/F19/F20.
No original instruction/system handoff disappears because files moved.

All post-refactor R1--R10 have their actual receiver: R2 nominal/model forms
in S2; R3/R4 memory/translation in S3; R5/R6 bus/decode/REP in S4;
R7 event/system in S5; R1/R8 selected/preview/budget in S6; R9/R10 actual
construction/reset in S7. Inspecting the current selector confirms SX cannot
fall through to the old compatibility function. That function remains a
distinct earlier-family unallocated behavior, not a second selected-SX path.

## Source And Model Limits Revalidated

Rehash the five external Intel originals used by the accepted feature/semantic
and timing work. All match their pinned records; none is downloaded, modified,
copied into the repository or reclassified as a new capability:

| External original basename | SHA-256 |
| --- | --- |
| `intel-486-family-programmers-reference-1992.pdf` | D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143 |
| `intel-486-embedded-developers-273021-001-1997.pdf` | BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3 |
| `intel-486sx-272769-004-2004.pdf` | A35A0A428168ED45428DD886876238424A228B8DB0583EE74A1815A88BF421CD |
| `intel-ia32-system-programming-253669-087-2025.pdf` | C34B30715B46AE549B4936D0DFA171E5B149D799E307E0DF2990A4FA66CE288A |
| `intel-ia32-instruction-reference-325383.pdf` | F117CB21CE1F3B6258F5CE5F786E91665A133BF55E7BC2DC8CAE843B56E4FB8E |

The 1992 PRM and embedded developer/SX sources retain the selected hardware
eligibility and original nominal/access coefficients. Later public Intel
sources are used only for the already recorded VME algorithm and retrospective
details, not for newer CPU capabilities or silently different fault priority.
The exact fitted PC110 package remains the separate machine receiver's source
gate, not evidence that ordinary selected SX VME/SMM is unimplemented.

| Retained model or undefined boundary | Qualification meaning |
| --- | --- |
| IMUL and variable RCL/RCR | Original range/formula conflict stays explicit L2; no exact-silicon label. |
| SALC, UMOV, INT1 and undefined/alias forms | Existing selected historical/reference models and reserved negative forms stay distinct; no unsupported form is made a successful NOP. |
| Successful WAIT | Three-clock deterministic L2 endpoint of the original 1--3 range; MP/TS exception is a separate real path. |
| TLB internals and dirty transition | One actual TLB with source-backed commands; private replacement correspondence/rewalk choice is L2. |
| Partial-hit, cached/mixed/raced table and operand costs | Original cold coefficients plus actual physical-role observations; declared differential models remain L2. |
| Decode/redundant REP/AGI and unavailable next target | Existing explicit component/identity model; no second decoder or fabricated successful fetch. |
| Write FIFO, overlap and contention | Source four-slot/barrier facts plus L2 interval-level timing; functional writes remain atomic accepted transfers, not a second deferred-payload queue. |
| SMM hidden image/system timing | Source public fields with explicit L2 hidden-image and approximate/minimum timing envelope; no later CPU save map or physical PLL synthesis. |
| Undefined outputs and raw Debug edits | Undefined flags/register values are not exact assertions. Existing paused raw CR0/CR3 overrides are not guest MOV; the separate cross-family Debug policy TODO is retained, with shared invalidation ownership already proven. |

Audit the complete current TODO ledger rather than assuming it is empty.
Correct only its stale VME wording: selected SX VME is implemented; PVI or a
different variant remains separately source-gated. Existing descriptor naming,
raw Debug policy, broader x87, fault reporting, host-throughput, analog,
test-runtime and Shared portability/governance debts do not become accepted
CPU instruction/timing rows. Their original owners/conditions remain visible;
no applicable missing SX operation is newly deferred by this batch.

The initial source/test pass finds no gap, but the later C03/C13 candidate-cache
counterexample below disproves that initial conclusion. B1--B4 are admitted and
repaired before final whole-suite execution. Artifact provenance and
actual-commit review still precede completion.

## Whole T Code-Quality And Boundary Review

Use `git diff --numstat 606ee84f4 HEAD` for the complete T563 source baseline,
before the S8 corrective cohort, and separate C/H from CMake/manifests/docs/EXEs:

| Changed owner | C/H files | Added | Deleted | Net |
| --- | --- | --- | --- | --- |
| X86 Core production | 14 | 1601 | 301 | +1300 |
| X86 NXVM production | 3 | 10 | 9 | +1 |
| X86 Core tests | 20 | 4766 | 274 | +4492 |
| X86 NXVM tests | 2 | 7 | 4 | +3 |

The three X86 NXVM production paths are its two Board admission/reset owners
and task version header; no App-specific hardware is moved into neutral Core.
Eight changed CMake files total +45/-7, separately from the root preset's
three task-target updates and four manifest files. S8's later corrective delta
is reported separately below; its receiving artifacts must be rebuilt from
that repaired source, not merely relinked from this earlier baseline.

The sole changed public C/H interface is `x86-core/cpu/cpu_interface.h`.
Recheck its actual T-level diff: the approved existing external-cycle callback
returns matching extra wait clocks; the approved bus-advance operation consumes
sole-Core elapsed time/bus availability/invalidation cycles; the approved
observation tail copies target lexical components before entry. Two source/model
origins are append-only, and formula-role bits retain distinct meanings. Old
enum values are unchanged. There is no new public hardware layout, raw CPU/RAM
pointer, timing registry, device framework, worker or host clock. Lib/Emulator,
MyNES source/tests/EXEs and all four App sources remain unchanged throughout T563.

The code growth implements missing numerical/access/event allocation and its
real receiver proof. CPU alone owns decoder, cache/TLB, bus timing FIFO and
completed facts; Core alone owns pending completion and guest time; Board
alone owns PC reset/clock/DMA wiring. No App-name special case, second selected
timing/preview path or mirrored architecture state is introduced. Distinct
logical operand, descriptor, bus and accepted-entry observations have named
roles and one reset/publication boundary. Their positive test growth supplies
independent source values, actual guest effects and fault-prefix/purity checks,
not another production implementation used as an oracle.

The current-source searches include selected CPU/profile predicates, all
compatibility/source-origin branches, maximum/qualification consumers,
`completed_observation`/`next_lexeme_components`, timing miss/role/wait
producers, and Core-running test rebinding/CPU-profile assignments. The
nineteen removed bridges have not returned; pure CPU construction remains at
its own test owner. Existing local corpus/Types/DAG and receiving ownership
gates enforce the boundary, while numerical behavior is verified by actual
execution. No new mechanism or code cleanup is needed merely to make the
final report smaller.

## Fresh Complete Unit Qualification

Both original Ninja/ccache graphs build the complete unit aggregate and
specialized gates. Each strict direct-compilation matrix passes 498/498 rows,
zero deferred. Complete repository units pass **632/632 per width**, with
zero failures/skips: x64 77.38 seconds, x86 66.23 seconds. Each contains 629
non-desktop cases and three automated desktop-labelled cases, including all
166 X86 Core and 229 X86 NXVM cases. Native qualification is serialized across
widths. JUnit/logs are under ignored `build/t563-s8-frozen-*` paths.

No source/test/manifest edit occurs during this baseline run. Complete
registered integrations and final artifact review remain separate below;
unit success does not imply manual RDP or full Windows GUI acceptance.

## Confirmed Debug Candidate Cache Counterexample

The initial reconciliation above is not final acceptance. Continue checking
the C03/C13 Debug candidate boundary instead of trusting the green suite.
`x86_core_cpu_debug_patch_registers` copies the whole execution context, but
segment loading writes Accessed through its real bus. The copied cache receives
the write, while the original CPU cache can retain the old data. A later invalid
segment can also reject the register candidate after an earlier descriptor
write. This is distinct from the retained raw CR0/CR3 policy TODO.

An isolated selected-SX probe uses actual guest MOV to warm a GDT cache line,
then calls the existing Debug patch and executes another guest MOV. The first
uncacheable fixture correctly reads 93, so it is not sufficient proof. Enable
the existing declared cache-memory query, warm 92 and rerun on the same frozen
source. `build/t563-s8-debug-cache-probe.c` then reports:

```text
reject=0 status=0 es=10 memory=93 writes=1 guest_cached_read=92
reject=1 status=2 es=0 memory=93 writes=1 guest_cached_read=92
```

Thus C13's Debug/cache lifetime is incomplete even though the existing whole
units and integrations pass. Do not close this T or defer that applicable CPU
defect as unrelated policy. Preserve the pre-repair results as baseline only.

### Complete Repair Cohort And Ownership Decision

| ID | Complete boundary obligation | Planned single-owner repair and proof |
| --- | --- | --- |
| B1 | Validate the complete register candidate without architectural/bus/cache/TLB publication | Reuse the existing CPU preview/observe-only mechanism and owner-local descriptor write preflight. Reject an invalid later field before any descriptor write; retain the existing raw-control/field-order semantics. |
| B2 | Commit segment effects through the actual CPU cache/TLB owner | Reuse one private field applicator for preflight and commit. Temporary CPU/instruction candidates do not become another cache or bus owner; all real memory effects update the live execution context. Success keeps register and cached/physical descriptor results coherent. |
| B3 | Physical commit failure preserves accepted prefixes without publishing a failed register batch | Restore borrowed candidate pointers on every return. No provider replay/rollback, extra queue or retry. Keep accepted physical/cache effects coherent and invalidate existing execution assumptions as needed; register data publishes only on success. |
| B4 | All callers and context-sensitive cases retain their contract | Sweep six segment fields, pure scalar/flags/CR0/CR3/CR4 patches, prepare-entry and single-register wrappers, then the sole Core/Emulator Debug route. Add cache-enabled chip cases and actual public-Core regression; retain old families and ordinary guest descriptor behavior. |

Standing owner permission covers CPU/Core accuracy repairs with no new public
contract. Coordinator revises this S packet before implementation. Predicted
production C/H +65--110/-40--65; existing tests +160--280/-0--15. Only X86 Core
CPU implementation, interface comment, its existing CPU/Core tests and two
manifests may change, followed by eight receiving PC builds. No Lib/Emulator,
App/source, raw Debug policy, new public function/type/layout or persistent
state is admitted. Finish the existing baseline job before source/test edits,
then qualify the complete repaired batch, not just the first cache witness.

### Implemented B1--B4 And Final Freeze

Both baseline integration runs finish 48/48, zero failures/skips, before any
production/test/manifest change. The x86 DeskPro group completes in 172.26
seconds under its unchanged 300-second budget. All baseline results above
remain pre-repair evidence, not the final receiving qualification.

The CPU's private field applicator now serves one read-only preflight and
one live-cache commit. The former borrows separate CPU/instruction candidates
in the existing preview context, including existing descriptor write-access
preflight. The latter temporarily borrows only the CPU/instruction candidates
on the real context; it never copies back another cache/TLB/bus owner. Both
pointers are restored before returning. Only success publishes register data;
a physical failure leaves its real accepted memory/cache prefix coherent.
Candidate CR0/CR3 translations are invalidated after restoring live pointers,
including failed physical commit. No automatic provider replay is introduced.

The same-issue sweep confirms prepared entry also used a copied cache. It now
shares this private preparation mechanism, retaining its distinct CS/DS/ES/SS
order, scalar/flags publication and finish/cancel lifetime. Successful prepare
can set descriptor Accessed before finish, as before; cancelling does not
pretend to undo accepted physical writes. Invalid complete candidates now
reject before those writes. Debug keeps enum order and its original raw
CR0/CR3 administrative policy. Existing timing FIFO observations require an
active instruction, so paused Debug does not manufacture guest clocks.

Existing CPU Cache proof covers all six segment fields, GDT/LDT, SX and 386,
cache-warmed success, invalid later field, failing second physical write and
accepted-prefix coherence, WP/read-only/Accessed paging combinations and
prepared-entry success/cancel/rejection. The existing public-Core Debug test
executes LGDT and guest MOV, verifies rejected patch preservation, then checks
the successful descriptor through physical inspection and another guest MOV.
It uses public interfaces only, with no private CPU state bridge. Both focused
x64 tests pass before final whole-suite freeze.

Actual C/H change before final qualification is production +116/-58 (net +58)
and tests +266/-1 (net +265). Production exceeds the initial upper estimate by
six added lines because the same-owner prepared-entry receiver needs an
explicit preserved load-order table instead of silently adopting Debug enum
order. Two private helpers consolidate validation/commit and field application;
the existing descriptor preflight needs one private forward declaration. No
public function/type/layout, persistent state, thread, timing source, queue or
Lib/Emulator/App change is introduced. Two existing Core manifests are updated.

## Repaired Qualification: Native MIDI Stop

The first repaired x64 whole-unit run builds successfully, including both
specialized aggregates and the 498/498 strict matrix, but passes only 631/632
in 281.71 seconds. The sole failure is the unchanged
`lib.audio/midi/native_midi_output` assertion at create-result line 14. All
166 X86 Core cases and the new Debug/prepared-entry proofs pass. A single
isolated native recheck outside the restricted execution environment still
fails at the same assertion. Preserve both failures; they are not a pass or
proof of a CPU defect.

The MIDI test binary's last-write time precedes this source repair, and the
repair build log contains no MIDI test/backend recompilation. The same
unchanged Lib test passed in this S8's earlier complete 632/632 baseline.
No current native error code is inferred from the assertion alone. The S5
historical diagnosis observed a native-open error subsequently resolved after
the owner restored the Windows session, but it does not prove the cause of
this new occurrence. Ask whether the current session remains logged in and
connected; continue independent dual-width qualification without editing Lib,
changing host settings, suppressing the case or weakening an assertion.

Logs/JUnit: `build/t563-s8-repaired-units-x64.*` and
`build/t563-s8-midi-recheck-x64.xml`. These stop full acceptance until the
unmodified case and required complete receiving qualification succeed.

The unchanged MIDI test hash is
`912E98C308F82BE8C7621984AC669A3A259A957CC49C10B3DB69CD437A073941`,
identical to the earlier S5 native diagnostic. Reuse its already-verified
return-point breakpoint, with the x86 graph still only building and no other
native qualification active. The current debugger run reads EAX=1 immediately
after native `midiOutOpen`; both event creations have succeeded before this
call. Lib maps that native error to IO_ERROR, as designed. The inferior then
fails its existing assertion and exits 3; debugger-driver exit 0 is not a
passing test. Log: `build/t563-s8-midi-native-boundary.log`.

This confirms the current failing primitive, not the underlying Windows
driver/session cause. No Lib, native test, binary, host setting or service is
changed, and no MIDI playback is requested. The earlier successful S8 baseline
and current native error on exactly the same binary rule out a new compiled
CPU dependency as the explanation for this assertion. Other current native
desktop tests can pass without proving that this MIDI endpoint opens.

The repaired x86 whole-unit run also passes 631/632 in 85.49 seconds. Its sole
failure is the same native MIDI create test, with process exit 0xc0000409;
do not misclassify this assertion/abort as a CPU guest fault. All 166 X86 Core
cases and all three automated desktop-labelled unit cases pass on both widths.
Both complete builds and both 498/498 strict matrices pass, with no deferred
row. Preserve `build/t563-s8-repaired-units-x86.*` beside the x64 failure.

The complete repaired x64 integration inventory passes 48/48: Default 35
(including MyNES twelve once), XT three, AT five and DeskPro five. DeskPro
completes in 151.34 seconds with the unchanged budget. Restore the original
graph to Default and rebuild its artifact. The x86 equivalent remains in
progress separately; no whole-repository green claim is made.

## Final Independent Receiving Results And Hold

The repaired x86 registered integration inventory also passes **48/48**:
Default 35, XT three, AT five and DeskPro five, zero failures/skips. DeskPro
completes in 177.72 seconds under the same 300-second budget. Thus both
complete inventories cover 36 PC plus twelve MyNES integration executions per
width; no MyNES duplicate is rerun on the other three profiles. Default is
restored on both original graphs and its final artifact rebuilt. The combined
x86 command correctly exits 8 because its earlier unit failure remains;
its successful integration JUnit is not incorrectly marked failed or vice versa.

All eight final published PC EXEs pass the real product/INI identity harness.
PE machine is 8664 for x64 and 014C for x86; none retains a debug section.
Current [artifact identities](t563-s8-artifacts.json) record final sizes and
SHA256 after both complete receiving runs and all eight CLI checks. Four
owner INI hashes still match admission; external media/ROM and MyNES source,
tests and executable pair are unchanged. No App or Lib/Emulator source is
modified. All compilation/test jobs are terminal before this report.

| Final repaired qualification | x64 | x86 |
| --- | --- | --- |
| Full build and specialized aggregates | Pass | Pass |
| Direct strict compilation | 498/498, zero deferred | 498/498, zero deferred |
| Whole repository unit execution | 631/632; native MIDI create fails | 631/632; same native MIDI create fails |
| X86 Core units within that run | 166/166 | 166/166 |
| Automated desktop-labelled units | 3/3 | 3/3 |
| Whole registered integration executions | 48/48 | 48/48 |
| Automated desktop integrations within that run | 2/2 | 2/2 |
| Final published PC CLI checks | 4/4 | 4/4 |
| Manual RDP, physical sound/MIDI, full guest GUI and PC110 | Not executed | Not executed |

Final T563 C/H footprint against 606ee84f4, including this repair, is X86 Core
production fourteen files +1717/-359, tests twenty-one files +5032/-275;
X86 NXVM production three files +10/-9, tests two files +7/-4. The S8-specific
production/test sizes and necessary private helpers remain as reported above.
No second instruction/cache/TLB owner, timing source or input/publication path
is introduced. Every source/test change is within the admitted candidate
mechanism and its actual receivers; no formatting-only or App-specific fix
is mixed into it.

Coordinator review retains the completed B1--B4 implementation and receiving
results, but **does not accept or close S8/T563**: the mandatory whole-unit gate
is not green. No implementation P is committed or pushed while that required
stop remains. Do not bypass the native check, report the earlier baseline as
the repaired result, or change Lib/Windows outside the admitted CPU boundary.
The owner has been asked about the current Windows/RDP session. Resolve the
native-open prerequisite, then complete the unchanged test qualification,
actual-diff/actual-commit review and target-separated delivery. Remaining
work is qualification/delivery, not another unimplemented applicable CPU row.

## Owner-Approved Bounded Closure Amendment

On 2026-10-17 the owner explicitly asks to close S8, then admit S9 to audit
and import SoftPC's committed eight shared trees. This supersedes the prior
S8 hold only for that one unchanged native MIDI gate. Preserve 631/632 per
width, all earlier failures and the original full-T requirement; neither
whole-suite success nor complete CPU/T acceptance is inferred. The complete
B1--B4 code, manifests, source audit, 48/48 integrations per width and eight
qualified receiving EXEs are now deliverable under this narrow amendment.

Pin SoftPC 4a675c00390af2efb9adb9c5d0607d569ab0ba05 for the next audit.
Its shared roots are Lib, Emulator, X86 Debug and X86 IBMPC source/test pairs;
its pending App/root/docs/binary changes stay excluded. X86 Core is not among
those eight roots, so S9 must preserve this S8 CPU delivery. Initial read-only
comparison confirms the MIDI backend is unchanged: native MIDI moves from
unit to integration/native_audio with a resource lock. That fixes ownership,
not native midiOutOpen; S9 must not describe reclassification as a host fix.

Complete Shared P1 **16e7e3a97** is pushed after self-review of the actual
seven source/test/manifest paths. Its tracked +387/-64 includes +116/-58
production C/H, +266/-1 tests and five replacement manifest hashes. No Lib,
Emulator, App or owner-input path enters that commit. The separate NXVM P2
retains this complete amended packet, universe audit, true qualification
results, history and eight already-qualified EXEs. The owner-approved native
gate transfer is the only closure exception. Coordinator actual-commit review
and the governance closure still follow this delivery.

## Coordinator Actual-Commit Acceptance

Review the actual seven Shared paths in **16e7e3a97** and fifteen NXVM paths
in **f1174de9f**, not just their summaries. Check the candidate/publication
mechanism, all six segment/GDT/LDT/paging/failure/entry cases, public-Core
receiver and replacement hashes; then inspect the amended 16-field packet,
complete ledger, TODO clarification, history and eight binaries. Recheck final
JUnit: 632 executed/one native MIDI failure and 48 integration successes per
width, plus eight current recorded hashes. The pre-governance worktree is clean.

Accept and close S8 under the owner's 2026-10-17 amendment. The only exception
is the one native qualification gate, assigned explicitly to S9; no CPU row,
failed test or model limit disappears. No T closure, native playback, manual
RDP/full guest GUI or PC110 qualification is claimed. This governance update
changes no executable input and requires no timestamp-only rebuild. Next,
admit S9 from the clean pushed acceptance and audit the pinned incoming eight
trees before any source replacement.
