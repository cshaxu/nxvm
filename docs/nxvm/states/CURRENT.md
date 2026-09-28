# Project Status

## Current Work

M5 T539 remains open. S1-S8 are accepted. S9 is automatically admitted under
the owner's 2026-09-28 authorization for the XT PPI/keyboard batch.

| Task | Progress |
| --- | --- |
| T539 S9 | XT PPI/keyboard boundary review and implementation are active. FDC, HDC, video, CPU/FPU and final finite-ledger review remain; none is implicitly accepted. |

## Active S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S9; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-09-28 automatic-S authorization covers the approved independent-chip extraction. Targets are Shared and NXVM, delivered separately. MyNES and sibling repositories are read-only. |
| Objective | Consume both XT rows of the finite ledger: separate qualified PPI Mode-0 registers and the XT serial keyboard from board wiring, preserving one owner and removing the concrete keyboard-to-PPI dependency. |
| Non-goals | No complete 8255 Mode-1/2 or keyboard MCU, new timing grade, new board, host-input policy, asset/INI change or universal device framework. |
| Reference Baseline | 625ea7054, clean and pushed; S8's accepted source and artifacts remain the delivered baseline until S9 verification. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), its [contracts](../etc/architecture/t539-boundary-contracts.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and [S9 boundary](../etc/architecture/t539-s9-xt-boundary.md). |
| Files And ABI Surface | Shared x86 device interfaces/implementations, standalone tests/build/boundary checks/manifests; NXVM XT keyboard/PPI adapters, machine construction/reset/scheduler/input, relevant original tests, build entries, docs and eight receiving artifacts. |
| Applicable Rules | Execution packet/target separation/actual-diff review; Architecture sole ownership and neutral opaque boundaries; Coding Types-only, real-owner cleanup and original behavior preservation; Documentation authority/link consistency; source policy permits own-source relocation, not imported firmware/code. |
| Verification | Characterize original register/serial behavior and failed byte admission first. Full repository units on x64/x86; independent tools-off x86 chips on both; default integrations on both; XT/5170/Model40 boot once per width; specialized static aggregate, six manifests, docs gate, local links and diff checks. |
| Expected Markers | Existing XT PPI/BAT/parity tests retain their OK markers; new chip tests prove ports/BSR/directions and FIFO/BAT/inhibit/release/deadlines through public operations; full suites and artifact hashes are recorded, not inferred from compilation. |
| Asset Needs | Existing authorized external profile ROM/CMOS/media only. No copies or edits; repository unit tests use in-code values. |
| Reporting Requirements | Report boundary decisions and confirmed behavior differences before extraction; preserve original-case mapping, code-size change, exact verification and artifact hashes; review real diff then deliver separate target commits and push each immediately. |
| Stop Conditions | Stop for licensing, unsourced hardware behavior beyond the qualified model, required product-policy change or an unrelated concurrent edit conflict. Ordinary in-scope S admission needs no further owner prompt. |
| Exit Criteria | Both XT ledger rows have final dispositions; old chip-state copies and private peer dependency are removed; failure/reset/route rollback and all original cases are proven; required suites, eight current EXEs, manifests and governance pass; coordinator accepts actual diff and target-separated deliveries. |
| Original Owner Request | Extract independent chips into x86/devices, keep board composition in NXVM, preserve behavior and avoid duplicate paths; automatically admit each S without manual approval. |
| Similar-Issue Sweep | Review all XT serial completions (scan/BAT), receiver refusal, inhibit/clear transitions, FIFO pressure, reset and time conversion; test rejected admission before shared extraction rather than preserving counter wraparound as protocol. |

S9 progress: opaque PPI/XT keyboard source is extracted and NXVM is reconnected;
the old keyboard pair is removed. Original board cases, refused completion,
clock inhibit and eight registration rollback/retry cases have direct proof.
The release sweep also covers mode-control readiness and BAT completion while
clock/clear is inhibited. Both gaps have failing negative controls and passing
public-operation regressions after repair. Final units pass 347/347 per width,
standalone chips 18/18 and default integration 20/20 per width. All six other
boots pass once; eight rebuilt artifacts and six manifests are verified.
Actual-diff coordinator review and target-separated delivery are in progress;
S8 remains the accepted delivery until those S9 closure steps complete.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope and
decisions. Automatic admission does not waive boundary review, target-separated
commits, complete tests or coordinator actual-diff acceptance.

M5 Td S174 queued this three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are current with
unchanged owner INI contents. MyNES retains its two unchanged 0043 receivers;
its T43 remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d and the AT
keyboard chain at eb1e2e208. Current NXVM source/artifacts are 6ae9802dc.
[S8 evidence](../etc/evidence/t539-s8-kbc-extraction.md) owns source/test mapping,
verification, hashes and actual-change acceptance. S8 reviewed the real chip,
board, test, diagnostic, build and document diffs; no item remains in its
bounded brief. Shared and NXVM deliveries are pushed to origin/master.

Final units pass 345/345 per width, independent chip suites 16/16 per width,
and default external integration 20/20 per width. Every other profile/width
boot passes once. The specialized static aggregate, six manifests,
documentation governance, 50 local links and whitespace checks pass.
Both reusable NXVM build trees are restored to the default configuration.
S8 standalone chip trees remain available for the next batch. No other product
or sibling repository changed. Cooked-history rollback debt remains in [TODO](TODO.md).
This bounded acceptance does not claim new hardware qualification or
indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification. [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
retains the DMA extraction, [S6](../etc/evidence/t539-s6-dma-first-service.md)
its first-service repair, and [S5](../etc/evidence/t539-s5-pic-extraction.md)
the PIC mapping/rollback proof.
