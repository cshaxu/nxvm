# T2 S37 Ordinary Multiplication/Division Nominal Timing

Baseline `988a439f6`, after accepted S36. This consumes the complete
ten-encoding ordinary native arithmetic nominal batch at original V07/V15,
not all CPU timing or the original game outcome.

## Source, Model And Complete Disposition

NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
[original copy](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-50..60 (PDF95..105), all eleven pages, were rendered and visually
inspected. Only the original V30 70116 columns are consumed, not V20
odd-address timings. Original instruction rows assume prefetched bytes and
zero extra waits; they already include operand/EA work.

| Family / register width | Register | Even memory | Odd word memory | Grade |
| --- | --- | --- | --- | --- |
| MULU byte | 22 | 28 | not a word | Conservative L2 endpoint of 21..22 / 27..28 |
| MULU word | 30 | 36 | 40 | Conservative L2 endpoint of 29..30 / 35..36 / 39..40 |
| MUL byte | 39 | 45 | not a word | Conservative L2 endpoint of 33..39 / 39..45 |
| MUL word | 47 | 53 | 57 | Conservative L2 endpoint of 41..47 / 47..53 / 51..57 |
| DIVU byte | 19 | 25 | not a word | Original fixed nominal source row |
| DIVU word | 25 | 31 | 35 | Original fixed nominal source row |
| DIV byte | 34 | 40 | not a word | Conservative L2 endpoint of 29..34 / 35..40 |
| DIV word | 43 | 49 | 53 | Conservative L2 endpoint of 38..43 / 44..49 / 48..53 |
| MUL immediate byte to word | 34 | 40 | 44 | Conservative L2 endpoint of 28..34 / 34..40 / 38..44 |
| MUL immediate word to word | 42 | 48 | 52 | Conservative L2 endpoint of 36..42 / 42..48 / 46..52 |

The primary manual explicitly says the ranges depend on data but supplies
no complete operand-to-clock predicate. The bounded L2 model selects the
upper documented endpoint for each successful form. It does not infer the
silicon multiply/divide microsequence or borrow an Intel slope. For example,
word register MUL reports 47 nominal clocks even when particular inputs
might complete earlier; DIVU word register has a sourced fixed 25.
Existing L2_DYNAMIC_ARITHMETIC identifies the modeled rows; exact DIVU
uses V30_NOMINAL_SOURCE. All remain incomplete elapsed-time subtotals.

Zero divisor and quotient overflow take the existing vector-0 entry path.
The successful software-entry fact excludes those operations from ordinary
success-clock selection. A combined entry subtotal belongs to the existing
V11/V15 receiver; do not charge a normal division row and then guess another
entry cost. Both source/L2 success and unknown DE/prefix results retain
unallocated total time, zero maximum and closed neutral Machine admission.
Extra segment/redundant/LOCK prefixes and 8080/mode forms are not silently
qualified by successful ordinary arithmetic.

## Ownership, Actual Change And Proof

Only the existing V30 timing selector changes: one local origin variable,
source/model row selection, immediate-form ModRM attribution and the existing
final result publication. There is no new production helper, persistent
state, decoder, arithmetic, operand read, clock, thread, interface/enum/layout
or old Intel evaluator change. Memory oddness comes from the existing
decoded original effective offset. The existing software-entry fact is
read, not mirrored.

The existing multiply/divide test owns one local independent explicit
register/even-memory/odd-memory row oracle. It checks exact ticks, opaque
form/key, formula inputs, source versus L2 origin, no observation, no event
time, repeat-none, unallocated total and zero maximum. Original complete
CPU/defined-flag, every ModRM/segment/alias, full-byte arithmetic, signed
bounds, immediate domains, code/data wrap, DE/IRET and accepted-prefix
provider-refusal assertions remain. Every failed transfer additionally
must have no successful timing origin/ticks.

Twenty new successful witnesses exercise both parity classes for all ten
forms, complementing all original 1,796,404 inputs. The existing actual seam
passes 1,796,424 inputs and V30:NOMINAL-MULTIPLY-DIVIDE:OK in each focused
width. Four old ALU unknown-clock arithmetic placeholders become the same
forms with one extra segment prefix: the unknown contract and all 917,460
ALU inputs / nineteen boundary cases remain, without expecting newly
modeled successful forms to stay unknown.

Counted Git numstat for three C files, excluding manifests/documents/assets:
production +24/-5/net +19; tests +60/-1/net +59; total code +84/-6/net +78.
Manifests separately +3/-3/net0. This is below estimate because existing
state/publication, source parity and actual test fixtures are reused.
The positive test delta has one independent oracle and twenty explicit
parity successes, not a second execution or testing framework.

## Similar-Issue Sweep

Queries cover the actual CPU selector, its adjacent arithmetic exclusions,
the original complete arithmetic test and Machine's admission/retirement:

- `rg -n 'instruction_software_entry|L2_DYNAMIC_ARITHMETIC|source_repeat_active' src/x86-core/cpu`
- `rg -n '0xf6u|0xf7u|0x69u|0x6bu' test/x86-core/cpu --glob 'cpu_v30*smoke.c'`
- `rg -n 'V30|source_timing_unallocated' src/x86-core/machine --glob '*.c' --glob '*.h'`

All F6/F7 /4../7 and 69/6B forms have one nominal owner. Existing TEST/NOT/NEG
and rejected /1 keep their distinct source/admission semantics. Immediate
multiply now participates in existing ModRM/EA attribution, without inventing
a group-extension key. Successful division and successful DE entry cannot
share a success-row claim. No source timing is looked up after reading a
mutated operand or re-observing a device. Non-string selection already clears
the existing repeat identity; it is unchanged. Intel models retain their
distinct source tables. Maximum/Machine remain gated. General entry,
prefix/alias/native/8080, physical bus/prefetch/wait/time and board/game
requirements retain every original V01--V17 receiver.

One ad-hoc manifest invocation used a nonexistent test-local verifier path;
the actual shared source verifier with CORPUS_ROOT=test/x86-core verifies
the test package. No verifier, manifest format or assertion changed.

## Final Qualification

Shared P1 `f5dd994a0` and NXVM receiving P2 `2611f6660` are pushed. The
single final qualification runner is terminal with exit zero. No production
or test edit occurred after its source/hash freeze. Both original Release
Ninja/ccache all-target builds pass; final full repository units pass
678/678 per width, including 191 X86 Core cases, zero failures/skips.
Full units take 310.98 seconds x64 and 123.33 seconds x86. Both seven-unit
focused groups pass. Both specialized gates retain 498 strict rows with
zero deferred and 1,389-input global Types coverage; historical warning
exceptions are unchanged, not an exemption-free strict-build claim.

Each width passes fourteen selected PC receiving integrations plus one
restored-default identity. DeskPro groups take 133.26/174.31 seconds within
unchanged budgets. Both original graphs are restored to default; profile
switches and restores recompile zero CPU objects. Eight PC 0564 releases are
rebuilt/deployed. All ten PE/size/hash identities match the measured
[artifact record](t2-s37-artifacts.json); both MyNES 0044 hashes and owner
INIs remain unchanged. Source guards additionally preserve Lib, Emulator,
CPU public interface, instruction handlers and the old timing-model owner.
[Results](t2-s37-results.json) retain exact suite and receiving records.

Both actual caches remain Release with the existing cmake/env launcher
setting the same persistent CCACHE_DIR and calling ccache. An initial
ad-hoc inspection incorrectly required the launcher to be the bare word
ccache; checking the actual documented command-list representation resolves
that inspection mismatch without changing configuration or any qualification
assertion. All ten actual objdump section inspections find no compiler debug
sections; PC retains its existing --strip-debug policy, not --strip-all.
Both source/test manifests, protected-input, whitespace and affected
documentation checks pass. There was no final build or dynamic test failure.

Actual committed-diff coordinator acceptance remains before closure. Full
external integration, manual desktop/RDP/audio, operand-specific silicon
timing, complete physical time, board/Machine and gameplay qualification
remain unclaimed. No MyRaidenII EXE exists yet.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S37, next unused S after accepted S36. |
| Admission And Approval | Owner's standing automatic single-person coordinator/executor MyRaidenII implementation and timing-upgrade approval; coordinator admits the complete V07/V15 ordinary multiplication/division nominal batch on 2026-10-18 at clean 988a439f6. Targets Shared CPU/tests, NXVM eight receiving artifacts/status, MyRaidenII records. No Lib/Emulator/MyNES or public contract change. |
| Objective | Give all ten ordinary byte/word MULU/MUL/DIVU/DIV and immediate-byte/word MUL encodings a source-graded nominal disposition at the existing selector. DIVU success uses original fixed rows; data-dependent ranges use their documented upper endpoint as an explicitly conservative L2 nominal model. Retain full semantic, accepted-prefix, failure and real DE/IRET proof at the existing test owner. |
| Non-goals | No new executor, operand read, result/state mirror, API/enum/layout, guest arithmetic/flag/entry semantics, Machine admission or total physical-time qualification. No assertion that a range endpoint is the true operand-specific silicon cost. DE zero/overflow time, extra prefixes, modes, bus/prefetch/waits remain original V11/V15 receivers. |
| Reference Baseline | 988a439f6; pinned NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed 12-50..60 (PDF95..105), all eleven pages rendered and visually inspected. Existing S19 complete native arithmetic/result/DE proof and S29 successful software-entry fact. |
| Candidate Proposal | proposals/m1-v30.md, original V07/V11/V15 ledger. DIVU register byte/word=19/25, even memory=25/31, odd word=35. Conservative L2 register upper MULU=22/30, MUL=39/47, DIV=34/43, immediate-byte/word MUL=34/42; memory adds six, odd word adds four. Use existing L2_DYNAMIC_ARITHMETIC origin only for range selections; exact DIVU uses V30_NOMINAL_SOURCE. Never add Intel EA or per-provider-cycle charges. A successful software-entry fact excludes DE paths rather than guessing the combined entry subtotal. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and source manifest; test/x86-core/cpu/cpu_v30_multiply_divide_smoke.c, the ALU test's four arithmetic unknown-clock placeholders and test manifest. Newly allocated ordinary placeholders become the same forms with an extra segment prefix, retaining unknown-publication coverage and original ALU counts. No header/registration or public function/type/enum/layout change. Estimate production +25..40/-5..10, tests +50..80/-0..5; existing owner plus one bounded test-local oracle. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/queue/roadmap/architecture/coding/source policy, shared Execution/Architecture/Coding/Documentation, architecture/coding governance and read-only PDF skills; NXVM receiving guide/current/architecture/coding/source policy and X86 Core README. One execution/data/time/publication owner, Lib Types, honest L2 grade, frozen proofs, target-separated Ps and actual committed-diff acceptance. |
| Verification | Extend the existing complete multiply/divide real execution matrix with independent explicit source-endpoint tables, exact key/input/origin/max/unknown checks and no additional observation. Preserve all original full CPU, defined flags, aliases, ModRM/segment/wrap, DE/IRET and refusal proofs; add explicit even/odd memory successes for every family. Transient focused selection: multiply/divide, nominal transfer, ALU, control flow, software arbitration, completion, form admission. Both original Ninja/ccache all-target builds -j8, complete 678 repository units -j4, specialized gates/498 strict rows and Types; 14 selected PC integrations plus restored default identity per width; ten PE/hash records and protected-input/docs checks. |
| Expected Markers | Existing V30:MULTIPLY-DIVIDE:OK count retained or expanded with original cases preserved; new V30:NOMINAL-MULTIPLY-DIVIDE:OK. All final focused/full suites zero failures/skips; exact DIVU versus explicit L2 origins, zero/unattributed DE/prefix forms, maximum0 and closed V30 Machine admission preserved. |
| Asset Needs | Original synthetic ROM-free unit inputs and pinned manual; reuse original cached graphs. Rebuild/deploy eight current PC 0564 releases, validate all ten PE/hash identities and unchanged MyNES0044/INIs/firmware/media/snapshots. No new ROM, imported code, MyRaidenII EXE or required native desktop input. |
| Reporting Requirements | Report source/model decision, file/diff estimate before implementation, actual failures and corrective decisions, counted production/test add/delete/net, retained unique paths and original ledger dispositions, complete final verification/artifact/commit proof and unverified whole-game boundary. |
| Stop Conditions | Public/source/license authority change, operand observation introduced for timing, Intel/guest semantics changed, source range represented as exact data-dependent silicon, owner-input mutation, or incomplete required qualification prevents acceptance. No lowered assertions/budgets, new persistent state, second interpreter/time path or eligibility promotion. |
| Exit Criteria | Every member of the ten-form batch has its documented fixed or explicitly graded L2 nominal success disposition and original DE/prefix/mode unknown receiver, proved at actual execution and failures. Both widths complete builds/units/gates/selected receiving integrations/artifacts/docs pass, complete target-separated Ps are pushed and coordinator actual committed-diff acceptance closes only S37. |
| Original Owner Request | Single-person two-role governed implementation of complete playable MyRaidenII, NEC V30 and needed facilities. Nominal CPU clocks are not complete physical timing, board/Machine integration, video/sound or the full gameplay outcome. |
| Similar-Issue Sweep | Audit all F6/F7 arithmetic subgroups plus 69/6B through the sole selector and actual ordinary test owner: register/immediate/memory aliases, byte/word parity, original versus mutated operand facts, DE versus success publication, unknown prefixes/modes, stale repeat identity, no extra observation, key collisions, source/L2 origins and existing Intel/max/Machine gates. Other source-unknown arithmetic/entry families retain original ledger receivers. |
