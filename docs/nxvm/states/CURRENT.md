# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |
| T546 S4 | Accepted after coordinator actual-change review of pushed Shared P1 2d9929742 and NXVM P2 ca016635d: complete source/caller/receiver proof, 537/537 units per width, original 58/58 external contexts and eight qualified 0546 PC artifacts. S4 closed; no active S packet. |

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

S4 is closed with no active packet. T546 remains open for S5-S19, including
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
T546 remains open for S5-S19. Complete frame/task/paging/delivery/RMW
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

S1-S4 are closed; T546 remains open for S5-S19. The owner-authorized
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

Accepted implementation baseline is S4: Shared CPU/tests `2d9929742` and
NXVM evidence/artifacts `ca016635d`. The eight 0546 PC EXEs use the corrected
production hashes and final artifact identities in S4 evidence, not the
retired S3 or earlier candidate hashes. Governance adds no executable input.

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
  owner INIs. MyNES retains its two 0.0.0043 EXEs in assets/mynes. PC hashes
  are in [S4 evidence](../etc/evidence/t546-s4-admission-next-fetch-design.md),
  unchanged MyNES hashes in [S1 evidence](../etc/evidence/t546-s1-reset-images.md); PE widths,
  current identity and absence of compiler debug sections are verified.
  Runtime Debug remains; only CPU-dependent PC products are rebuilt.
- External media masters remain unchanged after final S4 overlay integration.
  All recorded owned build/test handles are terminal. Ignored receiving caches
  remain needed for the next CPU repair batch's incremental regression checks.

## Next Work And Qualification Boundary

The owner-requested CPU instruction/function/timing repair remains open as
T546 after accepted S4, with S5-S19 mechanism/proof batches and final
qualification pending. Remaining candidates stay in [Queue](QUEUE.md). Concrete
Shared changes follow the automatic-approval boundary recorded in the
[proposal](../proposals/m5-cpu-audit-gap-repair.md).

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
