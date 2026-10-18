# T2 S32 Native Stack And Scalar-Port Nominal Clocks

Baseline `2438b61ff`, after accepted S31. This batch consumes the original
V15 stack/scalar-port source family. Final bounded qualification has passed;
coordinator acceptance remains. Complete V30 physical time, Machine/board and
playable game remain open.

## Primary Source And Dispositions

Original NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-25..27 and 12-146..153 (PDF 70..72,191..198) were rendered and
visually inspected, not inferred from OCR. Section 12-1's prefetched,
zero-wait nominal assumptions remain. No manual, external source or ROM is
imported. Source facts are independently implemented.

| Entire source family | Even-address clocks | All-odd clocks / remaining disposition |
| --- | --- | --- |
| PUSH/POP GP, segment or PSW; PUSH imm16 | 8 | 12 |
| PUSH sign-extended imm8 | 7 | 11 |
| PUSH memory / POP memory | 18 / 17 | 26 / 25 when both memory and stack addresses are odd |
| PUSHR / POPR | 35 / 43 | 67 / 75 |
| PREPARE level zero | 12 | 16 |
| PREPARE level n greater than one, matching BP/SP parity | 19+8(n-1) | 23+16(n-1) |
| DISPOSE | 6 using original BP | 10 using original BP |
| IN immediate port | 9 for byte or aligned word | 13 for odd word |
| IN DX; OUT immediate/DX | 8 for byte or aligned word | 12 for odd word |

The manual documents two-address stack operations as all-even/all-odd, not
mixed alignment. Mixed memory/SP for PUSH/POP and mixed BP/SP for nonzero
PREPARE remain zero/unattributed, not assigned a fabricated per-transfer slope.
PREPARE level one is omitted from its timing table and remains zero/unattributed.
The same page's transfer count also contradicts its explicit stack operations;
it is not used to synthesize clocks. POPR's explicit 43/75 row has a 32-clock
delta despite seven actual data reads: the source row takes precedence over a
guessed seven-times-four surcharge. The semantic seven-read contract remains.

Register r/m aliases FF /6 and 8F /0 remain executable under their prior
semantic model, but have no documented nominal register row and remain
unallocated. Prefix envelopes, string-I/O/repeat, control/entry, 8080 and
complete bus/prefetch/wait accounting retain their original V15 receivers.
No source-unknown class is moved out of the original T2 completion predicate.

## Owner And Data Flow

Only `cpu_timing.c`'s existing V30 selector changes. Completed decoder facts,
original SP/BP/DX and lexical immediates select source rows; later modified
SP/BP/port data cannot change the instruction's source context. Stack parity
uses original SP, except DISPOSE which accesses original BP. Source-known
memory and stack addresses must share parity. All segment bases are aligned,
so decoded offset parity is the physical starting parity.

Local stack surcharge is transient calculation, not persistent CPU state.
Existing MEMORY/SIZE/MODRM/effective-address/odd-word inputs report the
formula facts. Existing opaque source keys include the 8F group just as FF
already did, never a second admission table. No public declaration, enum,
layout, production helper, execution/flags logic, thread or queue is added.
The common publication and reset/failure lifecycle remain the sole owner.
S30/S31 source selection and Intel selectors retain their real separate
profile responsibilities. Every V30 total remains unallocated, maximum zero
and Machine admission closed; no nominal subtotal is promoted to elapsed time.

## Owned Proof And Initial Correction

Existing semantic tests are reused, not copied into another harness:

- Stack retains 3,356 successful cases: GP/aliases, all immediate bytes,
  memory/default/segment choices, overlaps and wrap, PUSHR/POPR, all 256 frame
  levels, allocation and BP/SP contexts. A semantic-input oracle checks exact
  source rows, keys/inputs/origins and explicit unknown classes. Existing CPU,
  complete memory, physical transfer/phase and accepted-prefix tests remain.
  Every existing data refusal also checks no completed timing; new fetch
  refusal checks cover actual fetch transactions.
- Segment retains its full original result/inhibition/interrupt/rollback proof.
  Its PUSH/POP execution seam now checks original stack parity and source
  identity. Existing data failures prove no successful timing publication.
- Flags retains all 406,800 inputs. Actual PSW stack execution gets source
  clocks; unrelated debug/prepared-entry queries are not counted as execution.
  Existing prefix, complete CPU/flags/MD and provider-refusal proof remains.
- Port retains original scalar, string, repeat, entry and partial refusal
  behavior. Scalar execution expands from 32 to 160 inputs with all four
  segment prefixes; unprefixed forms get exact source clocks and prefixes stay
  unknown. Immediate/DX, byte/word, odd/even and FFFF wrap are exercised.
  Every instruction fetch and every existing scalar-port refusal checks no
  completed timing; accepted first-lane output is never undone or replayed.
  A test-only fail-fetch control extends the existing fixture, not production.

The initial new Stack fetch test wrongly equated instruction bytes with
provider fetch transactions. A word-sized instruction read may fetch two bytes
at once. The observed FF memory PUSH completed normally when injecting the
nonexistent fourth fetch. The corrected loop uses the successful path's actual
fetch count, still refuses every actual transaction and keeps exact CPU/no-data/
no-timing assertions. This does not change production execution or weaken
accepted data-prefix assertions. Initial logs preserve the failure/correction;
the final complete suite must be a fresh zero-failure pass, not a hidden retry.

S30's 24,160 transfer and S31's 917,460 ALU/fifteen exclusion inputs remain
unchanged. Existing live-CPU nominal -> unknown -> nominal -> refusal proof
covers the same unchanged common publication owner.

## Similar-Issue Sweep

Queries:
- `rg -n 'timing_select_v30|oldcpu|stack_penalty|formula_inputs' src/x86-core/cpu/cpu_timing.c`
- `rg -n 'capture_timing|timing_result|fail_fetch|PUSH|POP|ENTER|LEAVE' test/x86-core/cpu -g 'cpu_v30*.c'`
- `rg -n 'V30|timing_maximum_ticks' src/x86-core/machine src/x86-core/cpu`

All native GP/segment/PSW stack callers and scalar-port encodings receive
the same selector/publication semantics. Original instruction decoding,
transfer/phase commit, register/flags and host-refusal rollback remain at
their real existing owners, not a second clock-generated executor. Existing
string-I/O/control/entry/8080 mechanisms retain distinct, unresolved source
accounting and are not assigned scalar costs. Prefixes cannot inherit the
underlying unprefixed row. No Lib/Emulator/product/INI/media fix is smuggled in.

## Counted Change And Qualification Boundary

Tracked C/H/CMake diff against baseline, excluding manifests/docs/artifacts:
production +48/-5/net +43; tests +120/-9/net +111; code +168/-14/net +154.
Manifests separately +5/-5/net zero. No new registration/framework/interface.
Positive test size is source-clock and failure proof in existing consumers,
not duplicate semantic implementation. Qualification and actual-change review
must reconcile these figures before acceptance.
Production is below the estimate because complete opcode families share
their documented row and the existing selector owns common publication.

## Final Qualification

Shared source/test delivery is `847c98c58`; NXVM receiving publication is
`86645b339`. Both target-separated deliveries are pushed.

Both original cached Ninja all-target builds pass. Complete repository units
pass 678/678 per width, including 191 X86 Core tests, zero failures/skips,
in 320.78 seconds x64 and 136.83 seconds x86. Both sixteen-test related
groups pass; all specialized gates pass, including 498 retained strict rows
and global Types coverage of 1,388 code/script inputs. Existing warning
exceptions are unchanged. No assertion, budget or admission rule is weakened.
The initial new fixture-count failure is retained above, not hidden by
retrying a failed final full suite.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus restored-default identity pass per width. Both original
graphs are default and profile switches recompile zero CPU objects. All ten
actual PE/hash identities pass; MyNES 0044, owner INIs and protected inputs
are unchanged. All owned qualification processes are terminal. Measured
[results](t2-s32-results.json) and [artifact identities](t2-s32-artifacts.json)
retain proof. Fresh complete external/native integration, manual RDP/desktop/
audio, physical CPU timing and gameplay are not claimed.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S32, next unused S after accepted S31. |
| Admission And Approval | Owner authorizes automatic continued single-person coordinator/executor MyRaidenII implementation. Coordinator admits native stack and scalar-port source-clock batch on 2026-10-18. Shared X86 Core CPU/tests, NXVM eight receiving EXEs/status and MyRaidenII records only. Existing Shared CPU approval applies; no Lib/Emulator/MyNES source or public-contract expansion. |
| Objective | Consume the complete stack/scalar-port nominal source family of V15: native GP/segment/PSW PUSH/POP, immediate PUSH, memory PUSH/POP, PUSHR/POPR, PREPARE/DISPOSE and eight scalar IN/OUT encodings. Allocate only directly supported original NEC rows; explicitly prove unsupported alignment/alias/level/prefix cases stay unallocated. |
| Non-goals | No instruction semantics, new CPU state/helper framework/API/enum/layout, L2 timing inference, string-I/O/repeat/control/8080/full physical timing, Machine/board/game eligibility or Lib/Emulator/MyNES edits. Nominal subtotal is never complete elapsed time; total eligibility false and maximum zero remain. |
| Reference Baseline | Clean pushed 2438b61ff. Pinned original NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, section 12-1 assumptions; visually inspected printed 12-25..27,12-146..153. S14/S15/S21 existing semantics and S30/S31 sole nominal selector/publication. |
| Candidate Proposal | proposals/m1-v30.md and original V15 source family. Extend the existing one selector using completed decoder, original SP/BP/DX and lexical immediate data. Exact table values, including POPR 43/75 despite seven data reads. Do not infer mixed-address slopes or missing PREPARE level-one rows; retain those complete unresolved classes in V15 for later source/L2 review. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; test/x86-core/cpu/cpu_v30_stack_smoke.c, cpu_v30_segment_smoke.c, cpu_v30_flags_smoke.c, cpu_v30_port_transfer_smoke.c and manifest, plus only directly affected prior timing exclusions if real. Estimate production +55..85/-0..8, tests +100..170/-5..15. No registration or new shared declaration. |
| Applicable Rules | Product guide/architecture/coding/roadmap/source policy, Execution lifecycle/coverage/artifacts/roles, Architecture/Coding skills, Documentation and PDF original-source review. Receiving NXVM guide/design/source policy. Lib Types, sole timing/state owner, source/test closure, one target per P and actual committed-diff review. |
| Verification | Independent source/form expectations at existing actual CPU seams: stack parity/wrap/original SP versus final SP and LEAVE BP; all immediate/GP/segment/PSW/set/frame variants; explicit source-unknown mixed alignments/level one/aliases/prefixes; scalar immediate/DX port width/parity/wrap, provider failures and no stale completion; S30/S31 proof retained. Both original Ninja/ccache all-target builds -j8, full repository units -j4, specialized gates/498 strict rows, fourteen receiving integrations and restored-default identity per width; ten PE/hash/owner-input checks. |
| Expected Markers | Existing V30:STACK:OK, V30:SEGMENT:OK, V30:FLAGS:OK cases=406800, V30:PORT-TRANSFER:OK and S30/S31 markers retained, with new nominal stack/port markers at the same tests. Both widths zero complete-unit failures/skips, all source gates and receiving cases pass; V30 max zero and Machine rejection preserved. |
| Asset Needs | Synthetic ROM-free vectors and existing pinned manual only. Rebuild eight current PC 0564 EXEs; MyNES 0044 unchanged; ten identities checked. No new firmware/media/snapshot/INI input or MyRaidenII EXE. |
| Reporting Requirements | Before edits source rows/unknown classes and estimated diff; during work actual findings and process progress; delivery actual production/test added/removed/net, preserved paths, source and verification evidence, artifact hashes and remaining full V30/game receivers. |
| Stop Conditions | New source model/public contract needed, old Intel semantic change, owner-input change or failed qualification blocks acceptance. No arbitrary timing, assertion relaxation, alias promotion, duplicate bus counting or partial-clock production eligibility. |
| Exit Criteria | Entire admitted source family has explicit allocated-source or retained-source-unknown disposition, actual executed semantic/failure/source-form proof, one selector/publication and no duplicated state/executor. Full dual-width unit/gate/receiving/artifact/doc qualification, target-separated pushed deliveries and actual-change coordinator acceptance. Only S32 may close; original full T2/M1 remains open. |
| Original Owner Request | Implement complete playable MyRaidenII in governed single-person/two-role mode, automatically continuing NEC V30 and required facilities without replacing game completion with CPU tests. |
| Similar-Issue Sweep | Audit every native implicit-stack/group alias/address/level/PSW/segment and scalar-port width/encoding, prefix/8080 isolation and all selector/preview/max/Machine consumers. Keep string-I/O/control/async and full bus time at their original V15 receivers; preserve accepted-prefix faults rather than compensating or re-executing. |
