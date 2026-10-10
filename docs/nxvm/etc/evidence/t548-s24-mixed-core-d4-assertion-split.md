# T548 S24 Mixed Core/D4 Assertion Split

## Decision

The Default-PC board directory contained three tests with two different
owners: generic Core construction/time/arbitration behavior and Model 40 D4
board behavior.  Source location was not accepted as evidence of ownership.
Each predicate was mapped to its narrowest owner before moving any test.

| Former mixed source | Predicate | Canonical receiver |
| --- | --- | --- |
| `machine_time_smoke.c` | Core time configuration and machine time behavior | `test/core/board-base/machine_time_smoke.c` |
| `machine_time_smoke.c` | D4 pending refresh deadline wins over an unrelated scheduled deadline | `test/app-mydeskpro386/unit/profiles/d4_refresh_deadline_smoke.c` |
| `machine_competition_smoke.c` | Core DMA/hold competition and no-wait behavior | `test/core/board-base/machine_competition_smoke.c` |
| `machine_competition_smoke.c` | D4 attachment configured state | Existing `core_machine_d4_platform_smoke.c` |
| `core_machine_port_assembly_smoke.c` | Generic Port-B assembly rollback | Existing `test/core/board-base/composition/port_assembly_core_smoke.c` |
| `core_machine_port_assembly_smoke.c` | D4 Port-B attachment rollback | `test/app-mydeskpro386/unit/profiles/d4_port_b_assembly_smoke.c` |

The old competition D4 setup performed no D4 behavioral assertion beyond its
already-covered configured-state check.  It was removed rather than retained
as a duplicate fixture.

## Verification

Fresh isolated Ninja graphs passed the four changed routes on both widths:

- x64: `unit.core-machine-time-smoke`,
  `unit.core-machine-competition-smoke`,
  `unit.model40-d4-refresh-deadline-smoke`, and
  `unit.model40-d4-port-assembly-smoke` — 4/4.
- x86: the same four routes — 4/4.
- `core.manifest` and `core.test-manifest` — pass on x64 and x86.
- `verify-fixture-shapes` — passes on x64 and x86, retaining its exact
  135-direct-constructor classification after its source inventory was moved.

This S changes tests, registration, manifests and evidence only.  It does not
claim desktop, external-media or full-suite qualification; those remain later
T548 receivers.
