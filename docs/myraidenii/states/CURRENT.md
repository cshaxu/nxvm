# MyRaidenII Status

## Current Work

M1 T4 remains open after accepted S1:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | Finite Video/Sprite convergence ledger; no invented receiver admitted without a lawful flag-family consumer contract. | P1 `ac0872270`; coordinator review: documentation governance and `git diff --check` passed. |
| S2 | Lawful-source disposition: existing sources cannot establish the sprite-control consumer contract, so code remains correctly blocked. | P1 `f67876e8e`; coordinator review: documentation governance and `git diff --check` passed. |
| S3 | Public FPGA implementation provenance review; it is MAME-derived/GPL and supplies no independent Video/Sprite contract. | P1 `ec71c2227`; coordinator review: documentation governance and `git diff --check` passed. |
| S4 | Owner-authorized L2 sprite-flags latch from reference-observed behavior; no imported implementation or claimed decode/render effect. | P1 `2276bbe2c`; selected-set x64/x86 both advance past the prior flags stop; complete repository units pass 669/669 on x64 and x86. |
| S5 | Selected-set-observed `0x696--0x697` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `bc67b7bbc`; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s5-selected-control-hole-l2.md). |
| S6 | Selected-set-observed `0x69c--0x69d` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `e317a10b8` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s6-selected-control-hole-l2.md). |
| S7 | Selected-set-observed `0x692--0x693` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `f4ebdcf4f` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s7-selected-control-hole-l2.md). |
| S8 | Exact selected-set-observed `0x69a--0x69b` Board-local write-only L2 hole. | P1 `fa34720df` accepted by coordinator review; x64/x86 focused Board and selected-set probes advance to the independent next stop `0x690`; [L2 evidence](../etc/evidence/t4-s8-selected-control-hole-l2.md). |
| S9 | Exact selected-set-observed `0x690--0x691` Board-local write-only L2 hole. | P1 `b576fcae5` accepted by coordinator review; x64/x86 focused Board and selected-set probes reach the separate COP-owned `INVALID_STATE` boundary; [L2 evidence](../etc/evidence/t4-s9-selected-control-hole-l2.md). |
| S10 | Bounded Board-owned COP failure attribution for the selected set. | P1 `6a79f1102`; x64/x86 probes both identify command `0x5205`, first failed DMA address `0x6a0`, and existing `INVALID_STATE`; complete repository-only units pass 642/642 on each width. [Evidence](../etc/evidence/t4-s10-cop-failure-attribution.md). |
| S11 | L2 sprite-protection upload aperture from independently observed reference address ownership. | P1 `a9c048fd9`; x64/x86 focused Board and selected-set probes reach the separate Video/CRTC receiver at `0x600`; complete repository units pass 642/642 per width. [Evidence](../etc/evidence/t4-s11-sprite-protection-upload-l2.md). |
| S12 | L2 Video-owned CRTC register aperture from independently observed reference address ownership. | Active: investigate and, only if the bounded read/write contract is confirmed, admit `0x600--0x63f` through the existing Video owner. |

## Active Packet — M1 T4 S12

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S12 follows accepted S11 P1 `a9c048fd9`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits an external-emulator behavioral L2 model when physical L3 material is absent. It permits behavior only; no source, table, ROM, trace or algorithm import. |
| Objective | Establish the smallest Video-owned receiver for the selected set's next exact stop at `0x600`, and prove bounded forward progress without treating the Video area as generic Board RAM. |
| Non-goals | No physical CRTC timing claim, rendering/decryption/palette implementation, generic control RAM, new executor, Shared/API change, source translation, protected-ROM inspection, trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `a9c048fd9`; both selected-set probes advance through the S11 COP upload aperture and fault at `0x600`. The pinned external reference map assigns `0x600--0x63f` as read/write CRTC ownership. |
| Candidate Proposal | One reset-cleared 64-byte Video CRTC register aperture. It preserves byte reads/writes only within `0x600--0x63f`; Board translates physical addresses to existing Video ownership and retains all neighbouring refusal. |
| Files And ABI Surface | MyRaidenII `core/video.[ch]`, `core/board.c`, existing Board smoke, CURRENT and one evidence record. No Shared component, public cross-component ABI, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address/ownership fact is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; manifest, documentation-governance and `git diff --check`. |
| Expected Markers | The aperture preserves byte order and reset, accepts only `0x600--0x63f`, rejects immediate neighbours, and both widths advance past the prior `0x600` stop to one same next independently owned boundary. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record local stop and external ownership separately; state the L2 limitation; report exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Required algorithmic behavior, non-byte width semantics that Core cannot lawfully split, contradictory reference/runtime evidence, Shared change, protected-data exposure, divergent host-width result or cleanup failure. |
| Exit Criteria | The bounded Video receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or a conflicting contract is refused and transferred. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check all `0x600--0x63f` bytes, surrounding `0x5ff`/`0x640`, reset/readback ownership, and ensure Board has no duplicate Video state or generic fallback acceptance. |

## Retained Packet — M1 T4 S11

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S11 follows accepted S10 P1 `6a79f1102`; one session performs coordinator and executor roles. |
| Objective | Admit the smallest L2 Board receiver for the selected set's COP `0x5205` destination `0x6a0`, based on independently observed emulator address ownership, and prove bounded forward progress without importing external implementation. |
| Non-goals | No emulation of external sprite-protection algorithms, CRTC/Video behavior, generic control RAM, Shared/API change, protected ROM inspection, trace capture, playable-game claim, or physical-hardware-accuracy claim. |
| Candidate | One reset-cleared, write-only, Board-owned `0x6a0--0x6bf` sprite-protection upload aperture. It accepts only contained writes, rejects reads and inspection, and preserves no public readback contract. |
| Source Policy | MAME address ownership is behavioral L2 context only. No source code, tables, ROM content, payload, PC/register data or external algorithm is imported. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set startup probe with transient staged program lanes; complete repository dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The aperture's contained-write/read-refusal contract is covered; both widths advance beyond the prior COP stop to the same independent next receiver; reviewed evidence and one MyRaidenII P are committed/pushed. |
| Stop Conditions | The aperture needs readback, algorithmic behavior, an external source translation, a Shared change, ambiguous width semantics, divergent host-width result, protected-data exposure or staging cleanup failure. |

## Retained Packet — M1 T4 S10

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S10 follows accepted S9 P1 `b576fcae5`; one session performs coordinator and executor roles. |
| Objective | Attribute the selected-set COP `INVALID_STATE` to the existing pending operation and first failing Core DMA address without logging protected payload or inventing a receiver. |
| Non-goals | No COP algorithm/mapping acceptance, generic trace recorder, second execution path, Shared/Core API, ROM dump, full instruction trace, Board fallback route, or playable-game claim. |
| Candidate | One reset-cleared, Board-owned copy of the immediate COP failure observation; a read-only MyRaidenII Board getter for the existing bounded integration probe. COP records only operation discriminator, command/mode value, first failed bus address and existing status. |
| Files And ABI Surface | MyRaidenII Core private COP header/source, Board interface/source, existing Board unit and selected-set integration probe only. No Shared ABI change. |
| Source Policy | The observation is neutral runtime metadata under the existing owner-supplied opt-in probe. It must not print or retain ROM bytes, PCs, register dumps or raw trace. |
| Verification | Focused x64/x86 Core Board smoke and selected-set probe; complete dual-width repository unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The probe emits a bounded, stable fault attribution on both widths, synthetic failure/reset coverage proves no stale diagnostic state, and one reviewed MyRaidenII P is committed/pushed. |
| Stop Conditions | Observation would require protected payload, ambiguous multiple failures, a Shared change, a new execution path, or contradictory dual-width result. |

## Retained Packet — M1 T4 S9

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S9 follows accepted S8 P1 `fa34720df`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x690`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `fa34720df`; after exact routes at `0x692`, `0x696`, `0x69a` and `0x69c`, both widths fault at `0x690`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S9 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x690`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S8

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S8 follows accepted S7 P1 `f4ebdcf4f`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x69a`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `f4ebdcf4f`; after exact routes at `0x692`, `0x696` and `0x69c`, both widths fault at `0x69a`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S8 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x69a`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S7

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S7 follows accepted S6 P1 `e317a10b8`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x692`, the current reference-map gap and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `e317a10b8`; after the exact `0x69c--0x69d` route, both widths fault at `0x692`. The current reference map leaves its local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if the replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine the refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S7 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after a production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x692`; otherwise report the retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S6 follows accepted S5 P1 `bc67b7bbc`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits a bounded L2 model from other emulators where L3 material is unavailable. It permits behavioral facts only, not source/code/table/ROM import. |
| Objective | Use the exact selected-set fault point at `0x69c`, the current reference-map gap and actual forward-progress replay to admit only the smallest state-free Board operation that makes the existing program advance. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, reference implementation translation, Shared/Core API change, ROM retention, or physical-hardware-accuracy claim. |
| Reference Baseline | `bc67b7bbc`; dual-width bounded probe passes `0x696` then faults at `0x69c`. Current MAME RAIDEN II map has a gap between its sprite-buffer trigger at `0x68e--0x68f` and next receiver at `0x6a0`, providing L2 map-disposition context only. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback by one exact two-byte route only if the selected-set replay advances and all local read/inspection/neighbour/boundary tests retain refusal; otherwise retain refusal and record/transfer the result. |
| Files And ABI Surface | `test/myraidenii/integration/selected_set_startup_probe.c` for bounded diagnostic evidence; if proven, `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S6 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide, execution, architecture, coding, documentation and source-policy rules. The precise reference fact is L2; local program behavior is a bounded external integration observation. |
| Verification | x64/x86 selected-set probe with existing transient staging/cleanup; focused Board x64/x86 test; complete repository-only unit suites per width after production change; documentation governance and `git diff --check`. |
| Expected Markers | Both widths stop at the same named address before the change; a state-free exact route accepts only its two byte lanes and retains reads, inspection, mixed widths and neighbours as errors; the probe advances beyond `0x69c` or the refusal is retained with a justified record. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record the selected-set fault fact separately from the reference-map fact, L2 limitation, exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Read/stateful access, access width not representable by existing callback contract, reference contradiction, required new owner/API, protected-data exposure, or failed staging cleanup. |
| Exit Criteria | A bounded L2 disposition resolves the operation; any accepted Board-only route has focused and full dual-width proof/evidence, or an incompatible operation is explicitly retained/refused and transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require physical-level details from the owner; use other emulator observable behavior as an L2 reference when L3 material is absent. |
| Similar-Issue Sweep | Review each existing selected write-only route and the immediate `0x690--0x69f` neighbours for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S5 follows accepted S4 P1 `2276bbe2c`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorized L2 behavioral models from other emulators when L3 material is unavailable on 2026-10-19; no external source import, translation, algorithm/table reuse or protected ROM retention is authorized. |
| Objective | Establish the exact selected-set-observed `0x696--0x697` V30 control write as a Board-local, write-only, no-state L2 hole, then prove that the real program advances past it on both host widths. |
| Non-goals | No Video/Sprite register, RAM, decoder, decryption, palette, renderer, Shared API, Core CPU change, generic control-window acceptance, or claim of original-hardware accuracy. |
| Reference Baseline | `2276bbe2c`; the bounded selected-set probe reaches a terminal fault with code `0x696` only after the completed `0x6ce--0x6cf` sprite-flags route. Current MAME RAIDEN II map is an L2 behavioral reference and does not assign this span a device receiver. |
| Candidate Proposal | Add one two-byte replacement route to the existing selected-control write-only hole owner; retain explicit rejection for all reads, inspection, adjacent bytes and wider mixed accesses. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board unit smoke, S5 evidence and CURRENT only. No public API or ABI changes. Expected production +5--10/-0--3; test +12--24/-0--4; refine after actual change review. |
| Applicable Rules | MyRaidenII guide, architecture/coding/source policy, execution and documentation rules; source policy permits L2 behavioral contracts but prohibits external implementation import. |
| Verification | Focused x64/x86 Board smoke; bounded selected-set probe on x64/x86 with transient staged lanes and cleanup; documentation governance, manifest if applicable and `git diff --check`; complete repository units per width before S closure. |
| Expected Markers | Synthetic actual Board writes accept only the exact two bytes and preserve no readback; selected set advances beyond `0x696`; all unrelated control addresses still reject. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only and staged transiently by the existing probe; no bytes, screenshots or traces retained. |
| Reporting Requirements | Record exact route, L2 limitation, reference distinction, actual diff, synthetic/selected-set results, cleanup and any next stop without claiming playable output. |
| Stop Conditions | A read, stateful consumer, width other than the bounded word, contradictory reference behavior, required Shared/API change, protected-source exposure or probe cleanup failure. |
| Exit Criteria | Exact Board-only route, tests and evidence are complete; dual-width complete units pass; actual change review and one MyRaidenII P are committed/pushed. |
| Original Owner Request | When no physical-level evidence is available, use other simulators as L2 references rather than blocking progress; do not require the owner to supply more detailed materials. |
| Similar-Issue Sweep | Recheck the complete `0x400--0x7ff` control route table for accidental widening, duplicate device ownership, readback or generic-RAM fallback; classify every hit. |

M1 T3 remains open after accepted S1--S25:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |
| S8 | Video-owned L2 raster/VBlank receiver, actual IRQ/reset proof and test build wiring | `34e22a100` |
| S9 | Finite initial COP upload/trigger inventory and first strict rejection | `456cde1fb` |
| S10 | Uploaded transfer family and approved Shared callback-fault publication repair; eight receiving releases | `f2ca9ae01`, `990ff6eda`, `af1649f60` |
| S11 | L2 position/velocity updates and common COP memory transport; explicit 2a05 non-eligible disposition | `b977bfa48` |
| S12 | Complete nine-ID geometry contract, result/config gaps and minimal Types review request; no code changes | `87a1ac547` |
| S13 | Complete four-ID L2 collision/result receiver, bus/failure/latch/reset proof and S12 status-prefix correction | `b3098766d` |
| S14 | Complete decimal register family, byte/mode/reset/guest proof, exact old failure-stage preservation and safe pointer offsets | `c28a162d5` |
| S15 | Complete COP-owned register inventory, explicit unmodelled routes and shared stored DMA-mode readback | `fb5e14f7b` |
| S16 | Complete low-control operational refusal, preserved valid/mixed routes and isolated actual-COP failure matrices | `acc9cc10c` |
| S17 | Independent L2 angle-approach memory effect, target/step configuration and true-byte Core DMA transport | `b3a7cc0a6` |
| S18 | Approved optional Types math declaration vocabulary, existing runtime contract and unchanged product linkage | `9b41a82ca` |
| S19 | Complete nine-ID L2 numeric/result chain, common dispatch status, bounded arithmetic and failure-prefix proof | `02a72979a` |
| S20 | Whole residual COP/RNG/DMA contract, existing live timeline capability and cohesive implementation receivers | `d20d65ee0` |
| S21 | Complete nominal L2 RNG aliases, existing timeline observation and actual Board/guest/reset proof | `bede72c2a` |
| S22 | Complete mode-banked DMA/copy/fill/sort receiver, one pending owner, and actual Board proof | `52fc45e98` |
| S23 | Complete Video-DMA/palette/brightness receiving-evidence gate; no invented implementation | `c48f9e02c` |
| S24 | Complete COP 2a05 offset-add receiver and bounded actual-bus proof | `7d37760f7` |
| S25 | Bounded selected-set startup diagnostic and explicit Board/COP-owner transfer | P1 `c75a016d0`; P2 state receipt; P3 owner correction |
| S26 | Selected-set `0x43e` write-only Board hole and bounded next-stop transfer | P1 `42845208d` |
| S27 | Exact `0x6c8--0x6c9` selected write-only span and bounded Sprite/Video transfer | P1 `f792feffa` |
| S28 | Sprite-flags receiver boundary and transfer to queued Video work | P1 `b2a2d00d5`; P2 closure receipt |
| S29 | Complete refusal/evidence boundary for `1905`, `9100` and `9900` | P1 `a5904833b`; P2 closure receipt |

The owner approved advancing Video's prerequisite inside T3 and delegates
routine same-T S planning. B04 now has an actual modeled periodic receiver;
full Video rendering and DMA qualification remain queued work. The next S
receives B07/B08 COP operation contracts/evidence and real-set startup proof.
It may not copy reference code, fabricate results or silently close unresolved
COP obligations. No further approval is needed for ordinary same-T ordering.

S26 admits the selected-set-observed `0x43e--0x43f` write-only hole as a
Board-local L2 route. The previous CPU fault is a correct V30 16-bit
`MOV moffs, AX` write, while the pinned reference map assigns no receiver to
that word. The route stores no state and has no readback; all other unknown
control addresses remain explicitly refused. The same two-width probe now
passes that point and reaches a new `0x6c8` write. That later address is not
absorbed by S26 and remains the next Board/Video map-evidence receiver.

Sequential coordinator actual-change review
accepts S10's Shared P1 f2ca9ae01, NXVM receiving P2 990ff6eda and MyRaidenII
P3 af1649f60. Both full unit suites pass687/687; selected PC receiving
integrations pass14/14 per width. Eight current0564 PC EXEs are rebuilt,
deployed and hash/PE verified; MyNES and protected inputs remain unchanged.
The [S10 review](../etc/evidence/t3-s10-cop-transfer-review.md) archives its
packet, code delta, incomplete first x64 attempt, final proof and cleanup;
[results](../etc/evidence/t3-s10-results.json) retain exact release identities.
Build now contains only the persistent .ccache/.ninja caches.

The complete S10 batch is upload/register admission plus the explicit L2
5205/5a05/f205 transfer family, not a generic programmable COP. Other B07/B08
numeric/geometry/collision/unresolved-trigger/DMA-sort operations and genuine
selected-set startup remain T3 obligations. S9's initialization observation
is not a new S10 ROM run. No complete external suite, desktop/RDP, physical
timing or playable game is claimed. T3 and M1 remain open; ordinary same-T S
planning remains owner-delegated.

Sequential coordinator actual-change review accepts S11 P1 b977bfa48.
0205/0905 now have explicit L2 macro effects and41 new actual guest/failure
cases in the existing executable;2a05 still rejects because its write width
and neighbouring-word effects are not established. Its same-T B07 semantic
reconciliation receiver is mandatory before T3 can close. The original
three-ID batch is not misreported as wholly implemented.

Both final full suites pass687/687 with no failures/skips (x64 106.12s,
x86 126.27s). Ten EXEs and protected inputs remain unchanged; only local
COP source/test and MyRaidenII records changed. Build contains only caches,
with no owned temporary output left. The [S11 record](../etc/evidence/t3-s11-cop-state-update.md)
retains the complete packet, source identities, actual delta, qualification
and explicit unverified boundaries. S11 is closed; its execution qualification remains unchanged by S12.
Other B07/B08 numeric/geometric/collision/DMA/result and real-set startup
obligations remain open. S9's22 uploaded IDs are not the full game's universe.

S12 research/design is accepted after coordinator actual-change review of
P1 87a1ac547. The [complete geometric receiver](../etc/evidence/t3-s12-cop-geometry-contract.md)
archives all nine forms, shared result/configuration dependencies, L2
limitations, independent oracles, exact future file/API estimate and packet.
No production/test/build/manifest or artifact changes occurred. Ten release
identities and protected inputs remain unchanged; document checks pass and
build contains only persistent caches. No new unit or game run is claimed.

The S12 proposal requested explicit owner approval for four optional Lib Types
mathematical names across six Shared files. That specific approval has now
been received and is being executed by S18; general product approval was not
used as a substitute. No workaround, raw CRT consumer access or private FPU
dependency is introduced. The next numerical receiver must consume this complete
numeric/result ledger and its evidence choices, not one first-failing opcode.
T3 and the full playable MyRaidenII goal remain open;
2a05, numeric/unresolved/DMA/result and genuine startup retain their original
required receivers. S12 is a complete contract audit, not implemented geometry.

Sequential coordinator actual-change review accepts S13 P1 b3098766d.
The complete four-ID collision batch now has explicit L2 position/bounds
capture and copied read-only results through the existing Core bus/service
path. The [S13 record](../etc/evidence/t3-s13-cop-collision.md) retains the
original packet, containment revision, actual delta, independent test matrix,
source/test/log hashes and qualification limits. It also corrects the S12
numeric entry-status contract without implementing geometry.

Final complete units pass687/687 per width (x64 146.13s; x86 148.06s);
the earlier503/687 deadline attempt is not counted as passing. Shared, all
other Apps, ten releases and protected inputs remain unchanged. All owned
processes are terminal and build contains only persistent caches. S13 is
closed; T3 and playable MyRaidenII remain open.
The remaining numeric/2a05/unresolved/DMA/result/startup batches keep their
mandatory receiving requirements; the accepted S14 decimal receipt follows.

Sequential coordinator actual-change review accepts S14 P1 c28a162d5.
The full decimal input/mode/result family now has an explicit original L2
implementation, independent literal oracles and actual V30/provider proof.
The [S14 record](../etc/evidence/t3-s14-cop-decimal.md) retains its packet,
failure-stage reconciliation, pointer review, actual delta and exact hashes.

Final frozen-source full units pass687/687 per width (x64 109.49s,
x86 129.18s). Shared, other products, ten release identities and protected
inputs remain unchanged. Owned processes are terminal and no disposable
task root exists; build contains only persistent caches. S14 is closed with
no active S packet at its acceptance. T3 and the complete playable MyRaidenII goal remain open;
numeric/math authorization,2a05, unresolved operations, remaining COP/DMA
decode and selected-set startup retain their required receivers.

Sequential coordinator actual-change review accepts S15 P1 fb5e14f7b.
The [S15 record](../etc/evidence/t3-s15-cop-register-inventory.md) archives the
complete map ownership inventory, packet, failure-stage reconciliation,
source/test/log hashes, actual delta and verification limits. Five missing
COP groups no longer succeed as RAM; unimplemented algorithms still fail
explicitly. The existing DMA mode shares stored GPR readback, with no new
state, public API or execution path.

Both final complete unit suites pass687/687 (x64 119.11s; x86 126.84s).
Shared, other Apps, ten releases and protected inputs remain unchanged.
Owned processes are terminal and build contains only persistent caches.
S15 is closed with no active packet. T3, M1 and playable MyRaidenII remain
open; the full low-control window, numeric/2a05/DMA/unresolved operations
and genuine startup retain their mandatory receivers. The independent
Shared complete-span query discrepancy is recorded in TODO, not repaired.

Sequential coordinator actual-change review accepts S16 P1 acc9cc10c.
The [S16 record](../etc/evidence/t3-s16-board-control-window.md) archives its
packet, one fallback-route design, exact retained13/25/14-row COP matrices,
actual Board CPU transfer/fault proof, frozen identities and qualification.
Every400h--7ffh byte has a device disposition; unknown data accesses no
longer succeed as hidden RAM. Existing valid multiword devices and real
boundary prefixes retain their original Core-owned transfer behavior.

Final strict cached builds and full units pass687/687 per width (x64
122.30s; x86 133.90s). Shared, other Apps, ten releases and protected inputs
remain unchanged. Owned processes are terminal; build contains only caches.
S16 is closed with no active packet. T3/M1 remain open: the Shared wide-query
classification debt, numeric/math authorization,2a05, remaining algorithms
and real-set startup retain their original receiving obligations. No desktop,
RDP, integration-suite or playable-game qualification is inferred.

Sequential coordinator actual-change review accepts S17 P1 b3a7cc0a6.
The [S17 record](../etc/evidence/t3-s17-cop-angle-approach.md) archives the
whole nine-ID dependency disposition, packet, independent L2 circular model,
actual53 command cases, byte/failure-prefix proof and exact run identities.
6200's memory effect and target/step capability now use the existing COP
store, admission and single Core DMA/safe-point path without new state/API.

Strict cached builds pass both widths. Full units pass687/687: x64 117.73s
before one final fixture row, followed by the final whole affected-unit
1/1 pass; x86 final687/687,129.64s includes that row. Evidence does not claim
the earlier x64 full run included it; unchanged686 binaries retain their
full proof. Shared, ten releases and protected inputs remain unchanged.
Owned work is terminal and build contains only persistent caches.
S17 is closed with no active packet; T3/M1 remain open. Common arithmetic
status/results, eight other numeric IDs, the explicit Shared math approval,
2a05/RNG/DMA/unresolved forms and genuine startup retain their full receivers.
There is still no playable MyRaidenII or desktop/RDP qualification.

Sequential coordinator actual-change review accepts S18 Shared P1 9b41a82ca.
The [S18 record](../etc/evidence/t3-s18-types-math.md) retains explicit owner
approval, the six-file scope, archived packet, exact aliases/link ownership,
actual delta and dual-width log identities. The additive optional header
provides no algorithm/state and leaves every existing interface unchanged.

Final full repository units pass687/687 per width, zero failures/skips
(x64 113.60s; x86 126.35s). Strict cached builds and subsequent no-work
incremental builds pass. Ten existing releases,33 tracked asset guards and
six media-role hashes remain unchanged; no existing product includes this
header and no release rebuild is required. Owned runs have terminated and
build contains only persistent caches. S18 is closed with no active packet.
The complete nine-ID numerical/result receiver is now unblocked by the math
vocabulary prerequisite, but remains unimplemented/unqualified as recorded
in S12/S17. T3/M1,2a05/RNG/DMA/unresolved forms and genuine startup remain
open; no desktop/RDP, external integration or playable-game pass is claimed.

S19 executor delivery completes the full nine-ID L2 numeric/result receiver.
The [S19 record](../etc/evidence/t3-s19-cop-numeric.md) retains all dispositions,
actual-bus/failure proof, the x87 truncation correction, frozen identities and
actual production+136/-3, test+333/-2 delta. Final complete units pass687/687
per width (x64 116.99s; x86 130.43s), zero failures/skips. Shared, ten release
identities and protected inputs remain unchanged. Sequential coordinator
actual-change review accepts P1 02a72979a against the complete nine-member
ledger and packet. The common register/dispatch/DMA path, all old literal
matrices and exact frozen source/test identities remain intact. S19 is closed
with no active packet and no owned temporary work left. T3/M1 remain open:
2a05, remaining unidentified/unresolved forms, RNG, sorting/DMA and genuine
selected-set startup retain their mandatory receivers. No rendering, sound,
external integration, desktop/RDP or playable-game qualification is claimed.

S20 executor delivery reconciles the complete residual COP/RNG/DMA batch in
the [residual contract](../etc/evidence/t3-s20-residual-contract.md). Existing
Core timeline observation removes the suspected RNG Shared-interface gap;
mode-banked DMA, Video latches, both2a05 outputs and unknown macro semantics
remain distinct required receivers. No source/test/artifact changes or new
dynamic qualification occurred. Sequential coordinator actual git show
review accepts d20d65ee0's four-file209-line record against the complete
residual ledger. Source/test delta remains zero and S19's exact687/687 dual
qualification is retained, not rerun or relabelled. Ten releases and all
protected guards are unchanged; no temporary work exists. S20 is closed with
no active packet. Ordinary local RNG implementation is unblocked without a
Shared edit; DMA, Video and residual/startup obligations keep T3/M1 open.

S21 executor delivery completes the mapped RNG family using existing Core
timeline observation, with no new state/API or Shared changes. The
[S21 record](../etc/evidence/t3-s21-cop-rng.md) archives the packet, explicit
L2 limit, actual production+18/-1 and test+121/-4 delta, independent raster
oracles and runtime guest MMIO. Final full units pass687/687 per width,
zero failures/skips (x64 387.69s; x86 171.12s). Strict cached and no-work
incremental builds pass. Ten existing releases and all protected guards are
unchanged; no affected existing EXE requires rebuilding. Owned runs are
terminal and build contains only persistent caches. Sequential coordinator
actual git show review accepts bede72c2a's six-file+298/-6 delivery against
the full RNG contract, archived packet and frozen qualification identities.
S21 is closed with no active packet. DMA, Video latches, remaining macros
and real selected-set startup keep T3/M1 open.

Sequential coordinator actual-change review accepts S22 P1 `52fc45e98`.
The COP has one raw-mode-selected 512-bank setup owner and replaces its prior
single pending-command fields with one discriminated pending operation; no
second queue, Core DMA route, public interface, Shared component, ROM, or
artifact path was added. The existing Board executable proves byte lanes,
bank isolation, invalid admission, copy/fill units, overlap, stable signed
sort, unknown sort ordering and reset. Final complete cached units pass
687/687 per width, and protected release/media hashes match after restoring
two unrelated MyNES EXEs rewritten by an aggregate build. The S22 evidence
archives exact hashes, L2 limits and remaining receiver boundaries. S22 is
closed with no active packet; T3/M1 remain open for Video-owned DMA, residual
macros and real selected-set startup.

Sequential coordinator actual-change review accepts S23 P1 `c48f9e02c`.
The receiver-3 evidence gate covers all ten modes (`14`, `15`, `80`--`87`),
their common registers and their private Video ownership without source import,
ROM inspection, special-case address, API or code change.  It correctly blocks
implementation until selected-set descriptor and palette/brightness oracles
exist; no unqualified mode is reported as a no-op or a RAM copy.  Documentation
governance and `git diff --check` pass.  S23 is closed with no active packet;
T3/M1 remain open for residual macros and genuine selected-set startup, while
the queued Video task owns later rendering/DMA qualification.

Sequential coordinator actual-change review accepts S24 after the final frozen
two-width qualification.  `2a05` now uses the one existing uploaded-command,
pending-operation and checked Core DMA route: for each slot it reads the
unsigned delta at `B1+1e+4i`, then writes widened sums at `B0+6+4i` and
`B0+1e+4i` in that order.  The existing Board executable proves all three
slots, carry, input/output ordering, pending/reset, source failure and late
DMA-lane prefix behavior.  Final full repository-only units pass687/687 per
width (x64 106.24s; x86 125.78s).  No Shared/public ABI, external source,
ROM/media, artifact, integration, desktop/RDP or playable-game claim changed.
The [S24 record](../etc/evidence/t3-s24-cop-offset-add.md) retains the
complete bounded model and frozen hashes.  S24 is closed.  T3/M1 remains open
for unresolved `1905`/`9100`/`9900`, Video qualification and genuine
selected-set startup.

S25's [bounded selected-set diagnostic](../etc/evidence/t3-s25-selected-set-startup-diagnostic.md)
uses the verified external program lanes without retaining protected payload.
It stops identically on x64 and x86 at a CPU-internal fault caused by a write
to `0x43e`, before a COP command checkpoint. A bounded Debug stack observation
confirms V30's existing 16-bit `MOV moffs, AX` branch, not a 32-bit CPU path.
This address is not established as a Board/COP register, so S25 does not widen
a route or weaken the explicit control window. Its owner is an open
MyRaidenII Board/COP mapping prerequisite; T3 remains open for that and its
existing COP/Video/startup receivers.

S26 accepts only the narrow selected-set `0x43e--0x43f` write-only Board
route. The existing Board executable proves that it does not supply readback
or weaken any other control-window rejection. The two-width selected-set probe
then advances from the prior stop by two instructions to the same new `0x6c8`
write on both widths. That later address is explicitly not absorbed: its
Board/Video receiver remains open, along with the existing COP, Video, and
startup requirements. See the [S26 record](../etc/evidence/t3-s26-control-write-hole.md).

S27 accepts only the independently evidenced `0x6c8--0x6c9` write-only span;
the real V30 word store reaches its two byte lanes and retains no state or
readback. Both selected-set probes next stop at the named `0x6ce--0x6cf`
Sprite/Video flags receiver. The [S27 record](../etc/evidence/t3-s27-selected-control-noop.md)
retains its exact boundary, cleanup and final 687/687 dual-width qualification.
S27 is closed; T3/M1 remain open for the receiver and genuine selected-set
startup, without any playable-game claim.

Sequential coordinator review accepts S28's source-policy-bounded disposition:
the selected write at `0x6ce--0x6cf` has a named Sprite/Video owner but no
independently qualified state, reset, decryption or frame contract. The
existing `0x6cb` program-bank receiver remains correct despite the older
reference-file comment naming `0x6c9`: the same fixed Raiden II map explicitly
maps the bank byte at `0x6cb` and leaves `0x6c8--0x6c9` unassigned. S28
therefore leaves S27's narrow no-op intact, imports no reference algorithm,
and transfers the flags receiver to the queued Video task. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S28 is
closed with no active packet. T3/M1 remains open for Video, sound, Product and
genuine selected-set startup qualification.

Sequential coordinator review accepts S29's complete three-form disposition.
The local classifier remains the only command admission/advance path; all three
forms still reject before creating a pending operation or result side effect.
No original hardware evidence was found to establish their operand/output,
reset or failure contracts, and source-policy rules prohibit deriving those
contracts from reference comments. The blockers remain T3-owned and are not
misrepresented as aliases, no-ops, Video or Sound work. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S29 is
closed with no active packet. T3/M1 remains open for the three forms, Video,
sound, Product and genuine selected-set startup qualification.
