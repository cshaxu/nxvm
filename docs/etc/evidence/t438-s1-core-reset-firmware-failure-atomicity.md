# T438 S1: Core Reset Firmware-Failure Atomicity

## Boundary

The Core machine is the sole owner of reset lifecycle and firmware-operation
failure state. A firmware reset callback runs through the existing private Core
firmware context; each existing Core firmware operation records its first
failure there while reset is active. The callback's own non-OK result still
wins when it returns one. No VM wrapper, parallel reset path, public ABI, or
generic rollback layer is introduced.

`core_machine_cold_reset` returns the resulting status through its existing
caller boundary. On a timeline or firmware failure it leaves the machine
`CORE_MACHINE_INITIALIZED`, so it cannot run and can be reset again after the
underlying fault is repaired. A successful reset still enters
`CORE_MACHINE_STOPPED`.

## Focused Proof

`core-machine-firmware-capability-smoke` first performs its existing successful
reset. It then makes a reset callback deliberately discard a failing firmware
port-write result. The smoke proves the reset returns `TYPE_STATUS_FAULT`, the
lifecycle is `CORE_MACHINE_INITIALIZED`, `core_machine_run` rejects that state,
and a restored port lets the next reset succeed. Output includes:

```
M5:T438:S1:FIRMWARE-FAILURE-PROPAGATION:OK
```

The MSYS2 UCRT64 build route was used with its own bin directories before the
WinLibs entries inherited by the Codex process. This is a process-local tool
selection, not a system configuration change. The focused smoke and the
documentation-governance target pass. The current aggregate gate reaches the
pre-existing T344 fixture-count assertion and stops there: it expects 71
direct constructors while the tracked tree contains 75. T438 changes no
fixture constructor and does not rebaseline that unrelated invariant.

## Similar-Issue Sweep

The tracked production reset callbacks are the default VM profile callback and
the Model-40 callback. The default path invokes helpers which intentionally
discard individual firmware-operation status; it is fixed once at the Core
operation boundary. Model-40 returns its reset status directly and is covered
by the same Core callback result boundary. The only VM reset caller goes
through `core_machine_reset`; there is no second production reset route.
`after_run` intentionally observes and handles probing failures itself, so its
Core invocation disables reset-failure capture rather than inventing a second
failure owner.

## Artifact And Size

The developer artifact target is `vm-0-5-0438`; its copied artifact is
`build/output/nxvm_0_5_0438.exe`, SHA-256
`C2A096C62DE932AC3D15A4107D53B001D981EA1A9FE6E3E81D40964309785920`.

The counted source/test change is 98 added and 34 removed lines, net +64:
the private Core state captures an otherwise discarded status, the existing
operation implementations funnel through it, and the focused regression proves
the lifecycle/retry contract. There is no retained duplicate implementation
path.
