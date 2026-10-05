# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 | Closed after corrective S12-S19 and whole-ledger acceptance on 2026-10-04. |

The owner-reopened composition extraction is complete. [T542 evidence](../history/M5-T542-shared-pc-machine-adapter.md)
maps every corrective member to its actual owner, callers, failure handling and
regressions. The [corrective proposal](../history/M5-T542-pc-composition-completion-proposal.md)
is archived. S19 delivery 6e1de211f is accepted after separate coordinator review.
No active S packet remains.

## Next Work

[Four independent PC Apps](../proposals/m5-independent-pc-apps.md) is the first
[Queue](QUEUE.md) candidate. Its shared-composition prerequisite is satisfied,
but the App split is not admitted and has no allocated task identifier.
Its separate naming/scope governance prerequisite remains.

## Retained Runnable Evidence

Source-changing S18 deliveries are Shared f1086b3bf and NXVM 844950505.
Eight optimized stripped 0.5.0542 EXEs remain in assets/nxvm/<profile>, with
unchanged owner INIs and runtime Debug. Their current SHA-256 values are in
T542's S18 table and independently confirmed unchanged during S19.

Fresh final acceptance: full units 504/504 per width; strict matrix 520/520,
zero deferred; all 58 external integration contexts pass once across four
profiles and two widths. Standalone ibmpc, canonical manifests, corpus/DAG,
specialized and documentation checks pass. These fresh S19 results supersede
historical S11 passes for this extraction. Owned temporary build/test trees
are cleaned; EXEs and external masters remain intact.

Lib/Common/x86 algorithms, MyNES, root rules/README and owner configurations
have no corrective-batch diff. The Common wake-failure and Shared vocabulary
follow-ups remain in [TODO](TODO.md); they are not claimed repaired here.

## Current Technical Baseline

- x86/chips owns independent chips; x86/core owns the sole neutral
  execution/time/memory engine. No product knows private chip state.
- ibmpc/board-common, board-at and board-xt own matching PC family mechanisms.
  Shared AT grammar/materialization has three independent consumers:
  default, 5170 and DeskPro; each supplies its own hardware values.
- ibmpc/machine owns one preparation/finishing, media/resource lifetime,
  execution/pacing and copied Common input/output/debug adaptation.
  App transfers a prepared construction and genuine Profile context;
  Core/board teardown precedes its one release.
- ibmpc/product owns shared INI/request/factory, command/hotkey/Debug,
  Common composition, process entry/banner and injected embed/deploy recipes.
  App supplies fixed identity, constructor and firmware inputs.
- App retains actual ROM layout/alias/provider and machine constraints.
  Genuine D4 and copied Model40 observations stay Model40-owned.
  No DeskPro-to-default/5170 private dependency remains.
- Production links only the selected composition. Multi-profile aggregation
  is explicit test-only input. Default/5170 retain cohesive translation units
  with build-only constructor selection; four top-level Apps are not yet built.
- NXVM retains runnable XT, AT, Model40 and default fixed products. PC110 is
  unimplemented. MyNES retains its unchanged 0043 pair and does not consume ibmpc.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board
extraction; [T541](../history/M5-T541-independent-pc-apps.md) closes shared PC
Product. T542 closes adapter and residual composition extraction, not new
hardware/timing or guest-software qualification. Queue retains those later
qualification scopes without reduced coverage.
