# T546 S8 FLAGS Privilege And Return Design

## Admission And Complete Batch

Continue from accepted S6 governance 741d00417, Shared 1db282a10 and NXVM
19945b2b9. Owner authorizes consecutive numeric S admission and existing
x86/chips, x86/core and ibmpc repairs. Timing upgrades are automatic;
downgrades, new public APIs and excluded-domain expansion require review.
Shared CPU/Core tests/manifests and NXVM four-PC receiving proof are admitted;
Lib/Common/MyNES and owner INI/media/firmware changes are excluded.
Preserve unrelated MyNES documentation. No S8 implementation has started.

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
