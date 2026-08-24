# M5 T459: VM Session Speed Policy

T459 owns the explicit NXVM session speed selection requested by the owner.
It keeps guest-machine timing in Core and makes the host waiting policy a
single VM-session concern.

## Closure

S1 implementation `cf44f9e5` adds the one session-owned `standard`/`turbo`
state, the retained Console selected-session route, and focused session plus
product proof.  Standard retains its existing profile behavior.  Turbo has no
arbitrary tick batch: after a Core interrupt wait it calls the existing
Core-time operation for exactly one tick and does not sleep.  No Core speed
name, host clock, profile/YAML decision, second scheduler or mode-specific
runner is introduced.

The full serial Debug current gate passes 294/294.  Documentation governance
passes, and `nxvm_0_5_0459.exe` is a stripped Release artifact, SHA-256
`D859A76FA522633E5CCB990BB44A1BBED22A24B97876EEE1826717B3DDA2B312`.
The retained [proposal](M5-T459-vm-session-speed-policy-proposal.md) and
[S1 evidence](../etc/evidence/t459-s1-vm-session-speed-policy.md) record the
caller sweep, measured non-result for a larger runner quantum, and the
separate source-backed profile-timebase/controller-deadline prerequisite for
full host-paced synchronization.

## Corrective S2 And Final Reclosure

The owner correctly rejected S1's VM-generated one-tick Turbo path: preserving
the Core's per-tick order was insufficient because VM still supplied the guest
clock input. S2 is accepted at `17e53ebc`. It removes that branch completely;
the only remaining VM Core-time call consumes the pre-existing configured
virtual-time source, never a speed choice or host backoff result. The runner's
fixed `Sleep(1)` is now explicitly L2 HLT host-load backoff, not a
synchronization algorithm.

`standard` and `turbo` remain visible stopped-session product selections, but
Turbo does not busy-spin, manufacture guest time or claim deadline
fast-forward. Correct Standard pacing and Turbo advancement remain explicitly
transferred to the source-backed profile-timebase and Core-composed observable
controller-deadline contract. The [corrective proposal](M5-T459-vm-session-speed-policy-corrective-proposal.md)
and [S2 evidence](../etc/evidence/t459-s2-guest-clock-ownership-correction.md)
retain that disposition. The serial Debug current gate passes 294/294, and the
rebuilt stripped `nxvm_0_5_0459.exe` is SHA-256
`ED4E79BF67C5A1B0C7953601933853247B0CC475E7DB67D883D15F65802F0973`.
