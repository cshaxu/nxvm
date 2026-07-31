# M1 Baseline Experiment Record Template

This record is created at M1 start and frozen after the first successful raw
baseline observations. It is a required M1 deliverable, not an M0 claim.

## Identity

- ntvdm64 revision and NXVM source commit:
- compiler, CMake, Ninja versions and executable SHA-256 values; Windows
  edition/build; and architecture:
- fixture logical names, sizes, and SHA-256 values:

## FDD Scenario

Use the imported baseline Console commands, substituting only the local fixture
path outside the repository:

```text
device display console
device fdd insert <fdd.img>
set boot fdd
start
```

- device configuration and boot order:
- accepted checkpoint or stop reason:
- expected display/trace marker:
- instruction, wall-clock, and no-progress budgets:

## HDD Scenario

```text
device display console
device hdd connect <hdd.img>
set boot hdd
start
```

- device configuration and boot order:
- accepted checkpoint or stop reason:
- expected display/trace marker:
- instruction, wall-clock, and no-progress budgets:

## Guest Probe Classification

`stop.com` and `reset.com` are local payloads, not boot images. Before either
becomes an M1 workload, record its lawful guest-visible placement, launcher,
instruction semantics, expected stop/reset behavior, and cleanup. A file name
alone is not evidence of its behavior. A probe without this classification is
not an M1 exit condition.

## Result

- actual observation, trace, and divergence report location:
- operator decision: accepted, failed, or unavailable:
- follow-up regression target:
