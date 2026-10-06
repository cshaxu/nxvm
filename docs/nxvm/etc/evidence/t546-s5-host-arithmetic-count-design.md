# T546 S5 Host Arithmetic And Count Design

## Admission And Complete Batch

Continue after accepted S4 ef30f1fc3. The owner automatically admits numeric
S tasks and existing CPU/Core/IBMPC owner repairs; upgrades are approved,
downgrades and new public APIs are not. Preserve unrelated MyNES documentation.
No S5 acceptance or P delivery exists at admission.

Consume the complete T544 host arithmetic/count receiver and S3's two inherited
reference guards, not only one failing multiply. Relevant original records are
T544 S3 F06/F07, S4 inherited arithmetic/count, S5 multiply/divide/count and
S6 host-representation/double-shift sections. Original-page/source decisions
and all actual callers precede implementation.

| Member | Current source observation | Required disposition |
| --- | --- | --- |
| Word DIV/IDIV concatenation | Three DX shifts occur before unsigned widening in operand/result captures | Widen before shifting; preserve source-defined quotient/remainder and existing host minimum/-1 guards |
| IMUL dword by byte | _a_imul3 bit=20 multiplies promoted signed int operands before assigning lib_i64 | Widen signed operands before product; sweep every product form, retaining already-safe ones |
| Bit masks | BSF/BSR and SHLD/SHRD construct signed 1 shifted to bit 31 before wide masking | Unsigned operand before shift, with valid index proof; sweep all similar literal expressions |
| SAR | Three widths right-shift negative signed host operands | Implement source sign fill explicitly with unsigned arithmetic, preserving original handler shape |
| Carry-ring count | RCL/RCR byte/word reduce loop count before storing opr2 and interpreting OF | Reconcile original/normalized guest count against reduced steps and every metadata/timing reader |
| Undefined double shifts | Word counts above width leave result unassigned while callers write it | Initialize deliberate bounded output; tests exclude undefined exact results/flags and retain zero/access rules |
| Internal references | _m_read_ref/_m_write_ref use impossible interval conjunctions | Reconcile complete pointer provenance and widths before removing redundant guards or implementing one subtraction-safe whole-span owner |

Existing five-bit masks for 186/286/386 and full early CL counts are not
defects by themselves. Likewise safe word products, dword products already
widened and signed-divide host overflow guards must not be rewritten merely
because they resemble a bad expression. The scratch udf field currently has
no outgoing FLAGS consumer; do not pretend its wrong annotation proves a
wrong physical flag. Original count/definedness still needs truthful metadata.

## Design And Verification Boundary

Keep cpu_instructions.c's original tables and adjacent width branches. Prefer
an in-place cast/literal/sign-fill correction to a generic arithmetic facade.
Only a truly shared provenance/count boundary justifies a private helper;
do not add public setters, wrappers, mirrors or a parallel reference path.

Read each affected original instruction's operand, count and defined-FLAGS
clauses; source conflicts remain explicit before choosing a guest-visible rule.
Host signed overflow is not an emulator timing grade. A defined constant/formula
remains L3; source undefined output never becomes exact merely because an old
test asserts it. Guest quotient source disputes, fault return frames, complete
publication and other timing recipes retain their named T546 receivers until
their own source decisions, not automatic qualification from this S.

Code-owned test matrices vary CPU family, widths, count zero/one/width/ring/
31/32/255, signs/extremes, register aliases and memory access. Include source-
defined result/CF/OF and preservation checks, deliberate undefined exclusions
and register/reference eligibility. Mechanically detectable forbidden shapes
need a source guard alongside runtime tests. No external ROM/INI/media/font
input is admitted into unit tests.

Complete final unit suites on both widths, corresponding manifests/Types/
corpus/gates and eight current stripped 0546 products precede delivery. Original
58 external contexts are receiving regression evidence, each final group once;
they do not qualify all arithmetic semantics. Count changed source/test/build
lines and prove removal or explicit retention of every old path. Source/page
inventory and concrete repair choices are still in progress; no P is delivered.

## Original Pages And Concrete First Repair Decision

Fresh visual inspection of Intel 386 DX PRM 1990 PDF 460-461/17-142--143,
471/17-153, 482/17-164 and 484/17-166 confirms five-bit counts, single-rotate
OF, unsigned sign-preserving SAR and undefined double-shift boundaries.
SHA-256 is 9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1.
PDF 385/17-67 confirms sign-extended immediate-byte IMUL and full-product
CF/OF conditions; an initial rendering of PDF 384 showed the preceding IDIV
page and is not IMUL evidence. 286 original PDF 298/B-90 confirms carry-ring
and single/multiple OF rules; original SHA is
AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460.
8086/8088 original 1981 PDF 58/2-39 confirms full CL through 255 and SAR sign
fill/rounding; SHA is
3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5.
186 original and remaining flag/source clauses still need final reconciliation.

The reference caller inventory covers decoder-selected rr/rrm, arithmetic/
move/query results, far-load register destinations and port adapters.
_p_ins/_p_outs pass their local lib_u32 data through _p_input/_p_output into
these helpers at byte/word/dword widths. These are legitimate in-module
references outside cpu_state/instruction_state. All remaining targets are
decoder-selected CPU fields or typed local/instruction values; no public
arbitrary host-address API enters this path. Delete the two always-false
object-interval tests and misleading comments without adding a new guard or
object registry. Preserve the sole existing typed-reference copy primitives;
guest segmentation/page/port admission remains at its own boundaries.

The first source patch is limited to already justified host-safe corrections:
three unsigned DX pre-widenings, signed dword/byte multiply pre-widening,
unsigned mask literals before variable shifts, and explicit SAR sign fill
in each original width branch. Safe existing multiply/divide guards remain.
No count/undefined-output guest-visible choice is implemented before its
remaining original-source and consumer reconciliation. No runtime pass or
P delivery is inferred from these source decisions.

Initial production patch is applied in the same instruction owner: +34/-42,
net -8 lines. No public API or timing-number change; source SHA-256 is
1BA24803ACCF6EBBFC153021E77E65B4B9C63C288E30E913775496CF50F8EC59.
The manifest is refreshed to the S5 work revision. CPU-library and existing
IMUL/rotate/double-shift/bit-scan targets now build on both widths. These are
early transient diagnostics only; count/undefined-result choices, direct
extreme/provenance regression additions and full closure proof are pending.
Current timing producers already derive Group-2 work from old CX or decoded
immediate, not the carry-ring-reduced opr2; preserve those correct paths and
test their original-count behavior rather than claiming a new clock fix.

Count repair decision: preserve the normalized guest count in opr2 before
modulo-nine/seventeen loop reduction, and use that saved count for zero/OF
definedness. Timing already reads original CX/immediate; do not change it.
For SHLD/SHRD above width, initialize captured inputs/result deliberately;
retain the existing deterministic count=width computation as a permitted
choice, mark its source-undefined metadata and exclude exact undefined oracle
assertions. This avoids unnecessary compatibility churn without claiming the
manual defines that output. Zero-count shift metadata must not mark OF changed.

Reference cleanup decision: CPU trace call/block macros are empty, and after
removing the impossible checks the two _m_ref functions only forward the same
arguments to _kma_ref. Remove those redundant private wrappers and retarget
their complete caller set to the existing copy primitives. Register, local
port temporary and instruction-scratch paths remain one typed-copy owner;
guest memory/port validation is not removed. This introduces no API or helper.

## Current Applied Batch And Early Regression Evidence

The complete production patch retains the original handler tables and width
branches; only cpu_instructions.c changes, +128/-136 (net -8). Final working
source SHA-256 is 5C36782240D275F2307D08D3F1BE6B3092E713E951FD1464ED4EEFB08E9D37E8.
The redundant reference wrappers and dead SAR temporary are removed; the
existing _kma typed-copy primitives remain the sole reference path. No public
API, timing constant, Lib/Common/MyNES implementation or firmware/INI changes.

Fresh original 186 PDF 171/2-5 confirms modulo-32 counts, immediate sign
extension and INS/OUTS local transfer semantics; SHA-256 is
2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF.
PDF 172 is the adjacent BOUND/ENTER page, not count evidence. Fresh 286 PDF
299/B-91 confirms low-five-bit counts and unchanged flags at zero; PDF
305/B-97 confirms SAR sign preservation and base+one-clock-per-shift formula.
These supplement the original 8086 and 386 pages listed above. Originals and
renderings remain external/ignored; no acquired material is imported.
Fresh 386 PDF 378/17-60 and 383/17-65 also verify the complete DX:AX
dividend, quotient overflow and IDIV truncation toward zero/remainder sign.
No new tests assert a disputed signed minimum quotient or defined DIV FLAGS.

Existing owner-local tests are extended instead of another test executable:

- Carry ring: 1,056 contexts across five profiles, applicable widths, two
  directions, twelve count boundaries, incoming CF and register/memory.
  Independent width+1-bit closed-form oracle checks source-defined values,
  saved count and OF definedness, not reduced-loop OF.
- SAR: 440 contexts with division/floor oracle rather than copied shift loop,
  five profiles, applicable widths, ten counts, both signs and references.
- IMUL immediate dword/byte: 108 extreme/zero/signed contexts, independent
  widened product and range oracle, alias/register/memory and CF/OF.
- Word DIV/IDIV: 80 contexts across all five profiles and both references,
  high DX captures, signed truncation/remainder and zero/overflow delivery.
  Exact failed frame/publication remains S9's owner, not claimed here.
- Double shift retains all original contexts including word count=16 but
  removes exact assertions on source-undefined destination/FLAGS; checks
  undefined metadata and nonparticipants instead. Adds 32 above-width
  capture contexts for both directions/count sources/references.

Early added DIV vectors initially misexpected -1/2 as -1 rather than zero
and equated delivered #DE with terminal failure; explicit diagnostic output
identified both fixture/oracle errors. Corrections follow IDIV truncation and
the fixture's delivered_exception observation, with no production alteration.
Both widths now pass the updated arithmetic/rotate/double-shift tests.

The new host-arithmetic static gate rejects unwidened DX shifts, signed masks,
unwidened dword products, signed SAR and removed forwarding owners. Its
negative/selftest rejects five forbidden shapes and accepts three safe shapes.
Source/test X86 manifests, test Types and diff checks pass after refresh.
The ring matrix now also asks the sole timing selector to prove original-count
register formulas (8086 8+4*n; 186/286 5+n), rather than reduced ring steps.
Standalone strict Release compilation against each final CPU archive passes
this updated matrix on both widths; complete aggregate verification remains
pending. Earlier passes preceded this assertion. 8088 and 386 source allocation
remains distinct and is not guessed from the 8086 formula.
Existing bit-scan tests exercise bit 31; existing port_strings and execution_bus
tests cover typed local INS/OUTS and IN/OUT transfers, to be rebuilt against
this final source. Their full caller/provenance and original-count timing
proof still needs final reconciliation before claiming S5 acceptance.

The post-fix signed-expression sweep covers all CPU C/headers with
`rg -n '\(lib_i(8|16|32|64)\).*([<>]{2}|\*)|(^|[^a-zA-Z0-9_])1[ \t]*<<'`.
Remaining byte/word products are within signed int range; all dword products
widen both operands before multiplication. Signed DIV interpretation occurs
after unsigned concatenation. Far-pointer extraction shifts unsigned scratch
before conversion; the remaining literal-mask hit is only a comment. BSF/BSR
nonzero input and width-bounded iteration constrain indices to 0..31; double
shift loops run only with nonzero masked count <= width, giving 0..31 bit
indices. Exception delivery selects an existing mask bit, also within 0..31.
No signed host shift is retained for SAR. Count-zero double-shift handlers
explicitly skip writeback, so their early helper return does not read result.

All eight source/test manifests, X86 corpus and test Types checks pass.
Both full default build handles remain live; other PC product/harness builds
run serially per cache with MyNES explicitly OFF. Owner INI identities are
captured before receiving builds; no INI write is admitted. Full product,
unit/integration and actual-diff acceptance are still outstanding.
The registered complete unit universe is now 539 (original 537 plus the
host-shape gate and its verifier selftest). No prior unit entry is removed.
SAR extremes also check SF/ZF/PF against the independent result; the updated
strict standalone matrix passes both widths. Newly relinked port_io,
port_strings and execution_bus diagnostics pass both widths, confirming the
legitimate typed-local/reference caller path. Final full aggregates remain
mandatory and have not yet run while their full build handles are live.

## Complete Build And First Full Unit Outcomes

Both complete default Release builds end with exit 0. The final rotate source
is explicitly recompiled afterward to include the last SAR flag assertions.
The x64 bounded full-unit command passes 539/539 in 261.10 s, retained in
ignored build/t546-s5-final-qualification/x64-unit.log and its detailed log.

The first x86 bounded command returns exit 1 with its stdout ending at 522/539
passes; no single-test Failed/Timeout line or complete summary is present in
x86-msys-unit.log. This is incomplete aggregate evidence, not 539 passes.
Containment expiry under concurrent receiving-build load is the working
diagnosis, not an independently retained exact exception: the initial command
captured stdout but its thrown wrapper diagnostic was not saved to that log.
The copied x86-msys-unit-detail.log is stale (14:03 earlier diagnostics), and
LastTestsFailed.log is older still (05:25); neither qualifies this aggregate.
Do not blame their historical failure entries on the current patch.

Receiving product/harness build handle 46667 remains live after the polling
orchestrator is stopped. Keep its original command and logs; do not restart it.
After receiving builds cease, repeat the complete unchanged x86 unit route,
j4/300 s, preserving an explicit thrown diagnostic on failure and copying a
detail log only if its timestamp belongs to that run. No timeout relaxation,
case removal, source rollback, partial-suite combination or P delivery is
admitted by this containment diagnosis. Final qualification remains open.

## Quiescent Units And Receiving Artifact Inspection

All receiving product/harness builds finish with exit 0; default's explicit
vm-0-5-0546 target then succeeds on each width. The unchanged x86 aggregate
passes 539/539 in 125.66 s after those builds cease. Its complete summary and
fresh detail are retained as x86-msys-unit-quiescent.log and
x86-msys-unit-quiescent-detail.log beneath the qualification directory.
The first incomplete run remains retained, not combined with this pass.
No source change, removed case, larger deadline or timing rollback separates
the two runs; build-load interference remains the supported diagnosis rather
than an asserted instruction defect.

Both complete unit routes now prove 539/539: x64 261.10 s, x86 125.66 s.
Release PE inspection verifies all eight products have their expected machine
8664/014C, contain 0.5.0546 and have no .debug/.zdebug section. The one CPU
object is identical across all four same-width caches: x64
F82ACD51A18B2C8AD350F6452FE8ECA127CE90F2AB2A2040475DB86B4A62A4C0;
x86 EF3D93E04075ED737499FD32AC098D69949A4F32F84A987D49984F9CAEE636F4.
No per-App CPU fork or compile-time workaround is introduced.

| Product | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0546_x64.exe | BCC937D4F5F1601028AA0655D4167E364CD09FE3026A4B93B3C04D4DD5355612 |
| nxvm_default_0_5_0546_x86.exe | BC55BA906B6E10996DA3F734E222FB3AF6434A342C87029E29163D7726D3076B |
| nxvm_at_0_5_0546_x64.exe | 70FB4C8A0ACCD0A4C291CE87219755CBF8605B513006117C57FA516354334CFB |
| nxvm_at_0_5_0546_x86.exe | 3063A103027B607B83268706CC4F2537F0F3993F32C4C511E9BDFFDBA23838C2 |
| nxvm_xt_0_5_0546_x64.exe | 5CEA60497AB423EA3855BDDCDFD9FCFC43A69692E7FA688758049D0035C091AD |
| nxvm_xt_0_5_0546_x86.exe | 3D5E997EAF2B2F5BEE52BD6435C875A2953770A36933670A7A855671A4FBDDB4 |
| nxvm_model40_0_5_0546_x64.exe | 357AD51F0A58CC2B5BE1A5AE3D0E0E1E164D9F58C4EB2BC7FC27658557328B5A |
| nxvm_model40_0_5_0546_x86.exe | 0083DF5806A549FB75D1E6626DEEFF0DAE48C5933E48AA3EDBC27A022C6922B0 |

Counted C/test/build paths are cpu_instructions.c, four changed CPU test
bodies, test/x86/CMakeLists.txt and two new host-arithmetic CMake verifiers.
git diff --numstat plus line counts for the two new files gives +528/-144,
net +384. Production alone is +128/-136, net -8. Manifests, task records,
ignored outputs and deployed EXEs are excluded from that code count. Positive
test growth supplies the missing independent finite matrices and guard proof,
not another production abstraction or a parallel arithmetic implementation.

Five external master-image hashes still match the S4 baseline. MyNES EXEs and
all four owner INIs remain unchanged. Supplemental/gates and the original
58 receiving integration contexts remain pending; no P or closure is claimed.

Final self-review finds one direct regression hole: existing BSF dword input
0x80000120 stops at bit 5, although BSR sees bit 31. It does not dynamically
exercise BSF's repaired signed-shift boundary. Fresh Intel 386 PDF 347/17-29
and 349/17-31 confirm the nonzero source's first-set-bit index and ZF; zero
destination is undefined. The displayed pseudocode places register assignment
inside the loop, but the explicit description also defines the no-iteration
bit-0/high-bit cases; use that stated index contract rather than reproducing
the pseudocode placement typo. No timing interpretation is changed here.

Before delivery extend the existing CPU bit-scan test with every single-bit
16/32 source for both directions, register, register alias and memory forms,
preserved upper word/nonparticipants and ZF. Keep existing zero/illegal-profile
contexts. This is test-only: source hash and eight product hashes stay fixed.
Refresh its manifest, compile/run both targets, repeat complete units on both
widths and affected test-boundary checks after the final test corpus is frozen.
Earlier unit passes remain evidence for the earlier corpus, not final acceptance
of the new matrix. No additional production path, API or executable is added.

## Final Executor Qualification And Whole-Batch Self-review

The final 288-context single-bit matrix passes both widths. With that last
test change frozen, complete final units pass 539/539 per width, x64 116.97 s
and x86 118.33 s. Retained final-unit.log/detail.log files supersede earlier
passes for acceptance, while all incomplete/early results remain in the same
ignored qualification directory. No test is removed or containment relaxed.
Source hash and all eight recorded products stay unchanged after this test-only
addition. Final counted nine C/test/build paths are +582/-145, net +437;
production remains +128/-136, net -8. Test growth is independent proof, not a
new CPU mechanism. No new test executable is introduced.

Both supplemental routes pass 33/33 (211.06 s/194.02 s), and both complete
specialized/artifact-root/INI aggregates exit 0. The T345 duplicate-row fatal
message is the expected negative selftest and is followed by its passing marker;
it is not a hidden failing gate. Updated test manifest/Types and all eight
manifests pass again after the bit-scan addition. Documentation/diff checks
pass. Original receiving integration completes exactly 58/58, once per final
group, with unchanged predicates and external inputs:

| Group | Passed | Seconds |
| --- | --- | --- |
| default x64 | 22/22 | 52.75 |
| default x86 | 22/22 | 60.70 |
| AT x64 | 3/3 | 33.56 |
| AT x86 | 3/3 | 43.68 |
| XT x64 | 1/1 | 19.35 |
| XT x86 | 1/1 | 23.94 |
| Model40 x64 | 3/3 | 120.77 |
| Model40 x86 | 3/3 | 155.34 |

Executor actual-diff self-review covers the complete admitted batch, not just
the passing summary:

| Member | Source/caller/production proof | Regression/disposition |
| --- | --- | --- |
| Word dividend captures | Unsigned DX widening precedes each affected shift; existing divide guards remain | Five-profile high-word and signed/zero/overflow register/memory matrix, source-defined outputs only; accepted S5 host arithmetic |
| Dword-byte product | Both signed operands widen before multiplication; safe byte/word and already-widened dword forms stay intact | Extreme/zero/sign, alias/register/memory and CF/OF matrix plus all existing forms; accepted |
| Bit masks | Unsigned literal precedes every affected valid-index shift, including exception mask | All single BSF/BSR bits plus double shifts; static gate and selftest reject old shapes; accepted |
| SAR | Three original width branches explicitly retain sign with unsigned arithmetic | Five-family floor/sign/count/reference/result/defined-flag matrix; accepted |
| Carry counts/zero metadata | Saved guest count precedes loop modulo; zero/OF uses saved count; original timing producers retained | Full carry ring/zero/multiple-count matrix and original-count register formula checks; accepted |
| Undefined double shift | Captures and result initialized above width; count=width old permitted choice remains but is not a precise oracle | Defined cases remain exact; undefined cases check metadata/nonparticipants, never arbitrary output; accepted |
| Typed reference boundary | Complete decoder/register/far-load/local-port provenance permits existing copy primitives; impossible guards and forwarding wrappers removed | Full unit port/register/data/fault regressions and static no-retired-path check; accepted |

No public header/function/type, timing constant, board patch, Lib/Common/MyNES
implementation, firmware or owner INI changes. One instruction state owner,
original tables and one typed-copy production path remain. The check does not
assert an exact undefined outcome or a timing downgrade. Full failed frames,
privilege, task/paging/delivery, string effects, external bus/NPX and remaining
formula allocations retain their S6-S17 owners; no complete CPU or T closure
is inferred from S5. The earlier signed-minimum source dispute is not silently
resolved by the host overflow correction.

Every launched build/test handle is terminal. Raw research and generated logs
remain ignored for the immediate S6 receiver; protected inputs are not staged.
The five master media identities, four INIs and exact MyNES pair remain the
unchanged accepted baseline. Unrelated MyNES documentation stays outside this
delivery. Coordinator immutable-P review and pure governance acceptance remain
required after target-separated implementation commits are pushed.

Shared implementation P1 bdfe3b938 is committed and pushed to origin/master.
It contains only the two X86 corpora (11 files); the nine counted code/test/
build paths remain +582/-145, with manifest changes excluded. This immutable
source revision produces the recorded eight products from source hash 5C3678...
and the identical same-width CPU objects above. NXVM artifact/evidence P2 is
the receiving delivery, not a CPU fork. Coordinator review is still pending;
this executor self-review does not close S5 or T546.

## Coordinator Immutable-Change Acceptance

The session switches from executor to coordinator and reviews actual Shared
P1 bdfe3b938 and NXVM P2 04f0278ad against the original owner request, complete
S5 packet, seven-member batch and applicable rules. The committed CPU blob
matches the qualified source exactly; the diff retains handlers/call arguments,
corrects only the admitted arithmetic/count/definedness mechanisms and removes
the two pure reference wrappers. Five changed owner-local C tests, two guard
scripts and their registration/manifest changes supply direct proof rather
than another production path. The late BSF bit-31 proof gap is actually fixed
and included in the final full-unit logs, not merely promised.

Review checks both P scopes and their real diffs: Shared contains only X86
source/test; NXVM contains eight existing artifact paths and task evidence/status.
No public headers, Lib/Common/MyNES code/assets, INIs or raw protected inputs
appear. Counts, final 539/539 units per width, 58/58 original integration, both
33/33 supplemental/full gates, all eight manifests, PE/hash/object identities,
preserved failed/early diagnostics and terminal owned handles match the evidence.
The T345 fatal is an intentional passing negative probe. Rule/document review
confirms Current alone owns active status and the ledger still retains S6-S19.

Coordinator accepts the complete S5 batch and closes it through pure governance
P3. No active S packet remains until the next automatic admission. T546 and the
whole CPU repair goal remain open; no complete CPU, physical timing or later
frame/task/paging/delivery contract is claimed. NXVM/Shared implementation is
clean; unrelated MyNES documentation is preserved outside these commits.

Full dual-width units, gates, eight products, original receiving 58 contexts
and coordinator actual-diff acceptance are pending. No implementation P or
S5 closure follows from these early tests.
