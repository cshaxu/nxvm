# T539 S9: XT PPI And Keyboard Boundary

Baseline 625ea7054. The two finite-ledger rows cover four original files:
`xt_ppi_keyboard.c/h` and `xt_keyboard.c/h`. S9 is an extraction of their
qualified mechanisms, not full 8255 or keyboard-MCU qualification.

## Ownership

- Shared PPI owns Mode-0 direction/control, A/B/C output latches and BSR.
  Register selectors are local, never PC port numbers. Inputs and output
  direction/value observations are copied values. Unsupported modes retain
  their explicitly qualified baseline disposition, not invented handshakes.
- Shared XT keyboard owns its existing FIFO, one serial frame, clock-low
  reset qualification, BAT and deadlines. It receives line levels, configured
  service durations and a completed-byte acceptance callback. It has no PPI,
  PIC, machine, host-input or board-clock pointer.
- NXVM owns address decode, DIP inputs, parity/NMI, speaker connections,
  keyboard receiving latch/IRQ1 and PB6/PB7 wiring. The keyboard's transmission
  FIFO and the board's receiving latch are different hardware responsibilities,
  not duplicate state. Board time conversion stays with board composition.

Objects are constructed while stopped; routes publish only with live owners.
Failure unwinds routes and allocations. Reset retains frozen connections,
clears owned state and withdraws outputs before teardown. Synchronous signal
callbacks may deliver bounded input, not recursively run/destroy an owner.
No peer-private state or test-only snapshots cross the shared boundary.

## Characterization Before Relocation

The original PPI test covers DIP selection, speaker, BSR, repeated PA reads,
PB7 acknowledgement, IRQ1, parity/NMI gating, AT isolation, reset threshold,
BAT-to-serial order and FIFO overflow. Keep each case under its actual owner;
replace keyboard-private assertions with completed bytes/deadlines. Add
standalone direction/register and endpoint pressure/reset coverage.

Review found a specific untested failure path: serial completion decrements
the bit count to zero before asking the PPI to accept the byte. Refusal leaves
the frame active; the next advance decrements zero. A new characterization
must prove or refute the resulting wraparound before implementation. Reconcile
ordinary scans and BAT, occupied receive latch, inhibited lines, reset and
release together. Completed-but-unaccepted data must not fabricate more serial
edges, duplicate delivery or a busy deadline. This is a publication-boundary
repair, not a new physical timing claim.

The baseline negative control now reproduces the wraparound: leave a first
byte in the receiving latch, complete another 260-service-unit frame, then
advance 25 units. The original implementation fails the zero-remaining-bits
invariant. The local repair marks completed transmission as having no further
clock deadline and retries publication on receiver/line release. The expanded
cases cover scan versus BAT and release versus reset, with clock inhibit,
no premature overwrite, no duplicate publication and retained original cases.
They pass on both x64 and x86. This is intermediate proof only: extraction,
full receiver verification and artifact delivery remain required by S9.

Two other boundary checks became concrete regressions during extraction:
the eight port-registration allocation points rolled back their routes but
returned the cleared registration status; the adapter now saves the failure
before rollback and destroys the newly allocated PPI. Each point is tested
for failure, retained unrelated route, complete removal and successful retry.
The source sweep found no remaining identical rollback-then-read-status form.
The keyboard also reported a serial deadline while clock hold made advance
leave that deadline untouched. Its standalone negative control fails before
the fix; the query now reports no timed serial event until release, preserving
the remaining interval. BAT retains its independent countdown while held.

The shared components are `devices/ppi8255` and `devices/xtkeyboard`, each
Types-only with no direct peer dependency. Board conversion supplies four
durations using quotient/remainder ceiling arithmetic; this preserves normal
rates and avoids the old overflow-prone multiplication/addition. The PPI keeps
the original qualified mode-set latch retention and inactive Mode-1/2 behavior
explicit in its interface, without calling that full hardware qualification.
Boot diagnostics consume copied PPI pins and the keyboard deadline instead
of private mode/BAT flags. No diagnostic-only snapshot was added.

The release sweep also tests direction changes without a subsequent PB write:
restoring Mode-0 output while the receiver is empty republishes existing line
levels through the same observer, allowing a refused frame to be accepted.
No second acknowledgement queue is introduced. BAT finishing while clock or
clear is held previously left a pending result with no deadline after release.
Both negative cases fail before repair. BAT and scan frames now enter through
the same serial-start function; line release starts the pending BAT frame and
publishes its existing first-edge deadline. No extra polling is needed.

Review source evidence in the existing
[keyboard list](../evidence/t496-s1-xt-keyboard-function-timing-list-1.md) and
[phase audit](../evidence/t511-s7-xt-keyboard-ppi-phase-consumer-audit.md).
The latter tested successful transfers, not refused completion; it cannot
serve as proof of that negative path. Range-selected BAT/serial durations
remain L2; exact documented relations retain their existing classification.
Any changed hardware interpretation needs its own evidence, not a renamed API.

## Closure

Map each original responsibility and case to its surviving chip or board
owner. Verify independent Types-only builds, all receiver suites, one boot per
profile/width and eight artifacts. Review allocation/registration rollback,
deadline ordering and actual removed paths. The active packet owns current
status; this document does not claim implementation or acceptance.
