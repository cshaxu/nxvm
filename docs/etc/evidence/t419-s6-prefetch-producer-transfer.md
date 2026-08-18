# T419 S6 Prefetch Producer Transfer

`M5:T419:S6:PREFETCH-PRODUCER-TRANSFER:OK`

## Finding

The selected original DeskPro D4 material establishes discrete CPU PIPE TO
MEMORY hit/miss and idle/miss states. It does not supply an executable
asynchronous CPU request protocol for NXVM. The present Core CPU prefetch
implementation cannot truthfully publish the existing `OVERLAP_DECLARE` event:
`ExecInit` refills a persistent 15-byte prefetch buffer with one synchronous
logical read; that read reaches `_kma_read_physical`, which publishes BEGIN,
performs and commits the physical transaction, then publishes COMMIT before the
refill returns. A later refill begins only after that completed lifecycle.

Therefore no production predecessor remains in flight when the later prefetch
address is known. Data, page-walk and instruction-fetch paths likewise do not
satisfy the required instruction-prefetch predecessor predicate. The focused
S5 callback proof remains a contract test, not a production producer.

## Ownership And Profile Audit

The required mechanism would be a Core CPU execution and transaction lifecycle
extension, not a Model-40 feature. Model-40 may continue to select the existing
D4 `external_cycle_timing` policy only; IBM 5170 Model 339 selects the disabled
all-zero policy through the same Core configuration and has no alternate
prefetch or transaction route. No code change, VM scheduler, or second route is
admissible from this finding.

## Transfer

The retained receiver is an original- or reference-backed asynchronous external
CPU-prefetch producer that can prove an in-flight predecessor identity across
transaction publication, cancellation, reset, HOLD, refresh and retirement.
It must be admitted with a finite timing/trace contract before enabling the D4
page-hit result from production execution. Until then, synchronous completed
adjacency remains a miss. D4 PAL row/bank decoding, BCLK duration, CPU BWAIT,
and external bus-master phases remain transferred separately.

## Verification

This is a documentation-only stop/transfer after source and implementation
inspection. The existing S5 Core/Model-40/5170 regressions retain their prior
passing evidence and artifact hash. Documentation governance passes for this
transfer record.