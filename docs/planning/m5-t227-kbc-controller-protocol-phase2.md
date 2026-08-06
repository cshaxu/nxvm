# M5 T227: KBC Controller Protocol, Phase 2

## Scope

T227 extends the T226 byte path with a deliberately bounded 8042/keyboard
command contract. It owns no host input queue and adds no AUX mouse/IRQ12.

## Contract

- The admitted keyboard command subset is `ED` LED write, `EE` echo, `F0`
  scan-set query/select, `F2` identify, `F3` typematic write, `F4`/`F5`
  scanning enable/disable, `F6` defaults, `FE` resend, and `FF` reset.
  Each command's ACK/response order is port-probed. Invalid input returns
  `FE`; no unsupported command is silently accepted.
- The default profile has a set-1 physical mapper. Therefore `F0 00` queries
  set 1 and `F0 01` selects it; set 2/3 are explicitly rejected with `FE`.
  The controller command-byte translation bit is retained and observable
  through `20h`/`60h`, but it performs no invented conversion while the
  admitted physical set is already set 1.
- The KBC owns pending command parameters, active scan set, translation flag,
  LED state, typematic configuration, and elapsed-tick deadlines. Profile and
  platform never inspect or mutate them.
- Typematic initial/repeat and bounded command-response deadlines advance only
  through the core elapsed scheduler. Their frozen values originate in the VM
  profile and are copied into the one core-machine KBC instance at
  construction. The default PC/AT profile retains zero command-response delay
  until a calibrated hardware value exists, preserving ROM startup behavior;
  the owner-local probe exercises a nonzero delay. Host `Sleep` is a test
  watchdog only.
- Every command response, error, and delay has a focused owner-local port
  probe. Unimplemented commands respond with the documented RESEND/error value
  rather than impersonating wider compatibility.

## Evidence

### S4: Default Typematic Calibration Repair

The owner reported that `0.5.0225` is the last usable local build: every later
artifact can expand one physical key press into many guest characters. The
historical `0.5.0226` artifact is not valid T226 evidence: its recorded commit
contains documentation only, while the artifact was created before the T227
source commit that introduced the KBC scheduler path. Retain it only as a
reported regression specimen, not as a reproducible baseline.

`elapsed_ticks` is deliberately a deterministic completed-instruction clock,
not calibrated human time. Therefore the default PC/AT profile must configure
both typematic deadlines as zero, explicitly disabling core-generated repeat.
The KBC retains its command/state contract and supports auto-repeat only when a
future profile supplies two nonzero, calibrated deadlines. Repeated host
keydown messages remain ordinary host ingress and are not filtered or invented.

S4 changes only this default calibration boundary. It must not add a host-clock
shortcut, a second input queue, direct BDA mutation, or a second guest input
path. The focused owner probe must prove both that default-disabled typematic
cannot repeat after a large elapsed-tick advance and that an explicit nonzero
test configuration still repeats then stops on break. Retain DOS prompt,
`EDIT.COM`, Console/window input, current GCC/CTest gates, and artifact/SHA
evidence. The related future admission is a calibrated profile-clock contract,
not an unbounded timing project.

## Subtasks

- **S1:** complete. The contract admits only the documented set-1 subset;
  non-set-1 conversion and AUX remain non-goals.
- **S2:** complete. State lives exclusively in `core/machine/kbc`; the core
  scheduler advances typematic and optional command response deadlines while
  host ingress and ROM BIOS boundaries remain unchanged.
- **S3:** complete. `core-machine-kbc-controller-smoke` covers command bytes,
  set query/rejection, LED/typematic, scanning enable/disable, defaults,
  resend/reset, delayed responses, and a full typeahead FIFO with preserved
  command responses. `vm-dos-prompt-smoke`, `vm-dos-keyboard-smoke`, retained
  `EDIT.COM`, Console/debugger, and `current-gates-gcc` pass 59/59.

The original `0.5.0227` artifact is superseded by the T227 S4 fix below.

### S4 Completion

The default PC/AT profile supplies zero typematic deadlines. `core_machine`
preserves that explicit disabled state across construction and KBC reset; a
make byte starts core-generated repeat only when both frozen deadlines are
nonzero. Host key transitions, the mapper, FIFO, IRQ1, and ROM path are
unchanged. This removes the repeated-key flood without inventing host timing.

`core-machine-kbc-controller-smoke` now advances one million elapsed ticks
after a default-disabled make and proves that no second byte appears. It then
uses an explicit `3/2` deadline to prove repeat and break cancellation remain
available for a calibrated profile. `vm-default-pc-at-profile-smoke`, current
artifact-target verification, and GCC 16.1.0 `current-gates-gcc` passed with
71/71 CTest smokes, including DOS prompt, `EDIT.COM`, Console/window input,
and FDD/HDD boot coverage.

Task identity remains T227; because the historical artifact sequence already
reached `0.5.0236`, its repaired runnable artifact uses the next monotonic
revision: `build/output/nxvm_0_5_0237.exe`, SHA-256
`53D0FED1072840A8D0021B4EE0F4BB32414A25515E9CC4FBF47FAD315B40F743`.

### S4 Similar-Issue Sweep

Query: `rg -n 'kbc_typematic_(initial|repeat)_ticks|core_machine_kbc_set_typematic_timing' src tests docs CMakeLists.txt CMakePresets.json`.
The production hits are the immutable default-profile declaration, composition
copy into `core_machine_config`, core-machine construction, and the KBC owner;
all now preserve zero as the explicit disabled state. The focused probe is the
only intentional nonzero timing user and proves calibrated repeat separately.
No second input path, host timing source, or undeferred production hit remains.
