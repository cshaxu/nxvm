# T560 S3 Alignment And Read/Modify/Write

## Baseline And Scope

Investigation baseline: 1703eb3e1, after accepted S2. CURRENT admits C05/C09;
this record is evidence, not a second admission authority. The implementation
progress below does not constitute complete S3 qualification.

Use the externally pinned Intel 486 Programmer's Reference Manual (1992),
already inventoried by S1. No third-party implementation is imported.

## Source Findings

- Section 9.9.16 requires CR0.AM, EFLAGS.AC and CPL3 together. Implicit
  supervisor accesses, including descriptor loads, are exempt even when
  caused by a user instruction. Instruction fetch is not an operand check.
- Table 9-2 places operand segment faults before alignment faults and
  alignment faults before operand page faults. Do not substitute the ordering
  of a later CPU generation without source reconciliation.
- Table 9-6 distinguishes scalar words/dwords and composite operands. The
  adjacent page 9-25 text specifically permits a stored pseudo-descriptor at
  an address 2 modulo 4. Its word limit and following dword base cannot be
  modeled as an arbitrary six-byte scalar aligned by total transfer length.
- These are source anchors, not yet a complete instruction-form disposition;
  paired operands, implicit accesses and restart delivery need direct proof.

## Actual Access Owners

`src/core/chips/cpu/cpu_instructions.c` owns segment validation, translation,
logical reads/writes, access preflight and instruction handlers. Keep that
one path. `_kma_test_access` validates segmentation and invokes
`_kma_test_linear`; the latter already performs page-table commit operations.
It is not a side-effect-free query and must not be described as one.

`_s_read_cs` identifies fetch provenance and uses forced logical access;
descriptor table access uses supervisor linear access, and TSS uses forced
supervisor logical access. The alignment predicate must respect these actual
callers rather than treating every memory transfer as guest data.

At the baseline, the exception mask stopped architectural definitions at vector 16.
Vector 17 requires a Core-only exception constant and connection to existing
delivery/error-code/restart handling. No Shared interface, second exception
dispatcher or persistent alignment state is needed. Inspect all consumers
of exception masks, not merely the instruction raise site.

## RMW Inventory Seed

Whole-function scanning for both `_m_read_rm` and `_m_write_rm` identifies:

- ADD/OR/ADC/SBB/AND/SUB/XOR: RM8-R8 and RM16/32-R16/32 forms.
- ARPL; groups 80/81/83; XCHG byte and operand-sized forms.
- Shift/rotate groups C0/C1/D0/D1/D2/D3; unary groups F6/F7/FE/FF.
- CMPXCHG and XADD byte/operand-sized forms.
- SHLD/SHRD immediate and CL forms; BTS/BTR/BTC register forms; group 0F BA.
- Group 0F 00, group 0F 01 and FPU_ESCAPE also match textually but contain
  distinct read-only/write-only branches: inspect rather than classify their
  entire dispatch as RMW.

This is a seed inventory, not a coverage claim. Inspect suboperations and
direct logical transfers as well. CMP/TEST/BT must remain read-only. Zero
shift counts, adjusted bit-string addresses, immediate fetch order, ARPL
conditional write, register operands and legacy-family wrapping require
explicit dispositions. Existing XADD/CMPXCHG preflight does not establish
all inherited-form correctness or provider-failure publication correctness.

## Proposed Change Boundary

Use a private alignment check after segment validation and before operand
paging, with explicit handling of composite operands. Use one private RMW
read boundary for true destination-modifying forms, reusing existing write
access validation before the operand read. Preserve general read semantics.
Do not blindly replace every ModRM read in mixed opcode groups.

Audit flags, register results and memory publication after validation. A
provider failure is not interchangeable with a guest paging fault; tests
must distinguish them and record accepted external side effects accurately.
No rollback may invent a reverse device write.

Initial scope estimate remains source +200--450/-100--300 and tests
+400--700/-30--100. The Core exception constant/delivery sweep is included;
refine the estimate after the complete suboperation inventory. Public-six,
MyNES, INI and external assets remain unchanged.

## Verification Status

The first production increment adds vector 17, its zero error-code delivery,
and one private alignment predicate at logical read/write/access-preflight
boundaries. The predicate uses existing profile/CR0/EFLAGS/CPL/force inputs;
no new persistent state. Pseudo-descriptor stores explicitly check their
dword base component after validating the whole segmented range.

The existing control-state executable now adds 2,048 guest MOV combinations:
386/486, CPL0--3, AC/AM, word/dword, read/write, four address residues,
segment limit failure and missing operand page. Assertions cover delivered
exception/error/restart point, stacked error, CR2, register and target bytes.
Both existing Ninja/ccache trees build and execute `core-test-cpu_control_state`
successfully (`CONTROL-STATE:OK`), including all previous control-state cases.
The first compile caught use of an unavailable AC accessor macro; the final
code uses the existing EFLAGS constant directly. No warning suppression added.

The first increment's code delta was production +33/-3, net +30; test +104/-0,
net +104, from `git diff --numstat` over the changed CPU C/H paths.
Whitespace check passes. This is focused progress, not closure evidence.

The next increment adds one private `_m_read_rmw` owner: write preflight,
then the existing read. It replaces modifying reads in ALU, exchange, shift,
unary, ARPL and bit forms. Mixed groups select this path only for modifying
suboperations; CMP/TEST/BT and indirect call/jump/push operands retain normal
read access. Four duplicated XADD/CMPXCHG preflight blocks are removed.
The original table-style instruction handlers and execution path remain.

`cpu_rmw_access_smoke.c` drives actual encoded instructions with an observing
bus fixture. It covers byte/word/dword forms, all immediate ALU and legal
rotate/shift suboperations, SHLD/SHRD, BTS/BTR/BTC, XADD/CMPXCHG, ARPL and
read-only controls. Cases include writable success, readonly destination,
nonpresent destination, readonly second page and nonwritable segment. Fault
assertions require zero target reads/writes, unchanged operand bytes and
registers, correct write/read page-fault code and restart EIP. Read-only
controls prove readonly access is still accepted. These are access-order
tests, not substitutes for existing arithmetic-result qualification.

The RMW/control-state/XADD/CMPXCHG CTest selection passes 4/4 on both widths
after the complete suboperation expansion. Both graphs are reused. Remaining
functions containing ordinary reads and writes are mixed groups and FPU
dispatch; their individual branches still require the final caller review.

The same RMW matrix additionally injects operand-read and operand-write
provider failures. Both widths pass: these remain internal provider failures,
not guest exceptions; EIP, EFLAGS and registers are restored, operand bytes
remain unchanged, and failed transfers are attempted once rather than retried.
The injected write fails before accepting data. This does not prove atomicity
for a provider that accepts a prefix and then fails, nor authorize reversing
external device side effects.

## Complete Batch Mechanism Review

`cpu_alignment_access_smoke` covers native protected and VM86 references at
four address residues: word/dword stacks and strings, scalar reads, BOUND,
16/32-bit far pointers, SGDT/SIDT, byte and LEA controls. Valid exceptions
enter a kernel handler through deliberately odd GDT/IDT/TSS addresses;
these implicit supervisor references must not generate another alignment
fault. Dedicated checks cover odd instruction fetch, user DS descriptor
loads, and unaligned CPL0 LGDT.

The paired-read owner now validates the complete segment span, both component
alignments and both pages before transferring the first component. MOVS and
CMPS validate both operands of one iteration in segment/alignment/page order
before either data read. Fault assertions cover destination-page failure,
source-page versus destination-alignment precedence, destination-segment
versus source-alignment precedence, far-pointer/BOUND second-page failure
and REP with zero count. Earlier-family segment wrapping remains in its
original owner; full translation preflight is skipped for that legacy wrap.

Variable-count scalar shifts suppress 486 writes when the masked count is
zero; SHLD/SHRD retain their already-existing conditional writeback. Counts
0 and 32 preserve operand bytes and flags, perform no write, and still reject
nonwritable destinations. Immediate C0/C1/SHLD/SHRD decode their immediate
before operand access. Real page-boundary fetch regressions prove that an
unmapped immediate produces PF(read) before a readonly destination is touched.

| Caller class | Disposition and direct evidence |
| --- | --- |
| ALU register-source and groups 80/81/83 | All modifying destination reads use the RMW owner; every legal arithmetic suboperation has permission/provider cases. CMP branches stay read-only. |
| XCHG, XADD, CMPXCHG | One preflight owner, existing result tests retained; new failure matrix proves no read/write before rejected write permissions. |
| C0/C1/D0--D3, SHLD/SHRD | All legal rotate/shift suboperations and widths covered; zero-count behavior and immediate-fetch precedence checked. |
| F6/F7, FE/FF | NOT/NEG and INC/DEC use RMW; TEST/MUL/DIV and indirect CALL/JMP/PUSH retain their separate read semantics. |
| ARPL and BT/BTS/BTR/BTC, 0F BA | ARPL uses destination-write validation even on a conditional update; BT remains read-only. Register bit-string displacement is finalized by the existing decoder before RMW preflight. PRM 26-37/26-43 specifies the actual word/dword access range; alignment uses that operand width. |
| 0F 00 / 0F 01 | SLDT/STR/SMSW are write-only, LLDT/LTR/LMSW are read/system operations, VERR/VERW are descriptor checks. They are not missing RMW callers. SGDT/SIDT and paired table reads use their distinct component layout. |
| FPU_ESCAPE | No integrated FPU is present in SX. Floating-point execution remains the separately frozen C14/FPU contract; its mixed decoder is not an integer RMW repair target. |
| Stack, strings, paired and system references | Existing logical-access path retained, with the preceding complete-span and ordering proof; fetch and implicit supervisor paths remain exempt. |

Sweep commands matched `_m_read_rm`, `_m_write_rm`, `_m_test_access`, all
direct `_kma_read_logical`/`_kma_write_logical` calls and architectural mask
consumers under `src/core`. Actual diff review inspected all changed handler
branches, not merely their names. No general read was relabeled as write;
no state mirror, execution loop, retry or reverse provider write was added.
The four private helpers each own a real check/order decision: alignment,
RMW preflight, scalar zero-count writeback and two-operand string admission.
The sole public addition is the existing Core exception-mask vocabulary's
AC constant; no type layout or Shared ABI changes.

Guest segmentation/alignment/paging failures are prevalidated before operand
data effects. A provider failure is fatal through the existing internal fault
path, with CPU checkpoint restoration. A provider that has already accepted
data cannot be undone by CPU rollback; the implementation does not claim
an external transaction or silently retry accepted data. Existing Core memory
transaction tests remain part of full qualification.

## Qualification And Actual Scope

Production and test implementation are complete. The full repository unit
aggregate passes 599/599 on x64 in 238.86 seconds and x86 in 62.65 seconds.
It covers Lib, Emulator, x86, Core and all five Apps; three desktop-labelled
unit cases per width pass. Detailed CTest logs are retained at
`build/t560-s3-<architecture>-unit-details.log`, with aggregate output in the
adjacent `-unit.log`. Both unchanged Ninja/ccache graphs are used, with no
source recompilation per test. Core source/test manifests and documentation
governance pass; final eight-group release/integration results follow below.

Counted C/H/CMake paths against 1703eb3e1, including untracked new test files:
production two files +184/-78, net +106; tests four files +677/-0, net +677
(104 control-state lines, 253 RMW lines, 318 alignment lines, two registration
lines). Docs, manifests and EXEs are excluded. The test increase provides
observable bus ordering, fault/restart and cross-mode/width proof without a
new framework; the production increase is the four necessary decisions
listed above and their handler connections.

The sole retained interpreter, page translation and exception delivery paths
are used. Old CPU-family behavior remains explicitly isolated for alignment,
scalar zero-count writeback and two-address string preflight; complete units
exercise the retained families. The owner-approved Core AC mask is the only
public vocabulary change; no state layout, Shared-six, MyNES source/test,
INI, external ROM/media or firmware change is present. No manual RDP or
desktop guest acceptance is claimed by automated qualification.

The first default x64 integration run passed 34/35 but failed the Windows
checkpoint's final BDA observation after C: and directory listing succeeded.
Its original detailed log is preserved at
`build/t560-s3-x64-default-integration-failure-details.log`. Source tracing
shows `emulator_machine_shutdown` invokes the driver's stop request;
`vm_machine_control_stop` requests Core stop, whose run boundary can cold
reset and clear RAM. Reading the boot BDA afterward therefore depends on
whether that reset completed, and the test previously ignored read failure.

The packet includes the bounded test correction: observe/check BDA while
pause is confirmed, before shutdown. Failure diagnostics likewise precede
cleanup. No Core or Shared lifecycle semantics, timeout or assertion is
weakened. Similar-read sweep under `test/app-nxvm/integration` found this sole
post-shutdown success assertion; other reads are before cleanup or diagnostic.
The corrected x64 Windows checkpoint passes 1/1 in 7.84 seconds, retaining
the original C-drive/directory/HDD-count assertion. Its log is
`build/t560-s3-x64-windows31-corrected.log`; the initial failed aggregate is
retained separately. Final eight-profile qualification now runs on the
corrected revision. Full unit results remain valid because this correction
changes no unit executable or production input.

This adds 5/removes 4 App integration lines. Combined counted test scope is
five C/H/CMake files, +682/-4, net +678. The previously stated +677 figure
describes the Core-only test portion; no manifest exists for the App test tree.

## Final Release Qualification

All eight final integration aggregates pass on the corrected revision:

| Profile | Cases per width | x64 seconds | x86 seconds |
| --- | --- | --- | --- |
| Default NXVM | 35 | 50.88 | 61.75 |
| IBM 5160 | 15 | 27.35 | 38.03 |
| IBM 5170 | 17 | 46.69 | 60.22 |
| DeskPro Model 40 | 17 | 128.25 | 166.52 |

Each final group ran once. The default x64 initial failed run and its focused
post-fix regression are preserved and not hidden as a passing initial result.
Logs are `build/t560-s3-<architecture>-<profile>-integration.log`, with
adjacent `-integration-details.log` CTest copies; profile keys are default,
xt, at and deskpro. Each aggregate includes the same twelve MyNES cases;
replays are not distinct coverage or MyNES artifact changes. Automated native
Window/Console checks pass, but manual desktop/RDP acceptance is unexecuted.
The Windows31 predicate is its existing DOS-side C-drive/directory/BDA
checkpoint, not a claim that Windows GUI was launched.

Both original build graphs are restored to default. Common objects are reused
for each App; no new build/cache graph or per-App Shared recompilation is
introduced. Eight 0560 Release binaries are rebuilt/deployed and checked for
PE width and embedded version. Adjacent INI and both MyNES EXE hashes match
the pre-release inputs; source/test Shared-six paths have no changes.

| App/width | SHA256 |
| --- | --- |
| My5160 x64 | 27E48B310794CA0193F3671FC77547BC5D54AFF50616B1580AB2926F2E0154D6 |
| My5160 x86 | 1C17DF76CD2B250D034FA4F3FCF35C04754EC4545F48F16F42C57FE87C7242DC |
| My5170 x64 | AF47A7687E4C983DEC4951E9D71931BA3F544F945EED08AF9F33476A014706CD |
| My5170 x86 | E40134A329C1B2B974611D8A3D25D099921CDFF3703C5B7BD6AF873C5391FA28 |
| MyDeskPro386 x64 | 88869262D8A650D16646D7FB70976E6AB50A6920A9F3C92D64329634F1B1325A |
| MyDeskPro386 x86 | 59305E6B222C1E7386CCD1F83EDB972D14B6A56F8E4D2CECEE1598FFDF91EF84 |
| NXVM x64 | 830F090540142204273C7C597D2056C32AC7D0852C13A5235C39E27BC424F1FB |
| NXVM x86 | 7ACB4704EA74AC32C03A710C924C67A845672CEADABEEEE4C37BC2931F03B7F0 |

Executor actual-change review covers every modified/new source/test file,
registration, manifest, proposal/ledger/packet/history and asset identity.
No obsolete RMW preflight duplicates remain in XADD/CMPXCHG. Necessary
private helpers and existing non-RMW branches have the explicit reasons
above. The delivery is ready for one complete NXVM P and coordinator review;
it does not qualify the remaining T560 CPU rows or open production 486 use.

Coordinator actual-diff review accepts the pushed cb52586a9 delivery. Every
ordinary access/RMW batch requirement maps to the direct tests, whole suites,
final integration logs and deployed artifact checks above. Source/test
increase, the four private decisions and all retained branches are justified.
S3 closes after this review; the convergence ledger retains C13-linked entry
and nesting qualification and every remaining T560 row. No complete CPU,
manual desktop/RDP or stronger Windows predicate is claimed.
