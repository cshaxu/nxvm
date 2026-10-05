# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 | Closed after accepted corrective S20; shared PC composition and version extraction complete. |
| T543 S1 | Accepted: My5160 source/test/artifact split, complete verification and pushed actual-change review. |
| T543 S2 | Admitted: separate My5170 definitions from the mixed default/5170 translation units. |

Shared delivery eb5882c21 and NXVM delivery b10fc0540 pass separate actual-change
coordinator review. The [S20 ledger and verification](../history/M5-T542-shared-pc-machine-adapter.md)
record all six members, test/build corrections, source accounting and current
artifact hashes. Its [proposal](../history/M5-T542-at-composition-residual-proposal.md)
is archived. T543 now owns the next implementation packet below.

## Active S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T543 S2; S1 accepted at pushed cd173acb7 and fed7049b3, no identifier reused. |
| Admission And Approval | Human owner approved the four-App split and exact names on 2026-10-04, then required one App per S in order: My5160, My5170, MyDeskPro386, NXVM. Parallel src/test/assets ownership, unified docs/nxvm, tools/nxvm, version and MTSP sequence are approved. Owner additionally requires flat assets/<app> roots with no profile subdirectory; Td S176 reconciles the rule first. Allowed targets are NXVM and Shared PC build/ibmpc only; separate one-target commits. |
| Objective | Deliver My5170's fixed composition, entry, build, tests and artifacts without importing NXVM's default profile or copying shared AT mechanisms. |
| Non-goals | No DeskPro/default App cutover yet, no chip algorithms or timing grades, no Lib/Common/x86 source/test or MyNES changes, no external masters or unsolicited INI rewrites, no new runtime framework. |
| Reference Baseline | Clean pushed fed7049b3 and cd173acb7; accepted S1 My5160 0543 pair and unchanged six legacy PC 0542 artifacts. |
| Candidate Proposal | [Four PC Apps](../proposals/m5-independent-pc-apps.md); [T543 ledger](../history/M5-T543-four-pc-apps.md), S2 row and inventory. |
| Files And ABI Surface | Inspect complete default_profile/pc_at_profile.c, its descriptor declarations, machine_plan.c and external_pc_at_rom.c/h; move concrete IBM AT values/constraints into app-my5170 and genuinely shared plan/ROM/materialization mechanisms to ibmpc. Corresponding tests, CMake graph, binding, tools/docs and assets follow ownership. Reconcile the owner's My5160 flattening with the shared deployment/test path contract; later App rows use the same flat layout. Preserve existing opaque/copy ABIs; no registry, peer-App facade or copied Product runtime. |
| Applicable Rules | Read NXVM guide, CONTRIBUTING, rules/EXECUTION, ARCHITECTURE, CODING, DOCUMENT, NXVM design/ARCHITECTURE, CODING, UI, ROADMAP and source policy. Unique mutable owner, neutral dependency direction, complete App batch, numeric S/P, separate target commits, no protected raw assets, complete verification and actual-diff review. Architecture/coding skills apply; accepted Td S176 governs the flat roots and preserves one NXVM scope/sequence. No S2 edits of docs/rules. |
| Verification | Full run-unit-tests aggregates on x64 and x86; original IBM AT integration checks on both widths once; independent selected AT product builds; affected shared PC consumers' builds/boot checks if their executable inputs change; manifests/corpus/DAG/static composition gates; documentation governance; git diff --check; PE widths, optimization/stripping, retained Debug and SHA-256. All 58 original integration contexts remain mandatory at T exit. |
| Expected Markers | Every original assertion and affected boot predicate retained; selected AT graph excludes peer-App production source; verified My5170 0.5.0543 pair; no duplicate AT preparation/materialization or Product/Machine. |
| Asset Needs | Existing owner-provided IBM AT ROM/CMOS/font inputs and external media, unchanged. Reuse ignored caches. No new ROM acquisition or media copies; assets/my5170 directly holds the pair and INI. Rebase only relative media paths to the same external masters; preserve all other settings. Verify the owner's My5160 flattened pair and rebased INI. |
| Reporting Requirements | Confirm scope before code; report actual missing shared prerequisites before expansion. Delivery records moves, source/test added/removed/net count, sole runtime owners, complete results and artifact hashes; coordinator reviews actual diff before separate governance acceptance. |
| Stop Conditions | Missing or mismatched BYOB inputs/toolchain, lost original test/capability, peer-App production dependency, required Lib/Common/x86/MyNES edit, or unapproved INI/path/packaging policy change. Collect safe evidence; revise scope or obtain the named governance prerequisite rather than bypass a gate. |
| Exit Criteria | S2 ledger row accepted: IBM AT composition and matching tests/build/references migrated, no copy/legacy production route, required full units and affected integration pass, dual usable artifacts verified and pushed with source identity, shared PC receivers verified, manifests/docs/boundaries pass, actual-change review complete. |
| Original Owner Request | Admit the four-machine App split with flat assets/<app> directories as app-my5160, app-my5170, app-mydeskpro386 and app-nxvm; original default hardware becomes NXVM itself. Each S extracts one App sequentially, with parallel src/test/assets but shared docs, tools, version and MTSP numbering. |
| Similar-Issue Sweep | Search src/test/cmake/tools/current docs for IBM AT definitions, mixed default/5170 compilation guards, external PC/AT ROM provider and descriptor/materialization callers. Classify model values, shared mechanisms and test-only matrices before moves. Gate against peer-App production dependencies; preserve My5160 independence and historical evidence. |

## Next Work

T543 S1 is accepted; S2 is active. Planned S3 and S4 extract MyDeskPro386 and
NXVM, respectively; each follows the preceding accepted App delivery.
Later qualification candidates remain in [Queue](QUEUE.md). Td S176 supersedes
S175's profile subdirectory: migrated PC Apps deploy directly to assets/<app>.
Shared rule delivery f65685e35 passes actual-change review and both product
documentation checks. Its product authority correction restores S2; the owner's
My5160 file moves await implementation verification rather than being claimed
tested by this documentation-only Td.

## Retained Runnable Evidence

The current accepted delivery is Shared cd173acb7 and NXVM fed7049b3. T543 S1
provides a verified My5160 0.5.0543 pair under assets/my5160, with its
runtime Debug. The owner has flattened its directory and rebased the INI;
S2 will verify that receiving path before accepting the changed layout.
Six unmigrated PC EXEs remain 0.5.0542
under assets/nxvm. S1 hashes and proof are in the
[T543 ledger](../history/M5-T543-four-pc-apps.md); baseline hashes remain in
T542 S20. PE widths and absence of compiler debug sections are verified.

S1 verification: full units 506/506 per width; original XT integration reaches
installer-running once per width (18.46/24.77 seconds); 17 manifest/corpus/DAG/
negative checks, both specialized aggregates, documentation and diff checks
pass. No shared six-corpus or MyNES modification. S1 actual-change review is
complete and accepted; S2 admission makes no new runtime or artifact claim.

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
  with build-only constructor selection. My5160 now has its own fixed entry,
  composition and selected graph; the other three App cutovers remain planned.
- NXVM retains runnable XT, AT, Model40 and default fixed products. PC110 is
  unimplemented. MyNES retains its unchanged 0043 pair and does not consume ibmpc.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board
extraction; [T541](../history/M5-T541-independent-pc-apps.md) closes shared PC
Product. T542 closes adapter and residual composition extraction, not new
hardware/timing or guest-software qualification. Queue retains those later
qualification scopes without reduced coverage.
