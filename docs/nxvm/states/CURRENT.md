# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |
| T546 S1 | Complete executor proof: Shared P1 fc9a8b725 pushed; NXVM receivers/artifacts/evidence delivered next, then coordinator acceptance. |

## Active T546 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T546 S1 after closed T545 S7 P2 966249c20; numeric allocation reconciled with Git/current history. |
| Admission And Approval | Owner requested CPU repair after T545 closure and explicitly approves the listed S1 component changes on 2026-10-05. Approved: Shared CPU reset/FLAGS/MSW and matching owner tests/manifests; NXVM evidence, necessary receiver assertions and eight PC 0546 artifacts. No Lib/Common/MyNES or INI changes. |
| Objective | Consume the complete reset/architectural-image batch: early CS:IP/base, family cache/control reset, 286 MSW readout and generation-specific internal/load/outgoing FLAGS; repair after reviewed Shared design and verify the whole batch. |
| Non-goals | Board/BIOS reset overrides, new CPU families, Lib/Common/MyNES edits, global FLAGS mask for unrelated POPF/IRET/task privilege rules, timing guesses or premature task closure. |
| Reference Baseline | 966249c20; [T546 convergence ledger](../history/M5-T546-cpu-audit-gap-repair.md), T544 boundary and all five-family audits linked there. T545 complete runtime baseline remains accepted. |
| Candidate Proposal | [CPU repair proposal](../proposals/m5-cpu-audit-gap-repair.md), initial S1 and original eighteen repair/proof receivers plus final qualification. |
| Files And ABI Surface | Approved sole src/x86/chips/cpu reset/FLAGS/MSW owners; matching test/x86 reset/FLAGS/system/Core cases and three test/ibmpc oracle/setup receivers, manifests. NXVM timing fixture, current recipe/preset revision, packet/queue/history/evidence and eight verified PC artifacts. No new public API or App workaround. |
| Applicable Rules | Execution: whole-batch convergence, concrete Shared review, full dual-width units, separate targets and artifacts. Architecture skill/rules: one CPU state/time owner, no App repair. Coding skill/rules: original table handlers, Types vocabulary, no duplicate path. Documentation: truthful tier and status. Source policy/PDF skill: original identities and visually confirmed tables, external/ignored scratch only. |
| Verification | Read and render original reset/FLAGS/MSW source pages, inspect all load/image/reset callers and oracle assumptions; transient reset/FLAGS/system tests then complete 532-case receiving units both widths. After approved code changes, rebuild all four receiving PC x64/x86 pairs at 0546, retain INIs, verify PE/hash/strip and run affected boot groups once. All eight manifests and applicable corpus/Types/specialized/documentation gates. |
| Expected Markers | Source-conditioned reset/FLAGS/MSW matrices, retained first physical fetch, full unit zero failures, no required skipped boot/gate, all changed receiving artifacts current; source-undefined fields excluded from precise claims. |
| Asset Needs | Existing archived original CPU manuals; code-defined unit fixtures only. Existing external INI/media for receiving boot regression; no new ROM/media acquisition or master modification. |
| Reporting Requirements | Report whole S1 source/code/regression map and concrete pseudocode/diff estimate before Shared edits; report conflicts/L1/false-grade changes; deliver complete proof/artifacts/commits after implementation and actual-change review. |
| Stop Conditions | Unresolved original authority, unupgradable L1 or necessary downgrade requires owner disposition. Do not equate 286 IDT programmer/hardware source conflict with proven defect. Changes beyond this approved CPU batch require concrete review. |
| Exit Criteria | All S1 fields/forms/callers have direct repaired or source-proven non-applicable disposition, sources/regressions agree, full units/receiving artifacts/gates pass and target-correct P deliveries pushed; no pending reset/image member hidden in S7 privilege work. |
| Original Owner Request | Admit next T and correctly repair CPU instructions and timing according to the full audit list; finish previous T first. |
| Similar-Issue Sweep | All five implementations plus DEFAULT resolution, cold/processor resets, CPU contexts/opaque instances/Core/board callers; outgoing PUSHF and interrupt frames versus incoming POPF/IRET/task paths; no universal saved/writable mask. |

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
T546 S1 above, with eighteen coherent mechanism/proof batches and final
qualification. Remaining candidates stay in [Queue](QUEUE.md). Concrete
Shared repair review remains required by the [proposal](../proposals/m5-cpu-audit-gap-repair.md).

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
