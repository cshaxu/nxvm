# T386 S3: Shared Optional Second-8254 Owner

`M5:T386:S3:SECOND-PIT-OWNER:OK`

`M5:T386:S3:SECOND-PIT-ISOLATION:OK`

## Delivered Boundary

`core_machine_pit_initialize_at()` is the sole additional-topology mechanism.
It reuses the existing 8254 state machine and only changes the four registered
I/O ports; `core_machine_pit_initialize()` remains the `40h-43h` compatibility
wrapper. `core_machine_config` now has an optional auxiliary-PIT declaration.
When selected, `core_machine` owns its separate PIT state, advances it on the
existing PIT clock domain, resets/finalizes it with the machine and binds no
output provider. A false/zero configuration leaves default-PC/AT and Model-339
composition unchanged.

The focused machine smoke configures `48h-4Bh`, proves both port ranges are
registered, programs independent counter-zero values, advances one existing
machine tick, reads the auxiliary latch, and verifies cold-reset separation.
It also proves the auxiliary output slot is null, so it cannot assert IRQ or
NMI by implication.

## Verification

- Fresh Git Bash / WinLibs GCC 16.1.0 build:
  `cmake -S . -B build/t386-s3-gitbash -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
- Focused build and smoke passed with both S3 markers.
- Full registered current gate passed 252/252 after explicitly building five
  pre-existing CTest targets omitted from `run-current-smokes` dependencies.
  The initial aggregate invocation exposed that build-target omission; the
  complete CTest invocation, not the aggregate's partial build, is the
  acceptance evidence.
- Documentation governance and diff checks pass at review.

## Sweep And Transfer

The sweep covers every `core_machine_pit_initialize`, advance, reset and
finalize caller plus the port-registration lifecycle. The existing primary PIT
retains IRQ0; the new auxiliary PIT has no consumer. `48h-4Bh` is available to
a future private Model-40 composition receiver, but no profile/YAML selection
or firmware publication was added.

Failsafe output routing, port `61h`, D4 NMI/reset/A20 semantics and all
frequency/availability/service-placement questions transfer unchanged to T386
S4 and later DeskPro board timing. This is not a DeskPro boot, device-complete
or L3 claim.