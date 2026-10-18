# T2 S31 Fixed Native Scalar Nominal Clocks

Baseline `e6c1925d9`, after accepted S30. This consumes the fixed-cost
native scalar/flag subfamily of V15. Complete timing, V30 Machine eligibility,
board and playable game remain open. Final bounded qualification has passed.

## Source And Complete Admitted Rows

NEC October 1986 V20/V30 User's Manual, pinned SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Section 12-1's prefetched, zero-wait nominal assumptions remain those of S30.
Relevant PDF pages 74..86, 92..94, 110..126, 134, 139, 144 and 217
were rendered and visually inspected. Facts are implemented independently;
no external source code/manual/rendering or protected payload is committed.

| Complete source family / printed pages | Register | Aligned memory | Odd word extra |
| --- | --- | --- | --- |
| ADD/ADDC/SUB/SUBC/AND/OR/XOR r/m destination, 12-29..41 and 12-72..81 | 2 | 16 | 8 |
| Those operations with register destination | 2 | 11 | 4 |
| Their immediate forms | 4 | 18 | 8 |
| CMP in either register/ModRM direction, 12-65..66 | 2 | 11 | 4 |
| CMP immediate, 12-67 | 4 | 13 | 4 |
| TEST register/ModRM, 12-70 | 2 | 10 | 4 |
| TEST immediate, 12-71..72 | 4 | 11 | 4 |
| INC/DEC/NOT/NEG, 12-47..49 and 12-68..69 | 2 | 16 | 8 |
| CY complement/clear/set, DIR clear/set, IE disable/enable, 12-89/94/99/172 | 2 | not applicable | not applicable |

Accumulator-immediate encodings use the same four-clock register row.
Both documented word-immediate and sign-extended immediate forms are
included. Byte memory has no odd-word surcharge. CMP and TEST are read-only;
the read/modify/write row must not be applied merely because a decoder
records a memory operand. Every nominal row already includes EA and operand
access; no Intel EA or per-byte callback cost is added.

## Single Owner And Eligibility Boundary

The existing private V30 selector is extended, with its one initialization
and one result publication. No new helper, state, API, enum, layout or
execution/flag implementation is added. Existing S30 transfer selection
and old Intel selector chains remain the sole paths for their own forms.
The existing appended nominal-source origin is reused, never promoted to
physical elapsed time. Group keys now include the ModRM subopcode in
bits 9..11, alongside the opcode and memory-form bit, so CMP/TEST/NOT/NEG
rows are distinguishable. These are existing opaque CPU-owned source keys,
not a public enumeration or another opcode admission registry.

All V30 results still have unallocated total time, maximum zero and rejected
Machine construction. Prefix/alias/dynamic/8080 and remaining instruction
families retain their original source/model receivers. 82 remains an
unallocated executable alias; F6/F7 /1 are still prohibited, not enabled by
a cost table. Conversion/multiply/divide/shift/NEC-bit/control-stack/strings
are not assigned fabricated fixed clocks. Existing complete S30 transfer,
S17 arithmetic and S21 flag semantics remain intact.

## Proof Reuse And Initial Corrections

Inspection found the existing owned ALU unit already executes the complete
semantic/form/transfer/failure matrix. Rather than copying another harness,
one test-local clock oracle uses its semantic operation/form/width/address
facts, independent of the production opcode selector. Its original 917,460
success inputs now also check clocks, source/key/formula bits and total-time
non-eligibility. That count includes unallocated prefix/conversion cases;
it is not a claim that every case receives a nonzero clock. Original CPU,
flags, RAM, phase-order and accepted-prefix assertions remain unchanged.

ALU provider refusal proof is extended from data reads/writes to actual
instruction fetches. The first candidate test incorrectly required every
failed fetch to precede data reads. Actual TEST mem,imm reads the operand
before the immediate bytes; the baseline transcript proves that accepted
read prefix. The test now records bounded reads-before-fetch positions from
the successful real path, injects each observed fetch failure, and verifies
that exact prefix, no writes, preserved CPU checkpoint and no completed
timing. This does not alter the production decoder or weaken its original
data-failure assertions. The observed prefix is failure-order proof, not
an independent replacement oracle for architecture or clocks.

Fifteen neighboring dynamic/alias/prefix/control/conversion inputs prove no
nominal-row leakage. The initial boundary test wrongly treated prohibited
F6/F7 /1 as executable; it now explicitly requires rejection/no completion
for exactly those entries. Other entries execute and remain zero/unattributed.
No production form admission is weakened.

The existing flags unit retains its 406,800 semantic/debug/prepared-entry
inputs. Clock checks are added at its real instruction execution seam,
not its unrelated pure debug/prepared-entry queries. Both accepted flag
results and every existing provider refusal now check timing outcomes.
The S30 transfer unit retains 24,160 cases. Its out-of-batch ADD placeholder
is replaced by still-unallocated CBW in both exclusion and live-lifetime
vectors because ADD is deliberately admitted here; no transfer assertion,
clock, count or failure predicate is relaxed.

Initial estimates of a separate 240..360-line unit were refined before
test implementation to reuse the two real existing consumers. Production
is smaller than estimated because whole opcode/form families share their
stable nominal row; no new generic evaluator framework is introduced.

## Similar-Issue Sweep

Queries over production, tests and task evidence:

- `rg -n 'timing_select|timing_maximum_ticks|V30_NOMINAL|source_timing_unallocated' src/x86-core test/x86-core docs/myraidenii`
- `rg -n 'flagMem|mrm.offset|opcodes|V30_ALU_|V30_FLAGS_' src/x86-core/cpu/cpu_timing.c test/x86-core/cpu/cpu_v30_alu_smoke.c test/x86-core/cpu/cpu_v30_flags_smoke.c`

Every ordinary ALU direction, group immediate, byte/word, unary register/group,
read-only comparison/test and flag instruction is handled at the same owner.
Signed immediate width is selected lexically, not by result sign. Odd penalties
depend on original decoded address parity, not changed destination values.
Mode is the original instruction's MD; 8080 bytes cannot acquire native costs.
All other source families remain explicit unallocated V15 work, not shadow
Intel or fixed-tick fallback. Reset/refresh/failure clears and selection-before-
asynchronous-entry remain unchanged. The existing live-CPU transfer unit
proves subtotal -> unallocated -> subtotal -> failure publication lifetime.
Preview still observes only; maximum and Machine rejection are unchanged.

## Counted Change

Tracked C/H/CMake diff against baseline, excluding manifests/records/artifacts:
production +41/-3/net +38; tests +120/-9/net +111; code +161/-12/net +149.
Manifests separately +4/-4/net zero. No test registration/framework or new
production helper/state/interface is added. The test oracle, accepted-prefix
record and explicit exclusion inputs each have an actual regression owner;
no old instruction semantics, tests or receiving path is deleted.
Actual-change review must reconcile these counts and qualification before closure.

## Final Qualification

Shared source/test delivery is `9a438a0b9`; NXVM receiving publication is
`5ecd61022`. These are target-separated, immediately pushed deliveries.

Both original cached Ninja all-target builds pass. Complete repository units
pass 678/678 per width, including 191 X86 Core tests, with zero failures/skips,
in 307.55 seconds x64 and 129.99 seconds x86. Both fourteen-test related
groups, all specialized gates, 498 strict rows and global Types coverage of
1,388 code/script inputs pass. Existing warning exceptions remain unchanged.
Initial candidate assertion errors above are recorded, not hidden retries of
the final complete suite; no assertion or budget was weakened.

Eight current PC 0564 EXEs are rebuilt and deployed. Fourteen selected PC
receiving integrations and restored-default identity pass per width. Both
original graphs are restored to default; profile switches recompile zero CPU
objects. All ten deployed PE/hash identities pass. MyNES 0044 and all owner
INIs/media/firmware/snapshots remain unchanged. All owned qualification
processes have terminated successfully. [Results](t2-s31-results.json) and
[artifact identities](t2-s31-artifacts.json) preserve the measured proof.
Fresh complete external/native integration, manual desktop/RDP/audio,
physical CPU timing and gameplay are not claimed.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S31 after accepted S30, next unused S. |
| Admission And Approval | Owner directs automatic continued single-person coordinator/executor implementation. Coordinator admits the fixed-cost native scalar ALU/flag nominal-clock batch on 2026-10-18. Targets Shared X86 Core CPU/tests; NXVM eight receiving releases/status; MyRaidenII records. Existing Shared CPU authorization applies; no Lib/Emulator/MyNES source edit or new public interface is admitted. |
| Objective | Consume V15 fixed-cost native ADD/ADDC/SUB/SUBC/AND/OR/XOR/CMP/TEST, INC/DEC/NEG/NOT and CY/DIR/IE control nominal clocks, with documented register/memory/immediate/width/address-parity forms. Extend S30's one private source selector/publication mechanism; preserve all original semantic and physical-time receivers. |
| Non-goals | No dynamic multiply/divide/shift/decimal/conversion/NEC-bit/string/control-stack/8080 timing, prefix/alias timing envelope, new CPU semantics, Machine/board/game admission, public API/enum/layout, state, Lib/Emulator or MyNES edits. Total V30 time remains unallocated, maximum zero; no guessed clock or Intel fallback. |
| Reference Baseline | Clean pushed e6c1925d9. NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C: 12-1 assumptions; 12-29..41, 12-47..49, 12-65..81, 12-89, 12-94, 12-99 and 12-172 fixed scalar/flag columns, visually inspected PDF pages. Accepted S17/S21 semantic proof and S30 nominal contract. |
| Candidate Proposal | proposals/m1-v30.md and V15 fixed scalar nominal subfamily. The completed decoder record feeds the existing selector; no execution or flag logic is copied. Existing V30_NOMINAL_SOURCE reports only documented prefetched zero-wait subtotals, not full elapsed-time eligibility. Group subopcodes must have distinct source-form keys where they select different rows. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; extend the existing CPU ALU and flags units plus directly affected transfer timing-only assertions and test manifest. After reading their actual complete semantic/failure matrices, reuse them instead of duplicating a new execution harness. Refined estimate production +55..85/-3..10; tests +70..120/-5..15. No new public declaration/layout/state/helper framework. NXVM artifacts/status and MyRaidenII evidence separate Ps. |
| Applicable Rules | MyRaidenII guide/design/roadmap/source policy; Execution lifecycle/coverage/artifacts; Architecture/Coding and their local skills; Documentation and read-only PDF source review. Receiving NXVM guide/Current/design/source before release. Lib Types, sole selector/state owner, source/test owned together, one target per P and actual-diff coordinator review. |
| Verification | Independent original-manual clock expectations through real CPU execution over all eight ALU groups, documented ModRM/width/direction/immediate forms, addressing modes, parity/wrap, carry/value patterns, flags and every scoped provider refusal. Prove read-only CMP/TEST versus RMW costs, prefix/alias/dynamic exclusions, group-key identity and consecutive publication lifetime. Preserve S30 full transfer proof, S17/S21 results and old Intel paths. Both original cached Ninja builds -j8, complete repository units -j4, specialized gates and 14 PC receiving cases plus restored-default identity per width; ten PE/hash checks, MyNES/INI unchanged. |
| Expected Markers | V30:NOMINAL-ALU:OK over the retained 917,460 ALU success inputs, strengthened existing flag cases and retained V30:NOMINAL-TRANSFER:OK cases=24160; all partial V30 results remain unallocated with zero maximum. Complete dual-width units zero failures/skips, 498 strict rows and all related/receiving checks pass. |
| Asset Needs | Synthetic ROM-free CPU vectors and pinned read-only manual. Rebuild/deploy eight existing PC 0564 EXEs; verify ten identities with MyNES 0044 unchanged. No new ROM/media/snapshot/INI input or MyRaidenII executable. |
| Reporting Requirements | Before edits confirm source rows, sole owner and estimate; after implementation report actual counted added/removed/net C/H/CMake lines, focused/full qualification, receiving artifacts, failures/corrections and remaining complete timing/game boundaries. |
| Stop Conditions | Unresolved source row or necessary new model/public contract, changed old Intel semantics, external inputs or failed qualification blocks acceptance. Do not weaken assertions or promote partial nominal clocks to production eligibility. |
| Exit Criteria | Entire admitted scalar/flag nominal family has independent executed source/form and failure/lifetime proof, no duplicate execution/state/publication, old paths retained only for their real semantics; complete dual-width units/gates/receiving releases and manifest/doc checks pass, target-separated complete deliveries pushed and coordinator reviews actual changes before governance S closure. T2 remains open. |
| Original Owner Request | Implement full playable MyRaidenII in governed single-person/two-role mode, automatically continuing bounded NEC V30 and facility work without narrowing the objective to passing CPU tests. |
| Similar-Issue Sweep | Audit all scalar opcode directions/group extensions, read-only versus RMW transfer classes, immediate signed width, prefixes/aliases and nominal key lifetime; follow every V30 selector/preview/max/Machine consumer. Repair the owning nominal calculation or retain an explicit original V15 receiver; do not duplicate instruction semantics. |
