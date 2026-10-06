# T546 S1 Reset And Architectural Images

## Baseline And Source Review

The owner subsequently approves this concrete S1 modification on 2026-10-05.
The read-only baseline and review plan below remain historical; implementation
now changes only the approved CPU owners and their matching regressions.

Read-only Shared review against 966249c20; no CPU repair/test/manifest is
changed. This consumes the complete reset/image receiver of T544 and the
[T546 ledger](../../history/M5-T546-cpu-audit-gap-repair.md). Sources remain
external in the owner archive; original SHA-256 freshly matches:

- Intel 80186/80188 User's Manual 1985, 210912-001:
  2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF.
- Intel 80286/80287 Programmer's Reference 1987, 210498-005:
  AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460.
- Intel 386 DX Programmer's Reference 1990, 230985-003:
  9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1.
- Intel 80286 Hardware Reference 1987, 210760:
  D3ECE037A200B17EF32D78A055D0D96C3E47474D755D94569B9576A9A68C6915.
- Intel iAPX 86,88 User's Manual 1981:
  3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5.

Fresh page renders visually inspected: 186 PDF 247 / 2-81 Table 2-30;
286 PDF 183-184 / 10-5..10-6; 386 PDF 232-233 / 10-2..10-3 Figures
10-1/10-2 and Table 10-1; 386 PDF 285 / 14-7 item 12. The first 286
render showed the preceding control table, so PDF 184 was rendered/read
instead of inferring reset values from a wrong page. Hardware PDF 117 / 3-67
Table 3-6 confirms CS base FF0000, first fetch FFFFF0 and FFFF segment limits;
its IDT FFFF differs from the programmer 03FF, so preserve T544's explicit
programmer-selected disposition rather than treating one table as a new defect.
Ignored research scratch
stays under build/t546-s1-research for this active batch.

Fresh 286 PDF 329 / C-3 explicitly covers 8086/8088 compatibility and confirms
early FFFF:0000 and the 286 FF0000 base through the first far JMP/CALL.
Fresh 1981 PDF 119 / 2-100 gives portable flag-image masking advice; it does
not contradict the 386 compatibility paragraph's explicit early high-nibble
image observation. The 186 reset table corroborates its F002 image. These
source-conditioned image rules do not assert every historically undefined bit.

## Concrete Existing Owner Plan, Awaiting Review

| Fact | Existing mechanism | Proposed repair/proof |
| --- | --- | --- |
| Early reset entry | cpu.c reset_code_base and state_reset always emit F000:FFF0 | Select FFFF:0000 and FFFF0 base for 8086/8088/186; later F000:FFF0 stays. Preserve every physical first fetch. Update both context and opaque-instance matrices, not an App alias. |
| 286/386 CS cache | Sole reset assigns unlimited limit | Set documented FFFF limit; keep DS/ES/SS existing 64KiB default. Defined first-fetch/CS-relative boundary proof required; undefined fields get no hardware-exact claim. |
| 286 MSW | State memset clears low controls; SMSW exposes raw cr0 low word | Keep canonical defined controls separate from reserved readout. At existing SMSW producer emit FFF0 plus low four controls for 286; 386 MSW readout unchanged. Verify reset and post-LMSW register/memory readouts. |
| FLAGS incoming versus saved image | Separate helpers still share writable-defined mask | Keep canonical load mask; source-qualified outgoing image adds the early fixed high nibble where supported by each family's evidence. PUSHF and the sole real interrupt/exception frame share this one image helper. Do not add a second state field. |
| Later FLAGS and other reset state | 286 FLAGS=0002; 386 high fourteen flags undefined; 386 CR0 defined reset bits clear with other bits unspecified | Preserve documented low flags/control fields and existing deterministic unspecified defaults without calling them L3 silicon values. Do not inject a later CPU reset constant or arbitrary stepping. |
| 286 IDT/base source reconciliation | Programmer-selected IDT limit 03FF retained; software initialization text gives real-address view, hardware reset fetch uses high ROM | Do not silently switch to hardware FFFF IDT or change correct high ROM base. Original hardware page reconciliation remains required before full S1 proof. |

Conceptual changes use the existing owners only:

```c
reset(context):
    clear_existing_cpu_and_execution_state();
    early = family_is_8086_8088_or_186();
    CS.selector = early ? 0xffff : 0xf000;
    EIP = early ? 0 : 0xfff0;
    CS.base = existing_family_reset_base_with_early_base_corrected();
    CS.limit = 0xffff;
    initialize_existing_defined_fields();

flags_load_16(flags):
    return (flags & existing_family_writable_defined_mask) | 2;

flags_image_16(flags):
    return flags_load_16(flags) | source_qualified_family_image_bits;

SMSW:
    result = (family == 286) ? ((controls & 0x000f) | 0xfff0)
                             : existing_386_result;
    existing_checked_operand_write(result);
```

This is review pseudocode, not literal implementation or an additional API.
The early image classification still requires complete 8088/186 corroboration
and all outgoing consumers; one 8086 compatibility paragraph is not asserted
to settle every family. POPF privilege, IRET and task load policies remain
their distinct later receiver, not a reason to use one global permissions mask.

## Current Regression Gaps And Remaining Proof

cpu_execution_lifecycle_smoke hardcodes F000:FFF0 in its context and opaque
instance checks and only tests base/first-fetch/386 EDX, not cached CS limit.
cpu_pushf_popf_smoke masks early high image bits, so passing tests cannot
validate the specified saved image. Production SMSW has no 286 reserved-readout
selection. All are existing defects/oracle gaps, not downgraded source rules.

Core cold and processor-only reset both call the same chip reset owner.
Outgoing image helper has four uses: real interrupt/exception frames at
word/dword width and PUSHF at the applicable width/family branch. Incoming
load sites are task transitions, same/outer/VM/real IRET and POPF; keep their
existing load routing rather than install fixed image bits into stored state.
Only the outgoing-image helper and SMSW readout change alongside reset.
The existing control-state register SMSW case also encodes zero reserved bits
for 286 and lacks 286 memory success; it must be corrected with the producer,
not used as an independent oracle. CPU execution_bus uses manually assigned
old early reset fields and must follow the new per-family reset matrix.

Fresh unchanged baseline execution passes the five existing reset/FLAGS/MSW
contract tests on each width: 5/5 x64 in 0.14s and 5/5 x86 in 0.13s. Actual
assertion review above shows why these passes are not source correctness:
the early reset values and 286 reserved-bit oracle reproduce current defects,
and outgoing fixed image bits are masked out. These observations preserve the
pre-repair baseline and justify correcting tests with their owning mechanism;
they do not qualify or close S1. No executable or shared-file input changes.

Concrete Shared review is now requested for this complete bounded owner plan.
Estimated existing-file impact:
two production C owners and approximately three to six owner-local regression
receivers, with manifests; exact counts follow the complete caller inventory.
No public interface, new wrapper/framework or board-specific workaround is
planned. Implementation and complete dual-width/runtime/artifact proof remain
pending; S1/T546 are not closed and no P delivery is claimed.

## Approved Implementation In Progress

cpu.c corrects the three early families' reset selector/offset/base, and the
CS cached limit in the sole reset owner. Later high ROM bases, DEFAULT's Core
resolution to 386, defined low controls and programmer-selected 286 IDT stay
unchanged. Canonical private EFLAGS remains 0002; CPU state/Debug snapshots
keep their existing canonical representation. Guest PUSHF and saved frames
project the source-qualified fixed image at their existing outgoing helper;
they do not mirror reserved bits in stored mutable state. Incoming FLAGS load,
POPF privilege, IRET/task policies and scalar timing selection are unchanged.
The table-style SMSW handler applies 286 readout bits before its one checked
operand writer, preserving original 386 behavior and control storage.

Four existing x86 tests now directly prove five-family reset context/opaque
state and physical first instruction, fixed outgoing PUSHF bits, both software
and acknowledged external interrupt frame images, reset 286/386 SMSW and
post-LMSW readout plus register/memory destinations. No new framework, target,
public API or board test fixture is introduced. Initial changed-case runs
pass 4/4 per width (0.61s x64, 0.42s x86); full regression remains pending.
The first draft incorrectly attempted to read an address from a lexeme whose
ABI only carries lengths; actual instruction-observation address is used
instead, without extending the interface. Manifest patch generation was
corrected to apply entries in file order; both complete manifests now pass.

Task artifact version becomes 0546 through the existing NXVM recipe and only
the three NXVM product presets. Existing receiving caches must regenerate
before requesting the new target; early unregenerated AT/XT make invocations
correctly rejected unknown vm-0-5-0546, and explicit reconfiguration resolves
the tooling precondition. No old EXE is relabeled as new qualification.
All four products' x64/x86 replacements and full units/integration/gates are
in progress; MyNES's graph has no x86/IBMPC link and its artifacts stay untouched.

## Complete Receiving-Oracle Correction

The first x64 complete run executes 532 cases in 271.40s and fails nine
receivers; it is not accepted. The whole failure batch is classified and
corrected, with no additional production patch:

- CPU contract and neutral Core entry-plan tests selected 8086 but asserted
  the old F000:FFF0 image. Their real reset and failure-preserving entry-plan
  assertions now require FFFF:0000; all original entry/rollback checks remain.
- The 8086/8088 catalog and mixed-family T359 control/stack fixtures relied on
  reset accidentally establishing their chosen F000:FFF0 benchmark entry.
  They now use the existing public register-patch operation to explicitly
  select that ordinary execution context after reset (early families only in
  the mixed fixture). Branch/return/interrupt expectations and timing numbers
  are unchanged; this is unit setup, not an App/BIOS workaround.
- Board BOUND's real 186 frame oracle compared a canonical snapshot directly
  with the outgoing FLAGS word. It now includes that family's fixed image
  bits while retaining delivered exception, register, stack and handler proof.
- The 286 timing ledger expected raw CR0 rather than MSW in six register/
  direct/indexed, real/protected readouts. Source-conditioned FFFC/FFF1 images
  replace 000C/0001; its instruction clocks and control storage checks remain.
- Transaction/prefetch fixtures assumed reset CS had an unlimited cache and
  charged a third wrapped-tail prefetch. Existing ExecInit already bounds
  prefetch_bytes to CS.limit - EIP + 1: corrected FFFF reset limit yields
  initial/destination windows only. Paired transaction and explicit wait
  predicates now prove no tail beyond the reset segment, rather than preserve
  an invalid reset-context assumption. This is Core model proof, not a new
  hardware-prefetch or physical-time qualification. A legal wider non-reset
  span remains within S3's complete span/context receiver, not claimed here.

All thirteen affected reset/image/receiver cases pass per width (6.23s x64,
8.24s x86). A manual diagnostic initially selected a stale old root-level
prefetch EXE and got a false pass; registered CTest paths under test/x86 and
test/ibmpc expose the actual current failures and are the only accepted runs.
One build requested an incorrectly guessed board target; actual registration
is used on the corrected build. Neither observation weakens a test predicate.
The initially failed timing producers leave old result files, so downstream
passes from that failed run are not fresh proof. Final successful producers
regenerate all five timing result documents before accepted verification.

Fresh complete final units pass 532/532 per width: x64 103.34s, x86 290.57s,
eight jobs and unchanged 300-second aggregate budget. No failed/skipped case.
All eight 0546 products are optimized Release, have matching PE widths and no
compiler debug sections. Receiving boot and supplemental gates remain to finish.
No Lib/Common/MyNES input, INI or external media changes are made.

## S1 Batch Disposition And Final Receiving Proof

| Batch member | Direct disposition |
| --- | --- |
| Early CS:IP/base | Source-proven repaired in one reset owner; five-profile context/opaque-instance matrix and actual first instruction preserve physical first fetch. CPU contract and Core entry/rollback receivers use the corrected 8086 image. |
| 286/386 cached limits | Source-proven FFFF reset CS limit, existing defined data caches unchanged; boundary-related prefetch/transaction receivers correct their formerly unlimited-reset assumption. ExecInit's existing clip expression is read and retained. |
| Reset controls/MSW | Defined low controls clear; 286 register/memory SMSW exposes fixed FFF0 bits without storing them as CR0 controls; post-LMSW and six real/protected ledger forms prove the same readout. 386 behavior unchanged. |
| Writable versus outgoing FLAGS | Private canonical state remains distinct from source-qualified early image. PUSHF, software INT, acknowledged INTR and 186 delivered BOUND frames prove fixed image bits; incoming canonical load unchanged. |
| Other reset fields | Existing defined selectors, low flags, 386 DH identifier and first-fetch bases preserved. Tests mask source-undefined high EFLAGS bits; deterministic zeroing is not described as silicon-exact undefined state. |
| IDT/source/capability boundary | Original programmer-selected 286 03FF IDT disposition retained; hardware-table disagreement explicit, not silently resolved. 186 integrated relocation/chip-select peripherals are not represented by this instruction CPU profile and are not falsely qualified here. |

No new L1 or timing downgrade is introduced. FLAGS privilege, decode, exception/
task, span and timing discrepancies retain their complete S2-S19 receivers.
This closes only the approved reset/image batch, not all CPU functionality,
timing or physical synchronization.

All 58 original receiving integration contexts pass once, with original inputs
and checkpoints and new 0546 fixed products:

| Product | x64 | x86 |
| --- | --- | --- |
| NXVM/default | 22/22, 12.74s | 22/22, 14.43s |
| My5170/AT | 3/3, 35.07s | 3/3, 42.06s |
| MyDeskPro386/Model40 | 3/3, 59.83s | 3/3, 71.71s |
| My5160/XT | 1/1, 21.41s | 1/1, 25.11s |

All eight manifests and source/test Types/corpus checks pass. Supplemental
CTest checks pass 33/33 per width (358.92s x64, 209.43s x86); these static/
negative checks do not qualify CPU instruction timings. Both final specialized
aggregates, artifact-root/INI boundaries and documentation/diff checks pass.

All five external media hashes match the pre-S1 baseline in T545 S7 evidence;
INI files are unchanged. MyNES's two hashes remain BDE39D3B2C60CFDB6F6CCBE30650C167709D5699FBCBA271781D907C03AAF3C1
and 3B2E99A756AA583EE905329B60D66F6BEF07E03E98B22FE9CE68CEE988D89097.
After successful replacement checks, eight old 0545 PC EXEs are retired;
they remain recoverable in Git history. The asset tree has exactly ten EXEs.

| New 0546 artifact | SHA-256 |
| --- | --- |
| my5160 x64 | BBC741CF53CEBCA54DE5E9FB9365A8FD56739EEBF64FCAB02D7DC9F1982D30B3 |
| my5160 x86 | E70B4194B8001E770251A6C825ABC2C9DD05FE6A43FF2B15540F143CD7AA703A |
| my5170 x64 | 4A3A8B01E9EA5080ECB01835CC1B0246CCC8AB6CA19279FF24FE4D44098CA8CA |
| my5170 x86 | 71FAEEE9C6ADFBE04168AFAEC51F825645C6D88E110F83179289742F0D16D505 |
| mydeskpro386 x64 | CB53B64685744E55815827E030EAA8B89C0F3F47719346D12FC41107A10544F1 |
| mydeskpro386 x86 | 46497E1BF07DE08B071D504C37777A5EB8FDA46189498C90BC85D6B440279168 |
| nxvm x64 | E96824873CFD30FC160765D600B196BED86466E1729279A6612063603EE8E760 |
| nxvm x86 | 645814D28D75976B40B3E26FCA347748A94AAE76C6F27351A6090C7B3731C504 |

Counted tracked C/test bodies: fourteen paths, +121/-46, net +75 (Git numstat,
excluding manifests/docs/artifacts). Production alone: two paths, +15/-10,
net +5. Missing proof and explicit legal test entry account for the test growth;
no new entity, framework, production path or ABI. Three NXVM preset target
values and the two existing recipe revision lines change, with zero net lines.

Executor self-review inspects every changed source/test predicate against the
approved batch and original source, including all nine receiver corrections.
It confirms one reset/image/MSW owner, no new ABI, original table handlers,
owning tests, generation-qualified expectations and no weakened clock numbers.
The architecture/coding skills lead to explicit test setup and removal of false
reset assumptions rather than production compatibility patches. No live owned
build/test handle remains; caches support the next decode/admission batch.
Source originals and active task scratch remain available for convergence.

Shared code/test delivery and NXVM receiving/evidence/artifact delivery are
target-separated. Coordinator actual-change acceptance must still review those
pushed commits before S1 closure; T546 remains open for its complete ledger.

Shared implementation P1 is fc9a8b725, pushed to the existing origin/master.
Verified 0546 builds contain that exact CPU source plus the task's receiving
version declaration; hashes identify the builds independently of compiler
timestamps. NXVM delivery carries only its preset/recipe, explicit timing
fixture context, task records and the eight qualified product replacements.

NXVM implementation P2 is 912579a98, pushed. Coordinator actual-change review
accepts both deliveries and the complete requirement-to-proof map, including
source-defined versus canonical/undefined state and all receiver corrections.
Eight artifact identities and scope boundaries are rechecked against actual
files. Governance acceptance closes S1 only, with T546 and all remaining
instruction/function/timing batches open. No unsafe source exclusion or new
unupgradable L1 is introduced by this batch.
