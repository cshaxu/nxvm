# T459 S1: VM Session Speed Policy

## Owner And Boundary Sweep

`vm_session` is the sole mutable speed-policy owner.  Its public interface
accepts only `standard` and `turbo`; a change while the session runs is
rejected and resets the existing virtual-time source only when it succeeds.
The retained Console grammar reaches that owner only through the existing
selected-session adapter.  The production sweep covers every
`virtual_time_source`, `vm_session_virtual_time_on_waiting`, runner wait
branch, Console machine-provider operation and selected-session adapter
operation under `src/vm`; no second speed decision or profile-specific speed
route remains.

Core has no speed name, host-clock policy, Console grammar or profile
dependency.  Turbo calls the existing `core_machine_advance_time()` with one
tick only after the Core reports `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`.
That preserves the existing Core scheduler and device order; it does not skip
PIT, PIC, DMA, RTC, KBC, FDC, HDC or provider work.

## Product Behavior And Proof

`SPEED` reports the selected-session setting and `SPEED STANDARD|TURBO` changes
it while stopped.  The product lifecycle smoke sends `speed turbo` through the
real Console/provider/selected-session path and reads the selected VM session
through the existing manager operation.  The session smoke proves the default
is Standard, Turbo advances exactly one Core tick for both default PC/AT and
Model 339, and mutation while running is rejected.  Existing virtual-time
source and Model-339 virtual-time tests preserve Standard behavior.

The Release DOS-prompt smoke was replayed three times per mode:

| Mode | Seconds |
| --- | --- |
| Standard | 1.256, 1.001, 0.995 |
| Turbo | 1.001, 0.993, 0.997 |

The medians, 1.001 and 0.997 seconds, do not demonstrate a material DOS boot
speedup.  An experimental 8192-instruction Turbo runner quantum was therefore
removed: it had no repeatable benefit and would have created mode-specific
host-control latency.  Turbo's durable meaning is unrestricted interrupt-wait
progress, not an arbitrary CPU multiplier.

## Host/Guest Synchronization Transfer

This task does not claim L3 host/guest synchronization.  The recurring Core
timeline callbacks are per-tick maintenance, not guest-observable deadlines;
exporting them would be a false abstraction.  Also, project architecture does
not allow QPC or `Sleep` to manufacture guest time.  A complete paced mode
needs selected source-backed profile timebases and Core-owned deadlines from
every timed controller that could wake or visibly change the guest.  That
bounded admission condition is received by the [Core L3 integration and VM
contract export proposal](../../proposals/m5-core-l3-integration-vm-contract-export.md).
Until then Standard preserves its existing profile behavior and Turbo retains
the one-tick safe path.

## Verification And Artifact

- `vm-session-speed-policy-smoke`, `vm-product-console-lifecycle-smoke` and
  `vm-dos-prompt-smoke` pass.
- Existing `vm-session-virtual-time-s6-smoke` and
  `vm-model339-virtual-time-s17-smoke` pass.
- The Debug `current-gate` replay has 294 passed records and zero failures in
  `build/mingw-gcc-x64/Testing/Temporary/LastTest.log`.
- Documentation governance passes.
- The Release artifact is `build/output/nxvm_0_5_0459.exe`, SHA-256
  `D859A76FA522633E5CCB990BB44A1BBED22A24B97876EEE1826717B3DDA2B312`;
  `objdump -h` reports zero `.debug` sections.

## Minimalism Review

The implementation has one speed state, one session interface, one Console
adapter mapping and one existing virtual-time advance path.  It does not add a
Core facade, deadline API, profile/YAML setting, alternate scheduler or
per-mode runner.  The counted source/test surface is 220 added and 14 removed
lines, net +206, excluding documentation, CMake registration and generated
artifacts.  The positive delta is the requested Console/product boundary and
its end-to-end regression; the unproven runner-quantum branch was removed.
