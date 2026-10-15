# T560 S6 Feature And Exception Investigation

## Baseline And Scope

Baseline is clean a14f4420b after accepted S5 33cb2efe1. CURRENT owns the
active S6 packet. This is source/current-path investigation, not a completed
feature or exception qualification. C01/C10/C13/C14 and carried C05 exception
entry remain the full batch; none is discharged by one corrected instruction.
Shared-six, MyNES, owner INI, firmware originals and media are excluded.

## Source Applicability

The pinned Intel 272769-004 SX datasheet supplies the selected candidate's
VME-present/FPU-absent feature mask. The 1992 PRM and 273021-001 developer
manual supply the ordinary 486 exception, debug and SX software-emulation
rules; original identities remain in T558 evidence. They do not prove the
exact PC110 fitted stepping or authorize unrelated later-family capabilities.

The developer manual's printed appendix C-1 explicitly omits detailed advanced
features and refers to a separate confidential supplement. No such supplement
is acquired, imported or required as an implementation source here. Public
Intel architecture documentation supplies the VME algorithm, with selected
feature eligibility still grounded in the original SX datasheet rather than
the later manual's generic processor list.

Intel publicly serves Software Developer's Manual volume 3B, 253669-087US,
March 2025, at
<https://cdrdv2-public.intel.com/850979/253669-087-sdm-vol-3b.pdf>.
Acquired 2026-10-14 into the external CPU manual archive as
`intel-ia32-system-programming-253669-087-2025.pdf`; SHA-256
`C34B30715B46AE549B4936D0DFA171E5B149D799E307E0DF2990A4FA66CE288A`.
Ignored extraction is `build/t560-s6-public-system.txt`. Chapter 22 supplies
the public VME/interrupt-redirection rules. This does not authorize APIC,
PVI, PSE, long mode or a later CPU identity. An attempted download of Intel's
older 253668-021 October 2006 endpoint returned HTTP access denied; it left
no archived PDF and is not treated as an acquired source.

Intel's public combined instruction reference 325383-078US, December 2022,
was acquired 2026-10-14 from
<https://cdrdv2-public.intel.com/671110/325383-sdm-vol-2abcd.pdf> as external
`intel-ia32-instruction-reference-325383.pdf`; SHA-256
`F117CB21CE1F3B6258F5CE5F786E91665A133BF55E7BC2DC8CAE843B56E4FB8E`.
Ignored extraction is `build/t560-s6-public-instructions.txt`. IRET's
RETURN-FROM-VIRTUAL-8086-MODE algorithm (Vol. 2A printed 3-539--540)
retains GP for IOPL below 3; VME does not introduce a successful low-IOPL
IRET. Protected IRET (3-541--543) permits VIF/VIP restoration only at CPL0;
real and VM86 returns preserve them. Later-family portions of this combined
manual are not selected-SX capabilities.

## Confirmed Current Paths

| Capability | Current owner / observation | Required disposition |
| --- | --- | --- |
| C01/C10 CR4 | cpu_instructions.c rejects CR4 in decode and MOV; cpu.c debugger can store it without guest validation. | One selected-SX CR4.VME writer/validation rule, retained earlier-family rejection; audit debugger publication without a second feature store. |
| C10 flags | Selected reserved mask still strips VIF/VIP; CLI/STI/PUSHF/POPF reject VM86 IOPL below 3. | Actual virtualization, size/fault rules and one flags owner; no automatic software VIP mirror. |
| C10 software INT | _e_int_n rejects VM86 IOPL below 3 and never reads a TSS redirect bitmap. | Source-qualified bitmap/IVT versus protected IDT routing; keep hardware interrupts and INT3/INTO distinct. |
| C10 IRET | Existing VM86 return rejects low IOPL; existing stack peek/preflight is available. | VME return checks before architectural publication, retain normal protected and legacy return paths. |
| C13 GD | MOV DR does not test GD; real debug and exception delivery already exist. | Actual BD/GD fault and handler-entry semantics at existing owners, not monitor pause as a substitute. |
| C13 debug/exception | Existing breakpoints, TF/RF, segment shadow, task trap and nesting have partial earlier-family proof. | Complete selected-SX matrix, reserved fields, priority and actual entry stack alignment; do not infer qualification from storage. |
| C14 ESC | With EM/TS clear, selected SX reaches FPU_UNSUPPORTED instead of architectural NM. | All eight ESC classes trap without FPU/data-command publication; preserve fetch/LOCK priority and prior families. |
| C14 WAIT | Existing TS-and-MP predicate is correct; pending FPU/TEST paths require selected-SX applicability review. | Full bit/mode/prefix/fetch matrix; no fabricated integrated or external FPU. |

DR4/DR5 are labelled reserved in the original 486 source. Archived public
Intel volume 3B section 24.23.3 (printed 24-20) explicitly describes early
processors aliasing them to DR6/DR7. This is retrospective historical evidence,
not admission of later DE. The selected SX uses those same owned registers.

## Finite Proof And Single Owners

- CR4: both MOV directions, all indices, GPRs, code/prefix sizes, CPL0--3,
  VM86, unsupported bits, LOCK and every failed fetch; debugger and reset
  use the same selected capability/normalization rather than raw free storage.
- VME: real/protected/VM86, IOPL0--3, VME0/1, VIF/VIP combinations and
  both operand/address sizes for CLI/STI/PUSHF/POPF/INT/IRET. Include TSS
  limits/base/bitmap, all redirect bit values, paged supervisor table reads,
  guest IVT/stack failures and no premature flags/SP/CS/IP publication.
- Debug/exception: all actual DR indices and reserved/source-resolved forms,
  GD read/write, sticky causes, RF/TF, enabled/disabled matches, segment/task
  shadows, interrupt/fault priority and nested exception classes. Prove real
  handlers and frames, not shutdown from an invalid fixture.
- SX FPU: all ESC opcode classes, register/memory forms, EM/TS/MP/NE,
  real/protected/VM86, prefixes/LOCK, failed instruction fetch, denied operand
  and actual NM frame; WAIT separately uses its documented bit predicate.
  Integer XADD remains independent of EM/TS under the already recorded source
  conflict decision; it is not changed into an FPU instruction.

CPU owns architectural flags/debug/exception behavior. Core owns physical
routing, FPU binding and actual interrupt admission. No new manager, feature
registry, second interrupt dispatcher, public raw layout or flags mirror is
planned. Retain the original table-style handlers and distinguish genuinely
different real/protected/VM86 publication rules. Preliminary complete-batch
estimates remain source +250--550/-60--180 and tests +400--700/-20--80,
subject to the full source/form design before implementation.

The table records admission-baseline observations, not a claim that a later
repair is absent. S6 remains active; the bounded progress below is not a
complete batch, new release or whole-CPU qualification.

## SX ESC Implementation And Direct Proof

Intel 1992 PRM 19.2.3.2 (printed 19-3--4) requires absent-FPU NM for SX
floating-point instructions and separately explains the software EM/MP/NE
configuration. The existing ESC owner now tests selected SX alongside EM/TS,
before FPU dispatch or data access. Earlier profiles retain the same code.
Actual production change is +2/-1, net +1; no helper, API, persistent state
or alternate path is introduced.

The existing FPU-interface test adds +139/-0 lines for a same-owner probe and
the full ESC matrix: eight opcode classes, 16 EM/TS/MP/NE combinations,
two code sizes, four operand/address prefix combinations, register/memory
forms and real/protected CPL0--3/VM86, 12,288 cases. Denied memory operands
must not override NM; no operand read or FPU command may publish. Real and
protected handlers check restart CS/IP, preserved GPRs and the actual frame,
including VM86's nine saved fields. Fault-frame RF follows PRM 9.3.3;
the diagnostic snapshot remains the pre-fault flags.

The first test on unchanged S5 source fails at D8 C0 with all control bits
clear, confirming the missing selected-SX NM condition. The initial new probe
mistakenly counted the legitimate IVT read as an operand read; its counter is
now explicitly tied to the immutable test operand address. The new protected
frame expectation initially omitted RF; it was corrected to the existing
source-qualified fault image, not patched in production or dropped.

The real-only first matrix and retained XADD/CPUID cohort passed 3/3 on both
widths. After adding all modes, the final expanded existing FPU test passes
on x64 and x86 in the original release-ccache graphs:
`build/t560-s6-sx-esc-mode-matrix.log` and
`build/t560-s6-x86-sx-esc-mode-matrix.log`. All old FPU assertions remain.
These are focused results, not final full-unit or artifact qualification.

Remaining C14 includes WAIT applicability, illegal LOCK and failed-fetch
ordering plus actual Core binding proof. C01/C10/C13, exception-entry
alignment/nesting and complete S6 qualification remain mandatory. The latest
verified eight EXEs are still the accepted S5 delivery, not new S6 artifacts.

## CR4, Virtual Flags And IRET Progress

Selected-SX MOV CR4 now reads/writes the actual architectural register;
only VME is accepted. Guest and debugger writes share one private value
validator, and invalid debugger patches reject before any register mutation.
Earlier guest personalities retain their CR4 rejection. VIF/VIP are real
EFLAGS bits, not a mirrored pending-interrupt state. Existing CLI/STI and
PUSHF/POPF owners implement low-IOPL VM86 virtualization, including VIP/TF
faults, 16-bit-only flags operations and physical IF preservation. The initial
RF-clearing choice is corrected by the selected-source reconciliation below.
The one VM86 eligibility predicate owns this selected-family distinction;
no public API, type, persistent field or second dispatcher is added.

The new same-owner VME test proves 256 valid MOV CR4 forms, 31 invalid
guest/debugger values, earlier-family rejection, 14,336 flags combinations
and four stack-versus-permission cases. Existing CPUID, PUSHF/POPF, IRET,
VM86 IRET and FPU regressions passed with it on both widths before the
additional IRET repair. No full S6 qualification is implied.

Making VIF/VIP architectural also requires auditing every flags loader.
Real and VM86 IRET preserve them; nonzero-CPL protected IRET cannot
restore them. These corrections live at the four existing return owners.
Eight real/VM86 return cases deliberately put inverse VIF/VIP bits in the
stack image. A further 384 low-IOPL IRET cases vary VME, IOPL, virtual
flags, code size, prefixes and denied stack limits, proving actual GP frames
and the instruction-start restart point. The selected-SX fault branch no
longer replaces the saved instruction-start CPU state with post-fetch state.
It does not add a virtual IRET success path. Older profiles' retained
checkpoint behavior remains for the inherited-corpus review, not silently
requalified here.

The latest six-test cohort passes 6/6 on each width in
`build/t560-s6-x64-iret-final-cohort.log` and
`build/t560-s6-x86-iret-monitor.log`. Core source/test manifests, production
dependency boundaries, NXVM documentation governance and diff whitespace
checks also pass. INT redirection, complete CR4
privilege/fetch/LOCK proof, remaining flags/task writers, GD/debug/exception
and WAIT/Core proof remain open. No Windows desktop, RDP, full units or
new S6 release is claimed.

## Software INT Redirection Progress

Intel 253669-087US 22.3.3.4--5 (printed 22-19--20) and 325383-078US
INT algorithm (Vol. 2A 3-518) define the source-qualified normal route.
The selected-SX VM86 INT imm8 owner now reads the redirect byte from the
existing cached 32-bit TSS: I/O map base minus 32 plus vector/8. TSS reads
retain supervisor translation; a clear bit reads the four-byte guest IVT
entry at user linear vector*4, not the protected IDTR. The existing stack
and segment owners create a six-byte frame and real-mode CS cache while
VM86 remains active. IOPL below 3 pushes IF=VIF and IOPL=3, then clears
VIF; IOPL3 clears physical IF. Both clear TF after saving its old value.

A set bit retains the protected interrupt owner at IOPL3 and GP at lower
IOPL. Only INT imm8 calls this helper. INT3, INTO, exceptions and hardware
interrupts retain their distinct original entry routes. The private helper
owns a genuinely different TSS/IVT/frame contract, not a forwarding layer,
second interrupt dispatcher, state mirror or public interface. This phase
adds +56/-1 production lines and +111/-0 test lines relative to the prior
IRET snapshot (the two TF assertions change within the added test).

The actual CPU test covers all 256 vector bits, four IOPLs, virtual and
physical IF values, code size and both prefixes: 32,768 successful redirect
cases. It verifies exact saved IP/CS/FLAGS, three writes, six-byte SP change,
target CS cache, preserved VM and cleared TF/appropriate interrupt bit.
Twelve low-IOPL set-bit cases verify actual GP restart frames. Twelve more
cases verify protected set-bit INT and INT3/INTO entry with actual VM86
nine-field frames. INT3/INTO use a TSS without the redirect-map extension,
so accidental bitmap consultation is rejected rather than hidden.

Latest related seven-test cohorts pass 7/7 per width in
`build/t560-s6-x64-int-regression.log` and
`build/t560-s6-x86-int-regression.log`. These are focused instruction proofs,
not full qualification. TSS absent/malformed/bounds classification, paged
supervisor-versus-user accesses, IVT/stack provider failures and retained
accepted effects still require explicit source/actual-route review. Current
invalid-map GP guards are not declared silicon-qualified by normal-map tests.
Hardware interrupt admission, CR4 privilege/fetch/LOCK, GD/debug/exception,
WAIT/Core binding and complete S6 qualification remain open. No new release
or Windows support claim is made.

## General Detect And Single Debug Entry

Intel 1992 PRM 11.2.2, 11.2.3 and 11.3.1.3 define DR7.GD protection,
DR6.BD status and automatic GD clearing on debug handler entry. Selected-SX
MOV DR now checks GD at the common decoded register-access boundary before
reading/writing the architectural operand. BD is retained by the existing DB
fault-checkpoint rule. The existing exception entry clears GD for selected
SX DB; no DR7 mirror or special retry path is introduced.

The same-owner sweep found that deferred debug traps used the external IRQ
entry. They now call the existing exception entry, so fault, task-debug and
deferred-debug delivery share the actual DB entry contract. External IRQ/NMI
delivery still uses its own existing entry; no PIC vector is reclassified as
a debug exception merely because its vector number is 1. The generic trap
route consolidation also runs the retained earlier-profile debug cases.

Relative to the preceding INT snapshot, production changes are +11/-1
(two private bit constants, one common access check, one entry clear and one
trap call replacement). The existing debug test adds +109/-0, no new fixture
or registration: six supported DR indices, both directions, code size,
operand/address prefixes, real/protected mode and initial RF, 384 cases.
The GPR operand in this phase is EAX. Actual DB handlers verify restart IP,
fault flags/frames, unchanged EAX and DR0--3, sticky old DR6 plus BD and
cleared GD. Two NOP single-step cases prove GD clearing on traps without
adding BD, with next-IP rather than fault-IP diagnostics.

The first new test build mistakenly named a nonexistent array-count macro;
it uses the existing sizeof idiom instead of extending Types. The first
five-byte real-mode case then overwrote IVT vector 1 because the base fixture
loads code at address zero. The new matrix uses guest code at 10000h and
keeps IVT separate; no production behavior or assertion was weakened.

Latest related cohorts pass 5/5 on each width in
`build/t560-s6-x64-gd-regression.log` and
`build/t560-s6-x86-gd-regression.log`. This does not finish C13: other GPRs,
privilege/LOCK/fetch/invalid-form priority, all DB causes and entry failures,
same-level exception alignment/nesting and the complete flags/task sweep
remain obligations. INT malformed-map/paging/failure qualification and C14
WAIT/Core proof also remain. No full-unit, artifact or S6 completion claim.

## WAIT And GD Priority Progress

Intel 1992 PRM WAIT (printed 26-280) checks pending numeric exceptions and
raises NM only for MP and TS together. Initialization table 10-3 (10-4)
separately shows that EM does not change WAIT's TS predicate. Selected SX
has no integrated FPU; its WAIT must not enter the retained early-family
external TEST wait. The existing handler now keeps pending-FPU/TEST behavior
inside the non-SX branch, after the unchanged MP-and-TS NM check. This phase
is +7/-8 production lines, net -1, with no helper, API, state or second wait
path. Actual Core absent-FPU binding proof remains a separate required check.

The existing FPU CPU test adds +63/-0 lines: 768 cases across real mode,
protected CPL0--3 and VM86, 16 EM/TS/MP/NE combinations, two code sizes and
four prefixes. Nonfaulting WAIT preserves the entire CPU except next IP;
faulting WAIT verifies the actual NM frame. The injected TEST callback always
returns asserted if called, but SX must never call it or enter a wait state.
All old FPU/ESC assertions are retained.

The existing debug test adds +50/-0 for 72 actual LOCK/privilege cases.
With GD set, illegal LOCK wins with UD; nonzero CPL/VM86 DR access yields
GP. Both preserve DR6/DR7 and the GPR rather than publishing BD or clearing
GD. Actual no-error/error-code and VM86 save frames are checked. GD fetch
failure and other GPR/invalid-form priority remain unqualified by this test.

The original special-register MOV description (PRM 26-213) says the ModR/M
mod field is always 11. The modern instruction reference describes mod bits
as ignored. This investigation does not silently import the latter into
the selected historical contract or change current decoding; exact invalid
encoding/source applicability remains in the complete form review.

Latest cohorts pass 4/4 per width in
`build/t560-s6-x64-wait-priority-regression.log` and
`build/t560-s6-x86-wait-priority-regression.log`. They include actual Core's
existing FPU interface test, proving retained 286 ESC restart, external TEST
and wait-interrupt behavior; that test's earlier-family cases do not prove
selected-SX Core binding. Full S6, eight releases and T560 remain open.

## Actual Core SX FPU Qualification And ESC LOCK

The production Core creation validator still rejects SX. This is intentional
until S10, not a missing test to be fixed by opening admission early. Existing
same-owner Core qualification tests already create an admitted machine and
bind its owned CPU to the pending SX personality using the real bind-profiles
interface. The FPU test follows that pattern; nominal one-tick CPU timing is
copied by bind, not stored as a pointer. It proves semantics/connection, not
486 timing, board initialization or product admission.

The existing NM helper accepts the tested CPU personality, keeping the old
386 callers and real handler/frame/run/HLT assertions. SX covers all eight
ESC classes with 16 EM/TS/MP/NE combinations (128 actual Core runs), plus
16 WAIT cases. Faulting WAIT uses the real NM handler; successful WAIT can
execute the following NOP rather than remain suspended. Core owns the same
memory/FPU/executor connection and FPU profile NONE. No new production API,
state, constructor path or test framework is added. This file changes +55/-6.

The CPU FPU test reuses its actual NM-frame checker for no-error-code
exceptions and adds 6,144 illegal-LOCK ESC cases: eight opcodes, 16 control
combinations, six modes, both code sizes and four prefixes. Actual UD wins
over absent-FPU NM, no operand/FPU command is delivered, and restart/frame
proof is retained. The file's cumulative S6 diff is +245/-0 against baseline;
this phase increases it by net 43 lines over the prior WAIT snapshot.

Both widths pass the five-test cohort in
`build/t560-s6-x64-core-sx-lock-regression.log` and
`build/t560-s6-x86-core-sx-lock-regression.log`, including the unchanged
production CPU-profile gate. No Shared or App production changes occur.
Failed instruction fetch, WAIT LOCK, memory ESC forms under LOCK, complete
INT paging/failure and C13 flags/exception proofs still remain; this is not
whole C14 or S6 completion. The next test estimate is revised to +1000--1500
/-20--80 for the entire S6, including the already-added 424-line VME test,
owned matrices and pending failure proof. Production estimate stays bounded
by existing owners; no framework is justified by test growth.

## S6 Instruction Fetch Failure Proof

The existing decode-admission fixture now has one explicit test-only control
for rejecting instruction-prefetch reads as well as required fetches. Old
cases leave it false and retain their original behavior. Initial SX failure
injection did not fire because the instruction came from actual prefetch;
the corrected injection rejects that read and the eventual required read.
Speculative retries may make the injection count greater than one, but a
nonzero count, terminal CE and no architectural publication are all required.
No CPU behavior is changed to accommodate the fixture.

One named table covers CLI/STI/PUSHF/POPF/IRET/INT/WAIT/CPUID, both CR4 and
DR directions, all eight register ESC classes and a memory ESC with the
actual 16/32-bit absolute-address encoding. Across code sizes, both prefixes,
16 EM/TS/MP/NE settings and every required byte, 8,192 real-mode failures
verify full t_cpu equality, original restart IP, no data reads/writes, no
port completions, no FPU callback and no spurious delivered exception.
GD is set throughout, so a failed instruction byte cannot publish its BD
fault first. This is provider I/O failure/CE proof, not a hardware PF matrix.

Only `cpu_decode_admission_smoke.c` changes, +95/-2 lines. It extends the
existing fixture and table runner, without a new framework/public interface
or production state. Forms carry an explicit memory-encoding tag rather than
implicitly selecting behavior by a table position or dimension.
Related cohorts pass 4/4 per width in
`build/t560-s6-x64-feature-fetch-regression.log` and
`build/t560-s6-x86-feature-fetch-regression.log`. Core manifests, dependency
boundaries and documentation checks are refreshed after the test edit.
Protected/VM86 fetch priority, INT paging/TSS/stack failure and remaining
C13 flags/exception qualification are still open. No full S6 qualification,
new PC artifact or complete 486SX support is claimed.

## Paged VME INT And Accepted Stack Prefix

The new paged test exposed an ordering defect in the first S6 VME helper:
the whole six-byte paging probe used the frame's lowest address as CR2, and
IVT was read before any pushes. Intel 253669-087US 22.3.3.4 lists saved
FLAGS/CS/IP before IVT lookup; 325383-078US INT pseudocode has that same
order. The helper now uses the existing segment-only stack-frame check,
then the three existing scalar pushes, then guest IVT lookup and target
publication. This phase moves/replaces +5/-5 production lines; no new helper,
state, rollback storage or alternate path is added.

`cpu_vme_smoke.c` adds 119 lines relative to the preceding 424-line snapshot.
Ninety-six actual paging cases cover four IOPLs, four prefix combinations
and six route/failure choices. TSS fixed stack fields and I/O-base/bitmap
straddle two supervisor-only pages; guest code, IVT, user stack, GDT/IDT and
kernel exception stack have explicit separate mappings. Normal redirection
proves kernel TSS reads and user IVT/stack access. Other cases prove IVT
supervisor-page rejection (PF 5), absent IVT (PF 4), readonly user stack
(PF 7), missing TSS I/O-base page (PF 0), and a cross-page stack where FLAGS
succeeds before CS reaches an absent page (PF 6).

The first readonly-stack test failed with CR2=27FAh from the old whole-frame
probe; the corrected scalar FLAGS write yields 27FEh. IVT failures retain
all three accepted stack words, not a fictional storage rollback. The
cross-page case retains only FLAGS; rejected later writes do not appear.
Architectural checkpoint restoration and actual kernel PF frames preserve
the original guest SP/flags/CS and all nine VM86 fields, with exact CR2,
error code and unchanged non-SP GPRs. Page-table A/D effects are not confused
with guest stack writes.

Related cohorts pass 5/5 per width in
`build/t560-s6-x64-vme-paging-regression.log` and
`build/t560-s6-x86-vme-paging-regression.log`. The final stronger GPR/full-frame
assertions also pass the VME test on each width in
`build/t560-s6-x64-vme-paging-final.log` and its x86 counterpart.
Malformed/absent redirect-map classification, provider-level I/O failures,
segment-boundary cases, complete flags/debug/exception and whole-S6
qualification remain open. This is no production admission or new release.

## Flags Writer Ownership Sweep

Reset clears CPU storage, including CR4, and sets canonical initial FLAGS.
Task switch does not transfer CR4 because the TSS has no such field. The
32-bit task and privileged VM86 return paths use the existing flags loader;
16-bit task loading uses the word image. POPF and nonprivileged IRET retain
their distinct permission masks. Direct SX task-form proof remains open;
retained 386 task regression does not discharge it.

Debugger EFLAGS publication bypassed selected-SX reserved-bit normalization
and fixed bit 1. The reserved-mask decision and 32-bit normalization now live
once in the CPU private header, used by the instruction loader and selected
debugger patch. The old instruction-local mask implementation is removed,
and callers use the private owner directly. These two inline helpers own
distinct mask/normalization rules, not a second flags store or public API.
The real/early-family loader still owns its different word-image contract;
old debugger personalities retain prior write behavior.

This phase changes production +27/-19, net +8, and VME tests +27/-0.
Sixty-eight bit/zero/all-one debugger samples compare SX canonical bits and
retained 386 writes through actual patch/read interfaces. All 31 invalid CR4
patch cases now include an EFLAGS mutation, proving whole-candidate rejection
before normalization publishes. No new type, public API or transaction path.

Both widths pass seven-test cohorts in `build/t560-s6-x64-flags-owner.log`
and its x86 counterpart, including CPUID/flags, PUSHF/POPF, real/VM86 IRET
and retained task/cross-width cases. An initially nonexistent task target was
corrected from actual registration; no target was invented. Whole task/flags,
failure/exception, full-unit and release qualification remain open.

## Task Flags Matrix Completion

Closer inspection corrected the earlier coverage description: the existing
task-completion matrix already included selected SX RF/TF/AC/ID, not only
386. It lacked VIF/VIP, reserved-bit input and explicit CR4 retention. The
same table now uses a 16-bit loop index for 512 SX combinations per route,
plus the original 16 combinations per 386 route. Five actual task routes are
direct JMP, CALL, task-gate CALL/JMP and IDT task gate. No new fixture or
production implementation is introduced.

Incoming SX flags exercise VIF/VIP and all reserved fields with fixed bit 1
also absent in half the images; expected canonical bits are independently
specified rather than computed by the production helper. Every transfer
checks exact outgoing flags stored in the old TSS and unchanged CR4.VME.
CR3 normalization/TLB invalidation, NT, RF/TF completion and prior assertions
remain intact. The file changes +23/-7 lines.

Both actual CTest logs report `Task completion flags cases=2640 failures=0`:
2,560 SX and 80 retained 386 cases. Related cohorts pass 3/3 per width in
`build/t560-s6-x64-task-flags-regression.log` and its x86 counterpart.
This proves the 32-bit task flags gap, not every task/exception condition.
Same-CPL alignment/nesting still needs exact source/route reconciliation;
no blanket AC suppression is justified or implemented from this inspection.
Malformed VME map, provider/segment/fetch priority, remaining debug and
full S6/artifact qualification stay open.

## Same-CPL Entry Alignment And Nested AC

Primary alignment eligibility remains Intel 1992 PRM 9.9.16: AM, AC and
CPL3, with implicit supervisor references excluded. The public INT exception
reference in the archived 325383-078US explicitly permits AC for unaligned
user-gate stack pushes. No source supports blanket suppression of AC at
every interrupt/exception entry, so none is added.

The existing alignment fixture now adds 128 cases: INT versus UD fault,
16/32-bit same-CPL user gates, four stack residues, AC on/off and four
prefix combinations. A legal inner-ring AC handler and TSS are configured;
GDT/IDT/TSS are deliberately odd-addressed as in the retained fixture.
Successful entries check exact word/dword flags/CS/IP frames and unchanged
bytes outside the frame. Faults check zero user-stack writes, actual kernel
AC entry and full old SS/SP/flags/CS/IP/error frame. UD uses fault IP and RF;
INT uses its following IP. Nested UD-entry AC is delivered, not reported as
a successful UD or an invented DF.

The first success assertion wrongly equated three logical pushes with three
physical write callbacks. An unaligned store can split into multiple beats;
the test now checks exact frame content and untouched surrounding storage,
while the AC case still strictly requires zero user-stack writes. No CPU
code or original assertion was weakened to compensate. The existing entry
implementation passes, so this phase is +76/-0 test and zero production.

Both widths pass four-test cohorts in
`build/t560-s6-x64-same-cpl-regression.log` and its x86 counterpart.
This proves the tested same-CPL software/fault entry class, not all IRQ,
page/segment priority, provider failure or nested-exception combinations.
The remaining finite C13/failure inventory and whole S6 qualification remain
open; no complete CPU or new release is claimed.

## VME Provider Failures

The same VME test adds a private read/write failure fixture, not a production
error flag or alternate execution path. Six transfer points cover TSS I/O-base
and redirect-byte reads, each of FLAGS/CS/IP writes, then IVT read. Four IOPLs
and four prefixes give 96 cases. Each selected provider failure must occur
exactly once and remain the existing internal CE/I/O failure, not a fabricated
guest PF. Full CPU storage equals the instruction checkpoint afterward.

Aligned stack-write counts and exact frame words verify no retry/replay:
zero, one, two or three accepted pushes remain according to the failing
stage, and rejected words retain their sentinel. This fixture fails before
accepting the selected physical transfer; arbitrary byte-prefix acceptance
inside one failed provider callback is not newly qualified here.
The added 101 test lines include the private fixture and necessary read/write
injection callbacks; production is unchanged.

Related cohorts pass 3/3 per width in
`build/t560-s6-x64-vme-provider-regression.log` and its x86 counterpart.
Core manifests and dependency/documentation checks are refreshed. No full
suite, release or S6 completion is implied.

## Remaining Finite S6 Qualification Inventory

These rows retain the admitted packet, not a new task or narrower exit:

- R1: complete CR4 privilege/VM86/LOCK/form matrix, debugger valid writes and
  reset retention; earlier-family and failed-fetch proof remains linked.
- R2: privileged/nonprivileged IRET and all remaining flags-writer forms,
  16-bit task upper flags, virtual pending priority and exact source conflicts.
- R3: malformed/absent TSS-map classification with explicit source/model
  limits, full bitmap/segment/wrap bounds, hardware IRQ non-redirection and
  protected/VM86 fetch priority. Normal, paged and provider INT proofs above
  do not replace these cases.
- R4: GD GPRs/forms and historical DR aliases now have direct proof; complete
  RF/TF/breakpoint/shadow/entry-failure priority; preserve the one DB owner.
- R5: finite nested-exception classes and remaining same-level IRQ/page/segment
  competition. The directly proven INT/UD-to-AC class is not all nesting.
- R6: remaining SX WAIT LOCK and memory-ESC LOCK/fetch forms; review whether
  actual Core mode-specific connection adds behavior beyond the direct CPU
  and real Core proof already recorded.
- R7: source/code-quality/actual-diff review, stable-snapshot full dual-width
  units and affected eight profile integrations/artifacts, complete P push
  and coordinator acceptance. No partial P/release substitutes for this exit.

T560 S7--S10 and all other convergence rows remain unchanged and mandatory.

## R1 And R6 Form Qualification Update

CR4 normal MOV proof now includes protected CPL0 as well as real mode:
512 cases over both directions, all eight GPRs, both code sizes, prefixes
and VME0/1. Another 1,152 cases prove actual UD for LOCK and GP at nonzero
CPL/VM86, with unchanged GPRs and CR4, restart flags and complete error/no-error
frames. Two valid debugger writes verify readback and reset clearing CR4
and restoring FLAGS=2. Invalid-bit/atomic-patch and earlier-profile cases
remain. The same fault checker now handles its real protected/VM86 frame
variants without a duplicated fixture or const-discarding cast. This phase
increases the existing VME test by net 60 lines to 731 lines.

The existing SX LOCK table expands rather than adding another test path:
WAIT plus eight register ESC and eight absolute-memory ESC forms, across
16 control settings, six modes, two code sizes and four prefixes, total
13,056 cases. Memory forms use correct 16/32-bit displacement encodings
and an inaccessible DS operand; UD must win without operand/FPU access.
Relative to the prior matrix this is +16/-4 test lines, net +12. No production
implementation changes in either form batch.

The CR4/control/decode cohort passes 3/3 per width in
`build/t560-s6-x64-cr4-regression.log` and its x86 counterpart. The combined
VME/FPU/decode/actual-Core cohort then passes 4/4 per width in
`build/t560-s6-x64-cr4-fpu-forms.log` and its x86 counterpart.
R1/R6 direct form evidence is now linked, but full closure still requires
the protected/VM86 fetch and exception-priority inventory, actual source/diff
review and R7 qualification. These passing groups do not redefine that exit.

## Historical DR Aliases And Full GD Registers

The DR decoder adds three lines mapping selected-SX DR4/DR5 to owned DR6/DR7
after GD/privilege checks. The existing debug test adds 512 actual read/write
forms over both aliases, all GPRs, code sizes, prefixes and real/protected
modes. Its GD matrix includes all eight indices and GPRs: 4,096 cases.
DR7 retains 486 behavior, not later hardwired fields. Older-profile rejection
tests remain regressions, not new hardware qualification. Both widths passed
four-test cohorts in `build/t560-s6-<width>-dr-alias-regression.log`.

## Selected-SX Nested Exceptions

Intel 1992 PRM 9.9.8/Table 9-4 defines benign, contributory and page-fault
pairing; 9.9.9 says 486 does not generate vector 9. The existing classification
matrix adds SX and AC, excluding vector 9 for SX and AC for older profiles.
All retained earlier-family cases remain; 1,321 actual helper decisions pass.

The existing four paged delivery cases now execute on both 386 and SX. SX
checks GP-to-PF serial delivery, PF-to-contributory DF and PF-to-PF DF, exact
CR2/error/handler/stack and delivered-fault identity. Missing target descriptors
fault at linear 1308: after GP this is serial PF; after PF it becomes DF.
Mechanically reusing 386 expectations first exposed those two differences.
SX expectations follow the absent descriptor page and source pairing, not a
production classification change or newly qualified inherited 386 semantics;
that broader review remains mandatory in S8.

This phase changes one test +28/-15 lines, no production code or helper.
Alignment/debug/fault-event/VME cohorts pass 4/4 per width on the original
cached graphs. This is focused R5 proof, not all entry/fetch/IRQ priority or
full S6/release qualification. No desktop/RDP test was performed.

## Existing Debug, Entry Failure And Hardware Routes

Seven existing debug functions now accept the actual CPU profile and run on
both 386 and SX: execution/data breakpoints, RF fault/trap images and failed
entry rollback, ordinary transfer RF clearing, IRET completion, comparator
causes and MOV/POP SS shadow boundaries. The existing SS/LSS profile table
also adds SX. No production debug algorithm or duplicate test fixture is added.
An initial SX shadow assertion assumed the 386 DR6 reset value zero; explicit
SX expectations now preserve its documented FFFF0FF0 reserved bits, including
when a new cause is ORed in. The normal RF-suppressed breakpoint case checks
the same initial value. This phase changes the debug test +39/-34 lines.

The existing interrupt fixture accepts a selected profile rather than forcing
386. Actual entry read/write provider failures at GP and DF, plus software,
NMI and INTR external-error-bit cases, now also execute on SX. A provider
failure remains terminal CE, never architectural DF/shutdown; accepted stack
effects remain checked. The fixture/caller changes are +20/-14 lines.

The existing VME test adds 64 NMI/INTR cases over four IOPLs, real IF and VIF
on/off, and both code sizes. Every software redirect bit is clear and the
real IVT deliberately names a different handler. NMI always reaches the
protected IDT; INTR follows physical IF only, not VIF. Delivered cases verify
all nine VM86 frame fields and the real kernel target; masked cases verify no
stack writes or state transition. Two private callbacks supply pending INTR
and its vector, using the existing fixture and bus contract. This is +73 test
lines, no production state, API, queue or fallback. Source: archived Intel
253669-087US chapter 22.3.1--22.3.2, alongside original 486 VM86 entry rules.

Both widths pass the final four-test alignment/debug/fault/VME cohort. Actual
CTest logs are retained as `build/t560-s6-<width>-debug-hardware-routes.log`.
The packet estimate includes the new VME file and is refined to +1800--2400/
-60--160 test lines; this does not enlarge the admitted semantic universe.

Source review also found a POPF/RF wording conflict, resolved in the following
selected-source correction rather than silently importing modern behavior.

## POPF RF Selected-Source Correction

Original 1992 Intel486 PRM 11.3.1 exempts POPF/POPFD from successful-completion
RF clearing. Its instruction entry at 26-235 explicitly says POPF does not
affect RF/VM. The selected embedded-family 273021-001 section 4 likewise
exempts POPF. T559's accepted flags contract already selected preservation.
Public 325383-078US POPF note 1 instead says RF clears; it is not evidence
that this later rule applies to the selected historical SX candidate.

Therefore remove S6's unsupported two-line selected-SX RF clear and its
two-line test expectation. Existing permission masking and the single debug
completion owner already preserve RF. Extend the existing POPF completion
test to SX: 16 RF cases and 32 TF cases total with retained families, checking
independent old/popped values, word/dword stack advance and actual step traps.
The existing VME matrix still varies incoming RF and now verifies preservation
through its low-IOPL virtual POPF path. No new state, API or model is added.
This is correction to the established source contract, not an accuracy downgrade.

Both widths pass five affected tests, rebuilt/relinked against the current
CPU library. Logs: `build/t560-s6-<width>-rf-source-regression.log`.
Other R2/exception obligations and final full-S6 qualification remain open;
these focused tests do not qualify production 486 admission or Windows 3.1.

## VME Map, Stack And Six-Mode Fetch Boundaries

The existing failed-fetch table now runs in real, protected CPL0--3 and VM86
modes, retaining both code sizes, prefixes and 16 control combinations.
There are 49,152 configured cases, including 2,048 MOV CR/DR ModRM positions
not fetched because the known privileged opcode already rejects nonzero CPL.
These cases strictly require no injection, no dependent data/write/port effect,
resident architectural shutdown through the intentionally absent IDT, and full
CPU checkpoint preservation. The remaining 47,104 require an actual injected
provider failure and unchanged CPU, no dependent effects or FPU command.
The first extension incorrectly assumed every encoded suffix was required;
inspection of the existing permission owner corrected the test's boundary,
not production privilege order. This phase adds 20 lines to that test.

The existing VME fixture adds 256 valid bitmap-edge cases: first/last vectors,
bit7-to-next-byte boundaries, relocated maps and maximum 16-bit I/O-map base,
four IOPLs and four prefixes. The full 32-byte map lies in the inclusive TSS
limit. Every bit except the selected vector is set, so a wrong byte or bit
cannot accidentally pass through zero-filled storage. Source: archived Intel
253669-087US 22.3.3/Figure 22-5 and the actual instruction algorithm.

Another 160 cases exercise SP 0--7/FFFE/FFFF, word-push wrap versus a single
word crossing the segment endpoint, exact FLAGS/CS/IP storage and preservation
of ESP's non-addressed high half in the guest. Source: original Intel486 PRM
23.7 item 7. Invalid word crossings require actual SS delivery, not accepted
stack writes. The normal and fault paths use the same existing CPU owner.

These cases exposed a real selected-SX privilege-entry bug: with guest ESP
A5A50001, the 32-bit kernel gate loaded only SP from its TSS and retained the
old high half. Original PRM INTERRUPT-FROM-V86-MODE explicitly assigns full
ESP from TSS.ESPO (printed INT instruction algorithm); B controls subsequent
implicit stack addressing, not this full saved-pointer load. The existing
32-bit interrupt owner now publishes full ESP for SX VM86-origin entry,
without a new helper, state or public API. Earlier profiles retain
their existing behavior pending full inherited review. The 64 hardware-route
cases also now use nonzero guest ESP high bits and verify full saved frames.

The same-shape sweep found call-gate32, outer RET and outer IRET publications
at the existing CPU owner. Their exact operand/TSS-width distinctions and
SX direct proof remain mandatory R2 work before S6 acceptance; they are not
declared fixed, transferred outside T560, or hidden by this IRQ repair.
The separate direct VM86 IRET path already assigns full ESP. Malformed TSS
classification is also not proved by these valid-map tests.

Both widths pass final debug/fault/decode/VME cohorts 4/4, with current CPU
objects linked, in `build/t560-s6-<width>-vme-boundary-regression.log`.
No full-unit/integration qualification, new EXE, partial P or desktop proof
is claimed. R2/R3 source and remaining entry-priority review, then R7 final
qualification remain open.

## Gate Width And Original eSP Contract Reconciliation

The same-shape publication sweep does not justify blindly replacing every SP
assignment with ESP. Original 1992 PRM instruction notation defines `eSP` by
the current stack descriptor B bit. The chapter 26 CALL, outer RET and outer
IRET algorithms use `SS:eSP`; the VM86 interrupt algorithm
instead explicitly loads full `ESP` from TSS.ESPO. Keep the distinct original
contracts, rather than importing later generic full-ESP pseudocode. Existing
outer IRET, RETF and immediate RETF success tests now run on SX as well as
386, checking nonzero incoming high ESP, B-selected return and unchanged frames.

New actual 32-bit call-gate tests reuse the outer-return fixture: 32 SX cases
over caller code size, operand/address prefixes, target-stack B and zero/two
parameters. They check exact IP/CS, full old ESP/SS, copied arguments and final
pointer. Their initial failure was GP(50) for a legitimate 16-bit CALL to a
32-bit gate, not a stack publication failure. The old dispatcher chose a gate
handler from the calling instruction's operand width. PRM 24.4.2.1 (24-6)
explicitly assigns width to descriptor type 4 or 12, allowing an unmodified
16-bit caller to use a 32-bit gate. The existing SX dispatch now selects from
that already-read descriptor (+3/-1 source lines); the existing 16/32 handlers
remain the sole implementations. Earlier profiles retain their regression
behavior for the mandatory inherited-corpus review, not a new hardware claim.
No descriptor reread, wrapper, state or public interface is introduced.

The outer-return test changes +78/-9 lines in this phase. Normal 32-bit gate
coverage is not all 16-bit-gate/TSS-width/failure combinations; those remain
explicit inherited/entry review obligations, not assumed passed from this table.

The VME test also adds 512 protected same-CPL IRET cases: CPL0--3, independent
incoming/frame VIF/VIP, both code sizes and all operand/address prefixes.
Only 32-bit CPL0 returns restore the frame's VIF/VIP; nonzero CPL and word
returns preserve old high flags. Exact returned CS/IP/SP and complete flags
are checked through the real CPU. Source: archived public Intel IRET algorithm,
restricted to the already selected VME feature contract. Production flags code
already passes; no permission workaround is added.

Both widths pass six final transfer/debug/fault/decode/VME tests linked to the
current CPU. Logs: `build/t560-s6-<width>-gate-iret-flags-regression.log`.
R2 task-word flags/virtual-pending priority, remaining R3 malformed-map source
classification and entry-priority review, and R7 whole qualification remain
open. No complete S6 P, new release or Windows/desktop qualification is claimed.

## Word Task Flags And Instruction-Boundary Virtual Pending

The existing word-task completion matrix now includes SX across direct JMP,
CALL, task-gate CALL and IDT task gate. It varies old/new TF, old RF, all 16
combinations of old AC/ID/VIF/VIP and reserved/fixed-bit incoming word images:
1,024 SX cases plus 48 retained older-family cases. It verifies canonical
incoming word flags with no stale high flags, source-dependent NT, no premature
step trap, unchanged CR4.VME and exact outgoing FLAGS in the old word TSS.
Production already passes through the existing loader; only the existing test
changes (+25/-6). Source: original 486 task-state save/load format and canonical
FLAGS contract; no parallel task state is added.

Intel 253669-087US 22.3.2 explicitly states that simultaneous VIF/VIP at the
beginning of an eligible instruction generates GP. A first partial reading
of the preceding STI/POPF paragraphs wrongly suggested there was no global
check; an uncommitted local removal was immediately reverted after reading
the following paragraph. The CPU keeps its original source-mandated boundary
check; neither that erroneous deletion nor its expectations enter delivery.
This is not evidence for suppressing the check to make a test pass.

Add 64 NOP cases across IOPL, VME enable, code sizes and prefixes: 24 eligible
inconsistent states require actual GP with complete VM86 saved frames; 40
non-eligible states execute unchanged except IP, with no writes. The existing
flags matrix retains global GP plus the instruction-specific STI/POPF checks.
Hardware-route tests now include VIP, with 104 valid NMI/INTR combinations;
the 24 inconsistent eligible pre-instruction cases are covered by the explicit
GP test instead of being misclassified as successful IRQ entry. The VME file
grows by 35 lines in this phase. There is no net production change here.

Both widths pass seven current-library task/debug/outer/fault/decode/VME tests.
Logs: `build/t560-s6-<width>-word-task-pending-final.log`. These supersede the
temporary cohort that ran during the incorrect guard-removal experiment.

Read-only reference investigation located Bochs 2.6 compatibility-source
`cpu/cpu.cc` (SHA256
`3C0327B54395A1CB3E64ACD7713BDB3FE659136C5FB688588B6A4CA9C8AC807D`),
which independently implements the pending boundary, and `cpu/vm8086.cc`
(`EFCE9A489674B1AF36E401C6775967CB3ECC30E78676022BC3B1C8C9E8CC3CDB`),
whose short-header/out-of-limit bitmap guards use GP. 86Box revision
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, clean during inspection,
`src/cpu/x86_ops_int.h` hash
`F92E7105D6EAE56FDDA44730CD5315F308302E0B73D77EC4661FB2598B3E5262`,
uses a different truncated offset calculation. These are behavior references,
not silicon authorities or imported/translated implementation. They do not
resolve all malformed-map classifications; that R3 qualification remains open.
The PC110-QEMU research checkout contains a nested older QEMU tree and did not
supply a matching VME implementation in the inspected target files.

No external source, protected input, Shared-six or MyNES file is changed.
Remaining malformed-map/source/entry review and final R7 qualification still
precede a complete S6 P/release. No full CPU or desktop support is claimed.

## Malformed Map Guard Regression And Evidence Grade

The public VME algorithm defines valid bitmap placement and redirection, but
does not specify every malformed cached-TSS configuration examined here.
Do not infer exact silicon behavior from a guard or from another emulator.
The existing short-header and selected-byte-out-of-limit GP guards have the
hash-pinned Bochs reference described above as L2 support. Rejecting an I/O
base below 32 is a conservative whole-map guard, not a demonstrated fitted-SX
exception rule; the different 86Box offset arithmetic is not silently adopted.
No production guard changes in this phase and no additional fallback is added.

The existing VME failure fixture now covers five guard inputs: TSS limit 66,
selected redirect byte beyond limit 83, and bases 0/1/31. Four IOPLs, four
prefixes, valid/invalid guest stack and independent stack-write/IVT-read failure
injection give 320 cases. Each proves actual GP/error/full VM86 frame, zero
guest-stack writes, no injected later access and unchanged sentinel bytes.
The short TSS still includes the ring-zero stack fields, permitting direct GP
delivery rather than confusing the original guard with a nested entry failure.

This adds 46 lines to the existing test file, with no new fixture, production
state, public interface or exception path. Tests qualify the declared guards
and failure ordering; they do not upgrade the two evidence grades above.
Invalid cached-TR validity/type inputs and exact silicon classification remain
part of the source/entry reconciliation, not newly proven by these five rows.

Both widths pass seven final task/debug/outer/fault/decode/VME tests using
current CPU objects. Logs: `build/t560-s6-<width>-map-guard-regression.log`.
Full S6 actual-diff/coverage review and R7 units/integrations/artifacts remain
required. No complete P, new release or desktop qualification is claimed.

## Same-Level Hardware Entry And Publication Scope Review

The existing same-level alignment table now has four real event sources:
software INT, UD fault, INTR and NMI. Gate word/dword size, four stack
residues, AC on/off and four prefixes give 256 cases. The existing fixture
adds an interrupt line/acknowledgment counter and two provider callbacks;
all former callers retain a clear line. INTR is acknowledged once, NMI never
uses that provider. Valid entries verify exact three-field frames and untouched
guard bytes. Misaligned entries verify actual kernel AC delivery, zero user
stack writes and complete error/IP/CS/flags/old-SP/SS frames. Hardware entry
saves the completed NOP's following IP, while software/fault entry retains its
own restart boundary. These added paths pass without a production AC exemption.
This phase changes the existing alignment test +25/-1 lines relative to its
previous S6 form; no additional fixture, test target or production path appears.

Actual production-diff review also narrows the previous full-ESP repair to
selected-SX VM86 origin. Original PRM ordinary protected INT algorithm uses
`new SS and eSP`; only INTERRUPT-FROM-V86-MODE explicitly loads full ESP.
Applying the new full-width condition to every protected source was broader
than its evidence. The condition now tests the already-owned vm86_origin bit.
The existing IDT fixture accepts a profile, and its small-stack test runs on
386 and SX with incoming ESP=12348800, proving preserved high bits and exact
saved old ESP in ordinary protected entry. The VME corpus proves the distinct
full-width load with nonzero high bits. The IDT file changes +18/-14 lines;
no public interface or state is added. This is a source-scope correction,
not justification for keeping an untested extra path.

Both widths first passed the eight-test IRQ/alignment/task/debug/transfer
cohort in `build/t560-s6-<width>-same-cpl-irq-regression.log`. After narrowing,
the x64 IDT/VME/alignment three-test cohort passed. A whole-repository unit
checkpoint completed in the original two release/cache graphs, logging to
`build/t560-s6-<width>-full-unit-checkpoint.log`. Both builds and full unit
aggregates returned success: 602/602 tests per width. CTest elapsed time was
242.10 seconds for x64 and 65.23 seconds for x86. These are observed terminal
results, not inferred from the focused groups. The aggregate included three
desktop-labelled units per width; it does not qualify manual desktop/RDP
operation or the external integration groups. No integration, release or S6
acceptance is claimed by this checkpoint. Source/entry reconciliation, final
actual-change review and eight applicable artifact/integration groups remain.

## Final Source And Writer Reconciliation

The preceding progress sections are chronological, not independent remaining
task lists. The R1--R7 inventory is reconciled below against the current code;
the final qualification job is separate from the earlier 602/602 checkpoint.

Actual review removes the extra `iomap_base < 32` rejection. The public Intel
map description locates the selected byte relative to the I/O base; it does
not justify rejecting every low base before calculating that byte. The one
existing unsigned offset calculation and inclusive TSS-limit check now own
this boundary. The hash-pinned Bochs reference above supports this 32-bit
arithmetic as an explicit L2 choice for malformed placement, not an exact
fitted-SX silicon claim. The 86Box truncated-offset variant is not mixed in.
This removes two production lines, with no new helper, state or interface.

The existing map-edge table now uses literal expected offsets rather than
copying the production arithmetic. Fifteen rows across four IOPLs and four
prefixes give 240 cases, including first/last bits, relocation, maximum base,
three selected-byte underflows and four low-base references whose selected
byte is in bounds. These distinguish the removed blanket rejection from the
selected-byte model. The former low-base guard test is retained only where
its selected byte really is beyond the limit. Normal-map semantics remain
source-defined; malformed arithmetic remains L2, not a newly invented L3 rule.

The existing failure table also covers invalid cached TR, busy 16-bit TSS and
available rather than busy 32-bit TSS. Its seven rows give 448 cases over IOPL,
prefix, guest-stack denial and later provider failure. Four rows deliver actual
GP with a usable kernel stack; the other three cannot service the subsequent
task faults and reach the existing shutdown owner, with zero memory writes,
unchanged CPU state and a shutdown diagnostic rather than a fictitious GP
completion. The first test expectation incorrectly prohibited any diagnostic;
the existing ExecFinal shutdown notification resolves that fixture error.
These cache-state probes prove the declared guard/nesting model, not a claim
that public Intel documentation specifies every malformed hidden-cache input.

Outer IRET now has direct selected-SX virtual-flags proof in its existing test:
three source CPLs, four old and four new VIF/VIP images and four return forms
give 192 cases. Source CPL0 can load the new bits only from the dword frame;
nonzero CPL preserves them and IOPL. Existing high-ESP, stack-B, frame,
segment, GPR and earlier-profile assertions remain. The VME test separately
adds 256 CPL0 IRET-to-VM86 cases over old/new virtual bits, four IOPLs, code
size and address prefix, checking the full nine-field frame, complete ESP,
real-mode segment state and CR4. Neither addition changes production code.

| Row | Current disposition and direct evidence |
| --- | --- |
| R1 | CR4 guest/debug/reset/privilege/LOCK and required-fetch matrices are present; earlier guest rejection remains. The private validator and actual CR4 field are the only write rule/state. |
| R2 | Reset, guest task-word/dword load, POPF, real/VM86/same/outer IRET and debugger writers are reconciled. New outer/to-VM86 cases complement the existing task and same-level matrices. Prepare-entry's distinct input mask permits only ordinary FLAGS, so it cannot bypass VIF/VIP or reserved-bit rules. RF uses the pinned original 486 contract, not conflicting later-family wording. |
| R3 | Valid map/stack/segment/page/provider/fetch/hardware routes have direct proof. Malformed selected-byte arithmetic and cached-TSS guards have the declared bounded L2 model above; exact fitted silicon is not inferred. No second offset, interrupt path or fallback remains. |
| R4 | GD/all registers/aliases, RF/TF, execution/data breakpoint, SS shadow, required fetch and failed-entry paths run through the existing DB owner. Trap entry changes to that owner preserve the earlier profiles' external-error and no-error-frame behavior; SX alone clears GD. |
| R5 | Existing alignment_access_order, immediate-fetch and system-reference tests own segment/page/AC ordering; the expanded same-level table owns INT/UD/INTR/NMI stacks. The finite classification table and actual GP/PF/DF/provider-failure cases cover the carried exception batch, not a claim about every inherited instruction's entry combination. C15's whole inherited corpus remains S8. |
| R6 | Full selected-SX ESC/WAIT modes, controls, prefixes, LOCK and required fetch are covered. Actual Core NM delivery is tested. Inspection of core/x86/cpu_bus.c confirms wait/escape/command callbacks are mode-independent: CPU alone owns CPL/VM86/EM/TS/MP decisions. No additional Core mode mirror or duplicate tests are needed. |
| R7 | Full final unit job is running in the original graphs. Final gates, eight profile/width integration and release groups, complete-P delivery and coordinator actual-change acceptance remain required. |

The final source count is +199/-56 (net +143) across the three changed CPU
files, including removal of the extra low-base guard. The private flags
helpers replace the former instruction-local mask; the VME helper owns real
INT redirection; the eligibility predicate serves its actual instruction
callers. No public interface, persistent-state field, manager, execution loop
or Shared-six change is added. Positive code implements previously absent
selected-SX semantics, not an alternative compatibility path. Existing compiler
warning exceptions are not represented as exemption-free strict compilation.

The offset cohort passes 3/3 per width in
`build/t560-s6-<width>-map-offset-regression.log`. The final TSS/outer cohort
passes 3/3 per width in `build/t560-s6-<width>-outer-flags-regression.log`.
The later to-VM86 addition has passed x64 directly and is included in the
running full dual-width job. Its terminal results must be recorded separately.
The original 602/602 checkpoint is not used as proof for these later changes.
S7--S10 remain mandatory; selected CPU qualification is not Windows or PC110
runtime admission. No external source was copied, translated or imported.

## Qualification Gate Finding: Core Test Ownership

The first x64 specialized-gate run rejects private `executor_cpu` access in
machine_fpu_interface_smoke.c. That fixture explicitly owns public neutral
Core construction and must not silently become an internal CPU-profile test.
Its earlier passing runtime checks did not discharge this static boundary.
The initial specialized aggregate therefore failed; the current release job
continues collecting the eight independent integration groups, not declaring
the entire S successful after that failure.

The correction restores that public FPU test byte-for-byte to the admission
baseline. The actual selected-SX Core proof moves into the existing internal
core_machine_executor_run_smoke.c, already classified as owning Core's actual
CPU lifetime and execution binding. It covers eight ESC groups and WAIT over
all sixteen EM/TS/MP/NE combinations: 144 cases. It checks the real NM frame,
preserved GPRs/flags, handler HLT, and ordinary WAIT/NOP progress when NM is not
required. Core is still constructed through the admitted 386 route, then its
actual owned CPU is selected only inside the internal unit, as in the existing
cache integration tests. This neither opens production 486 admission nor adds
a public/test API, constructor, fixture wrapper, source file or registered test.

No gate script or whitelist changes. The unchanged CPU lifecycle gate reports
all 44 owners valid and the unchanged shape gate reports its 135 constructor
sources valid after the move. The initial manual invocations used relative
inputs incorrectly; the registered absolute argument forms then passed. These
are static results, not runtime proof for the newly located unit. Both widths
must rebuild and rerun the final whole-unit suite after the current integration
job finishes; only tests, their manifest and evidence change during this move,
so integration/artifact results from the unchanged production source remain
applicable. The failed x64 specialized aggregate must also pass in full before
delivery. Future batches run the relevant ownership gates before whole units
to catch this class before spending the full qualification budget.

The relocated test required an explicit core/x86/debug_interface.h include;
its first strict compile rejected implicit declarations rather than accepting
an accidental transitive dependency. This also appeared in the concurrent
x86 specialized build before the include repair. The header is now explicit;
the x64 Executor/public-FPU cohort rebuilds and passes 2/2, with the original
public fixture unchanged. No production or gate modification is used to make
that compile succeed. Both specialized aggregates still require final reruns.

The complete x64 specialized rerun then finds one pre-existing Jcc source-shape
assertion left behind by S5's shared LOCK transaction ownership. The existing
gate expected transaction_begin immediately after the observe-only return;
the actual bus correctly places it inside `if (!locked)`. Only that assertion
is updated (+2/-1), retaining the exact observe-only direct return before any
real transaction. All other preview, target-lexeme, timing and evidence checks
remain. A multiline sweep of cmake/nxvm finds no second instance of the old
observe/transaction adjacency. The unchanged production bus is inspected;
the standalone corrected Jcc gate passes. This is not a runtime/cache fix,
not a waived gate and not proof that the complete aggregate has passed.

## Final Implementation Delivery Candidate

The final test-ownership snapshot passes both complete specialized aggregates
and 602/602 full repository units per width. Logs are
`build/t560-s6-<width>-specialized-owner-final.log` and
`build/t560-s6-<width>-whole-owner-final.log`. Unit elapsed times are 47.93 s
for x64 and 50.64 s for x86. The relocated Executor test is rebuilt on both
widths, not inferred from its earlier x64 selection. The restored public FPU
fixture is byte-identical to admission. No lifecycle/shape gate is relaxed.
The final indexed new VME file also passes the global Types vocabulary gate.

All eight release/integration groups pass once against unchanged production
inputs. Logs use `build/t560-s6-<width>-<profile>-integration.log`:

| Profile | x64 | x86 |
| --- | --- | --- |
| default-pc-at-80386-1440k-hdd | 35/35, 55.29 s | 35/35, 67.44 s |
| ibm-5160-model-268-360k | 15/15, 29.95 s | 15/15, 39.48 s |
| ibm-5170-model-339-1200k | 17/17, 47.20 s | 17/17, 58.00 s |
| compaq-deskpro-386-model-40-1200k | 17/17, 131.19 s | 17/17, 172.41 s |

These are 84 executions per width, including the existing twelve MyNES
registrations repeated by each profile aggregate; they are not 84 unique
PC cases. The first wrapper exits nonzero because it retains the two initial
specialized failures. That exit is not relabelled as success: the separately
logged final full specialized reruns discharge those failures, followed by the
final whole-unit runs above. Production files do not change during the test
ownership correction, so the eight integration groups are not unnecessarily
repeated. Both original Ninja/ccache graphs are restored to default. The final
x86 specialized pass relinks its default artifact; its deployed identity/INI
test is rechecked, 1/1 in 0.46 s, without claiming another complete boot group.

Eight stripped Release artifacts retain version 0.5.0560. The existing PE
verifier accepts 8664/014C as appropriate, and section inspection finds no
compiler debug/stab sections. The original release job compares all four
adjacent INIs and the MyNES EXEs before/after and finds unchanged bytes;
final Git comparison independently confirms the excluded paths are unchanged.
The implementation commit containing this record identifies the source and
test snapshot; these hashes identify its final deployed bytes:

| Artifact under assets/ | SHA-256 |
| --- | --- |
| nxvm/nxvm_0_5_0560_x64.exe | 70C241907441F901E7DF876461CA2B666A0E1F599092FD11BF4519DB56D1CD76 |
| nxvm/nxvm_0_5_0560_x86.exe | 7192239139067A09C79B8D6FE57DFD9F3189260CB578B42A1EC26802A6046AD7 |
| my5160/my5160_0_5_0560_x64.exe | 711FC384AEC9B21460CA46714BF1912C1CD9A4475578543369DB292CC87B7CB4 |
| my5160/my5160_0_5_0560_x86.exe | A36F3A5DF9BEFE5A7BFC292C7502F63E70E4113AF59580A10F3F89CD048AC389 |
| my5170/my5170_0_5_0560_x64.exe | EC59D54F46519B5E61B2C2939A5D88741CFFEF70EA36A3E6E420AF5CE2026C8D |
| my5170/my5170_0_5_0560_x86.exe | 7E465ED60083B9F4C7D22FB814BBD4330F810D14930EB0FCD431FA89704DDBE1 |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | BF6CC411CC53044CF3154F57E9D4FC2521440D88207902B916D1460F06155C11 |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | BCCE651299F5F6CD24F6E97B9BDD6C6E17A5BC05EEB712A5581C2AE5F0EA0305 |

Actual source/test counting uses the Git numstat against admission, plus the
1127-line new VME file; documentation, manifests and EXEs are excluded. CPU
production is +199/-56, net +143. Tests are +2171/-110, net +2061. The owned
Jcc gate is +2/-1, net +1. Total counted code is +2372/-167, net +2205.
The added code supplies absent semantics and direct table-driven proof, not
a manager, state mirror or compatibility route. Changed private helpers and
their distinct responsibilities are reviewed above; public APIs and persistent
state fields are unchanged. Existing compiler-warning exceptions are retained,
not represented as all-source exemption-free strict compilation.

Final source review keeps the selected-SX instruction tables, one flags owner,
one DB/exception dispatcher, one TSS offset and one Core executor. Earlier
families retain their branches and regressions. Source and test Core manifests,
Core dependency boundaries, whitespace and documentation structure pass. This
is executor self-review of the complete candidate, not coordinator acceptance.
The following P must include every changed source/test/gate/document and all
eight artifacts; no partial implementation commit is an admitted delivery.

R1--R6 are reconciled at the stated primary-source/L2 grades, and R7 runtime,
gate and artifact proof is now present. Complete-P push and coordinator actual
change review still precede S6 acceptance. Exact fitted PC110 stepping and
malformed-input silicon behavior are not promoted from the declared models.
No manual RDP/desktop interaction, complete Windows support or complete 486SX
CPU qualification is claimed. C11/C12 SMM/power, C15 whole inherited semantics,
C16 timing and C17 production admission remain in mandatory S7--S10.

## Coordinator Actual-Change Acceptance

The one-session coordinator reviews pushed implementation
926bfe1855ef9959a0c1b2519ff9c0f7b37b3382, rather than accepting the executor
report alone. Actual source, test, gate, manifest, document and artifact changes
are reconciled with the original request, active packet and R1--R7 inventory.
The review accepts the single flags/CR4 rule, real VME redirection, unified DB
entry, source-scoped gate/ESP handling and SX absent-FPU semantics. It checks
that the old instruction style and earlier-family branches remain, private
helpers have distinct responsibilities, accepted effects are not replayed,
public FPU proof is restored, internal Executor proof has its actual owner,
and the Jcc gate retains its observation/transaction ordering assertion.

The review also checks original-source versus L2 grades, the finite test
tables and failure outcomes, final whole-unit/specialized/integration results,
all eight artifact hashes, PE/Release identities and unchanged excluded inputs.
No partial group, initial failed wrapper, compile-only result or manual RDP
assumption substitutes for the final proof. The three remaining strict-matrix
deferrals and existing warning exceptions are not relabelled as passed strict
source. No Shared-six, MyNES, App source or owner-input change is admitted.

The coordinator accepts P1 and closes S6 through the subsequent purely
governance P2. Its document edits change no executable input; the eight P1
artifacts remain current without another rebuild. T560 remains open with
S7--S10 mandatory, and production 486 admission is still closed. No CPU
obligation is moved out of this task to manufacture task completion.
