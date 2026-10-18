# T2 S39 Native Data Range Nominal Models

Baseline `eb1299862`, after accepted S38. This consumes all four remaining
ordinary native INS/EXT register/immediate range rows and CVTWL at original
V04/V07/V14/V15, not complete physical CPU time, neutral Machine or gameplay.

## Original Source And Explicit Model

NEC October1986 V20/V30 User's Manual SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed12-21..24 and12-64 (PDF66..69,109), all five complete pages, are rendered
and visually inspected in this S. No implementation/source/manual/ROM payload
is imported or compiled.

| Complete ordinary encoding | Original even / odd ranges | L2 endpoint even / odd |
| --- | --- | --- |
| INS register,0F31 | 31..117 /35..113 | 117 /113 |
| EXT register,0F33 | 26..55 /34..59 | 55 /59 |
| INS immediate,0F39 | 67..87 /75..103 | 87 /103 |
| EXT immediate,0F3B | 21..44 /25..52 | 44 /52 |
| CVTWL,99 | 4 or5 according to data, no selector given | 5, no memory term |

These are explicitly conservative nominal models using the existing
L2_DYNAMIC_ARITHMETIC origin, never a claim of the real operand-specific
microsequence. INS register's printed even upper117 exceeds odd113; that
fact is retained, not normalized into a guessed common odd penalty. The EXT
register even line prints70115 although the facing header, odd line, chip
family and surrounding entries concern70116. The contextual even row remains
an explicitly modeled source-typo interpretation, not falsely exact evidence.
No relationship between offset/length/value and microcode cost is invented.

CVTWL's upper5 likewise does not guess which sign/value selects4 versus5.
The original source/undefined/alias semantic qualifications remain. Other
BCD rounding, prefixes, ordinary8080, asynchronous/DE entry, physical bus,
prefetch and waits retain the full original V01--V17 receiving domain.
The total remains source-time-unallocated, maximum0, Machine admission closed.

## Unique Owner And Necessary Diff

Only the existing private V30 selector changes. It reads copied original
SI for extraction or DI for insertion, then publishes implicit-memory/parity,
existing suffix identity and L2 origin at its existing final boundary.
No operand observation, reread, length decoder, mutable timing cache, second
entry/execute path, state, production helper or public function/enum/layout
is added. Intel timing, handlers, Lib and Emulator remain unchanged.

Actual Git numstat counts one production C and three existing test C files,
excluding documentation, manifests and artifacts: production+17/-1/net+16;
tests+47/-28/net+19; total code+64/-29/net+35. Manifests separately+4/-4/net0.
The source is slightly below the estimated addition range; three extra deleted
test lines versus the estimate come from reindentation of the existing matrix
body under its real added parity axis and its source-comment correction.
No unrelated formatting is performed.

The existing bit-at-a-time independent field oracle is unchanged. Every old
offset/length/register/output-alias, wrap, segment prefix, defined-flag, exact
memory and callback-order assertion remains. The complete ordinary matrix
now runs both even/odd addresses:66048 combinations plus60 existing boundary/
prefix routes give66108 counted successful runs.
The unused SI/DI index has opposite parity throughout that matrix, directly
rejecting a selector that uses the wrong implicit index while preserving the
full independent CPU/memory result oracle. Original real examples,
invalid-form/truncation probes and all twelve provider-refusal routes remain.
Their failure assertions already prohibit successful clocks/origin publication.
One existing execution seam additionally compares independent conditional
source rows, suffix key, implicit memory/odd inputs, L2 origin, unallocated
total/max0, event0 and no observational reads. No new helper or target.

The ALU's original917460 inputs retain their full sign-extension and defined
CPU proof; every unprefixed CVTWL now expects5/L2, while CVTBW retains2/source.
Nineteen adjacent unknown/refusal witnesses remain. Newly allocated unknown
placeholders are replaced by legal extra-prefixed bit/CVTWL counterparts.
The nominal-transfer's original24160 inputs likewise retain its complete
publication/failure/Intel contrasts. Its live success -> unknown -> success
sequence uses prefixed CVTWL so a legitimate new clock row cannot erase the
important no-stale-publication contract. No assertion or budget is weakened.

## Similar-Issue Sweep

Exact queries:

- `rg -n 'suffix == 0x(31|33|39|3b)u|case 0x99u|L2_DYNAMIC_ARITHMETIC' src/x86-core/cpu/cpu_timing.c`
- `rg -n 'CVTWL|0x99u|0x39u|unallocated|EXCLUDED' test/x86-core/cpu --glob 'cpu_v30*'`
- `rg -n 'source_timing_unallocated|maximum_ticks|observations|timing_result.ticks' test/x86-core/cpu/cpu_v30_bit_field_smoke.c test/x86-core/cpu/cpu_v30_alu_smoke.c test/x86-core/cpu/cpu_v30_nominal_transfer_smoke.c`

All four suffixes share the one existing NEC suffix dispatch/publication.
The original implicit index is authoritative, not the advanced SI/DI or an
output-alias byte. Source-table parity is not data-callback counting.
CVTWL has no memory/EA/parity term. The two unknown-placeholder owners and
live-stream owner are updated at their existing production-execution tests.
The independent field and ALU oracles retain distinct semantic responsibility;
neither is another timing executor. Existing source-fixed versus model rows,
prefix refusals, no failed publication and Intel guards remain.
Full timing/prefix/8080/semantic-source questions keep their original receivers.
The bit/nibble unit's suffix catalogue is form-admission coverage, not another
timing owner; it stays unchanged. The memory-transfer unit compares wrapped
execution against its nonwrapped reference and retains all original clocks/
key equality and host-failure checks, while the field seam supplies independent
source expectations. Decimal/packed-BCD literals99 are operand values, not
CVTWL encodings. The source selector's remaining L2 arithmetic sites are the
separately qualified multiplication/division families, not duplicate field
models. No unclassified duplicate production hit remains in this batch.

## Pre-Final Review Correction

The first candidate passed both seven-unit focused groups. While the first
full x64 unit suite was still running, actual-diff review identified a test
blind spot: SI and DI were always initialized equally, so both parity rows
alone could not reject selection of the wrong index owner. Coordinator
clarified the existing original-index proof; the executor deliberately stopped
the sole owned runner and its process tree before changing test inputs. Its
handle is terminal, all identified children are absent and both graphs remain
default. This interrupted suite is not a pass or a test-failure claim.
The partial log is retained as
`build/myraidenii/t2-s39-pre-review-partial-unit-x64.log`; original focused
XMLs use `t2-s39-pre-review-focused-{x64,x86}.xml`.

Only the existing matrix setup changes: the unused SI/DI index now has
opposite parity. Production is unchanged. Full CPU/memory comparison keeps
checking that unused index is untouched. ALU's source comment is corrected
to include its already used12-64 conversion page. Final proof restarts with
frozen corrected inputs; no assertion/budget is weakened. Prevention: unequal
values are required when two candidate owners could otherwise be confused.

## Final Qualification

Both final seven-unit focused groups, original all-target builds and complete
678/678 repository units (191 X86 Core) pass per width, zero failures/skips.
Full units take124.92/140.61 seconds; the66108 field seam takes0.286162/
0.296119 seconds. Both498-row retained-strict and1389-input Types gates pass;
historical warning exceptions remain, not exemption-free strict compilation.

Eight PC0564 EXEs are rebuilt/deployed. Fourteen selected receiving integrations
plus restored-default identity pass per width. DeskPro groups take141.91/
168.18 seconds. Both original graphs end at default; profile switches/restores
recompile zero CPU objects. All ten PE/hash/debug-section checks pass. Actual
cache launchers retain Release/persistent ccache. MyNES0044, INIs and protected
source/interface/handler/model/firmware/media inputs remain unchanged. The
sole final runner is terminal exit zero; no final build/dynamic test fails.
Original interrupted pre-review proof is retained separately above.

[Results](t2-s39-results.json) preserve exact counts/names/logs;
[artifacts](t2-s39-artifacts.json) preserve all ten deployed identities.
Shared P1 `de015fde0` and NXVM receiving P2 `e453f8f1f` are pushed;
coordinator actual committed-diff acceptance follows the records delivery.
No fresh complete external suite, manual desktop/RDP/audio, physical time,
Machine/board or gameplay qualification is inferred. T2/M1 and the original
full goal stay open. There is still no runnable MyRaidenII EXE.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S39, next unused S after accepted S38. |
| Admission And Approval | Owner's standing automatic single-person coordinator/executor MyRaidenII implementation and timing-upgrade approval; coordinator admits the complete remaining range-valued native data nominal batch on 2026-10-18 at clean eb1299862. Targets Shared X86 Core/tests, NXVM eight receiving artifacts/status and MyRaidenII records. No Lib/Emulator/MyNES or public contract change. |
| Objective | Allocate all four native INS/EXT register/immediate encodings plus CVTWL using explicit conservative source-range upper endpoints at the sole V30 timing selector. Preserve original semantics, complete actual data/alias/defined-flag/failure proof, source-model grading and original unallocated elapsed-time receiver. |
| Non-goals | No instruction/executor/decoder, operand reads, mode/entry/priority, public API/enum/layout, persistent state, Intel or Lib/Emulator change. No claim of actual input-specific microcode, new 8080/prefix/BCD rounding/physical bus/prefetch/wait cost or Machine/board/game admission. |
| Reference Baseline | eb1299862; original NEC October1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed12-21..24 and12-64 (PDF66..69,109), five pages rendered and visually inspected. Existing S6 bit-field execution and S17/S34 arithmetic proof; original S35 ranges remain historical evidence, not current allocation. |
| Candidate Proposal | proposals/m1-v30.md and original V04/V07/V14/V15. Existing L2_DYNAMIC_ARITHMETIC labels conservative endpoints: INS register117/113, immediate87/103; EXT register55/59, immediate44/52 (even/odd implicit memory); CVTWL5. EXT register even row prints70115 amid70116 context: retain that source typo explicitly; no exact-silicon assertion. Original SI for extraction and DI for insertion determine parity; no per-transfer slopes or length-to-microcode guesses. Field publication records implicit MEMORY/ODD and suffix identity; CVTWL has no memory term. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; existing cpu_v30_bit_field_smoke.c, cpu_v30_alu_smoke.c, cpu_v30_nominal_transfer_smoke.c and test manifest. Estimate production +18..28/-1..3; tests +35..75/-12..25, no new helper/target/header. Extend complete field matrix to both parity axes; retain every ALU/transfer input and nineteen ALU unknown/refusal boundaries with legal extra-prefixed counterparts where newly allocated placeholders no longer apply. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/queue/roadmap/architecture/coding/source policy, Shared Execution/Architecture/Coding/Document and Contributing; architecture/coding-governance and read-only PDF skills; receiving NXVM guide/current/architecture/coding/source policy and X86 Core README. Unique execution/data/timing/publication owners, Lib Types, honest L2, frozen proof, one target per P and coordinator actual committed-diff acceptance. |
| Verification | Actual complete four-form field execution matrix covers every offset/length/register alias at even and odd memory with the unused SI/DI index having opposite parity, boundaries/segment prefixes/real examples/invalid forms/all existing provider refusals; independent source-row/key/input/origin/no-observation expectations. Preserve all917460 ALU inputs including full CVTWL sign-extension domain and all24160 transfer inputs, failure/publication/Intel guards. Transient focus: bit-field, ALU, nominal-transfer, memory-transfer, decimal, software-arbitration and completion. Both original Ninja/ccache all-target builds -j8, full678 repository units -j4, specialized498 strict rows/Types,14 selected PC receiving integrations plus restored identity per width, ten PE/hash records and protected-input/docs checks. |
| Expected Markers | Retain V30:BIT-FIELD:OK, V30:NOMINAL-ALU:OK cases=917460 and V30:NOMINAL-TRANSFER:OK cases=24160; add V30:NOMINAL-FIELD:OK cases=66108 for the both-parity matrix. All final suites zero failures/skips; explicit L2_DYNAMIC_ARITHMETIC, no extra observations, unknown extra-prefix disposition, source_timing_unallocated/max0/closed Machine preserved. |
| Asset Needs | Original synthetic ROM-free inputs and reusable Ninja/ccache graphs only. Rebuild/deploy eight PC0564 EXEs, verify all ten identities and unchanged MyNES0044, INIs, firmware/media/snapshots. No new source/ROM import or MyRaidenII EXE; retain manual and proof/logs, remove five owned temporary source renderings after review. |
| Reporting Requirements | Report source/model/owner decisions, exact paths and estimated diff before implementation; preserve actual failures and corrections. Deliver actual production/test added/deleted/net, retained unique paths, complete batch disposition, dual-width qualification/artifacts/commits and remaining full-goal boundary. |
| Stop Conditions | New public/source/license authority, hidden guest/Intel semantic change, source range promoted to exact silicon fact, new execution/timing state or observation, protected-input mutation or failed qualification prevents acceptance. No weakened assertions/budgets or fabricated eligibility. |
| Exit Criteria | All five range-valued native data encodings have explicit L2 nominal endpoints at the existing selector and actual full execution/failure proof. Complete required dual-width builds/units/gates/selected integrations/artifacts/docs pass; complete target-separated Ps pushed; coordinator actual committed-diff review accepts and closes only S39. |
| Original Owner Request | Single-person two-role governed implementation of complete playable MyRaidenII, NEC V30 and needed facilities. This source-graded batch does not complete full CPU physical time, neutral Machine, board/video/sound or gameplay. |
| Similar-Issue Sweep | All four field suffixes and CVTWL source ranges/callers, original versus mutated SI/DI/AX/alias inputs, implicit-memory parity, source/L2 origins and keys, unknown placeholders/live publication reset, each existing host refusal, no extra observation and Intel/max/Machine guards. Remaining native/8080/prefix/BCD/entry/bus/model questions retain original V01--V17 receivers, not removed to narrow completion. |
