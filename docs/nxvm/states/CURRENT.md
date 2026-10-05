# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 | Closed after accepted corrective S20; shared PC composition and version extraction complete. |
| T543 S1 | Accepted: My5160 source/test/artifact split with complete verification and pushed actual-change review. |
| T543 S2 | Accepted: My5170 source/test/artifact split; Shared e9bb0dc55 and NXVM fe4ca8107 reviewed against the original request and complete S2 evidence. |

T543 remains open. No implementation packet is active between S2 acceptance
and S3 admission. Next is MyDeskPro386; S4 finishes NXVM and the family cutover.
The [proposal](../proposals/m5-independent-pc-apps.md) and
[convergence ledger](../history/M5-T543-four-pc-apps.md) retain the original
four-App scope and all 58 integration contexts required at T exit.

## Current Technical Baseline

- x86/chips owns independent chips; x86/core owns the sole neutral execution,
  guest clock and memory engine. Lib/Common/x86 source and tests are unchanged.
- ibmpc/board-common, board-at and board-xt own common/family PC mechanisms.
  ibmpc/machine owns execution, pacing, media lifetime and Common adaptation;
  ibmpc/product owns one parser, command/Debug/UX path, entry and version.
- My5160 and My5170 have independent fixed bindings and compositions.
  Shared pc_at_profile, pc_at_rom and pc_at_preparation own common AT projection,
  ROM mapping and candidate lifetime. Model constraints remain App-owned.
  No default/5170 mixed production translation unit remains.
- DeskPro and default still live in app-nxvm pending their sequential cutovers.
  Model40 retains sole D4 state and copied observations. Test-only family
  fixtures and cross-profile matrices remain live with S4 as their receiver;
  they are not production dependencies between Apps.
- Four PC machines remain runnable. PC110 is not implemented. MyNES retains its
  unchanged 0043 pair and does not consume ibmpc.

## Retained Runnable Evidence

Accepted source deliveries: Shared e9bb0dc55 and NXVM fe4ca8107. My5160 and
My5170 retain optimized stripped 0.5.0543 x64/x86 pairs and adjacent owner INI
directly in assets/my5160 and assets/my5170. Accepted Td S176 requires no profile
subdirectory; rebased INI paths retain identical external masters, access modes
and non-path settings. DeskPro/default retain rebuilt 0542 pairs at their legacy
assets/nxvm profile roots until S3/S4. All eight PC hashes and INI hashes are in
the T543 ledger. Runtime Debug remains present.

S2 proof: full units pass 506/506 per width; original AT Console, CMOS and boot
predicates pass on both widths; every affected receiving PC product is rebuilt
and passes its original boot checkpoint once per width. Both specialized
aggregates and 19 manifest/corpus/DAG/layout/negative cases pass per width.
PE width, optimization, compiler-debug stripping, hashes, documentation
governance and diff checks pass. The initial AT CMOS fixture failure and its
path-independent identity correction remain recorded in history.

Coordinator actual-change review accepts the moved algorithms and unique
owners, original test predicates/configuration semantics, scoped commits and
deployed files. This is S2 acceptance, not a whole-T integration claim.
The latest complete 58-context integration proof remains T542 S20; T543 must
rerun every original context at final exit. Earlier proof and source accounting
remain in indexed history. No active owned test/build process remains from S2.

## Next Work

S3 will move Model40 composition, D4, ROM declarations, copied observations,
tests and fixed build binding into app-mydeskpro386, deploying directly under
assets/mydeskpro386. S4 retains default hardware as app-nxvm, reconciles all
family test owners and finishes references and independent builds.
Only an admitted packet may execute either delivery.

Later qualification candidates remain in [Queue](QUEUE.md). Common wake-failure
and Shared vocabulary follow-ups remain in [TODO](TODO.md); this split does not
claim to fix them or qualify new hardware/timing.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board;
[T541](../history/M5-T541-independent-pc-apps.md) delivers shared Product;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) closes Machine and
composition prerequisites. None replaces later qualification.
