# T546 S4 Admission Versus Next Fetch Design

## Baseline And Scope

Continuation against accepted S3 governance 97c5028e2, Shared 15a2399d0 and
NXVM artifact/evidence d875a72fe. The owner's automatic existing-owner boundary
permits CPU/Core/IBMPC and corresponding tests, not Lib/Common/MyNES, firmware,
new public interfaces or silent source/timing exceptions. No implementation
change or runtime acceptance occurs at this inventory checkpoint.

This consumes the complete T544 admission/next-fetch receiver, not only an
ordinary page-end NOP. Original T544 cross-family/386 findings and S2's source
decision distinguish instruction completion from attempted subsequent fetch.
S2 deleted _s_test_eip/_s_test_esp and their generic post-body invocation;
S4 still owes the whole retained-transfer and publication proof.

## Existing-Owner Inventory

Owner approval on 2026-10-06 permits appending L2_CONTROL_MODEL to the
existing timing-origin enum, preserving every previous value and signature.
All accuracy upgrades are automatically approved; downgrades still require
owner approval. Missing next-instruction decoding uses one nominal opcode
component plus the source-backed transfer base, explicitly L2 rather than the
unallocated one-tick fallback. Available decoding retains its original exact
formula. This applies to all three next-lexeme consumers, including both Jcc
forms; no new public function/type or speculative architectural fetch occurs.

Read/search cpu_instructions.c for every _kma_test_logical/_kma_test_access,
_s_test_cs, call/jump/return/interrupt/gate/task target and zero-byte stack
reference. Trace init/lookahead/preview and timing consumers into Core's one
retirement/fault publisher. No new dispatcher or public access surface is
proposed.

| Access-preflight owner | Baseline line | Target context |
| --- | --- | --- |
| _ser_call_far_call_gate_32 | 3367 | 32-bit call gate |
| _ser_call_far_call_gate | 3488 | Word call-gate branch |
| _ser_int_protected_16 | 3739 | 16-bit interrupt/trap gate |
| _ser_int_protected_32_outer | 3910 | Outer 32-bit interrupt/trap gate |
| _ser_int_protected_32_same | 4004 | Same-level 32-bit interrupt/trap gate |
| _ser_ret_far_outer | 4144 | Outer RET |
| _ser_jmp_far_call_gate | 4261 | Gate JMP |
| _s_task_validate_code_selector | 4423 | Incoming task code cache |
| _ser_iret_protected_outer | 5282 | Outer IRET |
| _ser_iret_protected_same | 5361 | Same-level protected IRET |

Direct regression mapping for these ten changed owners:

| Owner | CPU matrix receiver |
| --- | --- |
| _ser_call_far_call_gate_32 | TRANSFER_GATE_CALL/code32 and inner dword CALL |
| _ser_call_far_call_gate | TRANSFER_GATE_CALL/word and inner word CALL |
| _ser_int_protected_16 | TRANSFER_INT16/TRAP16 and inner word INT |
| _ser_int_protected_32_outer | Inner dword INT |
| _ser_int_protected_32_same | TRANSFER_INT32/TRAP32 |
| _ser_ret_far_outer | TRANSFER_OUTER_RET |
| _ser_jmp_far_call_gate | TRANSFER_GATE_JMP |
| _s_task_validate_code_selector | Four TRANSFER_TASK outgoing/incoming format combinations |
| _ser_iret_protected_outer | TRANSFER_OUTER_IRET |
| _ser_iret_protected_same | TRANSFER_SAME_IRET |

All positive paged rows preserve the current transfer and attribute subsequent
PF to the target; mapped counterparts prove non-mutating timing lookahead.
Inner gate tests qualify the absent-target seam, not all inner-frame rules.
The shared logical-only helpers still own valid selector/cache/limit admission.

- _s_test_cs and ordinary near/far helpers already check logical targets only.
  Keep legitimate source-defined target limits and verify no page preflight.
- Ten code-target access preflights remain: source baseline lines 3367, 3488,
  3739, 3910, 4004, 4144, 4261, 4423, 5282 and 5361. Resolve each caller's
  descriptor/mode/phase against original sources before changing it.
- Descriptor, TSS, old/new stack and real frame accesses are different from
  a later executable-byte fetch. Their access checks must not be globally
  changed to logical checks.
- Outer RET already leaves restored ESP plus immediate unchecked; retain and
  prove that boundary instead of inventing another stack-endpoint check.
- Ordinary EIP/ESP setters, completed POP/RET adjustments, conditional branch
  decisions and the next attempted fetch require separate state/fault/context
  observations, not a test that only notices eventual handler entry.
- Preview and control-transfer timing lookahead must not generate the next
  fetch's architectural fault/A-D effects or obscure an unresolved source
  timing input. New/unupgradable L1 or false-tier changes require reporting.

## Original Source Leads And Dependency Limits

Fresh original-page verification precedes implementation. Existing source
identities are preserved in S3 design and the T544 family records. Relevant
leads include 286 programmer section 7.4.2 (last valid byte/prefetch), 286
hardware section 3 (prefetch stops at code limit), and 386 section 6.5/RET
(restored outer ESP is checked when subsequently accessed). Do not treat this
lead list or previous green tests as fresh S4 source proof.

Fresh S4 rendering/visual inspection confirms two applicable original pages:

- Intel 80286/80287 programmer reference (1987), SHA-256
  AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460,
  PDF 139/7-13 section 7.4.2: the limit identifies the last code byte;
  speculative prefetch itself does not produce the code-limit fault.
- Intel386 DX programmer reference (1990), SHA-256
  9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1,
  PDF 166/6-18: restored outer ESP plus the immediate adjustment is not
  checked against the destination limit until a subsequent stack operation.

Use only the relevant paragraph: the earlier 286 page's expand-down prose
does not supersede the explicit empty-range source reconciled in S3. The
386 table's repeated Return-CS error-code cells remain the existing S11
source-reconciliation receiver, not authority for this target/fetch seam.
Fault return addresses, paging/task phases, every target instruction and the
time/preview consumers still need original-page and code reconciliation
before implementation. Ignored renderings remain under build/t546-s4-research;
no original or derivative manual enters the committed corpus.

Fresh 386 original pages 185/7-9, 206/9-2, 217/9-13 and 224-225/9-20--21
are rendered and visually inspected. They distinguish pre/post-switch error
contexts, fault versus trap return locations and paging's actual memory
references. The CALL algorithm on PDF 358-360/17-40--42 checks the destination
descriptor/offset and publishes CS:EIP; it does not add a target-byte read to
the transfer's operand/frame operation. Read-only Bochs branch_far32 likewise
validates the code limit, loads CS and sets EIP; it is corroboration, not copied
implementation or independent timing authority.

The existing control/stack next-term evaluator requires a decoded following
lexeme. Its failure allows family/compatibility fallbacks; a missing fetch
page can therefore expose an existing source-unallocated timing path. The
raw component and Core publication cases must distinguish this from an
early architectural PF. Do not claim unknown m as L3 or keep an incorrect
target page preflight to avoid the allocation issue. L2 upgrade/source
classification remains under whole-mechanism review before implementation.

A new code-owned paged transfer prototype is registered for near/direct-far/
call-gate JMP and both code widths, checking completed target state/PTE and
the subsequent PF context. It supplies its own GDT/IDT/page tables/bytes,
never external ROM/media. This initial subset is diagnostic, not the whole
ten-owner/context proof. The first build invocation cannot find the new target
in the old Make graph; an explicit configure followed by that target build
now runs against the preserved cache. No production source or product changes.

Gate/frame widths, descriptor rights/publication, task incoming context,
serial exception delivery, complete paging/RMW admission and timing outcomes
retain S6/S9/S11-S13/S16-S17 owners. S4 qualifies the target/next-fetch seam,
not those whole mechanisms. Every discovered dependency must remain in the
full task ledger with its named receiver; no first-failure-only disposition.

## Verification Boundary

Implementation checkpoint, not closure: ten code-target calls now retain
logical checks without an early target-page access. The next-term helper owns
the approved missing-decode model; both 386 Jcc consumers reuse it. The selector
resets each candidate's origin and preserves a successful L2 classification,
preventing failed candidate metadata from classifying a subsequent candidate.
Existing timing-origin values are unchanged (compile-time compatibility=11
assertion); the new origin is appended. No Lib/Common/MyNES inputs changed.

The expanded code-owned x64 targeted test passes on 2026-10-06: twenty paged
cases (near/direct-far/gate JMP and short/near Jcc, both code sizes, absent or
mapped target) and four 286/386 next-decode segment-limit cases. Absent targets
commit before the subsequent PF, with target fault EIP/CR2. Timing preview
leaves target PTE A/D, CR2 and diagnostics unchanged. Mapped MOV AX/EAX has two
components; its exact timing differs from the one-component L2 by one tick.
286 complete MOV uses three bytes and differs by two ticks. Known-decode paths
retain their original origin, and neither model path is source-unallocated.
Source/test x86 manifests and the shared test Types boundary pass at this
checkpoint. This does not qualify remaining gate/interrupt/return/task seams,
complete units, product binaries or final external integration.

Further x64 expansion passes forty paged contexts by adding same-level 16/32
interrupt/trap gates and same-level call gates. Call-gate test width matches
the current operand width: the initial word CALL through a dword gate exposes
the already inventoried operand-versus-gate-width selection defect. It remains
an explicit S11 receiver, not claimed repaired or made legal by the fixture.
The gate/interrupt target seam passes without modifying any actual frame
preflight. Expanded x86 qualification also passes. The prior three targeted
CPU tests (decode admission, address spans, initial transfer seam) passed on
both widths after relinking against the modified CPU library.

Code-owned owner-local matrices cover all applicable families/widths/modes,
ordinary endpoint retirement, logical invalid target versus unmapped valid
target, transfer/frame effects and subsequent fetch attribution. Current-frame
and descriptor failures remain current-instruction events. Preserve existing
assertions unless original sources prove the setup/oracle wrong.

Next checkpoint: x64 passes 72 paged contexts plus four next-decode limit
contexts. New cases exercise same/outer IRET and RET, and all four outgoing/
incoming 16/32 TSS combinations on 386, each code size and mapped/absent target.
16-bit incoming tasks select a 16-bit stack: their SP is usable, whereas an
ESP built from the architecturally unspecified high half cannot be assumed
valid in a 32-bit stack. The initial 16-bit-TSS/32-bit-stack fixture correctly
failed during later PF delivery and was corrected, not used to weaken paging.
Full task privilege/admission/publication rules remain S12 dependencies.

Fresh original PDF 396/17-78 and 466/17-148 were rendered and visually read
using the PDF skill. The same pinned 1990 manual defines same-level protected
IRET=38, outer IRET=82 and outer RET=69, with no m term in these rows. S4's
whole next-term sweep found the existing same-level IRET erroneously added m;
it now returns exactly 38. This is an automatically approved accuracy repair,
not a downgrade or a missing-fetch L2 estimate. Tests independently require
38/82/69 for both target-page outcomes, without a source-derived oracle.
Same-level RET retains 32+m; available decoding remains exact and missing
decoding uses the approved L2 estimate. No whole CPU timing qualification is
implied by correcting this specific row.

The 72+4 CPU contexts and existing protected-IRET/cross-width-task state tests
pass on both x64 and x86 after relinking the modified library. A Core-owned
extension to the existing instruction-timing test also passes both widths:
short JMP from reset reaches the final legal CS offset containing an
incomplete MOV. Its successful retirement publishes exactly one observation,
L2_CONTROL_MODEL, unavailable next-components and eight guest ticks. Only the
subsequent attempted decode delivers GP at target EIP=FFFF, with zero new
retirements/ticks and elapsed time still eight. The initial test incorrectly
looked for a terminal first_fault; actual valid vector delivery belongs to
last_delivered_exception, now checked explicitly as GP rather than ignoring
the exception. No new production publisher or CPU/Core API is introduced.

Remaining before S4 qualification: inner-privilege gate seams, explicit
negative logical-target/current-frame checks and ordinary unused ESP/EIP
endpoint proof reconciliation; complete final-unit rebuild/run, static gates,
eight optimized stripped artifacts and original 58 external contexts. These
targeted results do not constitute S4 closure or a delivered P.

Final targeted x64 checkpoint adds six inner-privilege CALL/INT seams: the
source CPL3 cache and descriptor, gate DPL, TSS SS0/ESP0 and user page rights
are explicit; completed target CS/CPL/SS is observed before later PF. Matching
CALL operand/gate widths retain the separately documented S11 dependency.
Fourteen logical-target negatives (near/far/gate JMP, gate CALL, INT, RET,
IRET, both code widths) still deliver current-instruction GP; two actual IRET
frame-limit failures still deliver SS. Target PTE/CR2 remain untouched by
those logical negatives. Five-family legal NOP-at-limit cases with unused
out-of-range ESP retire normally. Together this is 103 CPU-owned contexts;
the Core publication case adds one separately owned context, not another
chip-only declaration of Core correctness. Major positive transfer forms use
named enum values rather than opaque numeric selectors. Full double-width
build and qualification now follow this checkpoint; no prior test executable
is treated as current solely because its previous run passed.

Fresh visual PDF 398/17-80 and 468/17-150 confirm the IRET/outer-RET
algorithm's logical code limit and actual stack checks before CS:EIP load;
these pages do not authorize speculative target-byte access. Full operand,
descriptor, conforming-code and later-privilege cleanup obligations remain
their existing S11 receiver, including the original source-conflict entries.
Both active full build caches are Release with REPOSITORY_BUILD_MYNES=OFF.
The original MyNES 0043 hashes remain unchanged at build admission. Source
and test manifests, Types and diff whitespace checks pass after the final
targeted expansion. Full builds are live, not complete verification evidence.

Earlier actual-change self-review (before the later INT/task sweep) recorded
the four production paths total +33/-27,
net +6 lines relative to the accepted baseline: ten in-place access-to-logical
replacements, one enum append, candidate-origin preservation, one shared
next-term estimate and removal of IRET's incorrect m addition. There is no new
execution/preview/publication owner and no second source model. Counted
source/test/build paths (excluding manifests, docs and generated assets) total
+560/-28, net +532, including the 469-line new CPU matrix and the existing
Core test extension. The increase supplies 103 chip contexts and one Core
publication context, not additional production layers. All eight current
corpus manifests pass. Full builds continue; these counts and manifests are
not substitutes for final run evidence.

Complete final units on both widths, corresponding corpus/manifest/Types and
specialized gates, all actually affected stripped 0546 PC pairs and original
58 external contexts precede delivery. MyNES and every INI/media master remain
unchanged. No P or S4 acceptance is asserted by this design inventory.

Later full-source review, while the initial builds remain live, finds two more
false m consumers: protected 386 INT3/INT and taken INTO. Fresh visual PDF
391/17-73 confirms fixed 59/99/119 clocks for same/inner/VM86 transitions;
none has m. Both producer branches now preserve those exact values. Additional
INT3/INTO same/inner tests extend the planned CPU matrix to 119 contexts;
they are not yet accepted runs. Current four-file production count is
+39/-35, net +4; counted source/test/build including the 492-line CPU matrix
is +589/-36, net +553. These supersede the earlier count checkpoint.

The two currently live builds started before this last correction. Even if
they finish successfully, a subsequent incremental rebuild is mandatory
before full unit/gate/artifact/integration proof. New model source hash is
76C0DB7B2428CE975643EA39CE8E8C637FDD87B9EBBE48ADC8FF88A563EE76CC.
The INT task-table note explicitly says approximate: task-matrix source/tier
reconciliation remains the complete S16 receiver and is not certified L3 here.

The same term sweep identifies three task CALL/JMP consumers (two immediate
and one FF indirect branch) that also appended an unsupported m. Fresh visual
PDF 358/17-40 and 404/17-86 show ts and 5+ts, with the same exact two-by-three
task-format table and separate task-gate columns. A private CPU-owned table
now selects outgoing TSS format, incoming TSS format/VM state and gate route;
the three consumers share it and no longer preview a next instruction. The
four existing protected direct-task regressions independently expect
285/285/310/392 clocks, unchanged by target decode availability. Remaining
task transition semantics, approximate INT task clocks and remaining IRET task-context rows retain
S12/S16 receivers; this is not whole-task qualification.

Latest production count is +70/-47, net +23 across the same four files;
source/test/build count is +623/-48, net +575 including the 495-line CPU matrix.
The current model source hash is
D0246946078DAA02347D232F6C0470A829F0CE37CEFFBBBD41C7A8FFA95CF96F.
Earlier build starts and targeted passes are diagnostic checkpoints only;
final incremental rebuild and the expanded tests remain required.

During the final incremental build, its logs confirm the latest CPU library
and cpu_transfer_boundary target have rebuilt on both widths. That already
finished, pure-memory target passes x64 (0.15 s) and x86 (1.00 s), covering all
119 CPU contexts against the current D0246946 model hash. Its generated logs
are x64-transfer-targeted.log and x86-transfer-targeted.log in the qualification
directory. The complete builds are still live; this early bounded probe is
not a substitute for the subsequent full 537-unit run per width.

Four already rebuilt owner-local state regressions also pass both widths:
cpu_idt_privilege_entry, cpu_software_int_state, cpu_protected_iret_state and
cpu_task_switch_cross_width_state. Logs are x64-control-targeted.log and
x86-control-targeted.log; totals are 4/4 in 0.43 s and 1.59 s. These check
collateral state effects on the changed transfer owners, not their complete
later S11/S12 contracts or the pending whole-unit requirement.

Exit-predicate review identifies a missing direct case for the already retained
outer RET immediate rule: restored ESP+8 exceeds its new stack limit, but the
return and following NOP must succeed because neither accesses that stack.
Two code-size cases are added using only the existing paged fixture and original
PDF 166/6-18 authority. Production remains frozen. The CPU test now has 121
planned contexts/540 lines; counted source/test/build is +668/-48, net +620.
After the live builds terminate, the modified CPU test target must be rebuilt
before full unit; prior 119-context runs do not prove these added cases.

Both final-source full incremental builds finish with exit zero. After they
terminate, the CPU probe is rebuilt separately for the final two cases and
passes all 121 contexts: x64 0.14 s and x86 0.83 s. Final-transfer build/run
logs are retained per width. Model SHA remains D0246946; final CPU test SHA is
E3851340866B2AACB4C2465E5737E55067231F0CEDAC3FB661098850479C8D53.
Both CTest registries contain 537 unit tests. The complete x64 aggregate is
now live with four workers and the unchanged 300-second bound, preserving
its detailed CTest log; x86 follows sequentially. No complete unit result
or product artifact acceptance is asserted yet.

First complete x64 unit run completes all 537 tests in 238.51 s: 536 pass,
only machine-80386-timing-manifest-runner fails. Logs are x64-unit.log and
x64-unit-detail.log. Its IRET detail reports run/source=38, not a production
fault; CS/EIP stay zero only because the failed expected-tick check short
circuits before filling the snapshot. The receiving oracle still expected 39.

The whole receiving oracle sweep changes 28 numeric lines without deleting
cases or changing handlers/structure: three protected same-IRET expectations
39->38; same/inner/VM86 INT observations 60/100/120->59/99/119; direct/gate and
indirect/gate task expectations 394/403/399/408->285/294/290/299. The task
fixture's two 2B-limit, type-81 descriptors and 44-byte incoming image are
16-bit TSSs; operand-size prefixes do not change that fact. Original 17-40/86
tables give 285 or 294, with 5 extra clocks only for indirect forms. This is
manual/fixture reconciliation, not adopting observed output as the oracle.
Other valid CALL/RET m additions and VM86 IRET=60 remain untouched.

Corrected x64 receiving runner passes in 5.33 s. Its existing approximate INT
task and remaining task-IRET context assumptions are still explicit S16 debt;
passing this runner does not qualify those old values. The first full failed
run is preserved. Both receiving targets are rebuilt, and complete x64 units
are rerunning with the original four-worker/300-second policy before x86.
Counted source/test/build is now +696/-76, net +620 (the receiving change is
28 added/28 removed lines); no further production edit was needed.

Corrected complete x64 units pass 537/537 in 117.63 s. Logs are
x64-corrected-unit.log and x64-corrected-unit-detail.log; the original failed
full run remains separate. x86 corrected receiving runner passes in 8.98 s;
its complete 537-unit aggregate now runs under the same j4/300-second policy.
Production remains D0246946 and no oracle case/count was removed. Full gates,
eight actual PC artifacts and all original 58 external contexts still precede
implementation delivery and acceptance.

Complete corrected x86 units pass 537/537 in 221.41 s, within the unchanged
j4/300-second bound. x86-unit.log and x86-unit-detail.log preserve all results.
Both final unit handles are terminal. Specialized/artifact-root/INI gate
aggregates now run per width; supplemental checks and actual eight-product
and original external qualification remain pending.

Earlier build checkpoint: both initial full builds returned exit zero but
preceded the final INT/task correction. The subsequent final-source build
logs are x64-build.log and x86-build.log. Their later successful exits and
the complete unit outcomes are recorded above; the initial exits alone did
not qualify the final source or any of the eight external product contexts.

First specialized aggregates fail only T435's old origin assignment-count
predicate: its '= ' substring also counts the new '==' comparison and assumes
the former two sites. The checker now distinguishes assignment from equality
and checks three assignments, specifically two UNATTRIBUTED resets and one
successful origin assignment. One selector/one Core timing invocation/one raw
retirement publisher and board-input prohibitions remain unchanged. A standalone
check passes; corrected full aggregates are running. This changes the receiving
cmake/nxvm gate, not Shared production or unit behavior. The CMake MATCHALL
patterns omit terminal semicolons to avoid counting list delimiters as matches.
Latest counted code/test/build is +705/-78, net +627; the initial failed gate
logs remain separate from corrected runs.

Both corrected full specialized/artifact-root/INI aggregates return exit zero,
recorded in x64-corrected-gates.log and x86-corrected-gates.log. T435 now passes
without reverting source. The printed T345 duplicate-row fatal message is an
intentional negative self-test followed by its successful self-test marker,
not an aggregate failure. Actual product build starts at default x64 and
33 supplemental checks start on x86; other products/external contexts remain
pending. An artifact-root pass verifies placement, not current executable
source identity, which is still owed separately for all eight PC products.

Actual vm-0-5-0546 default x64 product target finishes successfully and deploys
the architecture-verified x64 PC EXE; default-x64-product-build.log records it.
x86 supplemental checks finish 33/33 in 382.98 s (layout self-test 382.87 s),
under their existing per-test rules, not the separate unit aggregate deadline.
x64 supplemental and AT x64 product/harness build remain live. Default x86
product build begins only after its supplemental process exits. All receiving
caches still use Release and REPOSITORY_BUILD_MYNES=OFF. No actual-media or
eight-product acceptance is claimed from this one product's success.

x64 supplemental checks pass 33/33 in 270.93 s (types-layout self-test
270.77 s), completing both width batches. Default x86 product target also
finishes successfully; default-x86-product-build.log records its architecture
check. Deployed default pair is independently read as PE machines 8664/014C
with no .debug/.zdebug-named sections; SHA-256 x64 is
2A4C8C439BE490F0C05070776338F37DDCD8000E8641C6FF1CD2025315B2DFEC,
x86 24D31344F125123FF0C48D406287CD69DD1871BB44FC7969E103E51468E0EEF0.
AT x64 product builds successfully after its preserved Make regeneration
(294.4 s); the requested boot harness continues building in that same command.
AT x86 generation remains live. These are producer/placement facts, not
external boot acceptance; no original external context has been rerun yet.

Both AT product/boot-harness commands return zero. AT x86 generation takes
156.2 s; its deployed EXE is architecture-checked by the target. Current AT
SHA-256: x64 B916B6894FDEDF86CA94EAAF1EC0EAA9E97FF71A7E839B962ADC1DA6E713A92D,
x86 8CE9985125C9BB89F4CF2376FE15E87C720B3E0D615B16B2223E7E689059A72B.
AT console/CMOS integration targets are explicitly rebuilt as well, so every
one of its three original external contexts uses current linked code. XT
product/harness builds now run per width. Owner INIs and MyNES remain
unchanged; integration starts after final product/harness qualification.

AT console/CMOS integration builds finish on both widths, with separate
at-x64-integration-build.log and at-x86-integration-build.log. XT x64 product/
harness command returns zero; XT x86 compiles and both Model40 builds remain
live. Immediately before external qualification, all five FDD/HDD master
hashes match the accepted S3/S2 baseline, and neither owner INIs nor MyNES
production/assets are changed. A newly observed unrelated MyNES Queue and
proposal edit is preserved and excluded from this CPU task's commit targets.
This prevents an eventual CPU delivery from claiming the whole repository is
clean by deleting or sweeping another product's concurrent documentation.

XT product/harness builds finish with exit zero on both widths, recorded in
xt-x64-product-build.log and xt-x86-product-build.log. Six actual PC EXEs are
now rebuilt; Model40 x64 compiles and Model40 x86 generation remains live.
No live build is restarted for an observation timeout or lack of immediate
output. Final PE/stripping/source and external-run proof waits for these
last two products and their three integration targets per width.

All eight deployed 0546 PC EXEs are now generated. Independent PE reads find
8664 for every x64 and 014C for every x86 filename, with no .debug/.zdebug
section names. Model40 final integration links remain live; no external run
is accepted yet. The same-width instruction/model objects match across all
four caches, corroborating a sole compiled CPU owner rather than stale
profile-private copies. Instruction/model object SHA-256 respectively:
x64 B67B8E8DE638454842AB3E4A6420490D39B3457DAC78C798840F833448F03A89 /
8F3F146126E721E7D6215665CCA76EF75D3A2B8ACC9DC6EEC29A4F04936B52A4;
x86 57FFF6AAEF48677631D2DC68598314C5286DD1C77FA1E6B4D7E1CC26F560F752 /
4237329B58A9BC9FA525C52647F025A6EC9A8B8FFF6F630138C55548FEA3264B.

| Product | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT | 7A301CB238BB28C9802AC444238EC2CAEFEBCC7F17941FEDA4AFEDCFEC230009 | 67BB8659D808049C996FFBFCC23C8D65786CCE2BADB4CEA07E537A4BD3B0220F |
| AT | B916B6894FDEDF86CA94EAAF1EC0EAA9E97FF71A7E839B962ADC1DA6E713A92D | 8CE9985125C9BB89F4CF2376FE15E87C720B3E0D615B16B2223E7E689059A72B |
| Model40 | ACB7CFC7BE1DE83804DBF670E6C540471779BA6F53BBB0683FB068E4149A3085 | 45298A6012E58367D54C95B0C41905206D6DD5365E12E55F0AD8633FAC81C6FE |
| Default | 2A4C8C439BE490F0C05070776338F37DDCD8000E8641C6FF1CD2025315B2DFEC | 24D31344F125123FF0C48D406287CD69DD1871BB44FC7969E103E51468E0EEF0 |

Both final Model40 commands return zero, including product, floppy boot,
console lifecycle and CMOS seed executables. All owned build handles are
terminal. Original external qualification begins sequentially at default
x64 (22 contexts), through RunTestAggregate with j4/300 seconds and unchanged
case checkpoints/per-test allowances. Each final-production group runs once;
logs and detailed CTest logs are copied before another group reuses the cache.
No original external context is deleted, replaced by a unit or inferred from
an executable hash. Runtime qualification remains pending until actual runs.

First original external group passes once: default x64 22/22 in 19.56 s,
retained as default-x64-integration.log and detailed log. Default x86 now
runs sequentially with the same INI/media/checkpoint and j4/300 policy. No
passed group is sampled again and no remaining context is inferred passed.

Default x86 passes 22/22 once in 22.05 s, with default-x86-integration and
detail logs. Thus 44/58 original contexts are accepted so far; AT x64's three
contexts now run sequentially. No input or checkpoint has been revised.

Both AT groups pass once: x64 3/3 in 34.62 s, x86 3/3 in 45.85 s. Original
5170 boot checkpoints, console lifecycle and CMOS seed predicates remain
unchanged. Corresponding at-width integration and detail logs are retained.
Accepted original external count is now 50/58; XT x64 runs next, without
repeating any successful default or AT group.

XT passes its original 360KB boot context once per width: x64 1/1 in 19.80 s,
x86 1/1 in 26.90 s. Retained xt-width integration/detail logs prove the same
unchanged INI/media checkpoint. Accepted original count is 52/58. Model40
x64's three contexts now run, preserving its original allowance; x86 follows.

Model40 x64 passes 3/3 once in 126.74 s, boot case 126.17 s inside its
unchanged allowance. The separate console/CMOS cases pass as well. Accepted
original external count is 55/58; final Model40 x86 three-case group is now
live. The successful logs are preserved; no configuration or timeout change
was used to obtain the result.

Final Model40 x86 passes 3/3 once in 152.54 s, boot case 151.73 s inside
the unchanged allowance. All original 58/58 external contexts now pass:

| Group | x64 | x86 |
| --- | --- | --- |
| Default | 22/22, 19.56 s | 22/22, 22.05 s |
| AT | 3/3, 34.62 s | 3/3, 45.85 s |
| XT | 1/1, 19.80 s | 1/1, 26.90 s |
| Model40 | 3/3, 126.74 s | 3/3, 152.54 s |

Every named build/test handle is terminal. Final eight manifests and test
Types check pass; all five external master hashes and the exact MyNES pair
still match their accepted baseline. INIs, Lib/Common and MyNES production/
tests/assets are untouched. Unrelated MyNES documentation remains preserved.
Executor actual-diff self-review reads four production changes, full added
CPU fixture, Core publication extension, receiving numeric-oracle changes,
stronger seam predicate, manifests/build registration and product identities.
The complete admitted batch is mapped to original pages and all ten owner
regressions, not accepted merely by a smoke or boot result. No new executor,
preview/paging owner, product branch or public function/type is introduced;
only the explicitly approved origin enum append changes the public surface.

S4's target/fetch seam and source-backed m-versus-ts selection are ready for
target-separated delivery. Full stack/gate/task/paging/exception/RMW and the
remaining approximate INT task/IRET/task-clock qualification remain their
named S6/S9/S11-S13/S15-S17 receivers. This is not T546/CPU/physical-time
closure. Coordinator post-P actual-change acceptance is still required.

Shared implementation P1 2d9929742 is committed and immediately pushed to
origin/master. Its nine paths are only src/test x86; the public change is the
approved appended timing-origin value with every old value preserved. NXVM
receiving checker/oracle, task evidence and eight artifact paths are the
separate P2 target. Unrelated MyNES documentation is excluded, and no force
push, history rewrite or destructive worktree cleanup occurs.

Coordinator post-delivery review reads the actual immutable Shared P1
2d9929742 and NXVM P2 ca016635d diffs, not just these result tables. Changed
source/test bytes match their reviewed commit objects and final run sources.
The original request and sixteen-field S4 packet map to the ten caller
matrix, negative admission/real-frame cases, endpoint/RET-immediate rules,
non-mutating preview, explicit L2 origin, exact fixed/task clocks, Core
retirement/time publication and all receiving proof. All named exit markers
have direct evidence; S6/S9/S11-S13/S15-S17 residual contracts remain explicit
and are not certified by this seam closure. Review accepts both delivered P
targets. Pure NXVM governance P3 closes S4 and removes its active packet;
T546 stays open. Only unrelated MyNES documentation remains in the worktree.
