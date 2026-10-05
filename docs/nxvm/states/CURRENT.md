# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 | Closed after accepted corrective S20; shared PC composition and version extraction complete. |

Shared delivery eb5882c21 and NXVM delivery b10fc0540 pass separate actual-change
coordinator review. The [S20 ledger and verification](../history/M5-T542-shared-pc-machine-adapter.md)
record all six members, test/build corrections, source accounting and current
artifact hashes. Its [proposal](../history/M5-T542-at-composition-residual-proposal.md)
is archived. No S packet or successor admission remains active.

## Next Work

[Four independent PC Apps](../proposals/m5-independent-pc-apps.md) is the first
[Queue](QUEUE.md) candidate. Its shared-composition prerequisite is accepted,
but the App split is not admitted and has no allocated task identifier.
Its separate naming/scope governance prerequisite remains.

## Retained Runnable Evidence

Current source deliveries are Shared eb5882c21 and NXVM b10fc0540.
Eight optimized stripped 0.5.0542 EXEs remain in assets/nxvm/<profile>, with
unchanged owner INIs and runtime Debug. Their current SHA-256 values are in
T542's S20 table; PE widths and absence of compiler debug sections are verified.

Fresh S20 acceptance: full units 506/506 per width; all 58 original external
integration contexts pass once across four profiles and two widths. The 17
manifest/corpus/DAG/negative cases, both specialized aggregates, documentation
and diff checks pass. Cached Make compile flags cover 521 strict inventory rows,
zero deferred, plus the explicit observation test object; this is not a claim
that the Ninja-only direct-command gate ran. Pre-existing build caches were
reused and preserved; no new owned temporary package tree or active test process
remains. This evidence supersedes S19 for the changed runnable source.

Lib/Common/x86 source/test, MyNES, root rules/README and owner configurations
have no S20 diff. The Common wake-failure and Shared vocabulary follow-ups
remain in [TODO](TODO.md); they are not claimed repaired here.

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
  Shared Product owns PC identity/version; App supplies a fixed machine binding,
  constructor and firmware inputs.
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
