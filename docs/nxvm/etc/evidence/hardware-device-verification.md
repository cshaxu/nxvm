# Hardware Device Verification Template

This is the required verification shape for an admitted M5 machine-device
task. It does not define a device contract or authorize implementation. Evidence
authority remains [Evidence Policy](../operations/policy/evidence-policy.md); generated
DOS probes follow [Probe Format](../operations/templates/probe-format.md); optional reference work
follows [Differential Debug Policy](../operations/policy/differential-debug-policy.md).

## Required Task Shape

Every device task has exactly these bounded subtasks.

### S1: Contract And Port Probe

Record the real state owner, reset state, mapped ports/memory, supported width,
read/write sequence, and defined unsupported behavior. State exactly which
IRQ line, DMA channel, memory window, and firmware/profile binding are involved
or explicitly state that each is not applicable. Add a project-owned focused
probe before implementation changes.

Use `tests/machine/<device>_port_smoke.c` and the CMake target
`core-machine-<device>-port-smoke` for a core-owned device. A product/profile
binding uses `tests/machine/vm_<device>_port_smoke.c` and
`vm-<device>-port-smoke`. The success marker is
`M5:T<task>:S1:<DEVICE>:PORT:OK`, where `<DEVICE>` is uppercase ASCII.
Existing target names remain valid historical evidence; this convention applies
to newly admitted probes.

The probe asserts, as applicable:

- reset and configuration state;
- exact port or memory transaction sequence and resulting registers/status;
- IRQ assertion/deassertion and acknowledgement path;
- DMA request, channel, page/address/count, and completion path; and
- one negative or boundary condition without host or firmware shortcuts.

S1 exits only with an evidence record, a passing focused probe, and an approved
S2 scope. It does not change guest behavior.

### S2: Owner-Local Implementation

Implement only the S1 contract at the declared CPU, bus, device, firmware, or
profile owner. Preserve one state owner and one execution path. Platform code
does not mutate guest state; firmware configures/uses the device but does not
replace its guest-visible behavior. Extend the S1 probe for every implemented
branch and add any narrow integration probe needed to prove the declared
profile binding.

S2 exits with focused probes and the current static ownership/dependency gates
passing. Stop for a second state copy, a host shortcut, a raw profile mutation
of core internals, an unbounded trace, or a retained NXVM behavior regression.

### S3: DOS Or System-Image Regression

Add one bounded guest-visible regression that needs the device: a generated DOS
probe, owner-provided system-image checkpoint, or a redistributable fixture.
Name a new product regression `vm-<device>-dos-regression-smoke` with source
`tests/machine/vm_<device>_dos_regression_smoke.c`; use an explicit documented
exception when the device has no DOS-visible behavior. Its marker is
`M5:T<task>:S3:<DEVICE>:DOS:OK`.

The regression states its media/fixture identity, start checkpoint, input,
timeout/instruction budget, expected output or state checkpoint, and cleanup.
It must exercise the same state owner as S1, not a firmware, BDA, or host
shortcut.

## Retained Regression Matrix

Each task record marks every row `run`, `not applicable`, or `covered by` with
the exact existing test. No row may be silently omitted.

| Surface | Required when | Minimum evidence |
| --- | --- | --- |
| Focused device contract | Always | S1 port/memory probe |
| Owner/dependency shape | Always | current static gates |
| Full-PC boot | device can affect boot, IRQ, DMA, storage, timer, input, or video | FDD/HDD checkpoint as applicable |
| DOS prompt | device can affect boot or interactive DOS | current FDD prompt smoke |
| NXVM Console | Always for a runnable VM change | Console lifecycle smoke |
| Debugger/pause | execution, interrupt, timing, or input can change | debug pause and unified-debug smokes |
| Session isolation | session-owned device/provider state changes | two-session isolation smoke |
| Display/input | video or keyboard/mouse path changes | corresponding keyboard/VADP/DOS smoke |

`run-current-smokes` is the normal matrix execution vehicle. A task may add a
focused command but may not call a build-only target runtime evidence.

## Optional Bochs Differential

Bochs is optional, local-only diagnostic evidence after a project-owned S1
probe exists. It is never a runtime dependency, an implementation source, or a
substitute for S1/S3 acceptance.

The task record must declare a checkpoint schema with: fixture/config identity,
deterministic input seed, checkpoint number, linear PC, selected registers and
FLAGS mask, selected memory ranges, port transactions, IRQ/DMA events, and
allowed reference-specific masks. It also fixes these limits unless an
owner-approved record gives a smaller or justified different bound:

| Limit | Default maximum |
| --- | --- |
| Instructions | 100,000 per run |
| Wall time | 30 seconds |
| No progress | 10,000 instructions or 5 seconds without checkpoint/event advance |
| Raw trace | 8 MiB |

The runner owns a unique ignored output directory and every child process. On
success, divergence, timeout, no-progress, or trace-size limit it terminates
the process tree, joins it, closes trace handles, records only compact
checkpoint/verdict evidence, and deletes raw traces by default. Retain a raw
trace only through an explicit owner-approved exception; it is never committed
or copied to `build/output/`.

## Completion Record

The final task record names the device owner, S1/S2/S3 tests and markers,
matrix verdicts, evidence level, optional reference identity and budgets,
remaining unsupported behavior, applicable local rule items, and the task
artifact only when the runnable product changed. This template itself is a
documentation policy and produces no task artifact.
