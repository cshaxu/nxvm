# T459 S2: Guest-Clock Ownership Correction

## Defect

S1's Turbo branch supplied `source_ticks = 1` in VM composition and then called
`core_machine_advance_time()`.  Core preserved the resulting per-tick device
order, but VM still generated the guest-time input.  This was not a valid
host/guest synchronization implementation and contradicted the single
guest-clock owner boundary.

## Correction

`vm_session_virtual_time_on_waiting()` now obtains ticks only from its existing
configured source and never examines the selected speed.  The sole remaining
VM call to `core_machine_advance_time()` consumes that source result; no Turbo
or host-backoff branch supplies a tick.  The runner's unchanged `Sleep(1)`
when the waiting helper reports no advancement is explicitly documented as an
L2 host-load backoff.  It neither synchronizes wall clock to guest time nor
advances the Core.

Both Console choices remain visible and immutable while a session runs.  Until
Core exports composed, guest-observable controller deadlines under a verified
profile timebase, Turbo intentionally uses the same safe fallback: it does not
busy-spin, fast-forward, or claim an immediate performance change.

## Proof And Transfer

`vm-session-speed-policy-smoke` verifies both selections leave a source-less,
halted default session's Core elapsed tick unchanged and still reject mutation
while running.  Existing virtual-time-source and Console lifecycle regressions
cover the retained source path and selected-session route.  A production scan
of the VM session/product sources finds no `VM_SESSION_SPEED_TURBO` branch at
the remaining `core_machine_advance_time()` call.

The complete Standard pacing and Turbo deadline-fast-forward contract is
received by the [Core L3 integration and VM contract export proposal](../../proposals/m5-core-l3-integration-vm-contract-export.md): every selected
profile needs a verified physical timebase, and every relevant timed controller
must contribute a Core-owned, reset/cancellation-safe observable deadline.
QPC, `Sleep`, and recurring timeline callbacks remain ineligible as guest
clocks or deadlines.

## Verification

The focused speed, Console, DOS prompt and retained virtual-time-source
regressions pass. The serial Debug current gate passes 294/294. Documentation
governance passes. The rebuilt stripped Release artifact is
`build/output/nxvm_0_5_0459.exe`, SHA-256
`ED4E79BF67C5A1B0C7953601933853247B0CC475E7DB67D883D15F65802F0973`;
`objdump -h` finds zero `.debug` sections.
