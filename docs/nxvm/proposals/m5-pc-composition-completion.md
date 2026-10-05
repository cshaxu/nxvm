# PC Composition Extraction Completion

## Admission And Boundary

The owner reopens M5 T542 after accepted S11 on 2026-10-04 to complete the
remaining shared construction mechanisms before the four-App split. Baseline:
`a6f81ad8f`, with accepted S11 implementation `51a6d209e` / `8c24462b3`.
S1's original construction/factory inventory was not fully discharged by the
later adapter extraction. This corrective batch makes that residual explicit;
it does not invalidate the proven S11 package relocation or rewrite its evidence.

Consumers remain default, IBM 5160, IBM 5170 and DeskPro Model40, on x64/x86.
Lib/Common, MyNES, owner INIs, external masters, CPU/chip algorithms and timing
grades are excluded. No App split, runtime model registry, new device framework
or hardware qualification is admitted here. CURRENT owns active status; the
original [T542 proposal](../history/M5-T542-shared-pc-machine-adapter-proposal.md)
and [history](../history/M5-T542-shared-pc-machine-adapter.md) retain earlier proof.

## Shared AT Mechanism, Not Shared Machine Definition

**5170, default and DeskPro 386 all consume the common AT assembly mechanism.**
Shared `ibmpc/board-at` materializes explicit validated chip, bus, port and route
values. It does not select a machine or start with 5170 defaults and override
them for another model. XT uses `ibmpc/board-xt`, with only genuinely common
construction/asset mechanisms in `ibmpc/board-common`.

Each App retains its topology, CPU/clock and memory constraints, ROM layout,
CMOS contents, display choice, HDC personality, floppy mechanisms and wiring.
Model40 also retains D4 and its observations. Matching AT wiring/validation
can share code; unequal values are explicit inputs, not hidden model branches.
App-owned policy checks must remain App-owned, even when they currently sit
beside reusable structural validation.

Use existing `vm_machine_construction`, neutral Machine input and profile
configure/release boundaries. One prepared candidate owns resources until
successful publication; failure releases that candidate once. Core/chips keep
their sole state/time ownership. Do not add a second plan, state cache,
executor, inheritance tree or forwarding-only layer to facilitate extraction.

## Finite Corrective Ledger

Each row is an unresolved extraction member until its named S supplies direct
source, caller, failure and regression proof. Accepted retained differences
need their concrete App owner, not a blanket claim that all profile code is
special. Test/build consumers are part of each row's coverage.

| Member and observed source | Shared receiver / retained difference | Batch and required proof |
| --- | --- | --- |
| `profiles/selection_interface.h`, `machine_plan_interface.h`: neutral values mixed with model IDs and Model40 D4 declarations | Machine/board value contracts in ibmpc; fixed identity, constructors and D4 stay App-owned | S13: all four constructors use one neutral contract; no generic header exposes a Model40 observation or imports an App |
| `product/config.c`, `profiles/machine_factory.c`: common prepare/bind/release, INFO/speed and runtime-media adaptation | ibmpc Product/Machine at the existing binding boundary; App supplies fixed identity, assets and constructor | S14: one working factory path for all four; prepare/describe failure rollback; no runtime model lookup or permanent forwarding shim |
| `profiles/machine_plan.c`: CMOS/font preparation, floppy selection, validation/publication/release and test-only getters | Machine owns finishing coupled to its existing config/assets/construction; board-common retains physical geometry/ROM validation; genuine device constraints remain explicit App inputs | S15: optional/malformed assets, media bounds, failed publication and release covered; old shared helpers/getters removed or justified by a real production caller |
| `default_profile/pc_at_profile.c`, `machine_plan.c`; `model40/model40.c` imports default private header and calls `vm_profile_ibm_5170_values_create` | common AT materialization/structural validation in board-at; three independently specified App compositions | S16: all three AT consumers connected; Model40 has no dependency on default/5170 constructor or private header; original values/routes and D4 preserved |
| ROM preparation in default/5170, Model40 and XT construction | proven identical bounded copy/interleave/validation in board-common; ROM regions, aliases, chip sizes and model interpretation remain App-owned | S17: classify every preparation path; extract only matching semantics, preserve genuine layouts and fail malformed inputs without a second ROM backing/host BIOS route |
| `cmake/nxvm` source lists, firmware embedding and deployment; associated tests/gates | common build mechanics in a shared ibmpc build helper; fixed product selection/firmware declarations remain App-owned | S18: production links only its selected composition plus shared inputs; no sibling composition dependency; tests may aggregate profiles explicitly without becoming production inputs |

S12 freezes this ledger and the corrected three-consumer AT decision. S19
accepts the whole batch only when every row is directly proven or has a specific
retained App owner with evidence. Unresolved reusable logic blocks closure and
cannot be deferred to the App split. Similar-issue review covers all four
constructors, headers, asset preparation, factories, build descriptions and
their tests; XT is excluded only from AT-specific electrical materialization.

## Linear S Plan And Exit Criteria

S1-S11 remain historical accepted steps; none is reused or reopened. Only one
future S is admitted at a time through CURRENT; this list is a plan, not eight
simultaneous packets.

1. **S12: Corrective design reconciliation.** Record all six residual classes,
   three AT consumers, concrete receivers and preserved differences; update
   Current, Queue and successor prerequisites. Exit: bounded ledger and
   implementation batches, truthful reopened status, documentation gate and
   actual-change review. Documentation only; no new runnable claim or EXE.
2. **S13: Neutral construction contracts.** Separate reusable config/assets/
   construction values from fixed identity, concrete constructors and Model40
   observations. Update live callers/tests in the same S. Exit: one neutral
   public contract, no Shared-to-App import and no new mirrored plan.
3. **S14: Factory and Product adaptation.** Extract the common request/runtime
   conversion, preparation, cleanup, INFO and speed adaptation. App retains
   fixed binding/identity only. Exit: all four builds use one production path;
   original validation and failure cleanup remain intact.
4. **S15: Candidate finishing and ownership.** Consolidate CMOS/font preparation,
   floppy/media bounds and publication/release. Retire test-only plan getters
   through existing copied construction/fixtures. Exit: exactly one owner
   through success/failure, no double release and no lost media/asset rules.
   The existing Machine config/assets contract makes Machine the cohesive
   finishing receiver, avoiding a board-common-to-Machine dependency cycle.
   Four constructors return copied construction directly; their genuine context
   and configure/release callbacks remain App-owned. Floppy eligibility is an
   explicit input, never inferred from a model or another model's defaults.
5. **S16: Three-consumer AT construction.** Extract parameterized AT mechanisms
   and connect 5170/default/Model40 together, removing peer-profile coupling.
   Exit: independent model definitions produce unchanged electrical contracts;
   XT remains on its own family path; no 5170-then-override construction.
6. **S17: ROM preparation classification and consolidation.** Review all four
   asset-to-ROM preparation paths; share only identical mechanisms. Exit:
   each shared/retained path has an explicit semantic reason and regression,
   preserved ROM aliases/mapping, no copied generic loader per future App.
7. **S18: Build and test ownership readiness.** Consolidate build/embed/deploy
   mechanics and separate selected composition inputs from test aggregation.
   Exit: all four existing fixed targets build independently; Shared builds
   without App source; obsolete lists/gates are removed without lost assertions.
8. **S19: Whole-task acceptance.** Audit every ledger row, actual diff and
   retained owner; run full unit/integration/package/document checks and verify
   all eight artifacts. Exit: no residual shared mechanism, no peer dependency
   and successor proposal needs moves/bindings rather than copied implementation.
   Close T542 only after proof; do not automatically admit the App split.

## Verification, Delivery And Stop Conditions

Each code-changing S repairs direct callers, build/test ownership and manifests
as part of its delivery, runs the full repository-only unit suite on x64/x86,
and rebuilds affected fixed 0542 products. Focused selections belong only in the
active packet. Record added/removed/net tracked source/test code and review
actual changes against every ledger member consumed by that S.

Whole-T acceptance preserves all 58 external integration contexts, each once,
with unchanged inputs and acceptance predicates; complete units, independent
package/build/dependency/manifest checks and documentation governance must pass.
The last proven S11 baseline is 499 units per width and 58 passing contexts;
those are historical results, not fresh results for the corrective implementation.
Eight optimized stripped EXEs remain at `assets/nxvm/<profile>` with runtime
Debug present; changed artifacts have recorded source and SHA-256 identities.
MyNES is not an ibmpc consumer and is neither rebuilt nor modified.

Every commit has one target. Shared implementation/tests/build changes and NXVM
binding/evidence/artifact changes use separately scoped P commits with immediate
pushes under T542. Do not modify Lib/Common to make extraction convenient.
Stop for a required excluded change, a new hardware/timing contract or loss of
capability/coverage; revise the packet before proceeding. No silent downgrade,
new firmware shortcut, asset/INI rewrite or peer-App dependency is acceptable.
