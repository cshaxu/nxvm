# T546 S3 Address/Span Implementation And Proof

## Approval And Boundary

Owner authorizes automatic changes in x86/chips, x86/core and ibmpc on
2026-10-06, excluding the nonexistent x86/devices. This implements the
[reviewed scheme](t546-s3-address-span-design.md) against c3c9fd550; no
Lib/Common/MyNES/INI, public API, new CPU or board-clock change is made.
Current owns the active packet. This progress record does not accept S3.

## Complete Mechanism Map

| Receiver | Existing-owner implementation | Direct proof owner |
| --- | --- | --- |
| Word XLAT EA | Both BX+AL expressions use the existing word mask; dword addition stays separate. | All five profiles, overrides and retained wide-cache dword addressing in cpu_address_span. |
| Bit-string address | Remove both immediate displacement additions; reduce register-derived EA by address size. Widen the negative dword subtraction and use unsigned mask literals. | All four bit operations, word/dword, address-size pairs, signed extremes and raw high immediates. Existing bit-test wrong-neighbor predicates are corrected against Intel notes. |
| Real/VM/cached limit | Remove unconditional nonprotected DATA widening; actual cached bounds remain authoritative. | Legacy transfer wrap versus 286/386 rejection, real cached extension and canonical VM86 64K input. No invented wider VM86 cache is qualified. |
| Empty expand-down | Widen lower before limit+1 in DATA and STACK; retain the existing full-span upper subtraction. | Legal 286 non-big and 386 big/non-big boundaries, byte/word/dword, empty intervals and DS/SS exception-vector selection. |
| Composite fields | One private _m_read_pair preflights the complete logical span, then keeps both scalar transfers and packs their copied result. All fifteen BOUND/far/table-reader additions are removed. | Every eligible family/form/operand/address-size, legal endpoint, early wrap, first/second provider rejection and complete-span rejection. |
| Ordinary SS failure | Remove blanket real-386 SHUTDOWN from range checking. The existing real-exception finalizer also needs its missing SS path; reuse that owner for vector 12. | Real SS-selected operand with a separately valid delivery stack reports SS and reaches the handler instead of directly stopping/shutting down. |

No physical bus phase is collapsed. Existing early wrap policy decides the
second-field wrap; later operands are not masked to fit. Pair preflight only
checks logical bounds/rights, with no page translation or MMIO read. Complete
paging/RMW/frame/task/delivery policy remains in S6/S9-S15's full ledger,
not falsely qualified by these local predicates or ordinary CPU rollback.

## Similar-Issue Sweep

The original twenty-one additions become five: one paired-read second-field
advance plus four signed register bit displacements. The two high-immediate
advances disappear. The new source-shape gate permits advances only in these
two owners and rejects the old DATA widening, narrow expand-down addition and
unmasked word XLAT shapes. The original static decode gate is retained.

Source/test authority distinguishes cached primitive geometry from descriptor
encoding eligibility; no 286 big cache or wider VM86 cache is treated as legal
hardware input. Protected failed-delivery fixtures observe the first IDT gate
chosen, not claim that their final DF is a complete exception-delivery oracle.
All tests use code-owned memory and synthetic vectors, never external assets.

## Verification Progress

First dual-width address/bit/static selection passes 3/3 per width. It precedes
the final extended boundary/address-size matrix and is not final-source full
verification. Full receiving caches rebuild; source, test and object timestamps
must be reconciled before running the full suites. Required current-unit count
is 536 (534 accepted baseline plus owner-local runtime and static regressions).
No rule, clock row, assertion or timeout is weakened. All eight current PC
artifact pairs, original 58 external contexts, complete dual-width units,
manifests/Types/corpus/specialized/documentation gates and actual-change review
remain required before any P delivery or closure.

Executor field-layout review catches a conversion hazard before acceptance:
the two 386 indirect CALL/JMP bodies cast the packed pair to dword even for
word operands. Mask the offset by actual operand width; all other pair users
already decode their field widths. Strengthen the matrix with nonzero 0100h
selectors, rather than accepting a zero-selector coincidence. The four owned
build trees are explicitly stopped before this production refinement; no
partial build or pre-refinement object is acceptance evidence. Their ignored
incremental caches remain reusable after final source rebuild. No artifact,
media, user process or source file is removed by that process cleanup.

Extended final-source runtime matrix and original bit/operand receivers plus
the new static gate pass 4/4 per width: x64 1.62 s/x86 3.72 s. The matrix
includes all eligible paired-reader families, nonzero selectors, 16/32-bit
address/operand combinations, exact injected-failure address/restart point,
legal 286 expand-down inputs and 386 byte/word/dword spans. The source is now
frozen before the whole receiving rebuild; these local passes do not replace
the required full unit, external or artifact acceptance.

Final complete dual-width build handles finish successfully after source
freeze. All eight selected 0546 PC EXEs and their original receiving harnesses
rebuild with PE architecture checks; MyNES remains excluded and unchanged.
XT uses inspected generated leaf recipes: explicitly rebuild the CPU archive,
then relink the unchanged product/harness graph. The recipe retains strip,
architecture verification and deployment; only test registration changed the
otherwise unnecessary graph regeneration. Default product leaf relinking
uses its fully regenerated current graph. No project build configuration is
changed to gain this speedup. The full test aggregates still run all 536
registered unit cases in regenerated default caches, not a leaf-only subset.

## First Whole-Suite Failure Batch

The first complete x64 suite rejects acceptance: 526/536 pass in 292.21 s.
Ten failing registrations represent eight distinct test bodies/producers:
prefix attributes, far control transfer, 286 and 386 timing-manifest runners,
real-mode 386 address and REP-CMPS, XLAT final group and MOFFS. The x86
aggregate independently reproduces timing/prefix/XLAT failures, then reaches
its unchanged 300 s deadline after 275 completed tests. Neither is full proof.
No P is delivered, no timeout/original timing number is relaxed, and the 58
external contexts remain pending.

Actual consumer review identifies an introduced pair-result contract defect:
far CALL/JMP parse the new packed value correctly, but their existing timing
selector consumes crm as the original selector-only decoder value. Preserve
that value after extracting both fields in all four generation-specific
bodies. Otherwise an ordinary same-privilege far transfer is misclassified
as a gate (386 57 instead of the recipe's unchanged 39), and 286 cannot select
the original row. No timing-model API, clock row or second state is added.

Several old high-address success fixtures relied on the removed unconditional
real-mode DATA widening. MOFFS, XLAT and address32 prefix success cases now
explicitly supply the retained wide PE-clear DS cache authorized by manual
14.5. The new span matrix separately checks ordinary real/VM bounds; 67h
itself never grants a wide segment. Far-boundary and Core string fixtures
are also reconciled with their sources, not waived or silently deleted.
Core string fixtures now use distinct segment bases and legal endpoint offsets:
MOVSD consumes FFFCh..FFFFh, and its address32 indices become 10000h after
the one completed element. REP-CMPS/SCAS use FFFDh..FFFFh and retain their
count, compare-flags, override and distinct-physical-data assertions. This
does not turn a 32-bit offset beyond FFFFh into a valid ordinary real access.
Far-pointer boundary data is isolated from CS: early families read the
selector from the wrapped DS offset; 286/386 require GP and no committed
target. Original success/failure cases and clocks are preserved.

After these source-based corrections, all eight distinct previously failing
bodies/producers plus the new address-span matrix pass on rebuilt x64 targets
(9/9, 8.52 s). This diagnostic selection is not full acceptance. All rebuilt
artifacts from before the consumer fix are superseded candidates; complete
x64/x86 incremental rebuilds run before final-source qualification. Source
and test manifests are refreshed to the corrected bytes. Full units,
original external contexts and eight receiving artifacts remain required.

Post-correction receiving leaf rebuilds finish successfully for AT, XT and
Model40 on both host widths, including their original floppy-boot harnesses.
All six deployed replacements pass their PE architecture/stripping checks;
default remains owned by the two live full incremental builds. The exact
MyNES pair is still unchanged (S1's recorded hashes). Eight manifests, x86
corpus and address/span owner-boundary verification pass after the latest
source/test edits. These are build/static facts, not external boot acceptance.
Actual consumer sweep covers every timing-model crm use: only the four far
CALL/JMP bodies consume the paired value; multiply and bit-scan scalar
consumers are unaffected. No timing-model rewrite or public state is needed.

The private logical/ref sweep additionally finds two inherited impossible
interval guards in _m_read_ref/_m_write_ref (ref below an object's start AND
at/above that same object's end). They are not effective validation. These
host-reference helpers are distinct from S3's guest segment-span owner; retain
their complete decoder-produced-reference/host-integer proof in S5 rather
than patching the two boolean operators without checking all valid callers.
This is not a new timing downgrade or a public pointer/API expansion.

## Pre-Review Artifact Candidates

Both complete incremental build handles exit successfully. The default
product is then explicitly relinked/deployed from each current CPU archive,
like the other three receiving products. All eight PE widths match their
filenames and no compiler .debug/.zdebug sections remain. Runtime Debug is
retained. These hashes identify the pre-review candidates; full unit and
external qualification still precede acceptance:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0546_x64.exe | 18D72B21E558F09F85BAB5CE8EB7A6C159436921CFEFCE543787DBEAE65C6995 |
| nxvm_xt_0_5_0546_x86.exe | 9113C1ED047C12C5185FF95FA0ECF51CF27F25D55900F9F1D92FF30366FD1E7F |
| nxvm_at_0_5_0546_x64.exe | 47462A54110B6323A7F2C82B5B7FC7F727BEF1CF47C9B5018A5E3523C44F4402 |
| nxvm_at_0_5_0546_x86.exe | 44738C396A3D2F40962BBE6EDF2991A27EF15A6A5BD74DD599CA2A43D67A97D1 |
| nxvm_model40_0_5_0546_x64.exe | 31A10A7B0F2B90DA390B268C02C883B0B00E2020A57FAF0189D32B3789BF332B |
| nxvm_model40_0_5_0546_x86.exe | F5E5E5CFAA9B2182E24588C519E35472B9EC200C3DF417ECBE26F5484718A7C0 |
| nxvm_default_0_5_0546_x64.exe | B0D01C92145F7EAC3EB53FBFC67CFE6E5BDC4C228EE126EBBB4A615B31813F1B |
| nxvm_default_0_5_0546_x86.exe | A095C8A8B8F0C797036A791F0B441E1B7629766F493732F7CD625C2C35B9D9D2 |

The first post-build aggregate invocation uses a relative test directory;
the wrapper changes working directory before CTest applies --test-dir, so
that invocation exits without running any test. The corrected invocation
resolves the same directory absolutely; no test script, timeout or assertion
is changed. Full suites run sequentially by width with four jobs and the
original 300 s per-suite bound, with all owned builds terminal first.

Final-source complete x64 unit qualification passes 536/536 in 245.67 s,
four jobs with no competing owned build/test. The unchanged 300 s bound is
retained. Original timing-manifest, corrected receivers, new span regression,
static/negative, Lib/Common and board tests all remain registered. The same
sequential invocation also passes complete x86 units 536/536 in 229.62 s.
Both final logs are preserved under ignored build/t546-s3-final-qualification
before the original integration groups reuse their CTest directories. The
58 original external contexts subsequently pass once (default 22/22, AT 3/3,
XT 1/1 and Model40 3/3 per width), with their final logs in the same ignored
directory. Model40 boot takes 120.05 s x64/155.91 s x86 within its unchanged
allowance. No external input, checkpoint or accepted timing number changes.

## Actual-Diff Review Correction Before Delivery

Review finds one unintended change outside the admitted span dependency:
the real GP finalizer had been narrowed from a bit-mask test to exact GP
equality while adding the ordinary SS route. The producer still accumulates
exception bits. Restore the original GP predicate; do not claim a compound
exception-policy change from S3's span proof. The new exact ordinary SS route
and its vector remain unchanged; S9 retains complete pair/delivery authority.
No test oracle, public API, timing row or family exception grade changes.

This one existing-policy preservation changes the production hash to
CABC46E350FE481C434B0897607B1756A4F2699880EE5F90488E63C7C7967F7E.
The preceding unit/integration passes and artifact hashes are pre-review
evidence only, not final qualification for the corrected bytes. Rebuild the
affected targets and requalify the full receiving surface before P delivery.
The second verification is source-change-driven, not repeated boot sampling.

After the pre-review integration completes, all five external FDD/HDD master
hashes still match S2's identities, and both MyNES 0043 hashes still match S1.
No INI, Lib/Common or MyNES path is modified. Corrected-source receiving
rebuilds for AT/XT/Model40 finish on both widths; default and the complete
unit graph are still rebuilding before the next qualification. No P is
delivered from the superseded passes or artifact candidates.

## Counted Change Surface And Self-Review

Git numstat over the changed production/test/build descriptions, plus line
counts for the two new test files, gives eleven paths +596/-113, net +483;
exclude manifests, documentation, artifacts and ignored generated inputs.
The sole changed production C file is +82/-89, net -7. The positive test
delta supplies missing generation/width/mode/endpoint/physical-phase and
failure predicates through one table-driven matrix and existing fixtures,
not production scaffolding or a second state owner.

Actual production diff review covers every range/EA/pair caller, field decode,
timing consumer and real SS dependency. Actual receiver diff review preserves
original nonparticipant registers, memory, flags and count assertions while
replacing source-contradicted setup/expectations. No original test is deleted.
No public header, Lib/Common/MyNES, INI, device clock or firmware source is
changed. The unused RMW helper argument and complete frame/task/page/physical
publication contracts remain explicitly assigned to their later S batches.
Self-review is not coordinator acceptance or full-T CPU qualification.

## GP-Preserved Final-Source Rebuild

Both complete corrected-source builds finish successfully, and the default
product is explicitly relinked/deployed on both widths. All eight current
PC EXEs match their PE width and remain stripped of compiler debug sections.
They correspond to the frozen CABC46E3...C7967F7E production hash; they replace
the pre-review candidates above, whose historical passes are not reused.

| Artifact | Corrected-source SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0546_x64.exe | 4A5CE2E0EEDA3949F3AA44000B34F1581AD28E8F5A2D470B661C1DEDFB1A208A |
| nxvm_xt_0_5_0546_x86.exe | 70446AA5131F37B85B9F6690F77671EA565367B9FE6B75FD6BFBD204290D0DA6 |
| nxvm_at_0_5_0546_x64.exe | 1E2C9720E6500EA6D498DE982C74804E6F6DE87C68B0BB180325745550D51167 |
| nxvm_at_0_5_0546_x86.exe | 69C16E9C900437558C4650A677DCF05229419777E477EA771576428EF839471A |
| nxvm_model40_0_5_0546_x64.exe | FC75120263F8F20C570B800960A37DE790C739E7CB282F5240FD1B8FB8CF0A7C |
| nxvm_model40_0_5_0546_x86.exe | 26FE3DAF3B66646452FF760D66F8FA38D6F8113E9B990D03DC0E83BB404711C4 |
| nxvm_default_0_5_0546_x64.exe | 3C70099C6A87847D78D4CE9BA7450B1CCEE0E23F06A3FD6BA6F5AE17C7A3911E |
| nxvm_default_0_5_0546_x86.exe | C05D6985FCB7B4A4B2356284BBCEADA9079337A7A9430216261910F69E4A57AF |

Corrected-source full units run sequentially by width, four jobs and the
unchanged 300 s bound, after every owned build is terminal. Their new proof
is preserved separately under ignored build/t546-s3-gp-preserved-qualification.
Final units, original external contexts, supplemental/specialized gates and
coordinator acceptance remain pending. No P is delivered yet.

The GP-preserved final-source x64 complete unit suite passes 536/536 in
251.50 s, four jobs under the original 300 s bound. The x86 complete suite
is now active in the same sequential invocation. The CABC46E3...C7967F7E
production hash remains unchanged, and documentation governance/diff-check
pass. No superseded log is substituted for this result.

The same final invocation completes x86 units 536/536 in 229.74 s. Both
original full suites pass with four jobs and their unchanged 300 s bounds,
and both corrected-source logs are preserved in the separate qualification
directory. The final-source original eight integration groups now run once
each, preserving every INI, external master, checkpoint and timeout.

All corrected-source original integration contexts pass 58/58: default
22/22, AT 3/3, XT 1/1 and Model40 3/3 on each width. Each successful group
runs once; no input, checkpoint or timeout is changed. Model40 boot is
120.41 s x64/153.87 s x86, within the existing allowance. Their final logs
are preserved separately from the pre-review groups. Supplemental 33-case
CTest checks and both original specialized aggregates run next; no P is
delivered before those required exits and actual-change acceptance.

Final x64 supplemental CTest checks pass 33/33 in 207.87 s, including all
eight manifests, corpus/Types/DAG/ownership and their negative/self-test
receivers. The same sequential process now owns x86's 33 checks before both
specialized aggregates. External masters and the exact MyNES pair are again
rehashed after final integration and remain unchanged; all INIs are unchanged.

Both supplemental suites finish 33/33 (x64 207.87 s/x86 202.09 s), and both
specialized aggregates plus artifact-root/INI-boundary targets exit zero.
The expected injected duplicate-row error is explicitly accepted by its
negative self-test, not an ignored command failure.

Final code-style review aligns six indentation/comment-continuation lines
in the new span test only. Recompiled x64/x86 object hashes stay identical:
4B577396131DF3EF3FF9FD02404C1C25241025187067494DECEEB34EB61646D2 and
1D40CD6C301C333C09AB5976BA4D2278AB744CF3602A86DE10B344954AEB66BE.
The relinked test EXEs are not byte-identical, so rerun the full dual-width
unit suites before delivery rather than infer executable equivalence.
Production, all eight PC products and their final integration inputs remain
unchanged; no additional boot sampling is required. Refresh the test manifest
and verify the formatted-source bytes at the final gate. Counted lines are
unchanged. No P is delivered before the rerun completes.

Final formatted-source complete units pass 536/536 per width: x64 127.02 s
and x86 118.02 s, four jobs under the original 300 s suite bounds. Fresh
eight-manifest, x86 test Types/address-boundary, documentation and diff checks
pass after the formatting/manifest update. The unchanged CABC46E3...C7967F7E
production and recorded eight product hashes retain the final 58/58 external
proof; no product binary or firmware/INI/media input changes after that proof.
Both 33-case supplemental suites and both specialized aggregates pass, with
their expected negative self-tests explicitly accounted for. All recorded
owned verification/build handles are terminal. Detailed final runtime logs
remain in the separate ignored qualification directory.

Executor self-review maps every admitted address/span class and all fifteen
composite callers to the source/receiver ledger, checks the actual production
and receiver diffs, and retains S5/S6/S9-S15's distinct contracts. One range
owner, one early-wrap policy and one scalar-transfer path remain; no public
API or parallel mechanism is added. Scope exclusions and unchanged MyNES/
external masters are rechecked. The complete S3 brief is ready for sequential
Shared and NXVM delivery, followed by coordinator actual-change review and
pure-governance acceptance; this is not acceptance or full T546 completion.

Shared implementation P1 is 15a2399d0, pushed to origin/master. It contains
the sole production edit, thirteen Shared source/test/manifest paths and the
final formatted matrix. NXVM P2 owns the complete source/evidence record and
the eight verified stripped 0546 PC replacements identified above. No late
code, media, INI or MyNES change is added during target-separated delivery.

NXVM implementation P2 is d875a72fe, pushed. Coordinator switches roles and
reviews both actual commits, all admitted classes/callers, corrected oracles,
final runtime logs, field/timing consumers, artifact hashes and target scopes
against the original request and packet. The direct logs contain 536 passed
tests per final unit suite, 33 per supplemental suite and all 58 original
external contexts, with zero failed test records. Scope/ABI/owner and code-size
claims match the actual diffs; all later frame/task/page/physical publication
and host-reference findings retain their named receivers. No source conflict,
new unupgradable L1 or timing downgrade is waived.

Coordinator accepts the complete S3 delivery and closes S3 through a purely
governance P3. Current removes the active packet and advances only the accepted
S3 technical/runnable baseline. The full T546 S4-S19 objective and imported
T544 convergence universe remain open; unit/boot success is not full CPU
or physical-time qualification. No next S is admitted by this closure.
