# M5 Dependency Governance

## Decision

M5 closes only when the source and CMake target graphs follow the dependency
model in `docs/architecture/module-layout.md`. The `vm/` and `vdm/` roots are
the only composition roots. `machine`, `platform`, `product`, and `profile`
are peer providers and do not include one another within the same product form.

## Current Audit

The core runtime session/profile selection was removed by T13 S8. Remaining
debt is structural: `core/product/utils.c` reaches `vm/platform`; core
instruction/device code reaches VM device or default-profile firmware; and the
retained VM has direct `vm/machine <-> vm/platform` calls. The VDM minimal
shell has `vdm/platform -> vdm/machine` presentation routing. The historical
`nxvm-baseline-runtime` CMake aggregate also hides mixed ownership.

## Bounded Follow-Up

| Task | Outcome | Stop condition |
| --- | --- | --- |
| T14 S1 | Record the DAG, current dependency inventory, and M5 closure rule. | No runtime change. |
| T14 S2 | Define a target-by-target migration map and automated forbidden-include/target checks. | Stop if a retained Console/debugger path would need behavior change. |
| T14 S3 | Remove core-to-VM/default-profile dependencies through core contracts bound by VM root composition. | Stop before callback ordering or boot behavior changes. |
| T14 S4 | Move VM machine/platform/product/profile crossings into VM root composition adapters and dissolve mixed CMake aggregates. | Stop before input, display, media, or Console behavior changes. |
| T14 S5 | Apply the same provider/root-composition split to the VDM minimal shell without creating a user-facing VDM executable. | Stop before DOS-minimal behavior changes. |
| T14 S6 | Introduce `src/type.*` and `src/version.*`, then perform the final low-priority type/name migration required by the approved contracts. | Stop if a retained user-visible identity or C type compatibility changes. |
| T14 S7 | Verify the full source/target DAG, retained regressions, type/version foundation, and task artifact; decide whether M5 can close. | Stop if any forbidden edge remains. |

No task permits a new interface wrapper merely to satisfy a directory rule. A
contract is introduced only when it represents a real data, command, or host
capability boundary. Root composition binds concrete callbacks and owns
lifetime; core contracts never select a product.
