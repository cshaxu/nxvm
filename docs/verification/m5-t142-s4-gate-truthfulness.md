# M5 T142 S4: Gate Truthfulness Repair

## Scope

Repair two stale gate metadata entries without changing NXVM runtime code,
Console behavior, debugger behavior, or task artifact contents.

## Changes

- `tools/session-readiness-state.tsv` now inventories only the two mutable
  file-static states detected by `VerifySessionReadiness.ps1`: the explicit
  process-exclusive Console leases in `win32con.c` and `linuxcon.c`.
  Former `debug.c`, `aasm32.c`, `dasm32.c`, `console.c`, and `wait.c` entries
  were stale after their TLS/state migrations and would incorrectly fail the
  inventory's stale-entry check.
- `nxvm-current-gcc` now builds `nxvm-0-5-0142`, matching the current verified
  M5 artifact target rather than historical T118.

## Verification

```text
M5:SESSION-READINESS:mutable:src/vm/platform/linux/linuxcon.c:212
M5:SESSION-READINESS:mutable:src/vm/platform/win32/win32con.c:20
M5:SESSION-READINESS:OK
```

Windows GCC preset verification passed:

```text
cmake --build --preset nxvm-current-gcc
cmake --build --preset nxvm-current-gates-gcc
```

The gate suite included successful Console lifecycle, executor closure,
facade-ownership, live-machine-authority, dependency-DAG, session-readiness,
and C-facade checks. The current artifact starts with banner `0.5.0142`.
