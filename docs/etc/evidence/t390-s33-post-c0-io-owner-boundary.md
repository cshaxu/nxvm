# T390 S33: Post-C0 I/O Owner Boundary

## Observation contract

The existing test-only Model-40 retirement capture now records the first
successful I/O retirement after C0.  It records only direction and, for an
immediate-port encoding, the port number.  A DX-port encoding is represented
as an I/O event with an unresolved port; no register snapshot, address, raw
instruction bytes, ROM/media identifier, trace, or guest state is retained.

The synthetic regression covers the post-C0 immediate-port write path and
proves that it reaches the S33 marker.  The helper remains a pre-publication
test observer: it installs no port provider and makes no Core, VM, profile, or
timing-contract change.

## Contained replay

One owner-managed external-asset replay used the existing two-million
retirement containment limit and a sixty-second wall-clock cap.  It stopped at
the first post-C0 I/O retirement with 18,258 classified successful retirements,
zero source-unallocated observations, 82 aggregate forms, protected mode and
C0 observed.  The event was an immediate-port read of `61h`.

Temporary raw process output was deleted after the aggregate result was read.
This is a finite checkpoint observation, not a timing value, board-clock,
physical-retirement, firmware-completeness, or L3 conclusion.

## Owner disposition

Port `61h` is already owned by the existing D4 platform mechanism in Core and
selected by the Model-40 composition.  S33 made no change to that owner and
found no functional defect in the observed read: the replay retired it
successfully and continued under the ordinary containment contract.  The
result therefore does not justify a new CPU repair or a second VM-side port
implementation.

## Similar-issue sweep

The capture uses the existing copied retirement observation only.  Immediate
and DX I/O encodings are both classified; the latter deliberately stops with
an unresolved port rather than widening the public CPU snapshot.  No product
port provider, profile callback, raw-output path, or Core/VM interface was
added.

## Transfer

The next T390 receiver must choose the finite semantic consequence of the
post-C0 `61h` read, or a subsequent named I/O event if that consequence cannot
be represented from copied aggregate state.  It must reuse the existing D4
owner, preserve the Core/VM boundary, and not infer board timing from this
functional checkpoint.

## Coordinator acceptance

The coordinator reviewed pushed P1 f2422f68 against the admitted S33 packet and original request. The observer uses the existing copied retirement record, retains only aggregate direction/port state, and does not introduce a port provider, Core/VM interface, or profile behavior change. The focused CTest target, synthetic marker, contained replay cleanup, documentation governance, and diff hygiene pass. S33 is accepted; the next checkpoint must inspect the semantic consequence of the existing port `61h` read without inferring board timing.
