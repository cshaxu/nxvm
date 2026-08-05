# M5 T225: Time And PIT Closure

## Scope

T225 closes the remaining truth and evidence gaps in the elapsed-time/PIT
route before deferred KBC work begins at T226. It does not start KBC phase 1
or phase 2, CMOS/RTC, graphics, VDM, a host clock path, or a second scheduler.

Reference baseline: T222 PIT waveform/GATE, T223 keyboard compatibility repair,
the default PC/AT profile, and `0.5.0224` developer artifact.

## Subtasks

### S1: Actual Sequence And Contract

Record the real task sequence without rewriting pushed history. T223 is the
completed urgent keyboard compatibility repair; the originally scheduled KBC
work is deferred to T226. Define the remaining time contract:

```text
core elapsed ticks
  -> core-owned frozen PIT input-clock divider
  -> core PIT OUT0
  -> core PIC IRQ0 source and ordinary CPU interrupt delivery
  -> default-profile ROM INT 08h
  -> BDA 0040:006C daily counter and rollover byte
  -> default-profile ROM INT 1Ah/AH=00h observation
```

`elapsed_ticks` remains a coarse completed-instruction clock. It is not a PIT
input clock. A default-profile frozen integer divider converts elapsed ticks to
PIT input clocks in the core scheduler. Platform host time, renderer cadence,
and debugger inspection are excluded. The PIC is the sole IRQ delivery owner;
the ROM, not core or platform code, owns BDA mutation.

### S2: Core Divider And Profile Binding

Add the frozen default-profile PIT input divider to the core machine
configuration. `core_machine` owns the divider remainder and resets it with
the machine. The scheduler advances PIT only by accumulated input clocks,
while DMA/VADP keep their existing elapsed-tick contract. No provider or VM
outer loop may advance PIT.

### S3: ROM INT 08h And INT 1Ah

Install ordinary default-ROM handlers for IRQ0 vector `08h` and `INT 1Ah/AH=00h`.
The IRQ0 handler increments the BDA daily counter, sets the rollover byte on
the documented daily limit, sends the master PIC EOI through port `20h`, and
returns with `IRET`. The software service reads the BDA deterministically,
returns `CX:DX`, consumes the rollover byte into `AL`, and uses only the
ordinary IVT path.

### S4: Focused Core And Firmware Probes

Prove divider determinism, PIT period, IRQ0 source/CPU delivery, BDA tick
increment, `INT 1Ah` return values, rollover consumption, and reset/pause/step
behavior. Probes must run through ports, core execution, and ROM instructions;
they must not write the BDA directly as a substitute for IRQ0.

### S5: DOS/System-Image Time Regression

Add a bounded real FDD DOS timing fixture or equivalent system-image path that
observes guest time after boot. Retain FDD/HDD boot, prompt, `EDIT.COM`,
keyboard, Console, and debugger behavior. A timeout is a test watchdog only,
never a guest clock input.

### S6: Closure And Artifact

Run focused probes plus `current-gates-gcc`, record source/task mapping and
the final developer artifact SHA, and reconcile status/TODO wording so neither
T222 nor T223 claims unimplemented behavior. T225 owns one new monotonic
artifact revision.

## Rules And Stop Conditions

Applicable rules: one core scheduler and machine state owner; core contains no
VM/profile dependency; profile supplies frozen parameters and ROM only;
platform never mutates guest state; one ordinary CPU interrupt/IVT path; no
host-clock guest shortcut; no Console/debugger/start/boot behavior change.

Stop if implementation needs a second time state, a direct platform/profile
BDA write outside ROM execution, a firmware portal, a new executor/session, or
an unbounded wall-clock calibration exercise.

## Evidence

S1 records the true task sequence: T222 remains the completed PIT task, T223
is the emergency keyboard-compatibility repair, T225 owns this time/PIT
closure, and KBC phase 1 begins at T226.

S2 adds `pit_elapsed_ticks_per_input_tick` to the frozen core-machine config.
The default PC/AT profile supplies `4`; core owns the remainder, clears it on
cold reset, and alone converts elapsed ticks to PIT input clocks. DMA/VADP
retain their elapsed-tick inputs. `core-machine-pit-divider-smoke` proves that
eight completed one-tick instructions yield exactly two PIT input clocks.

S3 makes the existing vector `08h` firmware hook accurately named as the PIT
timer handler rather than CMOS/RTC IRQ8, corrects the BDA daily rollover to
`0018:00B0`, and exposes the timer handler and INT 1Ah service in the profile
firmware registry. `vm_machine_cmos_refresh()` no longer reads host time;
RTC register advancement and IRQ8 remain T229 work.

S4/S5 add `vm-timer-firmware-smoke`: it boots the real FDD image through the
normal runner, observes a nonzero BDA tick count at `0040:006C`, pauses at the
session boundary, then executes ordinary ROM `INT 1Ah/AH=00h` and confirms the
same `CX:DX` count. A separate rollover case seeds only the documented
one-tick-before-midnight BDA precondition, invokes the real ROM `INT 08h`, and
then proves that `INT 1Ah/AH=00h` returns and clears `AL=1`. It does not write
BDA memory to manufacture the IRQ result.

S6 completion evidence:

- `current-gates-gcc` passes after the T225 source/test integration.
- Developer artifact: `build/output/nxvm_0_5_0225.exe`.
- SHA-256: `BFDE2250D5BE475EF32C913E167822772875B147168160C3F597BDCA6F5EDBF6`.

**Status:** complete.
