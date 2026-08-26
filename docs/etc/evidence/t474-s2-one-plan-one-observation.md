# T474 S2 One Plan, One Observation

`M5:T474:S2:ONE-PLAN-ONE-OBSERVATION:OK`

## Retained Boundary

`core_machine_plan` copies `core_machine_config`, controller timing rules and
the neutral `core_machine_time_axis` before `core_machine_create_from_plan`.
Core stores the resulting time axis and publishes exactly one copied
`core_machine_time_observation`. VM session construction creates one plan,
passes it once to Core, and `waiting.c` consumes only that copied observation.

The plan's media/display/provider pointers are dynamic resource bindings, not
time, mode or frame owners. They are therefore retained outside timing
provenance. No profile name, profile callback or host counter enters Core.

## Sweep And Proof

- `machine_plan.c` validates the complete 30-capability declaration set and
  controller rule/clock-ratio consistency before publication.
- `machine_scheduler.c` derives pacing qualification and eligible deadline only
  from Core-owned copied state.
- `waiting.c` reads the copied observation and never calls the generic Core
  advance API; its HLT path requests only Core's deadline operation.
- `ctest --test-dir build/mingw-gcc-x64 -R
  "(core-machine-time-smoke|vm-session-speed-policy-smoke)" --output-on-failure`
  passed 2/2.

No new wrapper, state copy, mutable setter or configuration route is needed.
S3 receives CPU/transaction timing provenance; S4 receives ineligible
controller deadline owners.
