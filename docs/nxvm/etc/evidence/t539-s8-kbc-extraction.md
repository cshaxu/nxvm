# T539 S8: AT Keyboard Chain Extraction

Baseline 23b732201. Shared and NXVM are separate implementation P targets.
The owner's automatic-S authorization admits this bounded migration under
the [boundary review](../architecture/t539-s8-kbc-boundary.md), not a new
hardware feature, full 8042 MCU or timing-grade claim.

## Ownership And Actual Source Review

- `x86/devices/kbc8042` owns controller registers, pending controller writes,
  output origins, serial/reply buffering, translation and delivery deadlines.
- `x86/devices/keyboard` owns command parameters, scan set, LEDs, BAT/startup,
  accepted-output resend history and typematic. `ps2mouse` owns the existing
  AUX command/parameter and three-byte packet state.
- The old NXVM `kbc.c/h` implementation is replaced by endpoint construction,
  port 60h/64h handlers, IRQ1/12, A20/reset and timing attachment. There is no
  second command decoder, serial queue or endpoint state in that adapter.
- Controller connections are copied; their context is borrowed until destroy.
  Keyboard reply and repeat sinks are scoped. Commands may synchronously return
  replies through the single controller transport; no general device framework
  or endpoint-private pointer crosses the boundary. Only the stateless scan
  codec is directly shared with keyboard; the negative gate rejects other
  keyboard API calls from the controller.
- FF preserves ACK-before-defaults/BAT; F5/F6 preserve defaults-before-ACK.
  F6 retains the existing scanning latch. Controller 60h/D1h/D4h replace a
  pending keyboard parameter; other controller commands retain it. FE retains
  the existing transport-accepted (possibly translated) resend history.
  These are qualified-model preservation decisions, not new hardware claims.
- Port data consumption acknowledges the current byte origin before promoting
  its successor; serial, delayed reply, BAT and repeat phases retain their order.
  Deadline queries do not advance state. Profile status-poll timing is unchanged.
- Reset preserves timing/attachment and clears owned transient state. Destroy
  releases IRQ before its board receiver dies. Port-publication failure rolls
  back before destroying all three allocated chip objects; machine construction
  propagates failure. Earlier port errors remain errors.
- Removed two write-only controller flags, duplicate board AUX-presence storage,
  unused response-capacity exposure and the unreachable AUX scaling flag. E9's
  scaling bit remains zero. The public packet receiver enforces absent/disabled
  AUX admission already enforced by the old production caller.

## Original Tests And Diagnostics

- Controller, AUX and serial board tests retain port/PIC/reset/A20, ACK/BAT,
  self-test flush, translation, delayed reply, typematic break, typeahead,
  FIFO pressure and finalization scenarios. Private-field assertions become
  actual bytes, OBF, IRQ-source outputs or read-only endpoint deadlines.
- The 96-row pending-parameter characterization moved to the standalone
  controller test. Its direct composition also checks startup, reset pulses,
  absent AUX, IRQ origin and atomic saturation (21 packets accepted, the next
  rejected, exactly 63 bytes drained).
- Independent keyboard tests cover every command byte, every LED/scan-set/
  typematic parameter, callback ordering, startup/reset and repeat cancellation.
  Mouse tests cover every command/parameter and 648 report combinations,
  including failed admission without button-state commit.
- Four port-allocation rollback positions, collision/retry and prior-error
  handling check that controller, keyboard and mouse ownership is fully released.
- Default, 5170 and Model-40 composition tests observe AUX/output-port behavior;
  Model-339 timing checks observe its 4,000,000/800,000 service-unit repeat
  boundaries. A small NXVM-only fixture reads replies through real ports and
  configured deadlines; it is not a replacement controller model.
- Three boot diagnostics no longer inspect private FIFO/command/counter fields.
  Existing observed I/O remains; A20, board IRQ, copied BAT and deadline values
  replace inappropriate private dumps. No extra status reads perturb a live
  probe's response-poll ordering. Boot success criteria are unchanged.

## Verification

Final full units pass 345/345 per width (205.61s x64, 43.46s x86).
Default external integration passes 20/20 per width. Independent tools-off
chip suites pass 16/16 per width, including manifests and negative probes.
The specialized static aggregate, all six manifests and whitespace checks pass.
Documentation governance and local Markdown links pass.
All three diagnostic targets compile; they are not claimed as extra boot runs.

The intermediate build after removing duplicate AUX storage exposed seven
remaining test references. They were migrated to public admission/output
observations before the successful final suites. A concurrent static build in
the already-running x64 tree failed Ninja recompaction; after the active build
finished, the static aggregate passed. Neither attempt is counted as success.
No unrelated process or sibling checkout was changed.

Remaining-profile boots are each run once, not a repeated reliability matrix.
XT passes in 21.91s/25.35s and 5170 in 39.01s/49.70s (x64/x86).
Model 40 passes in 80.21s/98.51s. Both reusable build trees are restored to
default configuration. These single-pass results do not claim indefinite
absence of intermittent faults or a hardware timing upgrade.

Shared P1 eb1e2e208 is pushed to origin/master. NXVM P2 contains the receiving
adapter, tests, evidence and artifacts. Coordinator actual-change acceptance
follows that delivery; this record does not close the entire T539 inventory.

Production C/H totals +1707/-1221 (net +486); test C/H totals +999/-235
(net +764), including new files. Growth is explicit opaque contracts, lifetime
and board connection code plus independent endpoint matrices, not a second
behavior path. Counts exclude CMake, documentation, manifests and binaries.

## Artifact Identity

Eight optimized Release 0.5.0539 receivers have the expected PE 8664/014C
machine IDs and no compiler-debug sections. Runtime Debug remains supported.
Owner INIs have no content diff; MyNES and Lib/Common have no input change.

| File | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 1612731773BB56B360654D39EB290B6051D0D2E587DCA51CE7192CBC3734C29F |
| nxvm_model40_0_5_0539_x86.exe | 6EBB3512FA2300E31451EAE29E3F5E27203578028B4C90A8D649F6FB2EE16309 |
| nxvm_default_0_5_0539_x64.exe | 7F1A35F371068532879EB9449611C72270EFC72A51E8E484DA97400334204005 |
| nxvm_default_0_5_0539_x86.exe | 045C9B2B71F20775F436BF690BC84DA01FB36EDFEA7FA6CA8631FD3DFE2286B4 |
| nxvm_xt_0_5_0539_x64.exe | 7BDD1E7A421101750E38AE7F45FA594B96D218D39D4F0C601BA9A5FE03185B26 |
| nxvm_xt_0_5_0539_x86.exe | C74FE2C0EB1EFF601F54AAA5934A52F416678FE3084CEE8AD562C05F86020809 |
| nxvm_at_0_5_0539_x64.exe | 25F66CD5F6130C4418E3CE80385B5C3508D6E021A8EF745047189ABC98A8B02C |
| nxvm_at_0_5_0539_x86.exe | 6C83A82469E226F912B3BB4183E8507645F0345448A4DF20885B55C5EA080C1D |
