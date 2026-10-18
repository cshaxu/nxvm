# T2 S34 Native Shift And Conversion Nominal Clocks

Baseline `6164274a2`, after accepted S33. This consumes the complete ordinary
native shift/rotate nominal source family and source-known conversions at
original V07/V15. Final qualification passes; coordinator acceptance remains.
Complete V30 physical time, Machine/board and playable game remain open.

## Primary Source And Allocated Family

NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-100..141 (PDF 145..186) and 12-61..64 (PDF 106..109) were rendered
and visually inspected, including all seven operations' register and memory,
one/CL/immediate, byte/word and alignment rows. The manual's prefetched,
zero-wait nominal assumptions remain; no PDF, ROM or external source is imported.

| Native source forms | Nominal clocks |
| --- | --- |
| SHL/SHR/SHRA/ROL/ROR/ROLC/RORC, one bit, register | 2 |
| Same one-bit forms, byte/even-word memory / odd-word memory | 16 / 24 |
| Same seven operations, CL or immediate count, register | 7+n |
| Same counted forms, byte/even-word memory / odd-word memory | 19+n / 27+n |
| ADJBA / ADJBS | 3 / 7 |
| Radix-ten CVTBD / CVTDB | 15 / 7 |
| CVTBW | 2 |

Here n is the original unsigned count byte, including zero and 255. Counted
shifts are not masked to five bits or reduced by operand/ring width, nor
collapsed to the one-bit row when n=1. Original CL is authoritative even when
CL/CH/CW is the destination. The existing immediate decoder owns the immediate.
Memory's eight-clock odd-word delta is a source row, not callback counting.

CVTWL's 4-or-5 and multiply/divide's data-dependent ranges do not establish
which input selects which cost. They stay zero/unattributed pending source
or reviewed model, not invented midpoint/max costs. Packed ADJ4A/ADJ4S semantic
ambiguities, generalized conversion radices, prefixes, 8080, bus/prefetch/waits
and complete elapsed-time allocation remain original V07/V15 receivers.

## Unique Ownership And Initial Correction

Only the existing private V30 selector changes. It consumes decoded cimm,
old CL and decoded memory alignment; group keys and existing SIZE/ModRM/EA/
ODD_WORD inputs use the existing publication path. No operand/target read,
helper, public function/enum/layout, persistent production state, second
executor or Intel timing fallback is added. CPU execution semantics, form
admission and original failure rollback are unchanged. V30 total-time stays
unallocated, maximum zero and neutral Machine admission closed.

The first candidate failed at `V30:NOMINAL-SHIFT:FAIL op=0 form=2 width=1
dest=0 count=1/1 segment=0 ticks=7/8`. The new selector incorrectly used the
last opcodes[oplen-1] byte. Existing ExecBoundary prepopulates oplen with
available prefetch bytes, so that byte may follow the current instruction.
The fix consumes existing decoded cimm. No decoder edit or relaxed assertion
was needed. The failed candidate stopped before complete qualification; the
corrected code passes both initial six-test focused groups. One local runner quoting
error also stopped before compilation; it changed no production/test behavior.

The first full x64 run then passed 677/678 in 319.57 seconds: Transfer still
used CVTBW as an unknown-clock exclusion (`opcode=98 ticks=2/0`, cases=24129).
The focused follow-up exposed the same placeholder in its live publication-
clearing sequence (cases=24151). Coordinator added both directly affected
test corrections: use still-unknown CVTWL, retaining all 24,160 transfer cases,
including allocated -> unknown -> allocated -> host-failure clearing. CVTBW
itself remains exhaustively checked in ALU. No production change or assertion
relaxation is needed. Original failed XML/log and focused log are retained
under distinct ignored initial-run names; both final complete suites rerun.

## Proof And Similar-Issue Sweep

The original shift unit retains all 20,356 successful inputs: seven operations,
both widths, one/CL/immediate forms, all count bytes, both carries, every
register alias, even/odd-tail memory, five segment selections, value/count
boundaries and exact CPU/RAM/read-write ordering. Timing is checked at that
same actual execution seam. All original data-provider failure positions keep
their CPU/RAM accepted-prefix assertions and now assert no completed timing.
The existing fixture gains only an observational-read counter, reset after
preview, to reject additional timing reads. Original /6 rejection and Intel
count contrasts remain.
Zero-count memory writeback remains S11's explicit retained model; source
nominal clocks do not certify its electrical/device side effects.

All 817,152 decimal inputs keep full CPU/defined-flag, wrap/segment, no-data-
transfer and fetch/preview refusal proof and gain independent source clocks,
keys, no-observation/max and no-failed-retirement checks. All 917,460 ALU inputs
are retained; the full CVTBW domain gains its fixed row and the full CVTWL
domain remains unknown. The fifteen adjacent timing boundaries retain their
roles: now-allocated shift placeholders become prohibited /6 encodings and
the allocated CVTBW placeholder becomes prefixed CVTBW. Four added boundaries
keep both non-ten conversion radices and packed adjustments unallocated,
bringing the explicit boundary marker to nineteen. These are timing/refusal
checks, not a claim that packed-adjust semantics are qualified.

Queries:
- `rg -n 'opcodes\[[^]]*(oplen|-[[:space:]]*1)|cimm|oldcpu.data.cl' src/x86-core/cpu/cpu_timing.c src/x86-core/cpu/cpu_timing_model.c`
- `rg -n 'SHIFT|shift|cimm|actual_count|timing|unallocated' src/x86-core/cpu/cpu_timing.c test/x86-core/cpu/cpu_v30*smoke.c`
- `rg -n 'V30|timing_maximum_ticks' src/x86-core/machine src/x86-core/cpu`

No other timing selector indexes the end of oplen. Existing immediate/pointer
positions have their fixed encoding owner; ENTER's level and the radix byte
remain fixed-position fields, not a copied tail parser. All six group-2 opcode
forms share one selector branch; /6 stays rejected by the existing form owner.
No unknown source or real prefix form is promoted by a successful result test.
Existing Intel selectors retain their distinct profiles and source rules.

## Counted Change And Verification Boundary

Against baseline, excluding manifests/docs/artifacts, production +21/-1/net
+20; tests +52/-9/net +43; total code +73/-10/net +63. Manifests separately
+5/-5/net zero. This is below estimate because source facts are added inline
at existing selection and proof seams, with no new helper or registration.
The test-only observation counter measures real callbacks, not hardware state.

Shared source/test delivery is `1756ecf8e`; NXVM receiving publication is
`90b0d58f1`. Both target-separated deliveries are pushed.

Both original cached Ninja all-target builds pass. The final full repository
unit runs pass 678/678 per width, including 191 X86 Core tests, zero failures
or skips, in 112.84 seconds x64 and 139.61 seconds x86. Both seven-test focused
groups and specialized gates pass: all 498 retained strict rows and global
Types coverage of 1,388 inputs. Historical warning exceptions remain unchanged.
The earlier 677/678 run and follow-up failure are recorded above, not erased
by the passing final runs; no assertion, budget or admission is weakened.

Eight current PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus restored-default identity pass per width. DeskPro groups
take 134.19/178.03 seconds. Both original graphs are default and switching
profiles recompiles zero CPU objects. All ten deployed PE/hash identities
pass; MyNES 0044, owner INIs, firmware/media and snapshots are unchanged.
The qualification runner is terminal with exit zero. [Results](t2-s34-results.json)
and [artifact identities](t2-s34-artifacts.json) retain measured proof.
Coordinator actual committed-diff acceptance remains before S closure.
No whole external integration, manual desktop/RDP/audio, physical timing or
gameplay proof is inferred from these units.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S34, next unused S after accepted S33. |
| Admission And Approval | Owner authorizes continued automatic single-person coordinator/executor MyRaidenII implementation. Coordinator admits this V15 source batch on 2026-10-18 under existing V30 approval, targets Shared CPU/tests, NXVM eight affected receiving releases/status and MyRaidenII evidence. No Lib/Emulator/MyNES source or new public contract. |
| Objective | Allocate the complete seven-operation native shift/rotate nominal rows for one/CL/immediate counts, byte/word and register/even/odd memory; combine source-known ADJBA/ADJBS, radix-ten CVTBD/CVTDB and CVTBW. Reuse the one selector and original decoded/old-register facts. |
| Non-goals | No CPU semantics, new model/API/enum/layout/helper/state, copied executor, operand reads or Intel timing reuse. Packed decimal ambiguities, generalized radices, CVTWL and multiply/divide data-dependent ranges, prefix/8080/electrical/full elapsed time and Machine/game admission remain original receivers. |
| Reference Baseline | Clean pushed 6164274a2. Pinned NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C: printed 12-61..64 and 12-100..141, PDF 106..109 and 145..186, rendered/visually inspected. Existing S11 shift, S17 sign and S18 decimal semantic/failure proof. |
| Candidate Proposal | proposals/m1-v30.md, original V07/V15 source-clock receivers. One-bit rows are 2 register/16 byte or even-word memory/24 odd-word memory. Variable/immediate rows are 7+n register/19+n byte or even-word memory/27+n odd-word memory, n the original full unsigned byte, including zero and aliased CL. Source-known conversions are 3/7/15/7/2. No invented data-dependent row or total-time promotion. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; existing cpu_v30_shift_rotate_smoke.c, cpu_v30_decimal_smoke.c and directly affected cpu_v30_alu_smoke.c plus test manifest. The first full x64 run and focused follow-up exposed two old CVTBW unknown-clock placeholders in cpu_v30_nominal_transfer_smoke.c, including live publication clearing; coordinator includes both corrections to still-unknown CVTWL before editing, without changing scope/semantics. Estimate production +25..45/-3..8 and tests +65..110/-5..12. No registration/public declaration. NXVM artifacts/status and MyRaidenII records in separate target Ps. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/design/roadmap/source policy, shared Execution/Architecture/Coding/Documentation and architecture/coding governance skills; read-only PDF review. Receiving NXVM guide/current/design/coding/source policy. Lib Types, unique selection/publication and actual committed-diff acceptance. |
| Verification | Extend all existing shift/rotate and 817152 decimal plus 917460 ALU inputs with independent source clocks/keys/inputs, count zero/full byte/CL aliases, every actual data/fetch/preview refusal and original complete CPU/RAM/transfer checks; keep prohibited group-2 /6, prefix and source-unknown forms non-eligible. No extra timing reads. Focus seven units: four changed/directly affected units and form/native/completion contrasts, including all 24160 transfer cases; preserve first x64 677/678 failure and rerun both complete suites after the final test correction. Both original cached Ninja all-target builds -j8, full repo units -j4, specialized gates/498 strict rows, fourteen PC receiving integrations and restored identity per width, ten PE/hashes and unchanged protected inputs. |
| Expected Markers | Original V30:SHIFT-ROTATE:OK, V30:DECIMAL:OK cases=817152 and V30:NOMINAL-ALU:OK cases=917460 remain; add V30:NOMINAL-SHIFT:OK and V30:NOMINAL-DECIMAL:OK cases=817152. Both full suites zero failures/skips. V30 total-time unallocated, maximum zero and Machine rejection remain. |
| Asset Needs | Synthetic ROM-free vectors and pinned original manual; eight PC 0564 EXEs rebuilt/deployed, MyNES 0044 unchanged and ten identities verified. No ROM/media/INI/snapshot or MyRaidenII artifact change. |
| Reporting Requirements | Report source and ownership decisions/estimate before code; real progress/failures; counted production/test added/deleted/net, retained paths, source-unknown and failure dispositions, qualification scope, artifacts/commits and remaining full CPU/game boundary. |
| Stop Conditions | New public/source-model need, semantic change, owner-input mutation or failed qualification stops acceptance. Do not infer exact costs from a range, mask NEC counts, weaken tests/budgets or promote nominal subtotal to elapsed-time eligibility. |
| Exit Criteria | Complete admitted source family allocated at one owner with actual old-fact/source-key/cost/failure proof; source-unknowns retain original receivers. Both widths full builds/units/gates/selected receiving/artifact/docs pass; complete target-separated Ps pushed and actual committed-diff coordinator acceptance. Only S34 closes. |
| Original Owner Request | Governed single-person/two-role implementation of complete playable MyRaidenII, including NEC V30 and required board/host facilities; partial clocks or CPU units do not replace game outcome. |
| Similar-Issue Sweep | Inspect every group-2 opcode/subop and count origin, immediate/CL alias, memory parity, prefix/form admission, decimal/sign semantic owners, all timing-only exclusions, failure publication, preview/max/Machine consumers and old Intel selectors. Retain packed/data-dependent/other mode/full time work at V07/V15 rather than fabricate or duplicate. |
