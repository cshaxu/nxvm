# Project Status

## Current Work

M5 T539 remains open. S1-S15 are accepted. S16 is admitted for the FPU
extraction under the owner's automatic-S authorization.

S16 implementation verification is complete, pending delivery and coordinator
actual-commit acceptance. Both widths pass 368/368 units and 20/20 default
integrations; XT/AT boots pass once per width. Model40's instrumented probes
hit 90 seconds, but a controlled contrast with only the existing retirement
observer disabled reaches installer-ready on both widths within the same bound.
The evidence retains both failures and the successful contrasts. Eight Release
EXEs, tools-off 44/44 and six exact manifests are verified; both build trees
are restored to default. S16 is not yet closed.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S16; next identifier after accepted S15 P3 c0722284b. |
| Admission And Approval | Owner's 2026-09-28 automatic admission of all remaining chip batches; coordinator admits Shared and NXVM FPU extraction after the S16 boundary review. Normal push authorized; no force push. |
| Objective | Consume the complete fpu.c/fpu.h/fpu_interface.h ledger row: one opaque Types-only Shared FPU, public board/CPU connections, original semantics and test cases preserved, old owner removed. |
| Non-goals | No new x87 instructions or timing grades, CPU extraction, ibmpc migration, firmware changes, INI edits, Lib/Common or MyNES changes. CPU and all remaining ledger work stay in T539. |
| Reference Baseline | c0722284b, clean worktree; S15 runtime/artifact evidence is the before baseline, not proof of S16. |
| Candidate Proposal | [T539 proposal](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [S16 boundary and review](../etc/architecture/t539-s16-fpu-extraction.md). |
| Files And ABI Surface | Shared src/test x86 devices/fpu, standalone build/boundary/manifests; NXVM FPU consumers in CPU, machine, scheduler, timing, profiles, tests, timing-result verifier and CMake; eight NXVM artifacts and task records. MyNES link-input review only. |
| Applicable Rules | Task Reading Set; shared Execution, Architecture, Coding, Document; NXVM Architecture, Coding, source policy. Sole opaque owner, no private peer include, Types vocabulary, original handler style, one failure cleanup, no dropped coverage; S16 maps each invariant to proof. Existing embedded-ROM artifact authorization remains unchanged. |
| Verification | In each reusable build/t539-s3/nxvm-{x64,x86}: build -j8, complete ctest -L unit -j4 and ctest -L integration -j1 --output-on-failure; serialize host-runtime suites. Build/test tools-off fdc-independent; six exact manifests, aggregate/specialized static gates, git diff --check, documentation governance. Build all four products per width, validate PE/stripping/hash, deploy latest 0539 pairs. Run each vendor profile/width boot once with existing bounded harness; default integrations cover default pair. |
| Expected Markers | All complete required suites pass; chip builds with Types only; no old FPU source/private consumer; original compatibility, arithmetic, error, BUSY, wait and completion tests preserved; eight verified EXEs; unchanged owner INIs; explicit MyNES unaffected proof. |
| Asset Needs | Existing external BYOB firmware/media only; ROMs embedded by established build. No external unit-test inputs, acquired assets or changed masters. |
| Reporting Requirements | Confirm boundary, report meaningful implementation/verification nodes, record original-case mapping, actual diff size and exception limits, then actual-commit review and pushed delivery. |
| Stop Conditions | New unsupported behavior/timing downgrade, missing proof for original case, unexpected shared consumer, or contract expansion outside FPU batch requires coordinator review before continuation. No timeout becomes boot success. |
| Exit Criteria | Complete ledger row migrated and connected, no duplicate implementation, standalone and receiving checks pass, all affected artifacts current, manifests/provenance/docs correct; separate Shared/NXVM complete P deliveries immediately pushed, coordinator actual-change acceptance and governance P. T remains open. |
| Original Owner Request | Extract independent chips into src/x86/devices, retain NXVM board composition, automatically admit subsequent S tasks; preserve CPU families, semantics and original code style. |
| Similar-Issue Sweep | Audit all tracked FPU includes, embedded/private accesses, metadata/CPU pairing, construction/reset/destroy, ESC/WAIT, scheduler/timing, tests/build/gates. Replace every FPU-private consumer; remaining CPU private peers are inventoried for the next CPU batch, not hidden by this acceptance. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S15 | Accepted: Shared P1 522d0b27f and NXVM P2 88ae417ff move the sole video owner and reconnect board routes. Final units 367/367 and default integrations 20/20 per width; six vendor boots once; tools-off 43/43; six manifests, specialized/document gates and eight artifact hashes pass. |

Coordinator actual-commit review accepts the complete video/display ledger
batch: sole register/VRAM/frame ownership, typed memory and copied diagnostic
boundary, unchanged original scenario expectations, atomic registration/retry,
optional VGA route preservation and explicit retirement of unused presentation
helpers. The review inspected source/test/build/document changes, not only
green checks. Post-delivery standalone checks pass 43/43. There is no new
complete-silicon or timing-grade claim.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S15 boundary](../etc/architecture/t539-s15-video-extraction.md),
[S15 evidence](../etc/evidence/t539-s15-video-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, code-size review and delivery.
CPU/FPU and all remaining inventory dispositions still belong to T539.
Their dependency/boundary review precedes each next automatic S admission;
none is silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
88ae417ff with unchanged owner INIs. S15 evidence records SHA-256, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba and video
at 522d0b27f. MyNES retains its unchanged 0043 pair: its link inputs do not
include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S15.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
