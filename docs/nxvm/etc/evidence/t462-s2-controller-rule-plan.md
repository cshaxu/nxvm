# T462 S2: Controller Rule Plan

S2 adds one construction-only `core_machine_controller_timing_rules` value to
the existing opaque `core_machine_plan`.  It contains exactly four rule
qualifications: PIC visibility, DMA clock, DMA service phases and PIT clock.
It carries no controller pointer, callback, route owner, elapsed tick counter
or machine/profile name.

`core_machine_plan_set_controller_timing_rules` is a plan-builder operation,
not a live setter.  It derives the existing controller capability declaration:

| Capability | Accepted source-backed rule | Required existing value | Result otherwise |
| --- | --- | --- | --- |
| PIT | `SOURCE_RATIONAL_CLOCK` | Explicit nonzero `clock_plan.pit` ratio | Explicit L2 fallback |
| DMA | `SOURCE_RATIONAL_CLOCK` plus `SOURCE_DMA_SERVICE_PHASES` | Explicit nonzero `clock_plan.dma` ratio | Explicit L2 fallback |
| PIC | None in this batch | None | Explicit L2 fallback |

The plan validator rejects a source rule with the all-zero identity shorthand,
a DMA service rule without a source-qualified DMA clock, an unsupported PIC
rule, and any manually forged L3 declaration.  The validated plan is copied by
the existing `core_machine_create_from_plan` path before machine publication.
It does not add a clock domain: the scheduler still advances the existing DMA
and PIT clocks, and it does not reinterpret `dma_cycle_wait_quanta`, which
remains BUSRDY policy rather than the IBM five-clock term.

Focused proof built and directly ran these targets:

- `core-machine-plan-smoke`: copied L3 PIT/DMA declarations, retained PIC L2,
  all invalid combinations and the existing all-capability declaration checks.
- `vm-model-339-clock-contract-smoke`: retained the profile's sourced PIT
  ratio and reset replay.
- `vm-default-pc-at-profile-smoke`: retained the generic profile's L2 clock
  plan path.

All three pass.  The modified tracked source/test paths add 196 lines and
remove 4 lines: `machine_interface.h` (+17), `machine.h` (+1),
`machine_plan.c` (+93/-3), and `core_machine_plan_smoke.c` (+85/-1).
The positive change is one durable plan validation mechanism plus its rejection
proof; no legacy plan path is duplicated or retained.

`M5:T462:S2:CONTROLLER-RULE-PLAN:OK`

`M5:T462:S2:CONTROLLER-RULE-REJECTION:OK`
