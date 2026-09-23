# T474 S3 Profile Contract To Plan

`M5:T474:S3:PROFILE-CONTRACT-TO-PLAN:OK`

## One Construction Flow

Default PC/AT and IBM 5170 select CPU, instruction, transaction, clock,
time-axis, controller-rule and KBC values from their immutable descriptors.
`vm_profile_default_pc_at_cpu_contract_select` copies that complete selection
once; `vm_session_materialize_profile_core_config` copies it once into
`core_machine_config`; `core_machine_plan_create` retains the existing Core
validation and copied publication route. The selected default and 5170
transaction contracts are explicit zero contracts, so this changes neither
their retained L2/default behavior nor their pacing qualification.

Model-40 retains its existing nonzero transaction contract byte-for-byte, but
its profile now constructs the `core_machine_config`. Session composition only
marks its distinct private composition and requests that profile construction;
it no longer owns CPU, bus, transaction, retirement, clock or auxiliary-device
constants. No public API, callback, setter, Core profile branch or alternate
plan was added.

## Complete Sweep

The production VM sweep finds exactly two configuration producers:

| Producer | Consumer | Disposition |
| --- | --- | --- |
| Default PC/AT / IBM 5170 descriptor -> copied CPU contract | Session -> one `core_machine_plan` | Retained; transaction field now travels with the rest of the immutable selection. |
| Model-40 private profile initializer | Model-40 session -> one `core_machine_plan` | Retained; session-local duplicate producer removed. |

`core_machine_plan.c` remains the sole validation owner for transaction
contracts before Core publication. `machine_scheduler.c` remains the sole
elapsed-time writer. The profile values remain construction inputs only; they
cannot mutate guest time after plan creation.

## Proof And Simplicity

- Built `vm-default-pc-at-profile-smoke`,
  `vm-model40-private-composition-s7-smoke`,
  `vm-session-speed-policy-smoke`, and `core-machine-plan-smoke`.
- Focused CTest passed 3/3: default-profile selection, Model-40 private
  composition, and Standard/Turbo policy.
- Direct `core-machine-plan-smoke.exe` passed its plan declaration,
  validation, copy, transaction-contract and controller-rule checks.
- `rg` finds no session-local transaction assignment: session has one copied
  default/5170 materializer and one Model-40 profile initializer call.
- Tracked source/test delta is +50/-35, net +15 lines (documentation
  excluded). The net is the missing copied field, its profile-owned Model-40
  construction boundary, and one contract-copy assertion; the removed 35-line
  session producer is not retained.
