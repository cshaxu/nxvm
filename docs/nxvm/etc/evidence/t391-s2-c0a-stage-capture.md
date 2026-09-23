# T391 S2: C0A Stage Capture

`M5:T391:S2:C0A-CAPTURE:OK`

The existing test-only Model-40 capture now has an explicit C0A diagnostic mode.
It ignores every observation before the C0 protected-to-real transition, then
aggregates only until the existing first post-C0 I/O checkpoint. It uses the
existing copied observation and adds no Core/VM interface, port provider, or
runtime behavior.

The canonical synthetic capture now executes both its existing C0 sequence and
the C0A isolation regression. The latter proves the stage excludes all earlier
records and retains exactly three classified aggregate records through an
immediate read of port 61h.

One owner-managed transient replay reaches the same finite C0A checkpoint with
three classified successful retirements, three aggregate forms, zero
source-unallocated observations, no fault and no C1 transition. The forms are
an `0F 01` form, `FF /4`, and an immediate input form. No asset identity,
path, digest, byte, PC, raw trace or provenance is retained.

The next receiver is the complete three-unit C0A form/context ledger. This
capture makes no timing, physical-retirement, board, firmware-completeness or
L3 claim.

## Coordinator Acceptance

The coordinator reviewed P1 240d6237 against the S2 packet. It changes only the test capture and documentation; no Core or VM runtime surface changes. Focused canonical smoke, bounded aggregate replay, documentation governance and diff hygiene pass. S2 is accepted.
