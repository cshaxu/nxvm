# T2 S40 Native Stack And Control Nominal Models

Baseline `f3d93e811`, after accepted S39. This consumes original V06/V08/V15's
remaining ordinary native two-address stack/control, frame-level1 and existing
register-alias nominal batch. Full physical timing, Machine/board and game
eligibility remain original receivers, not waived by this batch.

## Original Source And Complete Model Disposition

NEC October1986 V20/V30 User's Manual SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original copy](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed12-143..153 (PDF188..198), all eleven complete pages, are rendered and
visually inspected. No external implementation, manual or ROM payload is
imported/compiled. Prefetched, zero-wait source assumptions remain.

| Entire member | Matching-parity original row | Missing-row L2 policy |
| --- | --- | --- |
| PUSH memory | 18 even/even,26 odd/odd | Either mixed direction26 |
| POP memory | 17 even/even,25 odd/odd | Either mixed direction25 |
| Indirect near CALL memory | 23 even/even,31 odd/odd | Either mixed direction31 |
| Indirect far CALL memory | 31 even/even,47 odd/odd | Either mixed direction47 |
| PREPARE n=0 | 12 even SP,16 odd SP | No BP read or new model |
| PREPARE n>1 | 19+8(n-1) even/even,23+16(n-1) odd/odd | Either mixed BP/SP uses the odd expression |
| PREPARE n=1 | Source omits the clock row | Extend n>1 expression to n=1:19 even SP,23 odd SP; BP parity irrelevant |
| Existing FF/6 and8F/0 register aliases | No alias encoding clock row | Canonical register8/12 by original SP |

Every missing row uses the existing L2_CONTROL_MODEL origin. The all-odd
source envelope is a conservative policy for mixed addresses, not a proven
physical bound or independently observed microsequence. Level1 is explicit
algebraic extrapolation, not a source transcription: its operations push old
BP and the new frame but read no BP chain word. A different BP parity must not
select the mixed-chain estimate until n>1. The source transfer-count note
contradicts its operations and is not used to invent per-transfer clocks.

Alias timing deliberately adopts the canonical register row as an L2 policy,
not proof that the alias and canonical encoding cost the same on silicon.
Existing alias guest semantics are unchanged, including original-SP versus
decremented-SP PUSH behavior. All other matching-parity rows, direct control,
returns, DISPOSE's original BP parity, PSW/segment stack and scalar ports keep
their original source grades. Prefix clocks, general alias silicon behavior,
8080, bus/prefetch/waits and complete elapsed time remain original receivers.

## Sole Ownership And Necessary Diff

Only the existing private V30 selector changes. Its existing final stack
adjustment owns the complete address-parity decision and charges once.
A transient local Boolean folds original SP with the decoded memory offset,
or original BP only for PREPARE n>1. It is not stored hardware state.
The same final key/origin/input publication is retained; no additional operand
or target reads, handler, instruction decoder, timing helper, state, API/enum/
layout, Lib/Emulator or Intel path is introduced. Total time remains
unallocated, maximum0, neutral Machine admission closed.

Actual tracked code numstat, excluding docs/assets/manifests:
production+17/-11/net+6; tests+41/-28/net+13; total+58/-39/net+19.
Manifests separately+3/-3/net0. Additions are below estimates because the
existing parity/publication block and actual execution tests are reused.
No helper framework or duplicated clock path is required.

The stack's independent word-transfer and full CPU/memory oracle is unchanged.
All original3356 success contexts remain. The frame matrix adds the missing
independent BP parity without repeating true overlap inputs:1024 new contexts
give4380. Every level0..255, allocation, scalar/register alias, even/odd and
boundary/overlap/prefix input remains, with all original provider/fetch refusal
assertions. A main-owned local counter is passed through the existing two
matrix builders and existing real-execution check, incrementing once per
successful semantic input; no global state or extra test entry is added.
The timing seam checks independent semantic model/source rows, key/group/
inputs/origin, no observations and closed max. Level1's unused BP parity is
explicitly contrasted by the new matrix.

Control retains all463982 complete CPU/flags/pointer/stack/target/return
inputs, accepted prefixes, invalid far-register forms, real CALL/RET and every
existing host failure. The original independent semantic oracle now evaluates
the explicit mixed-address model and source versus L2 metadata rather than
calling a mixed successful instruction unknown. Existing pointer-before-stack
overlap and observation checks remain. No assertion or budget is weakened.
Both final seven-unit focused groups pass on the frozen candidate.

## Similar-Issue Sweep

Exact queries include:

- `rg -n 'stack_penalty|oldcpu.data.(sp|bp)|mrm.offset' src/x86-core/cpu/cpu_timing.c`
- `rg -n 'mixed|alias|unallocated|timing|0xc8u|0x8fu' test/x86-core/cpu --glob 'cpu_v30*'`
- `rg -n 'completed_observation_valid|timing_result.ticks|observations' test/x86-core/cpu/cpu_v30_stack_smoke.c test/x86-core/cpu/cpu_v30_control_flow_smoke.c`

Every stack-penalty producer uses the same finishing owner. Only the actual
memory-plus-stack forms or n>1 frame chain have two parity inputs; ordinary
GP/immediate/segment/PSW stack, PUSHR/POPR, direct CALL/RET, BRK/RETI and mode
escape have only their existing SP input. DISPOSE retains BP as its sole
address. Native CHKIND already has its distinct source-range/entry model and
no generic stack penalty, so it is not charged twice. NEC fields and ordinary
data/ports have no stack penalty and retain their separate data responsibility.

The two independent execution proof owners are updated, not replaced by a
test-only timing entry. Other nominal-transfer/ALU exclusions are extra-prefix
or genuinely prohibited forms and remain valid; no additional placeholder
needs weakening. Source-known equal-parity rows remain source-graded; alias,
mixed and level1 rows are explicit models, not silently promoted to physical
eligibility. All original seventeen ledger rows and complete T2 scope remain.

## Final Qualification

Selected runtime input protection is explicit: before any selected receiving
integration starts, six media roles across the four unchanged App INIs are
resolved and SHA-256/size/mode/leaf identities recorded in
[media inputs](t2-s40-media-inputs.json). They are all overlay routes. The
frozen collector rechecks these exact originals after receiving execution,
separately from Git's unchanged INI/MyNES guard. Unit inputs are synthetic.
Firmware sources and byte-embedding contracts are unchanged and their existing
build/qualification checks remain mandatory. This is not a before/after hash
of the entire external archive or a claim about every unexecuted media route.

Both final original all-target builds, seven-unit focus groups, full678/678
repository units (191 X86 Core),498 strict rows and1389 Types inputs pass per
width, zero failures/skips. Full units take334.33/141.89 seconds.
[Results](t2-s40-results.json) retain actual unit and selected receiving cases.
Fourteen selected PC integrations plus restored-default identity pass per
width. DeskPro groups take122.66/164.84 seconds. Both original Release/Ninja
graphs return to default and retain persistent ccache launchers; profile
switches and restoration recompile zero CPU objects. Eight PC0564 EXEs are
rebuilt/deployed; all [ten identities](t2-s40-artifacts.json) have correct PE
width and no debug sections. MyNES0044/INIs and six selected original media
hashes/sizes/modes remain unchanged. Firmware source and embedding checks pass.

The owned qualification runner is terminal exit zero. No build/dynamic test
fails. One supplemental manifest command mistakenly referenced a nonexistent
test-local script; the actual source-owned verifier with test CORPUS_ROOT
passes. This command-selection error changes no source or test assertion.
No desktop/RDP/physical audio/controller, fresh complete external/native suite,
physical-time/Machine/board or gameplay qualification is inferred.

Shared P1 `460cb6cf1` contains five files (+61/-42 including manifests);
NXVM P2 `80c092a8f` publishes eight current PC artifacts and receiving status.
Actual committed-diff coordinator acceptance remains before S40 closes.
T2/M1 and the complete original objective remain open.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S40, next unused S after accepted S39. |
| Admission And Approval | Owner's standing automatic single-person coordinator/executor MyRaidenII implementation and timing-upgrade approval; coordinator admits original V06/V08/V15 remaining native stack/control nominal-model batch on 2026-10-18 at clean f3d93e811. Targets Shared X86 Core/tests, NXVM eight receiving artifacts/status and MyRaidenII records. No Lib/Emulator/MyNES or public contract change. |
| Objective | Give the complete PUSH/POP memory, PREPARE frame, indirect near/far CALL mixed-parity family and existing register PUSH/POP aliases explicit source-graded nominal dispositions at the sole V30 selector. Preserve known-source rows, every existing semantic/failure assertion and the complete original CPU/board/game domain. |
| Non-goals | No instruction/decoder/entry/priority semantics, public API/enum/layout, persistent state, operand observation, Intel or Lib/Emulator change. No prefix/8080/BCD/general bus/prefetch/wait or full elapsed-time qualification. Do not claim physical upper bounds or exact silicon timing from missing source rows; Machine/max remain closed. |
| Reference Baseline | f3d93e811; pinned NEC October1986 V20/V30 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed12-143..153 (PDF188..198), eleven pages rendered/visually inspected. Existing S14/S20 semantic and S32/S33 timing proofs. No external implementation import or new source/license authority. |
| Candidate Proposal | proposals/m1-v30.md and original V06/V08/V14/V15. Known matching-parity source rows unchanged. Mixed memory/SP or PREPARE BP/SP use the larger documented all-odd row as an explicit L2_CONTROL_MODEL estimate, not a proven physical bound or per-transfer slope. PREPARE level1 uses the existing n>1 formula extended algebraically to n=1 (19/23 by SP), explicitly L2; BP memory parity participates only at n>1 because level1 reads no frame-chain word. Existing FF/6 and8F/0 register aliases use canonical8/12 as an explicit L2 nominal policy, preserving their existing distinct SP-value semantics. One existing final stack-parity calculation folds the actual relevant addresses and charges once. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and source manifest; existing cpu_v30_stack_smoke.c and cpu_v30_control_flow_smoke.c plus test manifest. Estimate production +20..40/-8..18; tests +50..90/-20..40. No new production helper/header/state/target; only local parity calculation and a main-owned test case counter. Stack frame matrix gains the missing BP parity axis without duplicate overlap rows. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/queue/roadmap/architecture/coding/source policy; Shared Execution/Architecture/Coding/Document and Contributing; architecture/coding-governance and read-only PDF skills. Receiving NXVM guide/current/architecture/coding/source policy and X86 Core README. Single execution/data/time/publication owner, Lib Types, explicit L2, frozen evidence, one target per P and coordinator actual committed-diff acceptance. |
| Verification | Retain all3356 original stack cases, all256 frame levels/allocations/overlap and scalar/alias/segment/wrap/refusal assertions; extend complete independent BP/SP parity combinations to4380 successful cases, explicitly checking level1 ignores unused BP parity. Retain all463982 control cases, exact pointer-before-stack transfer, real CALL/RET, full CPU/flags/memory, prefix/invalid/fetch/data failures. Actual execution seams compare independent model/source clocks/key/inputs/origin and no observations/failed publication. Transient focus: stack, control-flow, nominal-transfer, segment, flags, software-arbitration and completion. Both original Ninja/ccache all-target builds -j8, full678 repository units -j4, specialized498 strict rows/Types,14 selected PC integrations plus restored identity per width, ten PE/hash and protected-input/docs checks. |
| Expected Markers | Retain V30:STACK:OK, V30:NOMINAL-STACK:OK and V30:NOMINAL-CONTROL:OK cases=463982; add V30:STACK-ALIGNMENT-MODEL:OK cases=4380 and V30:CONTROL-ALIGNMENT-MODEL:OK. All final suites zero failures/skips; known rows retain V30_NOMINAL_SOURCE, models use existing L2_CONTROL_MODEL, prefixes stay unknown, total unallocated/max0/Machine closed. |
| Asset Needs | Synthetic ROM-free inputs and existing original cached graphs only. Rebuild/deploy eight PC0564 EXEs, verify all ten identities and unchanged MyNES0044, INIs, firmware/media/snapshots. No MyRaidenII EXE or external source/ROM import. Retain pinned manual, logs/proof/cache; remove eleven owned temporary page renderings after review. |
| Reporting Requirements | Report source/model/owner decisions, files and estimated diff before implementation; retain real failures/corrections. Deliver actual production/test added/deleted/net, unique paths/state/counter duties, complete batch disposition, dual-width tests/artifacts/commits and original full-goal residual. |
| Stop Conditions | Public/source/license expansion, source-model estimate presented as physical bound, execution/Intel semantics changed, duplicate stack/timing path or state, extra observation, protected input mutation or failed qualification prevents acceptance. No weakened assertions/budgets or fabricated CPU/Machine eligibility. |
| Exit Criteria | Every member of the frozen stack/control mixed-parity/frame/alias batch has a known-source or explicit L2 nominal disposition through the existing selector and actual complete execution/failure proof. Required dual-width builds/units/gates/selected integration/artifacts/docs pass; complete target-separated Ps pushed; coordinator actual committed-diff acceptance closes only S40. |
| Original Owner Request | Single-person two-role governed implementation of complete playable MyRaidenII, NEC V30 and required facilities. A nominal stack/control batch is not full CPU physical timing, Machine/board/video/sound or gameplay qualification. |
| Similar-Issue Sweep | All stack-penalty consumers, explicit/implicit addressing and original versus advanced SP/BP/pointer, PREPARE0/1/>1, same/mixed parity including overlap/wrap, FF/8F aliases and illegal far-register control forms, key/group attribution, source/L2, failure/prefix/no-observation/max/Machine/Intel guards. Original V01--V17 and other data/prefix/8080/entry/bus/source-model/game receivers remain, not removed to narrow T2. |
