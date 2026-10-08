# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |
| T546 S4 | Accepted after coordinator actual-change review of pushed Shared P1 2d9929742 and NXVM P2 ca016635d: complete source/caller/receiver proof, 537/537 units per width, original 58/58 external contexts and eight qualified 0546 PC artifacts. S4 closed; no active S packet. |
| T546 S5 | Accepted after coordinator actual-change review of pushed Shared P1 bdfe3b938 and NXVM P2 04f0278ad: full host/count/reference batch, final 539/539 units per width, original 58/58 integration and eight qualified 0546 products are proven. S5 closed; no active packet. T546 remains open for S6-S19. |
| T546 S6 | Accepted after coordinator actual-change review of pushed Shared P1 1db282a10 and NXVM P2 19945b2b9: complete ordinary-stack/source/caller proof, final units 541/541 per width, original integration 58/58 once, supplemental 33/33 per width, both gates/eight manifests and eight qualified stripped 0546 products. S6 is closed with no active packet; T546 remains open for S8-S20. |
| T546 S7 | Accepted after coordinator actual-diff review of pushed Shared P1 0c04f2c24 and NXVM P2 0f86b1ce0: 36 duplicate aliases retired without removed assertions, 505/505 final units per width, dependency/cache and affected gate proof complete. No-op builds 1.37/1.05 s; cached CPU unit-dependency recovery 109.47/52.57 s. Unchanged binaries/inputs retain S6 integration proof. S7 closed; no active packet. T546 remains open for S8-S20. |
| T546 S8 | Accepted after coordinator actual-change review of Shared 9d5e475f7 and NXVM 626d504f5: complete FLAGS/privilege/IRET/RF/prior-TF batch, dual units 505/505, supplemental 33/33, both gates/eight manifests, original integration 58/58 once and eight stripped 0546 products. S8 closed; no active packet. T546 remains open for S9-S20. |
| T546 S9 | Accepted after coordinator actual-change review of pushed Shared 746e3e214 and NXVM 04bd419e1: complete seven-member arbiter/shadow/NMI/comparator/HLT/REP/receiver batch, final units 505/505 per width, original integration 58/58 once, supplemental 33/33, both gates/eight manifests and eight stripped 0546 products. S9 closed; no active packet. T546 remains open for S10-S20. |
| T546 S10 | Closed at owner direction: finalizer, resident shutdown/Core wait, approved query and real new-CS entry repair are separated for target-scoped delivery. The 78-owner CPU scan and 82/418 specialized gates pass on both widths; the one unrelated Lib Console viewport failure is explicitly moved to S11, not used as CPU evidence. |
| T546 S11 | Closed at owner direction after pushed Shared Console broker repair/test P commits `2e1c59660` and `89ae694ef`, plus receiving product P commits. Focused dual-width Lib evidence passes: 36/36 Lib units per width, including the native desktop smoke and Console contract. The broader all-App test/artifact exit was not rerun or claimed; it remains available for a later explicitly scoped qualification S. T546 remains open. |
| T546 S12 | Implementation P commits `6200790b4`, `8e2f71d41`, and `c23b67abb` repair the admitted IBM PC/Audio failure contracts. Its required complete qualification remains retained for later coordinator reconciliation; it is not claimed closed by this status transition. |
| T546 S13 | Implementation P commits `320fdc385`, `c498244b7`, `5c5b64725`, and `4b76caa9f` replace the IBM PC adapter's legacy Win32 key interpretation with neutral KVM events. Its receiving qualification remains retained for later coordinator reconciliation; it is not claimed closed by this status transition. |
| T546 S14 | Active: descriptor-query/table correctness. Repair generation-specific LAR/LSL descriptor admission, query treatment of an invalid LDTR, and the 80386 LAR output mask with source-conditioned owner-local regressions. |

## Active T546 S14 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: the owner directed return to the open CPU-repair T546 after closing the prior MyNES T44. S12/S13 have delivered implementation commits but retain their independent qualification records; this packet consumes the next planned CPU batch, S14. |
| Admission And Approval | The owner pre-approved x86/chips and x86/core correctness repairs, and all timing-accuracy upgrades, in the T546 proposal. This batch changes only the private CPU owner and owner-local x86 tests/manifests; no new public API, type, state owner, or product workaround is admitted. |
| Objective | Make LAR, LSL, VERR, and VERW obey their generation-specific descriptor-query semantics: the 80386 LAR defined-bit mask, 80286-versus-80386 system descriptor-type eligibility, and non-faulting negative result for a selector into an invalid LDT. Prove queries do not mutate descriptor accessed/busy state. |
| Non-goals | No gate/task-switch, ordinary segment-load, paging, timing-formula, App, IBM PC, Lib, Common, configuration, ROM/media, or external-emulator code change. No new CPU public API, descriptor cache, exception path, timing value, or profile heuristic. |
| Reference Baseline | Intel 80286/80287 Programmer's Reference Manual (210498-005, descriptor validation and VERR/VERW); Intel 80386 Programmer's Reference Manual (LAR/LSL descriptor-type tables and `00FxFF00H` mask); current CPU owner and the existing LAR/LSL/VERR/VERW owner-local fixtures. Bochs is cross-check-only and is not imported. |
| Candidate Proposal | [CPU repair proposal](../proposals/m5-cpu-audit-gap-repair.md), S14; [whole ledger](../history/M5-T546-cpu-audit-gap-repair.md). |
| Files And ABI Surface | `src/x86/chips/cpu/cpu_instructions.c`; `test/x86/chips/cpu/cpu_lar_lsl_smoke.c`; `test/x86/chips/cpu/cpu_verr_verw_smoke.c`; matching x86 test manifest only. Existing private helpers and original table-driven handlers remain the sole implementation path. |
| Applicable Rules | Shared Execution/Architecture/Coding/Documentation and NXVM Architecture/Coding; source/research policy for Intel/Bochs evidence. |
| Verification | Focused LAR/LSL/VERR/VERW/selector tests on x64 and x86 during development. At P completion, build both widths, run complete repository-only units on both widths, manifests and applicable x86 gates; rebuild receiving PC products only if the resulting Shared production inputs change their executable hashes. |
| Expected Markers | 80386 `LAR r32` clears the undefined high-limit nibble; 80286 rejects 386-only system descriptor types while preserving the destination; LAR accepts all Intel-listed gate types for each supported generation; LSL admits only Intel-listed non-gate types for each generation; an invalid LDTR produces ZF=0 without selector-derived fault; query operations leave descriptor bytes unchanged. |
| Asset Needs | None. |
| Reporting Requirements | Record exact manual sections, code/test delta, type/disposition matrix, timing disposition, similar-issue sweep, dual-width results, receiving-artifact hash determination, and any unupgradable L1 or source disagreement. Do not call undefined architectural output a deterministic Intel value. |
| Stop Conditions | A correction needs a public contract, new descriptor state/cache, changed ordinary selector-load semantics, altered timing formula, a non-x86 component, or a downgrade/source conflict. Stop and report rather than introducing an approximation. |
| Exit Criteria | The complete four-instruction query batch has source-conditioned disposition; each repaired behavior has direct owner-local regression on both relevant families; focused and complete dual-width units, manifests and x86 gates pass; actual diff is reviewed; any affected receiving artifacts are rebuilt/verified and all P commits are pushed. |
| Original Owner Request | “批准收口当前T任务，回到之前nxvm的cpu修复T任务”, following the owner’s standing automatic approval for x86/core correctness and timing upgrades. |
| Similar-Issue Sweep | Search every `_s_check_selector` caller and all LAR/LSL/VERR/VERW descriptor-type branches; distinguish query-only negative-result semantics from ordinary selector loads, and inspect descriptor-byte writes before/after every query owner. |

## Accepted S9 Asynchronous Arbiter

Coordinator actual-change review accepts Shared `746e3e214` and NXVM
`04bd419e1` against the complete seven-member packet and
[source/caller/receiving proof](../etc/evidence/t546-s9-asynchronous-arbiter-design.md).
One original arbiter and private shadow/input/service owner replace the
retired conflated flags; original handler tables remain. No public API,
instruction clock allocation or excluded component changes. The retained 186
predicates and HLT/step combination are explicitly reference-model L2,
not new physical L3 or a downgrade of existing exact instruction clocks.

Final complete units pass 505/505 per width (206.77/194.28 s), original
58/58 external contexts pass once, supplemental 33/33 per width, both full
gates/eight manifests and eight optimized stripped 0546 artifacts pass.
Production is net +69; twelve source/test paths are +752/-51, net +701,
mostly independent source-conditioned regressions. INIs, five masters and
Lib/Common/MyNES remain unchanged. S9 closes with no active packet; T546
retains complete S10-S20 fault/gate/task/page/NPX/time/source receivers.

## Accepted S8 FLAGS Privilege And Return

Coordinator actual-change review accepts pushed Shared `9d5e475f7` and NXVM
`626d504f5` against the complete eight-member packet and
[direct source/caller/receiving proof](../etc/evidence/t546-s8-flags-return-design.md).
POPF-local privilege masks, original IRET/scalar paths and one actual-task
outcome repair RF images versus rollback and prior-TF completion without a
new public API, timing value or parallel owner. Original table style remains.

Final complete units pass 505/505 per width (120.05/247.58 s), original 58/58
external contexts pass once, supplemental checks pass 33/33 per width, both
full gates/eight manifests and all eight optimized stripped 0546 products
are verified. Earlier failed or contained candidates remain explicit evidence.
Production is net +33; nineteen source/test/build paths are +802/-55, net +747,
primarily independent source-derived matrices. Lib/Common/MyNES, four owner
INIs and five media masters are unchanged.

S8 is accepted and closed with no active packet. T546 remains open for S9-S20;
full arbiter, gate/task/page/delivery/NPX/retirement and remaining timing/source
contracts keep their original receivers. Validated receiving caches and
task-local research/final logs are retained for the immediate successor.

## Accepted S7 Build And Verification Efficiency

Coordinator actual-change review accepts Shared `0c04f2c24` and NXVM
`0f86b1ce0` against the owner-directed insertion and
[complete efficiency evidence](../etc/evidence/t546-s7-build-test-efficiency.md).
One native graph and compiler cache replace repeated serial leaf scheduling;
one dependency-only unit build separates compilation from execution. Thirty-six
duplicate product aliases are removed, preserving all canonical bodies and
334 target identities. Positive/negative registration and timeout/tree cleanup,
affected build/artifact/INI gates, eight manifests, Types and docs pass.

Both final complete unit executions pass 505/505 once, 250.11/233.17 seconds,
without relaxing 300-second containment. Full-suite runtime is not claimed
faster; measured build/configuration and avoided-work savings are in evidence.
Exact S6 CPU objects, ten EXEs, four INIs and five masters are unchanged.
The qualified integration command is unchanged; its accepted S6 58/58 proof
is retained rather than repeated. No runtime source/API or Shared corpus body
changes. Counted build/tool code is +56/-20, net +36.

S7 is accepted and closed with no active packet. The owner moved former
uncommitted FLAGS S7 to deferred S8 and former S8-S19 to S9-S20.
Exact local reproducer patches/hashes, research and warm caches are retained
for that next admission. T546's complete CPU repair remains open; source and
runnable CPU/artifact baseline remain accepted S6, not a new CPU qualification.

## Accepted S6 Stack And Frame Publication

Coordinator actual-change review accepts Shared P1 `1db282a10` and NXVM P2
`19945b2b9` against the complete original S6 batch and
[source/caller/verification evidence](../etc/evidence/t546-s6-stack-frame-design.md).
One logical ordinary-frame owner precedes original scalar transfers; independent
ENTER attributes/full ESP, early wrap, POP aliases and sourced explicit SP
admission retain the original table style. The candidate's overbroad GP rule
is corrected by original 386 14-6, not by weakening the accepted SS oracle.
Core/chip assertions use independent actual-memory images; no MMIO undo or
new public API/timing value is introduced.

Final units pass 541/541 per width (248.83 s/233.31 s), original external
integration 58/58 once, supplemental 33/33 per width, both gates/eight manifests
and eight stripped 0546 artifact checks. Production net +19; ten code/test/build
paths net +826 supply the missing direct matrices and guards. INIs, five masters,
Lib/Common and MyNES are unchanged; unrelated MyNES documentation is preserved.

S6 remains accepted and closed; subsequent efficiency S7 acceptance is above.
T546 remains open for S8-S20;
full FLAGS/arbiter/gate/task/page/delivery/NPX/retirement and remaining source
 timing contracts retain their original receivers.

## Accepted S5 Host Arithmetic And Counts

Coordinator actual-change review accepts pushed Shared P1 `bdfe3b938` and
NXVM P2 `04f0278ad` against the complete S5 packet and
[direct proof](../etc/evidence/t546-s5-host-arithmetic-count-design.md).
Pre-widening, unsigned masks, portable SAR, guest count/definedness and
initialized undefined double shifts retain the original handler tables.
Trusted typed references use one existing copy path; no public API, timing
constant, new framework or App-specific CPU implementation is added.

Final complete units pass 539/539 per width (116.97 s/118.33 s), original
integration 58/58 once, supplemental 33/33 per width, both full gates and
eight manifests/qualified stripped 0546 artifacts. Earlier incomplete x86
containment and corrected early oracles remain explicit evidence, not passing
results. Production net -8; nine code/test/build paths net +437 supplies the
missing independent 2,004-context matrix proof and two guard registrations.
Owner INIs, five media masters, Lib/Common and exact MyNES EXEs are unchanged.

S5 remains accepted and closed; subsequent S6 acceptance is recorded above.
T546 remains open for S8-S20. Complete fault/task/paging/delivery/NPX/retirement and
remaining timing/source conflicts retain their original receivers.

## Accepted S4 Admission Versus Next Fetch

Coordinator actual-change review accepts pushed Shared P1 `2d9929742` and
NXVM P2 `ca016635d` against the complete S4 packet and
[direct proof](../etc/evidence/t546-s4-admission-next-fetch-design.md).
Ten target checks retain logical admission without speculative page faults;
one preview model distinguishes missing-input L2 from exact m/ts source rows.
Original handlers, real frame/descriptor checks and single retirement/time
owners remain. The only public extension is the explicitly approved enum append.

Final complete units pass 537/537 per width (117.63 s/221.41 s), original
integration 58/58 once, supplemental 33/33 per width, both full gates, final
manifests/Types and eight optimized stripped 0546 artifact checks. Production
net +23 lines; source/test/build net +627 supplies the missing 121-case chip
matrix, Core publication proof and source-derived receiving oracle/gate fixes.
INIs, five master images, Lib/Common and exact MyNES EXEs remain unchanged.
Unrelated MyNES documentation is preserved outside this task.

S4 is closed with no active packet. T546 remains open for S6-S19, including
full frame/gate/task/paging/delivery/NPX/RMW and remaining timing contexts.

## Accepted S3 Effective Address And Segment Spans

Shared P1 `15a2399d0` and NXVM P2 `d875a72fe` are pushed. Coordinator
actual-change review accepts the complete S3 source/caller and receiver batch
against the original request and [direct proof](../etc/evidence/t546-s3-address-span-implementation.md):
word/bit EA, cached and empty expand-down limits, all fifteen composite
readers, scalar bus phases and ordinary real SS delivery. The original GP
mask policy and original table handlers remain; no public API or second owner.

Final units pass 536/536 per width (127.02 s/118.02 s), original integration
58/58, supplemental 33/33 per width, both specialized aggregates, final
manifests/Types/documentation and eight artifact checks. Production net -7
lines; counted source/test/build net +483 supplies the missing matrix proof.
INIs, external masters and MyNES remain unchanged. S3 remains accepted; S4's subsequent closure is recorded above.
T546 remains open for S6-S19. Complete frame/task/paging/delivery/RMW
and the two inherited host-reference guards retain their explicit later owners.

## Accepted S2 Decode/Admission And Model40 Inputs

Shared P1 `34867b960` and NXVM P2 `01709c418` are pushed. Coordinator
actual-change review accepts the approved batch and
[direct proof](../etc/evidence/t546-s2-decode-admission-implementation.md):
complete units 534/534 per width, all 58 original external contexts, eight
manifests, applicable static/supplemental/specialized and documentation gates,
and eight verified optimized stripped 0546 PC artifacts. INIs, external masters
and the exact MyNES pair are unchanged. No public API or parallel owner is added;
production net +1 line. The Model40 conversion is L2, not physical L3.

S1-S6 are closed; T546 remains open for S8-S20. The owner-authorized
automatic boundary remains unchanged for the next admission. S1's
accepted reset/image proof remains in the [task history](../history/M5-T546-cpu-audit-gap-repair.md).

## Accepted T545 Baseline

Active T546 developer target is vm-0-5-0546 (0.5.0546), optimized stripped
x64/x86 pairs for all four fixed PC Apps. They are rebuilt and verified;
the former accepted 0545 baseline is retained in T545 history, not assets.
MyNES remains 0043 and is not rebuilt for CPU-only changes.

S7 implementation P1 is `90e91d721`, pushed to origin/master. Coordinator
actual-change review accepts the entire original request, S1-S6 ledgers and
original S7 verification, not merely a passing unit summary. See
[final proof](../etc/evidence/t545-s7-final-qualification.md),
[task history](../history/M5-T545-softpc-eight-corpus-refresh.md) and
[archived proposal](../history/M5-T545-softpc-eight-corpus-refresh-proposal.md).

All 58 original PC integration contexts pass once with unchanged checkpoints:
default 22/22, AT 3/3, Model40 3/3 and XT 1/1 per width. Complete PC/shared
units pass 532/532 per width; MyNES receiver units pass 43/43 and integration
12/12 per width. Eight manifests, 33 supplemental checks per width, both
specialized aggregates, artifact-root/INI and documentation checks pass.
S7 corrects only two NXVM static fixture checkers after S5 relocation; their
44-owner/133-constructor inventories remain intact. No Shared production,
API, App source, owner INI, media or executable input is changed by S7.

S2 imports exact committed SoftPC
`8124e551e841ccdec2ceb7f6a0f6ae5b513a7951` eight-root bytes. S3-S6 subsequently
complete Lib/Common/x86/IBMPC owner-local tests with refreshed test manifests;
current test bytes are not claimed identical to that historical upstream pin.
All preserved/relocated assertions and removed-file dispositions remain in
the S1/S2 inventory and S3-S6 evidence linked by task history.

## Current Technical And Runnable Baseline

Accepted implementation baseline is S9: Shared CPU/tests `746e3e214` and
NXVM evidence/artifacts `04bd419e1`, with production hash BEBC79F8… and final
artifact identities in S9 evidence. Earlier candidate identities are superseded;
this governance acceptance adds no executable input.

- Independent chips and sole CPU implementation live in x86/chips; neutral
  execution, memory/ports and guest time live in x86/core.
- IBMPC board-common/AT/XT own PC wiring; Machine owns one Common driver and
  pacing/media adaptation; Product owns one INI/command/Debug/UX/entry path.
- Four fixed Apps own immutable compositions and firmware bindings. Model40
  alone owns D4. No App production graph links a peer App.
- Shared tests follow their owners; actual model assertions stay App-local.
  The one external family harness executes each real fixed binding and its INI.
- Eight PC 0.5.0546 EXEs remain directly in assets/my5160,
  assets/my5170, assets/mydeskpro386 and assets/nxvm, with unchanged adjacent
  owner INIs. All eight S9 optimized stripped pairs are verified. MyNES now retains
  its separately accepted 0.0.0044 pair; its own [Current](../../mynes/states/CURRENT.md)
  owns those identities. CPU-only work must not rebuild or alter them. Qualified PC hashes
  are in [S9 evidence](../etc/evidence/t546-s9-asynchronous-arbiter-design.md),
  historical S1 MyNES hashes remain in [S1 evidence](../etc/evidence/t546-s1-reset-images.md); PE widths,
  current identity and absence of compiler debug sections are verified.
  Runtime Debug remains; only CPU-dependent PC products are rebuilt.
- External media masters are verified unchanged after final S9 overlay integration.
  All S9 builds/tests/gates are terminal; complete units, original 58 integration
  and supplemental routes pass. Coordinator acceptance is recorded above.
  Ignored receiving caches
  remain needed for the next CPU repair batch's incremental regression checks.

## Next Work And Qualification Boundary

The owner-requested CPU instruction/function/timing repair remains open as
T546 after accepted S9, with S10-S20 mechanism/proof batches and final
qualification pending. Remaining candidates stay in [Queue](QUEUE.md). Concrete
Shared changes follow the automatic-approval boundary recorded in the
[proposal](../proposals/m5-cpu-audit-gap-repair.md).

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
