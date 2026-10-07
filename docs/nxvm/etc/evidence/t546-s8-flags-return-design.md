# T546 S8 FLAGS Privilege And Return Design

## Admission And Complete Batch

Continue from accepted S6 governance 741d00417, Shared 1db282a10 and NXVM
19945b2b9. Owner authorizes consecutive numeric S admission and existing
x86/chips, x86/core and ibmpc repairs. Timing upgrades are automatic;
downgrades, new public APIs and excluded-domain expansion require review.
Shared CPU/Core tests/manifests and NXVM four-PC receiving proof are admitted;
Lib/Common/MyNES and owner INI/media/firmware changes are excluded.
Preserve unrelated MyNES documentation. S8 implementation and qualification
are accepted after coordinator actual-change review; its closure is recorded below.
The admission and pre-fix records below remain historical proof.

Consume the complete transferred FLAGS/return receiver, not merely the first
286 CLI failure. S1's accepted reset/canonical/image distinctions remain;
S6's corrected ordinary SS and scalar stack owners remain. Original T544
B05c/B05d and each family's F04/F10/debug/return findings are incorporated.

| Member | Current owner and whole required proof |
| --- | --- |
| Generation-defined FLAGS state/image | _e_eflags_load, _e_real_flags_image_16 and cpu.c masks; preserve S1 reset/writable/readout separation. Prove early reserved/undefined fields without invented precision. |
| 286 POPF | Earlier-family branch lacks real NT/IOPL preservation and protected CPL/current-IOPL masks. Fix at the sole POPF merge, not all task/IRET loads. Prove both preservation and allowed changes, stack rejection and other defined bits. |
| 286 CLI/STI | Privilege admission exists only in the 386 branch. Reconcile real/protected CPL/IOPL, IF state, failure/IP and actual delivery; earlier CPUs remain unprivileged. Shadow lifetime stays S9. |
| 386 POPF/PUSHF | VM86 POPFD is rejected solely for 66h despite the source's two widths. Reconcile VM/RF/IOPL/IF and reserved/high-word preservation, image clearing, privilege-before-transfer and each rejected stack reference. Pre-386 PUSHF has an unchecked push call and source starting-SP requirement to reconcile. |
| IRET FLAGS merge | Actual same/outer/real/VM return helpers and task return use distinct load contracts. Reconcile incoming width, old CPL/current IOPL, reserved/high bits, VM/RF/NT and failure publication; full descriptor/gate/outer-segment geometry remains S12/S13. |
| RF fault images | ExecFinal currently adds RF only for #DB. Reconcile fault/trap/interrupt/abort classification at the existing exception/image producer; prove saved versus live/rollback state, both incoming RF values, gate widths and failed delivery. Full ordered fault pairs/shutdown remain S10. |
| RF retirement | _debug_complete_instruction exempts only CFh. Source also names POPF and actual task-switching transfers. Preserve loaded task state without exempting ordinary CALL/JMP/INT or creating a second completion owner. |
| TF prior-state completion | Current old-and-current TF predicate suppresses a traced POPF clearing TF and can confuse outgoing/new-task TF. Prove prior versus newly enabled TF, INT/INTO, IRET and actual task outcomes. Debug/NMI priority, SS shadows and comparator cause/status remain S9/S13 receivers. |

## Architecture And Source Plan

CPU owns FLAGS, instruction outcome and exception image. Preserve original
table-style handlers, one scalar pop/push owner, one canonicalizer and the
existing completion/delivery owners. A mode/privilege merge is not writable
canonicalization; do not globally mask IOPL/NT to patch POPF. Saved image and
live handler state are distinct. No shared-framework or universal undo layer.

Read original early FLAGS/interrupt definitions, 186 inherited rules, 286
POPF/CLI/STI/IRET dictionaries and privilege chapter, and 386 POPF/PUSHF/IRET,
chapters 9/12/15 and effective-mode definitions. Existing T544 records retain
original archived file hashes/pages. Fresh original-page review resolves the
RF POPF chapter/dictionary wording and VM86 IRET pseudocode conflict before
changing a disputed predicate. OCR navigates only; source contradictions
are not averaged or resolved by a green current oracle.

Initial read-only code sweep confirms the 286 privilege omissions, VM86 66h
rejection, unchecked pre-386 PUSHF, RF-only-DB image and old/current TF
conjunction. Same/outer IRET already have separate partial privilege masks;
task loaders also call the canonicalizer. Review every caller and image
producer before choosing one complete existing-owner scheme. No code, API,
source clock, manifest or artifact is changed at this admission.

## Verification And Exits

Code-owned owner-local matrices cover all five retained profiles and relevant
mode/CPL/IOPL, incoming/popped bit combinations, operand/stack widths, source
admission, accepted transfer/failure and fault/return/completion variants.
Install real code-owned handlers when delivery is the claim; do not infer it
from missing-IDT shutdown or compare undefined fields as exact values.
Retain all original assertions and correct false oracles only by original
evidence; distinguish CPU rollback from accepted provider effects.

Each admitted member and all callers receive direct source/implementation/test
proof or source-proven non-applicability. Named later frame/task/page/arbiter/
delivery/time owners are not silently qualified. Before any P: complete dual
units, applicable guards/negative tests/eight manifests/Types/corpus/gates,
all eight optimized stripped 0546 products and original 58 integration contexts
once per final group. Preserve 300-second aggregate containment, existing
checkpoint/input identity and owner INIs. Count final production/test delta
and retained paths; target-separated pushed Ps receive coordinator actual-diff
acceptance before closure. Keep required ignored research/receiving caches
through S8 and its immediate successor. No partial implementation P.

## Fresh Original Pages And Initial Direct Reproducer

Original 286/287 PRM 1987 hash
AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460
matches the retained archive. Fresh visual PDF 294/B-86 explicitly preserves
real NT/IOPL and permits protected IOPL only at CPL0, IF only at CPL<=old IOPL,
without a privilege exception for POPF. PDF 238/B-30 and 314/B-106 explicitly
require protected CLI/STI GP when CPL>IOPL. The initial 313 render is STD,
not STI, and is not used as STI evidence; the corrected original 314 is read.

Original 386 DX PRM 1990 hash
9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1
also matches. Fresh visual 454/17-136 describes word/dword POPF and VM86
IOPL<3 rejection, with VM/RF unaffected. Its reversed bit names are a prose
typo, not a new layout. Fresh 208/9-4 and 265/12-7 require RF in the saved
fault image, distinct from arbitrary interrupts/traps. Page 265's grouped
POPF/IRET/task-transfer RF wording remains to reconcile against the specific
POPF contract before a production choice. Fresh 266/12-8 expressly uses prior
TF, excludes an instruction merely enabling TF, names INTn/INTO and new-task
first-instruction behavior. Original renders stay ignored, not published.

Add one independent 64-context matrix to the existing pushf_popf owner:
all eight original NT/IOPL states by all eight popped states in 286 real mode,
while checking defined other flags, actual pop/SP/IP, unchanged GPR/segments
and no fault. Both strict target builds succeed, and both widths return the
expected failing test status: 56 changed-privilege combinations fail, the
eight identical-state controls do not. Example original zero/popped IOPL1
incorrectly yields flags 1CD7 instead of preserving privilege bits.
The test asserts only the defined 7FD5 mask and uses code-owned bytes, not an
external ROM or a duplicate CPU model. Production and all products remain at
accepted S6; this is direct pre-fix proof, not an implementation or S8 closure.
The test manifest is refreshed. Render and reproducer handles are terminal.

## Privilege And Completion Reproduction

Extend the existing protected POPF fixture by its actual CPU profile, without
changing its bus or publishing a test API. The independent Cartesian matrix
uses four CPLs, eight old IOPL/NT combinations, eight popped IOPL/NT
combinations, both old/new IF values and each supported operand width. The
286 has 1,024 contexts: 624 fail on both hosts. The 386 has 2,048 contexts:
all pass on both hosts. Old CPL and old IOPL, not the popped IOPL or a new CPL,
determine the permitted IF/IOPL replacement. Other defined bits, successful
pop/SP/IP and unchanged register/segment participants are checked separately.
These results support a POPF-local 286 merge correction, not a global change
to task/IRET canonicalization.

The existing VM86 fixture now includes POPFD and IOPL 0/1/2 rejection for
the applicable forms. At IOPL3, POPFD reaches the installed GP handler at
IP=0100h, SP=8FD8h instead of consuming four bytes and completing at IP=2,
SP=8004h. Both hosts reproduce that incorrect prefix-only rejection. The
existing 32-bit PUSHF success case and existing rejection/frame assertions
remain; the complete independent stack/admission sweep is still pending.

Add direct completion matrices at the existing debug-state test owner.
Eight RF contexts cover word/dword POPF and both incoming/popped RF values:
four fail on each host because successful POPF incorrectly clears old RF.
Eight TF contexts cover both widths and both prior/new TF values: two fail
on each host, precisely the prior-TF-set/new-TF-clear cases. An actual
code-owned vector-1 handler, its saved return IP, delivered-exception snapshot
and DR6.BS prove delivery; a missing IDT or terminal fault is not the oracle.
The initial compile referenced the production-file-private DR6.BS macro;
the test instead uses the independently specified 4000h field, like its
retained assertions, and both strict builds then succeed. No public exposure
is introduced to accommodate a test.

Fresh visual 386 17-141/PDF459 confirms both PUSHF widths and VM86 IOPL
admission. Fresh 8086/8088 2-33/PDF52 confirms the defined POPF state and
word pop/SP order while marking image fields undefined in that edition;
retain S1's separately corroborated image contract. The 386 17-136 explicit
POPF contract says VM/RF are unaffected. Chapter 12-7 explicitly exempts POPF
from automatic retirement clearing. Their common concrete requirement is
to preserve old RF for POPF; its grouped statement about loading saved RF
cannot override the POPF-specific unaffected rule. IRET and actual task loads
remain distinct saved-state operations. No timing value or accuracy grade is
changed by this interpretation.

Read all current same/outer/real/VM IRET flag merges and both task publication
implementations. The outer word return lacks upper-word preservation; same
word return preserves it but reads a partly written local. Old privilege is
captured before return publication. The 32-bit same interrupt producer does
initialize oldcpl; it is not an uninitialized-CPL defect. Task transitions have
two real publication paths, so a completion decision cannot exempt CALL/JMP
by opcode alone or inspect only one task loader. Complete source/image/fault
and task-outcome reconciliation is still required before production changes.

All reproducers above are pre-fix evidence, not passing verification. The
CPU archive, eight PC products, Lib/Common, INIs and MyNES remain unchanged.
No S8 P or closure is eligible. A slow whole-book pypdf navigation attempt
was stopped after confirming its owned Python identity; no text from that
unfinished extraction is used as source evidence. All owned research/test
processes for this round are terminal. Keep original page renders and receiving
caches for the remaining S8 proof and its immediate successor.

## Deferred By Owner-Directed Efficiency Insertion

The owner pauses this uncommitted batch and moves it from S7 to S8 on
2026-10-06. No production repair or implementation P exists. Exact test edits
and their prior manifest are retained in ignored build/t546-s8-deferred,
with saved Git patches and hashes recorded by the active efficiency evidence.
Restore them only on S8 admission; do not compile intentionally failing S8
reproducers into S7's accepted-S6 performance baseline. Original pages and
caller findings remain valid research; the complete batch is not cancelled.

## Resume After MyNES T44

Owner resumes NXVM on 2026-10-06 after accepted MyNES T44. S8 is admitted
against unchanged S6 CPU/artifacts and S7 efficient build graph, working HEAD
d5ed4549c. Deferred hashes match. Restore the two test edits only; regenerate
the manifest with S8 identity, not the deferred file's obsolete S7 label.
Current MyNES is 0044 and must stay byte-identical. Historical S6 MyNES identities
remain historical; no production FLAGS repair or P exists at this checkpoint.

Restored files match the original deferred SHA-256 values 08CAF7A4... and
465F7095... exactly. Strict x64 builds of the correctly named canonical targets
x86-test-cpu_pushf_popf and x86-test-cpu_debug_state complete in four steps;
the transient focused execution again reports 56/64 real 286 privilege mismatches,
624/1024 protected 286 mismatches, correct 386 protected controls, VM86 prefix
rejection and RF 4/8 plus TF 2/8 mismatches. These are expected pre-fix failures,
not qualified units. The restored test manifest is refreshed under S8.

Original 286/287 and 386 DX PDF hashes are rechecked at the actual retained
manuals-nxvm/cpu archive. The read-only PDF skill is used for original-page
visual review. Fresh review confirms 286 B-86/B-30/B-106 and 386 17-136,
12-7/12-8, 9-4 and IRET 17-78--17-82. INT same/inner and VM frame pages are
also read before selecting an RF image versus live-state repair. Chapter 15-9
and IRET's VM exception list explicitly condition rejection on IOPL<3; the
17-78 unconditional VM branch remains a pseudocode conflict to reconcile
with the 1986 edition and independent reference before changing that path.
INC/INS pages 17-70--17-72 are not IRET evidence.

Source/caller sweep identifies the two actual task publications, all separate
IRET width/privilege merges, and distinct fault-image/rollback state. A task
completion decision cannot exempt ordinary CALL/JMP/INT by opcode. Saved RF
must not be injected into the live rollback checkpoint as a shortcut. No
concrete production patch is applied until the whole scheme is reconciled.

The first build command mistakenly used a _smoke target suffix and correctly
failed target lookup without compiling. A sandboxed target-help inspection
then stalled; its exact ninja-help child PID/parent/command was verified and
only that owned child stopped. The canonical targeted build outside the
sandbox and both bounded tests completed. No unrelated process or source was
changed, and no restart is required. Preserve research, warm caches and direct
failures for continuing S8; no partial commit is eligible.

## Existing-Owner Implementation And Developing Proof

The cohesive candidate keeps the original table handlers and canonicalizer.
POPF alone applies 286 old-mode/CPL/IOPL masks; CLI/STI include 286 privilege
admission; VM86 POPFD uses the same IOPL check as POPF. Pre-386 PUSHF now checks
the existing scalar push result. Word outer IRET preserves upper FLAGS, and
same-level word IRET initializes its partially populated local before merging.

One private per-instruction outcome marks the two actual task publication
paths. Initialize/reset/round entry clear it. RF completion exempts POPF, IRET
and actual task transitions, not ordinary CALL/JMP. TF completion uses prior
TF, except actual task entry and taken software interrupt forms; a newly
enabled TF does not trace the instruction that enabled it.

The exception producer temporarily prepares the RF fault image only for a
pending fault, not the direct incoming-task debug trap or DF abort. The
original rollback/diagnostic checkpoint is untouched. Failed delivery restores
the original FLAGS; ordinary gate entry clears live RF, and task entry keeps
its incoming TSS flags. Existing real dword frame production no longer truncates
its FLAGS image to a word. The existing word/dword geometry is not newly
qualified here; complete gate/delivery geometry remains S10/S12/S13.
Protected gate entry also clears NT at all three existing publication paths,
as the original INT operation requires. Bochs 2.6 exception.cc's separate
protected and real entry paths corroborate live RF clearing; this is read-only
behavior research, not copied code or a new timing-grade claim.

Current x64 development checks pass the complete retained POPF reproducer,
29 RF image/trap/rollback contexts, 148 CLI/STI privilege contexts with actual
16-bit GP delivery, 4,032 same/outer IRET FLAGS contexts across 286/386 and
80 actual JMP/CALL/gate/INT task transitions with independent old/new RF/TF.
The earlier conversation's 3,584 IRET count described the 386 subset; the
complete two-family total includes 448 additional 286 contexts.
Existing task16, VM86 IRET and real IRET checks also pass. These are development
results, not full-unit, external receiving or artifact acceptance.

Three obsolete oracles were corrected by the original contracts, without
removing input combinations: 286 real POPF cannot load NT/IOPL, and both POPF
widths preserve old RF. The direct FLAGS identity row now expects 286's old
privilege fields rather than successful loading of F000h. RF probe corrections
keep instruction storage away from IVT vector zero and recognize that a NOP
clears prior RF before its subsequent TF trap; they do not relax CPU behavior.

Dual-width development/build checks and the complete x64 unit dependency graph
are in progress. Full source/caller dispositions, final units, all gates,
original external contexts and eight affected PC artifacts remain mandatory
before any P. MyNES, Lib/Common, INIs, external masters and deployed EXEs are
unchanged at this development checkpoint.

The first complete x64 development sweep terminates normally in 233.65 s:
494/505 pass, eleven fail. Two registrations are the same protected-UD body,
and CLI/HLT/software-INT fixtures reuse the common interrupt/CLI receiver.
Read their actual handlers/frames, not just the test names. The affected fault
frame expectations omit RF in UD/GP/NP/SS/PF/DE and VM86 paths; IRQ, software
INT and trace-trap images remain separate unchanged controls. Correct only
the saved fault images to bit 16, not live state or diagnostic origin. The
real dword DE frame now retains its high RF image. No assertion is removed.
The historical "RF only for execution breakpoint" VM86 expectation is false
under the original 386 fault-image contract.

This sweep used the earlier candidate while further source/test changes were
being prepared; it is failure inventory, not final acceptance. Its process
tree is terminal. Do not count it as a passing final unit suite or rerun
whole units after every receiver edit. Rebuild the changed receivers and
prove their current narrow regressions first, then execute final full units.

Fresh visual original 286 PDF297/B-89 explicitly requires PUSHF shutdown at
starting SP=1; 386 17-141 specifies lack-of-stack shutdown as well. S6's
ordinary PUSH opcode rule explicitly excluded PUSHF into this receiver.
Extend that existing instruction-admission owner with the PUSHF-specific
286/386 condition and add all-five-family SP endpoint/wrap/shutdown controls.
The broader ordinary 286 PUSH/compound and ordered shutdown geometry remain
with their named task receivers; no whole-class shutdown claim is added.

## Complete Candidate Boundary Review

All eight admitted members now have an existing-owner candidate and direct
regression. Generation image rules retain S1; POPF-local privilege masks do
not change any task or IRET canonicalizer. Same/outer IRET matrices cover
4,032 cases; VM86 word/dword return independently checks RF restoration and
IOPL/VM preservation in eight cases. Thirty-two real IRET contexts check
prior TF versus loaded TF/RF, including the delivered trap's actual return
point. POPF's prior-TF matrix now covers all five families, 24 contexts, without
asserting an architectural DR6 value before 386.

Actual task completion is proven separately for both publication owners:
80 TSS32 JMP/CALL/gate/INT contexts and 48 TSS16 contexts across 286/386.
Incoming RF/TF is not outgoing-instruction state. Ten ordinary near/far
CALL/JMP/NOP controls ensure those opcodes do not acquire the task exception.
Fault-image failure controls now include the pre-decode instruction-breakpoint
failure as well as UD: both must retain old diagnostic/rollback RF. IRQ,
software interrupt, trace trap, task-switch trap and DF remain distinct.

The saved-image correction reaches every checked consumer of this mechanism:
chip/VM86, Core paging and PC UD/GP/NP/SS/DE/VM86 receivers. Existing full
assertions remain, including live flags, origin, error, stack and other registers.
The real dword image retains its old canonical low bits while adding previously
lost high bits; a temporary raw-low candidate failed DE/software-INT's reserved
bit controls and was corrected at the image producer, not by changing them.
Focused receiver retests subsequently pass. The SP endpoint fixture uses the
existing early scalar word wrap and shutdown diagnostic contract; an initial
contiguous read and fault-callback assumption were fixture errors, not sources.

No new public API, second canonicalizer/executor, timing value or accuracy
classification is introduced. No source conflict is averaged or hidden: the
VM86 IRET general pseudocode is qualified by the instruction's specific
IOPL<3 exception and Chapter 15's sensitive-operation rule. The existing VM86
admission remains; the new tests prove its width/FLAGS behavior, not every
gate geometry. Later S9-S20 obligations retain their original complete receivers.

Candidate source and tests are frozen for final qualification. Refresh complete
dual-width unit dependencies and affected receivers before execution; earlier
green development checks or the 494/505 failure sweep are not final acceptance.
Three changed source/test manifests and NXVM documentation/whitespace checks
pass; all unaffected Lib/Common/MyNES paths have an empty diff, and the exact
MyNES 0044 pair hashes still match. No P, product refresh or closure exists yet.

## Final Qualification In Progress

Frozen CPU instruction source is
BE62A9BB7D5A685BCA05D1BDC3F755E8C79438ACB96F7EA7FB1F10777F8817F9.
Final x86 repository-only units pass 505/505 in 247.58 s. All eight manifests,
changed test Types boundaries, source corpus and NXVM documentation/whitespace
checks pass. Full specialized aggregates pass on both widths after correcting
the T344 inventory's CMake-alias owner: an alias is not an exported Ninja
target. Resolve ALIASED_TARGET before recording/inspecting actual compiler
owners. All 514 current source rows and strict flags remain; no checker is
disabled, target skipped or source admission weakened. Packet records this
existing NXVM gate-owner correction. Its initial unknown-alias failure remains
failed evidence, not part of the subsequent passes.

The subsequent x64 full-unit attempt reaches the unchanged 300-second deadline,
with 294 completed passes and no reported functional failure before containment.
It overlaps old Make generation and receiving/gate rebuild work, so it is not
qualified. The job-tree runner terminates its owned test tree. Do not increase
the deadline, omit cases, or use completed passes as the full result. Finish
receiving builds and rerun the complete x64 route without competing builds.

Old receiving Make generation takes 252.3 s for AT in this round. AT's first
x64 product refresh completes, but continuing the old three-profile loop is
stopped at XT generation after verifying the exact owned parent, command and
five descendant PIDs. No sibling process is stopped and no file is deleted.
Reuse the validated per-width Ninja/ccache trees sequentially for each fixed
profile, restoring their original Default selection in finally. Fresh AT
configuration is 2.2/1.4 s and generation 3.7/2.4 s on x64/x86. This changes
only ignored build selection, not source topology, presets or owner INIs.
Each product still compiles its own immutable binding/firmware and artifact;
shared CPU/Core objects retain their one validated build path. No new build
wrapper, parallel runtime route or MyNES target is introduced.

Default's two optimized stripped 0546 products are refreshed and architecture
checked; the other receiving pairs are in progress. Do not publish S8 before
all eight products, original 58 external contexts, complete x64 proof and
coordinator actual-change review. T546 and S8 remain open with no P.

The isolated final x64 unit execution terminates successfully: 505/505 in
120.05 s, unchanged 300-second containment. Its earlier deadline is retained,
not combined with this complete pass. Per-width Ninja receiving loops build
all six non-Default products in seven native steps each and restore Default;
all eight 0546 EXEs pass fresh PE8664/014C and absent compiler-debug-section
inspection. Subsequent integration may relink them; final hashes are captured
after that last build, not from this preliminary artifact refresh.

Original external verification is in progress once per profile/width.
Current x64 Default 22/22, AT 3/3 and XT 1/1 pass; Model40 remains running.
Do not yet claim the full 58/58 result. MyNES and all four adjacent INIs have
empty diffs. The ignored x86 unit log is retained under the task's final
qualification directory; shared test/output code and master assets are not
used as writable fixtures.

## Final Receiving Results And Executor Review

All original 58 external contexts pass once, with unchanged checkpoints and
inputs: per width Default 22/22, AT 3/3, XT 1/1 and Model40 3/3. Model40 boot
is 107.20/139.31 s; its group is 107.84/139.85 s. These are valid boot proofs,
not an exact physical-time or all-instruction claim. Complete units are
505/505 per width (120.05/247.58 s). Earlier failures/deadlines above remain
explicit and are not passing portions stitched into these final results.
Both receiving caches restore Default after the sequential fixed bindings;
no runtime profile selection or extra production path is introduced.

Final products below supersede the earlier development artifact hashes. All
are optimized stripped Release, 0.5.0546, with PE8664/014C and no compiler
debug sections. They are generated from the frozen BE62A9BB... CPU source;
runtime Debug remains. All four owner INIs and five external media masters
match their pre-integration identities. MyNES 0044's two exact accepted
hashes remain unchanged; no Lib/Common/MyNES source/test/build/artifact edit.

| Product | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0546_x64.exe | 5E96719BD9A669B8ADAAAE605CEF750D8242F8EA473F9BA44D856590F1A3D7A2 |
| nxvm_default_0_5_0546_x86.exe | F2C38DA9B16C92E6D65B27C1FC39C2164C218A2C93ED8D9DF72624D407D7910E |
| nxvm_xt_0_5_0546_x64.exe | 124E52AEC574E2366464868E97820C5C2B8C708FBB49B3B638683298F7BC71D6 |
| nxvm_xt_0_5_0546_x86.exe | C9BC60068FC7A1D50E26AA114D5D4CDD4F5440298C95469C61A19B688CE450BA |
| nxvm_at_0_5_0546_x64.exe | CFFC894CE7ADA9AF477E6B4C15FF0B9B8AAFD33B77FBF25E43675160CAE3CCC2 |
| nxvm_at_0_5_0546_x86.exe | AB0F14A709B4E482496C57BB6DD5612FE48791D95DCA9A9B6CA0475140B8B1EA |
| nxvm_model40_0_5_0546_x64.exe | 6161D15097634D3C6D9F7266B33A45A7986E2E2A598C5A0BFA7CD31A1AD75F1F |
| nxvm_model40_0_5_0546_x86.exe | DDB98F2270A466C5B639CC89F7F0642B4761D64F92DCB0B156A49EFE30DE3D6C |

| Preserved external master | SHA-256 |
| --- | --- |
| fdd_1440k_msdos_500.img | FADEB3A27C6A0E1CF582DDE0B9AECB7E5D30678F2F967F2F4562F167CC0CB1D5 |
| fdd_360k_msdos_500.img | DE271368874209C07A2FC25C81C17529D4BDD7718B2731B86C49A2DA923A256E |
| fdd_1200k_msdos_500a_01.img | 0F51D92B482253FC468A2B470FFAB82DB43898D1C8B44E504808B7A3EF3D4BDE |
| hdd_50m_win31.img | 61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1 |
| hdd_40m_deskpro_386_blank.img | 2BBC68E612A72290A5181E070494A7580CBECE1F528F92F009A269DC05819E73 |

Owner INI hashes: NXVM A25B3012...; My5160 EC2AFB0E...;
My5170 F4D85EE2...; MyDeskPro386 C812A0C9.... Git INI diffs remain empty.

Actual source/test/build diff review covers nineteen counted tracked c/h/cmake
paths, excluding manifests, docs, generated logs and EXEs: git diff --numstat
against d5ed4549c gives +802/-55, net +747. Production CPU is +58/-25, net +33;
NXVM's owner-resolution gate adds four lines; tests are net +710. The positive
total is direct Cartesian/real-frame proof, not a new runtime abstraction.
The original handlers, scalar stack paths, canonical/image distinction and
single completion/delivery owners remain. One private task-outcome bit replaces
an incorrect opcode-wide approximation; no public interface or timing value
changes. The three changed manifests describe exact current bytes.

Every admitted source/caller member maps to the candidate/proof above; existing
success/failure controls and all original assertions remain except explicitly
sourced false oracles. Full gate/task/page/arbiter/delivery/time obligations
stay with S9-S20. This S neither closes T546 nor declares their context universe
qualified. No L3/L2 timing grade is lowered or invented. Raw reference source,
manuals and protected asset masters are not imported. Eight-corpus supplemental
checks and final coordinator acceptance still precede the pushed closure.

Final supplementary corpus checks pass 33/33 on both widths (256.44/219.95 s),
including positive/negative Types, DAG, public/source boundary and all manifest
checks. Both specialized aggregates and fresh documentation/whitespace checks
pass. All launched verification/build loops are terminal; the intentionally
stopped old Make loop is not a successful route. Current Ninja selections are
restored to Default. Fresh post-integration masters and exact MyNES pair match
the identities above, and adjacent INIs remain unchanged.

Executor self-review accepts the complete eight-member batch and its nineteen
actual code/test/build diffs for target-separated complete P delivery. Source,
ownership, original-style, false-oracle, failure-state, code-size, asset and
qualified-route reviews reveal no further in-scope gap. Preserve the CPU
research, final logs and receiving caches for coordinator review and S9;
no new T or hidden scope transfer is created. Coordinator actual-diff review
of immutable pushed Ps is still mandatory before S8 closes.

Shared P1 9d5e475f7 is pushed to origin/master. It contains exactly the twenty-one
CPU/test/manifest paths reviewed above; frozen BE62A9BB... source bytes and
receiving proof correspond to that immutable implementation. NXVM's following
P carries its four-line gate-owner correction, complete evidence/status/history
and eight receiving artifacts. No source, INI or master change is added after
qualification, and no partial implementation milestone is submitted.

## Coordinator Actual-Change Acceptance

Switch roles and inspect immutable Shared 9d5e475f7 and NXVM 626d504f5,
their actual CPU/context/handler, independent matrix and receiving-oracle
changes, manifests, gate owner resolution, documentation and eight artifacts.
The reviewed working bytes are identical to those pushed Ps; excluded-domain
and INI diffs are empty. The original request, active sixteen-field packet and
complete eight-member ledger map to the direct source/caller proof above,
not merely to a passing test count. No partial member selects closure.

Accept the original handler style, unique canonical/image/stack/completion
owners, private committed-task outcome, failure checkpoint and source-defined
privilege predicates. Removed prefix rejection, old/current TF conjunction
and RF checkpoint mutation have no retained parallel path. Source-qualified
oracle corrections preserve all input/assertion coverage; the positive test
delta supplies previously absent proof. No new public API, framework, clock
or timing grade is introduced. Explicit later geometry/arbiter/task/page/time
receivers remain unqualified, not silently claimed repaired.

Final complete units 505/505 per width, original 58/58 integration once,
supplemental 33/33 per width, both gates/eight manifests, documentation and
fresh artifact/master/INI/MyNES identities satisfy the assigned S8 exits.
The earlier failed/contained/abandoned routes are not acceptance. Verify
target-separated scopes, linear P allocation, baseline/hash/reference updates
and removal of the active packet. Accept and close S8 through the pure
governance P; T546 remains open for S9-S20. Preserve only needed current/next
research, final logs and receiving caches; no product or external master is
removed or modified by this review. Full CPU-goal completion is not claimed.
