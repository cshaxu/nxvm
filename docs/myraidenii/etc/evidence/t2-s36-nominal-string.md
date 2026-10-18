# T2 S36 Ordinary String Nominal Clocks

Baseline `7d7eadf5f`, after accepted S35. This consumes the complete ordinary
V30 string nominal-clock family at original V09/V15, not complete physical
time, Machine admission or gameplay.

## Original Source And Complete Family

NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-1..3,14..20,28..29 (PDF 46..48,59..65,73..74), twelve pages,
were rendered and visually inspected. Original-source clocks assume
prefetched instruction bytes and include operand/EA work; this is not
an Intel EA recipe or a count of actual byte callbacks.

| Byte/even-word source family | Primitive | Repeat setup | Per iteration |
| --- | --- | --- | --- |
| MOVBK / MOVS | 11 | 11 | 8 |
| CMPBK / CMPS | 13 | 7 | 14 |
| CMPM / SCAS | 7 | 7 | 10 |
| LDM / LODS | 7 | 7 | 9 |
| STM / STOS | 7 | 7 | 4 |
| INM / INS | 10 | 9 | 8 |
| OUTM / OUTS | 10 | 9 | 8 |

Each odd word endpoint adds four clocks to its row, except repeated MOVBK:
the printed odd/odd and odd/even iteration rows both specify sixteen, not
twelve for a mixed pair. That source distinction is intentionally retained.
Memory endpoints use original SI/DI, port endpoints use original DX; byte
operations do not acquire a word penalty. Source parity classes treat a
mixed pair as one odd endpoint, without inferring extra bus sequencing.

All fourteen opcodes have primitive and canonical-repeat dispositions.
F3 is canonical for transfer/load/store/I/O; comparison additionally admits
F2/65/64. One repeated instruction charges its printed aggregate setup once,
then only the documented iteration row on subsequent successful refreshes.
Zero count uses setup only with no memory/odd-word modifier. The separately
printed prefix clock is not added to a table aggregate a second time; this
does not claim full prefix/fetch/wait accounting. Redundant, segment, LOCK
and noncanonical repeat prefixes remain zero/unattributed at the original
V05/V15 receiver, as do other unallocated forms and modes. No total-time
eligibility or maximum is opened.

## One Repeat Owner And Real Commit Boundary

The existing private source-timing repeat recorder is reused by V30 and the
old Intel consumers. Only its static qualifier is removed and one declaration
is added to the existing private CPU header. Its body, identity, reset
fields and Intel callers are unchanged. No new production helper, mutable
state, public declaration/enum/layout, thread, clock, executor or decoder.
The V30 selector records the existing repeat phase and consumes original
decoded/count/index/port facts at the existing publication boundary.

A successful non-string or unsupported/prefixed selection clears the same
repeat identity. Reset retains the existing initialize/reset clear. Actual
V30 real interrupt entry now clears that identity at its existing successful
CS/vector commit, including when the handler aliases the outgoing REP's
CS:IP. Failed frame/vector/CS preparation returns before this new clear.
The change affects timing identity only, not guest registers, accepted
memory effects, priority, interrupt acknowledgement or execution semantics.
Source repeats still cannot qualify neutral Machine while full physical
time is unallocated; maximum remains zero.

Primitive form keys are their original opcode; canonical repeats add the
actual prefix in the high sixteen bits of the existing opaque key. NEC
extension keys retain low 0F, so the namespaces do not collide. Repeat/phase,
memory and odd-word flags reflect the actual source formula, without a
second count or observational operand read.

## Actual Proof And Failed Attempts

Three existing real execution owners share one CPU-local test assertion
header, with independent explicit even/mixed/odd source rows. It is a
bounded oracle used by Block, Comparison and Port, not a framework or
production entry. Original complete CPU/RAM/flags/read-write order, code
wrap, segment, overlap and provider-failure proof remains.

Block's matrix grows from 960 to 1,920 inputs by adding equal as well as
opposite endpoint parity; all original inputs remain. Canonical zero,
first/continuation/terminal, DF and unsupported segment forms use the same
actual seam. All 942,920 comparison inputs remain, gaining source clocks,
phase/key/input/no-extra-observation proof, including carry and zero
termination. Port retains its full scalar/string, endpoint parity,
zero-count, accepted-prefix and actual IRQ/RETI tests. Actual bit-width
native bus behavior is not changed to match source clock arithmetic.
All failed instruction transfers keep old state/prefix assertions and
cannot publish completed timing. NMI/INT and IRET restart preserve full
existing guest behavior.

The first strict build rejected a signedness mismatch in the production
and test parity-count ternaries. Both branches now explicitly use Lib u8;
no warning policy is relaxed and no dynamic test ran on that failed build.
The original log is retained as
`build/myraidenii/t2-s36-initial-build-x64.log`.

Both first focused groups then passed nine tests, but subsequent actual
source review found the same-address entry alias. The still-live all-target
build was explicitly interrupted (terminal exit one) before source edits.
That partial build is not qualification and no complete-suite pass is
claimed for it. A direct new same-address NMI/INT regression reproduced
`op=a4 prefix=f3 step=0 clocks=8/19 phase=4/3` in the original timing
candidate. Its failed log is
`build/myraidenii/t2-s36-entry-regression-before.log`.
The sole successful V30 real-entry commit now clears the old identity;
all six Block opcodes are tested for both interrupt sources, ordinary
handler/IRET and same-address handler. No second generation flag or
guessed restart heuristic is introduced.

Transfer's two old canonical carry-comparison unknown-clock placeholders
now legitimately have source rows. They become the same carry forms with
an extra segment prefix, preserving their unknown-publication purpose
without weakening form admission or losing any of its 24,160 cases.
Final focused groups pass; final complete qualification is recorded below
only after the frozen runner finishes.

Prevention: inspect repeat identity across commit/return and same-address
aliasing before freezing the full qualification candidate, rather than
using a passing ordinary IRQ/IRET path as proof of every restart.

## Similar-Issue Sweep And Delta

Queries include:

- `rg -n 'source_repeat_active|source_timing_record_repeat|repeat_phase' src/x86-core/cpu`
- `rg -n '0x64u|0x65u|unallocated|timing' test/x86-core/cpu --glob 'cpu_v30*smoke.c'`
- `rg -n 'V30|timing_maximum_ticks' src/x86-core/machine src/x86-core/cpu`

All seven string families share one selection/publication owner and the
existing repeat recorder. Unknown extra prefixes clear that identity rather
than retaining a stale continuation. The successful common V30 entry is
the one commit receiver; existing SX-only pre-entry and failed-system-event
rules are distinct, unchanged and retain the prior explicit TODO receiver.
Reset and mode/non-string boundaries retain one identity clear. Legacy
Intel evaluators retain their separate documented tables with the same
existing phase recorder; no Intel row/body is changed. Original V01--V17
and general entry/mode/prefix/physical-time obligations remain open.

Counted tracked code against baseline, including the new 68-line assertion
header: production +66/-8/net +58; tests +116/-6/net +110; total code
+182/-14/net +168. Manifests separately +9/-8/net +1 (one new test header).
The method is Git numstat for existing files plus the new header's complete
line count, reconciled again against staged/committed Git numstat. This is
below estimate because existing state/phase and three real proof seams
are reused; no registration or new runtime abstraction. Historical warning
exceptions remain unchanged, not an exception-free strict-build claim.

## Final Qualification

Shared P1 is `4a8b42a56`; NXVM receiving P2 is `d46350c28`. Both are pushed.

Both original cached Ninja all-target builds and final complete repository
unit runs pass 678/678 per width, including 191 X86 Core tests, zero failures
or skips, in 301.29 seconds x64 and 138.22 seconds x86. Both final nine-unit
focused groups pass, including the corrected same-address entry regression.
Specialized gates pass all 498 retained strict rows and the global Types
audit covers 1,389 inputs. Original comparison, ALU, flag, transfer, bit,
nibble, BCD, shift, decimal, control, stack, completion and arbitration
proof remains in the full suite; no warning exception or budget is weakened.

Eight current PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus restored-default identity pass per width. DeskPro groups
take 118.91/159.84 seconds. Both original graphs are default and profile
switches/restores recompile zero CPU objects. All ten PE/hash records pass;
MyNES 0044 and owner INIs, firmware/media and snapshots are unchanged.
The final qualification runner is terminal with exit zero. [Results](t2-s36-results.json)
and [artifact identities](t2-s36-artifacts.json) preserve measured proof.
Additional package inspection confirms Release/ccache in both actual caches
and no compiler debug sections in all ten EXEs, using actual objdump section
names (including COFF long names). The existing PC recipe uses --strip-debug,
not --strip-all: ordinary COFF symbols remain. An initial ad-hoc zero-symbol
assumption therefore does not describe this recipe or the no-compiler-debug
requirement; no build policy, binary, budget or qualification assertion was
changed to hide a compiler debug section.
Actual committed-diff coordinator acceptance remains before closure.
Complete external integration, manual desktop/RDP/audio, full physical
timing and playable game qualification remain unclaimed.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S36, next unused S after accepted S35. |
| Admission And Approval | Owner's standing automatic single-person coordinator/executor MyRaidenII implementation approval; coordinator admits original V09/V15 complete ordinary string nominal family on 2026-10-18 at clean 7d7eadf5f. Targets Shared CPU/tests, NXVM eight receiving artifacts/status, MyRaidenII records; no Lib/Emulator/MyNES source or public contract change. |
| Objective | Allocate documented nominal rows for all fourteen byte/word MOVS/CMPS/SCAS/LODS/STOS/INS/OUTS opcodes, primitive and canonical repetition with zero/first/continuation/terminal phases, parity and original count/index/port facts. Reuse existing repeat identity/state owner; retain complete result/bus/failure proof and source-unknown prefix dispositions. |
| Non-goals | No execution/decoder semantics, physical elapsed-time/entry/bus/prefetch/extra-wait allocation, prefix-stream model, new state/queue/thread/framework, public API/enum/layout or Machine/board/game admission. Only source table aggregates, not adding a separately printed prefix row twice. Extra segment/redundant/LOCK and noncanonical repeat forms remain original V05/V15 receivers. |
| Reference Baseline | 7d7eadf5f; pinned NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed 12-1..3,14..20,28..29 (PDF 46..48,59..65,73..74), twelve pages rendered/visually inspected. Existing S7/S9/S10/S25 semantic/failure/IRQ proof and sole source_timing_record_repeat owner. |
| Candidate Proposal | proposals/m1-v30.md and original V05/V09/V14/V15 ledger. Primitive base MOVS/CMPS/SCAS/LODS/STOS/INS/OUTS=11/13/7/7/7/10/10. Repeat setup=11/7/7/7/7/9/9, iteration=8/14/10/9/4/8/8. Apply source parity rows: per odd endpoint +4 except repeated MOVS any odd uses 16 per iteration. Canonical F3 for data/I/O and F3/F2/65/64 for comparison. Source aggregate setup charged once via existing repeat phase; unknown prefix/electrical terms do not open eligibility. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c, cpu_timing_model.c, private cpu_instructions.h and source manifest; existing block/conditional-repeat/port/nominal-transfer smoke C files, one CPU-local shared string timing assertion header and test manifest. Coordinator additionally includes one timing-identity clear in cpu_instructions.c at the existing successful V30 real-entry commit, with same-address NMI/INT handler regressions; no guest semantic or layout change. The first focused groups passed but actual review exposed this identity alias, so the still-live subsequent build was explicitly interrupted before source edits; it is not full qualification. Estimate production +70..100/-10..20; tests +145..235/-10..25. Existing repeat recorder loses static and gains one private declaration for actual same-component reuse; no interface.h or registration change. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/design/queue/roadmap/source policy, shared Execution/Architecture/Coding/Documentation, architecture/coding governance and read-only PDF skills. Receiving NXVM guide/current/design/coding/source policy and X86 Core README. Unique execution/data/time/publication/repeat owner, Lib Types, frozen proof, target-separated Ps, actual committed-diff acceptance. |
| Verification | Extend existing three actual string execution seams with independent row/phase/key/input/max/no-extra-observation oracles, both parity axes, zero/count/DF/prefix/termination, actual IRQ/RETI restart and failure no-completion proof; preserve 942920 comparison inputs and all original semantics/accepted-prefix effects. Nine transient units: block, conditional repeat, port, nominal transfer, memory transfer, interrupt, software arbitration, completion, form admission. Both original Ninja/ccache all-target builds -j8, full 678 repository units -j4, specialized gates/498 strict rows and Types, 14 selected PC receiving tests plus restored identity per width, ten PE/hashes, protected-input guards and docs. |
| Expected Markers | Existing V30:BLOCK-TRANSFER:OK, V30:CONDITIONAL-REPEAT:OK, V30:PORT-TRANSFER:OK and comparison cases=942920 retained. Add V30:NOMINAL-STRING:OK at original three owners; all final focused/full suites zero failures/skips, source time unallocated/max0/Machine rejection preserved. |
| Asset Needs | Synthetic ROM-free inputs, pinned manual and original reusable graphs only. Rebuild/deploy eight current PC 0564 EXEs; verify all ten identities and unchanged MyNES0044/INIs/media/snapshots. No new ROM, MyRaidenII EXE, external source or host native test requirement. |
| Reporting Requirements | Report complete source/owner decision and estimates before code; progress and actual failed attempts; final added/deleted/net by production/test, shared helper responsibility and retained unknowns, measured verification/artifact/commit proof and whole-game boundary. |
| Stop Conditions | New public/source-model need, semantic/Intel behavior change, source assumption presented as exact physical time, owner-input mutation or failed qualification prevents acceptance. No weakened tests/budgets, second repeat state, extra operand reads, per-callback source costs or eligibility promotion. |
| Exit Criteria | Whole fourteen-opcode nominal batch has allocated source rows/aggregate phases or explicit original unknown dispositions, directly proved at actual execution including parity/zero/failure/IRQ restart; both widths full builds/units/gates/selected receiving/artifacts/docs pass, complete Ps pushed and coordinator actual committed-diff acceptance. Only S36 closes. |
| Original Owner Request | Single-person two-role governed implementation of complete playable MyRaidenII, NEC V30 and needed facilities. Source clocks or CPU units cannot replace Machine, board, video/sound and complete gameplay outcome. |
| Similar-Issue Sweep | All seven string families, canonical/noncanonical prefix identities, setup/continuation/termination/reset/IRQ boundaries, original count and endpoint parity, unknown timing placeholders, failure publication, no-observation and key collisions, existing Intel repeat consumers and maximum/Machine gates. Preserve extra prefixes, modes and physical accounting at original ledger receivers. |
