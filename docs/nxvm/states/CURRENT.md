# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |
| T546 S2 | Final post-clock/post-fixture units pass 534/534 per width (x64 107.68 s/x86 107.07 s), original receiving matrix 58/58, eight manifests and required gates pass. Eight current 0546 PC artifacts verified; INIs/media/MyNES unchanged. Complete delivery and coordinator acceptance pending. |

## Active T546 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T546 S2 after accepted S1 P3 55348a583; next unused numeric S. |
| Admission And Approval | Owner explicitly approves the presented S2 concrete scheme on 2026-10-05: CPU-local decode/fetch/cursor/privilege corrections, corresponding tests and necessary postcheck removal; 286 excessive length uses UD following the specific 1985 source. On 2026-10-06 the owner approves Model40 board clock-input reconciliation against D3PE and the existing 16 MHz L2 macro axis. Any necessary new shared API requires separate review. Lib/Common/MyNES/INI remain excluded. |
| Objective | Reconcile and repair the complete runtime fetch/decode/admission batch: thirteen unchecked early decodes, generation byte limits, required fetch failure, code cursor and illegal-form/privilege admission, with preserved original handler style and one decode owner; reconcile the exposed Model40 clock-input unit mismatch without changing CPU costs. |
| Non-goals | Lib/Common/MyNES changes, a second parser/executor, copied CPU state ownership, arbitrary new CPU support, BIOS fixes, weakened source/timing oracles or partial S closure. |
| Reference Baseline | 55348a583 and [T546 ledger](../history/M5-T546-cpu-audit-gap-repair.md); T544 S2-S6 original source/form/caller findings, especially B05a and the thirteen-call receiver. S1 remains accepted. |
| Candidate Proposal | [CPU repair](../proposals/m5-cpu-audit-gap-repair.md), initial S2 and dependencies with S3/S4/S13/S14/S17; no receiver silently omitted or moved outside T. |
| Files And ABI Surface | CPU owner src/x86/chips/cpu/cpu_instructions.c; test/x86 decode/bus/address/paging and test/ibmpc preview receivers with manifests. Model40 profile clock inputs and matching App tests reconcile sourced rates on the existing macro axis. NXVM existing integration boot harness gains bounded fault/copied-retirement observations; no new production API, BIOS workaround, Lib/Common/MyNES or INI edit. Eight receiving PC artifacts after qualification. |
| Applicable Rules | Execution: whole batch inventory, concrete Shared review, full dual-width units and affected artifacts; source contradictions/L1 require disposition. Architecture/coding skills and rules: one owner, original table handlers, inward Types/opaque boundaries and similar-issue closure gate. Documentation: truthful current versus historical proof. Source/PDF rules: originals visually verified, external archive only. |
| Verification | Original 286/386 instruction-length and sequential-fetch pages plus early/186 source reconciliation; all decode/read/skip and thirteen caller paths; preview versus runtime/failure side effects. Model40 uses original D3PE pp. 4-5/22/31 and schematic sheets 5-6 recorded in S2 evidence, with ratio/unit and refresh-poll regression. Code-owned family/mode/size/form/failure matrices and complete units both widths, manifests/corpus/Types/specialized gates and eight current 0546 PC artifacts; original affected boot contexts once. |
| Expected Markers | Ten-byte 286 legal/eleven-byte UD under owner-approved source disposition; fifteen-byte 386 legal/sixteen-byte GP0; no invented early limit. Original failed fetch identity and no dependent effects; correct required-byte and publication boundary. No required test skipped or oracle altered from code alone. |
| Asset Needs | Existing original CPU and Compaq D3PE manual archives identified in S2 evidence; repository-only unit inputs. External INI/media only for approved receiving integration, with overlay masters/INI unchanged. |
| Reporting Requirements | Review [S2 design](../etc/evidence/t546-s2-decode-admission-design.md): thirteen caller sites, 224 direct IP advances, speculative/preview fill, privilege reconciliation and necessary S4 postcheck dependency. Report 286 conflicting exception sources and unproven 186 limit before Shared edits. Deliver counted diff, full proof and target-separated commits after implementation. |
| Stop Conditions | Report newly unresolved authority, L1/downgrade or a material boundary change beyond the approved scheme. Never mark a caller qualified while its required owner or legal boundary remains broken. The unresolved 186 source classification is retained, not silently qualified. |
| Exit Criteria | Complete admitted mechanism batch directly source-proven repaired/non-applicable, all variants and failed side effects handled, full units/receiving artifacts/gates pass and actual-change review accepts; no pending member hidden by a later checkpoint. |
| Original Owner Request | Correct the full CPU instruction/function/timing audit list, preserving first-principles modularity and original code style, not first-failure patches. |
| Similar-Issue Sweep | Entire CPU runtime code/read/skip/dispatch plus all direct _d_* callers, prefix/ModRM/SIB/displacement/immediate, every retained family and real/protected/VM86 size context; exclude assembler/diagnostic scanner as alternative execution authority. Inspect all Model40 clock-plan inputs and media duration units against D3PE pin wiring and the macro axis; sweep App fixtures for implicit identity-clock/fixed-byte assumptions. |

## Accepted S1 CPU Reset And Images

Shared P1 `fc9a8b725` and NXVM P2 `912579a98` are pushed. Coordinator actual-change
review accepts the complete approved S1 source/context/receiver batch and
[direct proof](../etc/evidence/t546-s1-reset-images.md). Complete units pass
532/532 per width; all 58 original external contexts pass once; eight manifests,
33 supplemental checks per width, both specialized and documentation gates pass.
The nine discovered receiving failures are corrected at their actual test
context/oracle, without another production patch or weakened timing numbers.
Production remains two CPU C owners, net +5 lines; no API or parallel state.
Four PC 0546 x64/x86 pairs replace 0545; INIs, external masters and MyNES are unchanged.

S1 closes with governance acceptance. T546 stays open for its complete S2-S19
ledger; S2 design is admitted by continuation above, not by S1 closure.

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

Accepted baseline remains S1 `55348a583`. Working 0546 PC EXEs are unqualified
S2 candidates during this repair, not the S1 hashes below; final-source units,
all receiving contexts and artifact proof must finish before acceptance.

- Independent chips and sole CPU implementation live in x86/chips; neutral
  execution, memory/ports and guest time live in x86/core.
- IBMPC board-common/AT/XT own PC wiring; Machine owns one Common driver and
  pacing/media adaptation; Product owns one INI/command/Debug/UX/entry path.
- Four fixed Apps own immutable compositions and firmware bindings. Model40
  alone owns D4. No App production graph links a peer App.
- Shared tests follow their owners; actual model assertions stay App-local.
  The one external family harness executes each real fixed binding and its INI.
- Eight optimized stripped PC 0.5.0546 EXEs remain directly in assets/my5160,
  assets/my5170, assets/mydeskpro386 and assets/nxvm, with unchanged adjacent
  owner INIs. MyNES retains its two 0.0.0043 EXEs in assets/mynes. All ten
  hashes are in [S1 evidence](../etc/evidence/t546-s1-reset-images.md); PE widths,
  current identity and absence of compiler debug sections are verified.
  Runtime Debug remains; only CPU-dependent PC products are rebuilt.
- External media masters remain unchanged after overlay integration. No owned
  build/test process remains. Ignored receiving caches remain needed for the
  next decode/admission batch's incremental regression checks.

## Next Work And Qualification Boundary

The owner-requested CPU instruction/function/timing repair is now admitted as
T546 S2 above, with eighteen coherent mechanism/proof batches and final
qualification. Remaining candidates stay in [Queue](QUEUE.md). Concrete
Shared repair review remains required by the [proposal](../proposals/m5-cpu-audit-gap-repair.md).

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
