# T474 S5 Standard And Turbo Lifecycle

`M5:T474:S5:STANDARD-TURBO-LIFECYCLE:OK`

## Two Modes, One Guest Timeline

Standard reads only the copied Core time observation. When its selected macro
rate is available, it compares already-completed Core ticks with the monotonic
host budget and waits only for a positive Core lead. It never produces a tick.
Turbo takes the same execution and HLT/deadline paths but skips that host wait.
For an unqualified profile, neither mode manufactures a pacing rate or a fixed
HLT delay.

On HLT, `vm_session_waiting_advance` reads the same copied observation in both
modes and invokes `core_machine_advance_to_next_deadline` only after the
Standard wait decision. Core selects the deadline and is the only writer of
elapsed guest time. Static production sweep finds no VM caller of
`core_machine_advance_time` or `core_machine_publish_elapsed_ticks`.

## Lifecycle Boundaries

- Session reset clears the one pacing origin before restarting Core, so an old
  host/Core comparison cannot cross reset.
- Speed selection is rejected while the session is running; a later start/reset
  establishes a fresh origin if the selected profile has a macro rate.
- HLT requires active run control; stop/cancel prevents deadline advance.
- Pause/stop and runtime debugger controls remain host-control boundaries.
  They do not expose or mutate a guest-time counter; the Core public advance
  operation still requires its stopped/paused lifecycle state.

## Proof And Simplicity

- Direct `vm-model-339-clock-contract-smoke` proves one source-qualified PIT
  deadline advances to the same Core tick in Standard and Turbo. Standard sets
  its pacing origin; Turbo does not.
- Focused CTest passed 5/5: Core time, session speed policy, session
  initialization/reset atomicity, Console lifecycle and debugger recording
  lifecycle.
- The speed-policy test proves unqualified Standard and Turbo leave Core time
  unchanged during HLT and reject a speed change while running.
- The initialization fixture now copies transaction, time-axis and controller
  timing fields alongside the existing immutable profile fields. This is
  test-only failure setup, not a product route; it prevents the fixture from
  drifting from actual profile construction.
- Source/test delta is +13 lines in the one affected test, plus this evidence;
  no runtime pacing or Core code changed. The retained production path remains
  Core observation -> VM wait decision -> Core deadline advance.
