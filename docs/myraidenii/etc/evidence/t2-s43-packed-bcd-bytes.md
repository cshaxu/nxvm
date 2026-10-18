# T2 S43 Packed BCD Full-Byte Semantics

Baseline `4ccb0e098`, after accepted S42. This batch consumes the complete
three-suffix ADD4S/SUB4S/CMP4S odd/even byte-operation and defined-flag
mechanism at V04/V07/V14/V15. It does not complete V30 or the game.

## Primary Source Reconciliation

NEC `16-Bit V Series Instruction`, U11301EJ5V0UMJ1, fifth edition,
published September2000; SHA256
`19A69547651AA65E32DF33F1BF84DEA095A00237F680A67B166213FCC6D96F6A`.
[Original document](https://www.ardent-tool.com/CPU/docs/NEC/V20-V30/v_series.pdf).
The cover, preface and reading instructions explicitly include uPD70116 V30,
not merely V30MZ or another later architecture. No manual payload or third-
party source is imported into the tracked corpus.

Nine full PDF pages1,6,7,26,70,168,180,181,195 are rendered and reviewed:
identity/applicability; printed15 ADD4S,59 CMP4S,157 SUB4S;169 timing
assumptions;170 first clock rows and digit-count note;184 table end. The digital PDF renderer
warns about unavailable Symbol/ArialUnicode fonts: some symbolic flag marks
are missing in the rendering. The complete prose/cautions, U entries, names
and numeric columns remain legible. Flag marks are cross-checked against
the digital text and the original scanned1986 printed12-42 (PDF87), also
fully rendered/reviewed. Do not call missing-rendered-glyph evidence a clean
new-paper flag-table rendering. The pinned1986 manual hash remains
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.

The1986 text says odd-count flags may not indicate the requested digit width,
and leaves the high result nibble unspecified. The later manual clarifies
the complete mechanism: operations always process even digit counts; odd CL
executes CL+1 digits, with result and flags for that full width. Zero-padding
the last input high nibbles is caller advice, not an implicit CPU mask.
For padded ADD of9+1 with CL1, result is10h and CF remains clear; padded
SUB of0-1 gives99h with CF set. A nonzero valid decimal high nibble participates
in the operation. The old local masking and undefined-CF/ZF policy are
superseded by this more explicit source applicable to the same original V30.

The even nominal19*m+7 table is retained. Both editions still write m as
half the digit count without an explicit rounding formula. Applying that
nominal row to rounded full-byte work for odd CL is explicitly the existing
L2_DYNAMIC_ARITHMETIC model, not a quoted exact cycle rule. Other prefix terms
remain at the existing sole finish. No total-time/max/Machine gate opens.

The new table also differs from1986 scalar ADJ4S/ADJBA clock entries. Those
scalar forms, arbitrary-input adjust semantics and non-ten conversion bytes
remain original V07/V15 receivers: this batch does not silently change prior
clock values/grades or promote a contradictory table to silicon truth.
The current source witness is recorded for their later complete reconciliation.

## One Production Mechanism

Remove the odd-tail source/destination masks from NEC_PACKED_BCD. Its existing
base100 operation, validation, memory provider, transfer order, carry chain,
zero reduction and final publication own every byte. Invalid decimal nibbles
and0/255 lengths retain existing diagnostic rejection, without a fabricated
guest INT6. Remove odd-only undefined CF/ZF annotation; the existing defined
carry/zero commit now applies to the rounded full byte width.

The existing V30 timing selector uses rounded byte count and labels only odd
subtotals L2. No new helper, field, decoder, queue, state, thread, public
API/enum/layout or alternative BCD path. Intel handlers are unchanged.
Production two existing C paths +5/-11/net-6. One existing test C path
+61/-31/net30; code total+66/-42/net24, from Git numstat excluding docs/
artifacts/manifests. Two manifests separately+3/-3/net0. Estimates were
production+5..18/-12..28, tests+65..145/-20..55; actual is smaller because
the complete mechanism removes the incorrect mask/undefined exceptions.

## Independent Actual-Execution Proof

The existing CPU-owned BCD unit retains its digit-at-a-time oracle, distinct
from production base100 arithmetic. It now checks both final nibbles and
CF/ZF for odd counts, removing the workaround that copied the actual high
result nibble into the expected image. Positive verification compares the
entire CPU image, masking only the four genuinely undefined OF/SF/AF/PF
bits, rather than only selected GPRs. Complete RAM, instruction size,
pure/truncated preview, no extra observations and transfer counts remain.

Original30,000 even operand pairs remain; all100x100 decimal byte pairs for
each suffix also run with CL1, adding30,000 odd cases. Existing all1..254
counts in both directions retain their inputs, including nonzero high digits.
The127 odd lengths are no longer accepted by ignoring high result bits/flags.
Prefix/count/address/wrap/overlap and original six-instruction examples
remain. Thirty old prefix/odd fixtures that used invalid F9/E1 bytes now use
valid99/91 to prove the full high digit; F9/E1 independently reappear in
the explicit rejection seam, rather than being erased by CPU masks.

One test-local table checks eight independently written padding/high-digit/
carry/borrow/zero witnesses across five source-segment selections and both
DF values:80 actual executions. Examples include ADD90h+10h producing00h
with carry/zero, CMP10h versus20h not falsely reporting equality and the
manual's padded9+1 and0-1 contrasts. It owns real primary-source witnesses,
not a production framework or a duplicate implementation.

The complete success seam has61,688 inputs: original31,608 plus30,000
odd-byte pairs and80 witnesses. Every24 existing data/write-provider refusal
is repeated for an odd5-digit and even6-digit three-byte operation,48 total.
All accepted RAM prefixes and whole preinstruction CPU rollback, absent
completion/stale timing and the executor-owned stop-consumption contract
remain checked. Original nine invalid-length/nondigit cases remain and six
odd high-nibble rejections add explicit source/destination coverage,15 total.

## Development And Similar-Issue Sweep

The initial actual execution of packed BCD and memory-transfer units passes
after removing the incorrect masks/undefined policy. Expanded independent
padding/failure/whole-CPU assertions also pass x64; both final five-case
focused groups subsequently pass. No failed production test is hidden by
a relaxed guest assertion or timeout increase.

Query `odd.{0,65}(undefined|unspecified|mask|unallocated)|unused.{0,30}nibble`
and the source/destination nibble operations across src/test X86 Core.
Only this packed-string owner contained the discarded-nibble mechanism.
Search all three suffix users, byte-count consumers, undefined-flag masks,
timing keys/origins and actual memory-transfer/fetch seams. Nondecimal
diagnostics remain intentionally separate; scalar adjusts and conversion
source contradictions retain original V07/V15, not guessed common behavior.
CPU/Intel execution outside the NEC BCD handler, public/private layouts and
the S42 repeat-byte identity owner are unchanged. Full V01--V17/proposal/
Queue and physical/emulation/Machine/board/video/sound/game scope remain.

## Qualification

Both original cached Ninja all-target builds and five-case focused groups
pass. Complete repository units pass678/678 per width, including191 X86 Core
tests, zero failures/skips; full units take305.61/138.07 seconds. Strict498
and Types1389 gates pass with historical warning exceptions retained, not
an exemption-free compilation claim. [Results](t2-s43-results.json) preserve
old/new markers, actual counts and receiving case identities.

All14 selected receiving integrations plus restored-default identity pass
per width. DeskPro groups take113.09/168.08 seconds. Eight PC0564 EXEs are
rebuilt/deployed; [ten artifact identities](t2-s43-artifacts.json) pass PE/
hash/absent-debug-section checks. MyNES0044 and INIs remain unchanged.
[Six media originals](t2-s43-media-inputs.json) retain actual pre-integration
hashes/sizes/modes after execution; this is not a whole external archive
audit. Existing firmware source/embedding gates stay unchanged. Both original
graphs finish at default, Release and the persistent ccache launcher retained;
profile switches/restores recompile zero CPU objects, checked with both
Ninja path-separator spellings.

Final owned runner53327 is terminal exit zero; the collector independently
checks frozen source/test/public/private-layout hashes, actual XML/logs,
restored graph/launcher/object reuse, all10 PE/sections/hashes and protected
inputs. Ten relevant reviewed page images are removed; the first mislocated
1986 INS page rendering was also removed when corrected to BCD12-42.
Pinned manuals/text, logs and reusable cache remain. No final build/dynamic
test fails. No fresh full external/native/desktop/RDP/audio/gameplay
qualification or MyRaidenII EXE is claimed.

## Admitted Requirement Packet

Shared P1 `bb03dcd87` and NXVM receiving P2 `71ce33f38` are pushed.
NXVM Current changes only four receipt lines; its unchanged historical suffix
was compared exactly and its per-file numstat/diff reviewed before commit.
This applies S42's recorded prevention rather than trusting topology alone.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S43; next unused S after accepted S42 at4ccb0e098. |
| Admission And Approval | Owner standing automatic implementation, single-person dual-role and V30/NEC semantic/timing qualification approval, continued2026-10-18. Targets Shared X86 Core/tests, eight NXVM receiving releases/status, MyRaidenII evidence. No Lib/Emulator/MyNES/public-contract change. |
| Objective | Consume the complete ADD4S/SUB4S/CMP4S packed-string odd/even byte-operation and defined-flag mechanism at V04/V07/V14/V15: stop discarding encoded high nibbles, compute full-byte CF/ZF for odd counts, qualify every documented1..254 length and source/model timing through existing execution/failure seams. |
| Non-goals | No second BCD handler, framework, execution clock or state. No public API/enum/layout, old Intel, Lib/Emulator/App configuration change. Packed scalar-adjust source conflicts/generalized radices, non-decimal/0/255 lengths, physical bus/full time/max/Machine/board/game retain original named receivers; no silicon claim for unspecified values. |
| Reference Baseline | Clean pushed4ccb0e098. Pinned1986 manual FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C and newly reviewed NEC16-bit V Series Instruction U11301EJ5V0UMJ1 fifth edition September2000, explicitly covering uPD70116 V30, SHA19A69547651AA65E32DF33F1BF84DEA095A00237F680A67B166213FCC6D96F6A; source https://www.ardent-tool.com/CPU/docs/NEC/V20-V30/v_series.pdf. Full relevant pages visually reviewed before implementation. |
| Candidate Proposal | proposals/m1-v30.md, original V04/V07/V14/V15. Latest primary description establishes full-byte operation when CL odd; caller zero-padding is advice, not CPU masking. Existing base100 execution and independent digit oracle own all bytes/CF/ZF. Existing even-clock source row remains; odd rounded-byte subtotal, if allocated, explicitly existing L2_DYNAMIC_ARITHMETIC because table m is written as digit_count/2 without rounding. No promotion to total-time eligibility. |
| Files And ABI Surface | src/x86-core/cpu/cpu_instructions.c, cpu_timing.c, source manifest; existing cpu_v30_packed_bcd and cpu_v30_memory_transfer tests/test manifest. Estimate production +5..18/-12..28; tests +65..145/-20..55, no new test target/fixture/state/public declaration. Receiving eight PC0564 EXEs/status and product-local evidence in separate target commits. |
| Applicable Rules | Product guide/goal/architecture/coding/roadmap/queue/proposal/full ledger/source policy, Shared Execution/Architecture/Coding/Document and Contributing, architecture/coding-governance and read-only PDF skills; receiving NXVM reading set and X86 Core README. One BCD execution/validation/failure/flag owner, Lib Types, source versus model grades, original Intel behavior, target-separated Ps and actual committed-diff closure review. |
| Verification | Actual guest CPU execution, independent digit-at-a-time complete byte/RAM/CPU/defined-flag oracle; all3 suffixes, all100x100 decimal byte pairs, all1..254 counts/DF, odd unpadded valid high digits versus zero-padding and carry/borrow/zero, full source overrides/alias/wrap, every observed applicable provider refusal before/after accepted prefix. Preserve original valid inputs rather than masking away real high nibbles. Keep existing diagnostic invalid-nibble/length failures explicit, no fabricated guest INT6. Both original Ninja/ccache all-target builds-j8, full repository units-j4, specialized strict/Types gates;14 selected receiving integrations plus restored identity per width; eight PC0564 releases and ten PE/hash/debug-section/protected-input identities. |
| Expected Markers | Existing V30:PACKED-BCD:OK, V30:NOMINAL-BCD:OK and V30:MEMORY-TRANSFER:OK retained, plus source-backed odd-byte result/flag witnesses with actual counts. Both complete unit suites zero failures/skips. Native opcode semantics corrected only for this sourced mechanism; all old Intel timing/handlers and V30 max0/unallocated/Machine guard preserved. |
| Asset Needs | Original synthetic vectors and pinned manuals only; read-only originals external. Ten reviewed page images at most under ignored build/research removed after review, retain manuals/logs/cache. Capture six media input identities before receiving integration; eight PC0564 outputs, unchanged MyNES0044/INIs/media/firmware/snapshots. No ROM import or MyRaidenII EXE yet. |
| Reporting Requirements | Report source conflict/reconciliation and complete mechanism before code, per-component estimate and actual Git added/deleted/net excluding docs/artifacts/manifests. Record failures and corrections, source/model choices, retained paths/state, exact dual-width proof/releases/commits and remaining full objective without implying gameplay or complete CPU. |
| Stop Conditions | Missing source authority or changed public/license boundary, new duplicate/mirrored state/decoder/BCD path, changed Intel semantics, unsafe accepted-prefix rollback, unbounded storage, invented silicon semantics, protected-input mutation or failed required verification prevents acceptance. New scalar timing-table conflicts are inventoried at original V07/V15, not silently changed under this batch. |
| Exit Criteria | All documented packed-string lengths use one full-byte operation/validation and defined CF/ZF owner; both independent oracle and actual provider/failure/alias/wrap seams prove it. Required complete dual-width qualification, eight receiving releases, manifest/evidence/doc checks and target-scoped commits pushed; coordinator actual-diff review closes only S43, T2/M1/full playable goal remain open. |
| Original Owner Request | Single-person dual-role governed implementation of complete playable MyRaidenII and its NEC V30/board/video/sound/input/host facilities. This BCD mechanism is one required batch, not redefinition of CPU or product completion. |
| Similar-Issue Sweep | All odd-count BCD byte counts/masks/undefined flags/nominal/source assertions across CPU instructions/timing, packed-BCD/memory-transfer units, scalar DAA/DAS/AAA/AAS/AAM/AAD and current evidence; fix the complete three-suffix shared owner. Retain full V01--V17, named unknown scalar/invalid-input/emulation/full-time/Machine/board/game receivers; do not rewrite historical source claims. |
