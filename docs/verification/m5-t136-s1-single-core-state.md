# M5 T134/T136 S1: Single Core State

Core contract smoke now uses `tests/support/core_machine_executor_fixture.h`
to prepare a real custom executor. Production `core_machine` no longer has a
test profile or test CPU/RAM/port state; its port provider table is a formal
`port_providers` registry used by the executor dispatch path.

Focused core, firmware, product-debug, and VDM two-session/presentation smoke
tests passed. `nxvm-vm-dos-prompt-smoke D:\\home\\repos.hobby\\fdd.img` passed.
The artifact is `build/output/nxvm_0_5_0136.exe`
(`283373A56B33085EFA9255F3113A56D635849AE5E0C9B34622107AC7F21A3736`).
