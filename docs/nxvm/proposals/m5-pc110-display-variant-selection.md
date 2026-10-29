# MyPC110: one executable per host architecture and selectable display variant

## Owner request and priority

First queued M5 T candidate after the currently active T580. Keep MyPC110 as
one product: publish only `mypc110_x86.exe` and `mypc110_x64.exe`, each with
both selected firmware variants embedded. Default to DSTN. The monitor command
`vdisplay=dstn` or `vdisplay=tft` selects the corresponding PC110 display and
firmware variant only while the machine is stopped. Reject the request in all
other states; never change firmware underneath a running or paused machine.
The next start must use the selected variant.

## Design and scope

Audit the actual product, Machine and Session contracts before deciding the
minimal safe reconfiguration path. App-local product code owns the command
and selection; generic monitor, Lib, Emulator and unrelated Apps must not
learn PC110 model names. The selected firmware and display configuration must
come from the same variant. Reuse the existing lifecycle and single machine
owner; do not introduce a second execution path or fake a reset by mutating
live firmware. If the existing stopped-machine contract cannot safely switch
the immutable firmware, design an explicit, failure-safe replacement at the
existing ownership boundary before changing shared interfaces.

Both approved external BYOB firmware inputs, including the existing
derived-DSTN provenance and any required font assets, remain outside the
repository as raw files. Each EXE embeds the selected validated payloads;
distinguish the derived DSTN image from a proven factory-original dump.
Preserve user INI, disks, snapshots and the two existing variant CMOS files;
audit and document which configuration is used after a switch, without
silently overwriting or migrating either. The default at a fresh launch is
DSTN. The owner did not request an INI setting or persistence of the command
selection across application launches.

Replace the existing four PC110 artifacts only after the two new artifacts
are built and verified; remove the superseded TFT-named EXEs from the asset
directory and Git as part of delivery. Retain the existing x86/x64 names and
architecture checks. Keep each product scope and commit target consistent with
repository governance.

## Verification and exit criteria

- Both EXEs contain the validated DSTN and TFT firmware and boot DSTN by
  default; verify PE widths and deployed identity.
- In stopped state, switch to TFT and start the matching firmware/display;
  stop, switch back to DSTN and start the matching firmware/display. Exercise
  repeated and invalid requests, lifecycle errors and failure ownership.
- Prove the command is rejected while running, paused and during transitions,
  with no partial variant or machine change.
- Cover the App command and binding, dual-variant CMOS ownership, existing
  configuration compatibility and the actual firmware/product path on x86 and
  x64. Distinguish automated diagnostics from desktop or user acceptance.
- Update source/test build graphs, documentation, release checks and assets;
  preserve shared component boundaries and manifest requirements. Run the
  affected builds, tests and required receiver regressions; review the actual
  diff, submit and push under the admitted T/S identifier.

This is a queued proposal, not an active implementation packet. T580 S88
remains active until separately accepted or otherwise directed by the owner.
