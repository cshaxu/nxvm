# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 | Closed after accepted corrective S20; shared PC composition and version extraction complete. |
| T543 S1 | Admitted: extract My5160 first; one complete App per sequential S. |

Shared delivery eb5882c21 and NXVM delivery b10fc0540 pass separate actual-change
coordinator review. The [S20 ledger and verification](../history/M5-T542-shared-pc-machine-adapter.md)
record all six members, test/build corrections, source accounting and current
artifact hashes. Its [proposal](../history/M5-T542-at-composition-residual-proposal.md)
is archived. T543 now owns the next implementation packet below.

## Active S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T543 S1; latest numeric T542 closed at 4c0c2db16, no identifier reused. |
| Admission And Approval | Human owner approved the four-App split and exact names on 2026-10-04, then required one App per S in order: My5160, My5170, MyDeskPro386, NXVM. Parallel src/test/assets ownership, unified docs/nxvm, tools/nxvm, version and MTSP sequence are approved. Allowed targets are NXVM and Shared PC build/ibmpc only; separate one-target commits. |
| Objective | Deliver the complete My5160 source/test/build owner, using the existing shared PC Product/Machine and XT board path without peer-App production dependency. |
| Non-goals | No AT/DeskPro/default source cutover in S1, no chip algorithms or timing grades, no Lib/Common/x86 source/test or MyNES changes, no external masters or unsolicited INI rewrites, no new runtime framework. |
| Reference Baseline | Clean pushed 4c0c2db16; accepted T542 S20 and current eight 0542 artifacts with history hashes. |
| Candidate Proposal | [Four PC Apps](../proposals/m5-independent-pc-apps.md); [T543 ledger](../history/M5-T543-four-pc-apps.md), S1 row. |
| Files And ABI Surface | Move app-nxvm/profiles/xt to app-my5160; XT-specific test/App binding/build/docs/tool references follow their owner. Inspect cmake/nxvm/NxvmProductProfile.cmake and NxvmProduct.cmake, root product selection, existing ibmpc/product build contracts and integration support. Reuse current opaque/copy ABIs; do not invent a registry or cross-App forwarding facade. |
| Applicable Rules | Read NXVM guide, CONTRIBUTING, rules/EXECUTION, ARCHITECTURE, CODING, DOCUMENT, NXVM design/ARCHITECTURE, CODING, UI, ROADMAP and source policy. Unique mutable owner, neutral dependency direction, complete App batch, numeric S/P, separate target commits, no protected raw assets, complete verification and actual-diff review. Architecture/coding skills apply; accepted Td S175 governs the new roots and preserves one NXVM scope/sequence. No S1 edits of docs/rules. |
| Verification | Full run-unit-tests aggregates on x64 and x86; original XT integration checkpoint on both widths once; independent selected XT product builds; manifests/corpus/DAG/static composition gates; Verify-DocumentationGovernance.ps1 -Product nxvm; git diff --check; PE widths, stripped optimization flags, retained Debug and SHA-256. All 58 original integration contexts remain mandatory at T exit. |
| Expected Markers | Units retain every baseline assertion; affected original boot acceptance passes; selected XT graph excludes peer-App production source; both usable XT EXEs at revision 0.5.0543; no obsolete XT source owner or duplicated Product/Machine. |
| Asset Needs | Existing owner-provided XT ROM/CMOS/font build inputs and external media only, unchanged. Reuse ignored build caches while needed. No new ROM acquisition or media copies; accepted Td S175 permits assets/my5160/<profile> with the identical INI and relative-path depth. |
| Reporting Requirements | Confirm scope before code; report actual missing shared prerequisites before expansion. Delivery records moves, source/test added/removed/net count, sole runtime owners, complete results and artifact hashes; coordinator reviews actual diff before separate governance acceptance. |
| Stop Conditions | Missing or mismatched BYOB inputs/toolchain, lost original test/capability, peer-App production dependency, required Lib/Common/x86/MyNES edit, or unapproved INI/path/packaging policy change. Collect safe evidence; revise scope or obtain the named governance prerequisite rather than bypass a gate. |
| Exit Criteria | S1 ledger row fully accepted: XT composition and matching tests/build/references migrated, no copy/legacy production route, required full units and affected integration pass, dual usable artifacts verified and pushed with source identity, manifests/docs/boundaries pass, actual-change review complete. |
| Original Owner Request | Admit the four-machine App split as app-my5160, app-my5170, app-mydeskpro386 and app-nxvm; original default hardware becomes NXVM itself. Each S extracts one App sequentially, with parallel src/test/assets but shared docs, tools, version and MTSP numbering. |
| Similar-Issue Sweep | Search src, test, cmake, tools and current docs for app-nxvm/profiles/xt, old App names and fixed-binding/source-selection references. Classify all production hits as migrated XT, shared mechanism, test-only aggregation or later App row; preserve historical evidence. Gate against peer-App production includes/links and stale selected-XT paths. |

## Next Work

T543 S1 is active. Planned S2, S3 and S4 extract My5170, MyDeskPro386 and NXVM,
respectively; each is admitted only after the preceding App delivery is accepted.
Later qualification candidates remain in [Queue](QUEUE.md). Td S175 reconciles
the approved parallel App artifact roots with shared scope/version/numbering.
Shared rule delivery e49f6ac5e passes actual-change review and both product
documentation checks; no source, INI or binary changes were made by that Td.

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
