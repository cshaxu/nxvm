# T421 S2 Port-B Exclusivity Corrective Evidence

`M5:T421:S2:PORT-B-EXCLUSIVITY:OK`

## Finding

T421 S1's shared speaker-line review raised the risk that planar PC/AT and
DeskPro D4 port-B configuration could become two providers for `61h`. Actual
Core review found that no duplicate registration is possible: both configuration
entry points use the same port registry, whose existing read/write occupancy
check rejects the second provider with `TYPE_STATUS_INVALID_ARGUMENT`.

The missing protection was a focused two-direction regression. This corrective
adds no production code or public ABI. It proves the existing Core owner:

- planar first, then D4: D4 is rejected; after freeze/reset, port-B low bits
  remain planar `04h`;
- D4 first, then planar: planar is rejected; after freeze/reset, port-B low
  bits remain D4 `0Fh`.

The read occurs after reset because bits 4--5 are dynamic timer/refresh status
and configuration-time reads intentionally have no reset-visible device state.

## Ownership and tier

This is a `generic-at` topology guard. The Core port registry owns the one
mutable port namespace. Model-40 continues to select the D4 provider and IBM
5170 Model 339 continues to select planar parity; neither VM composition adds
a port route, fallback priority or second speaker state. The D4 speaker truth
table remains T421 S1's original-source result.

## Verification

Passed after rebuilding `vm-0-5-0419`:

- `current.core-machine-d4-platform-s4-smoke` including both configuration
  orders and reset-visible first-provider preservation;
- `current.vm-model40-private-composition-s7-smoke`;
- `current.vm-ibm-5170-model-339-composition-smoke`;
- `current.core-machine-planar-parity-nmi-s3-smoke`;
- serial `current-gate`: 289/289 passed.

Documentation governance and `git diff --check` pass. The rebuilt artifact
SHA-256 is 859E1B93C6891E8EAAF0D98D4DBEF25F2383F911EC243390A50FB9A9CDBBA5BF. No source, firmware, media or third-party code was
imported.