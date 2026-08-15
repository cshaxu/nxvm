# T376 S5: Raw-IMG Sidecar Task-Closure Audit

`M5:T376:S5:TASK-CLOSURE-AUDIT:STOP`

## Requirement Reconciliation

| Approved requirement | Current proof | Disposition |
| --- | --- | --- |
| Raw `.img` stays independently usable; optional sidecar contains only address-mark metadata. | S2 FDD parser/serializer and `vm-media-provider-smoke`. | Proven. |
| Schema identity/geometry validation and raw-only default marks. | S2 strict parser, raw-only, stale-sidecar and reinsertion regressions. | Proven for the declared raw/sidecar representation. |
| One paired save/remove lifecycle with recoverable failure behavior. | S2 paired-save implementation and backup-collision regression. | Proven. |
| Read/Write Deleted Data and ordinary/deleted Control Mark results use existing FDC transfer, DMA, IRQ, reset and cadence owners. | S3 evidence and `core-machine-fdc-smoke`. | Proven. |
| Scan Equal, Low-or-Equal and High-or-Equal use the same owner with marked-sector behavior, DMA/non-DMA cadence and cancellation. | S4 evidence and focused FDC smoke. | Proven. |
| Regression for duplicate-CHRN selection, with an unambiguous per-physical-occurrence address mark. | The raw FDD has fixed geometry and logical-sector positions only; no CHRN ID records exist in raw bytes, sidecar schema or FDC lookup. | **Not representable under the approved pure raw-IMG boundary.** |

## Replay

The current GCC build replayed both focused task routes and the governance
gate:

```text
M5:T272:S2:VM-MEDIA-PROVIDER:OK
M5:T280:S2:ATOMIC-MEDIA:OK
M5:T283:S6:ATOMIC-SAVE:OK
M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T375:S20:FDC-DMA-CADENCE:OK
M5:T375:S21:FDC-SEEK-CADENCE:OK
M5:T375:S24:FDC-NDMA-CADENCE:OK
M5:T376:S3:8272A-DELETED-DATA:OK
M5:T376:S4:8272A-SCAN:OK
```

The T376 developer artifact remains the accepted local
`nxvm_0_5_0376.exe` recorded in S4. No external ROM, guest media or source was
introduced.

## Stop Condition And Required Decision

The original proposal explicitly prohibits a second image format, while its
test scope asks for duplicate-CHRN selection. A raw sector-payload image has
no encoded sector-ID occurrences, so an implementation cannot distinguish two
same-CHRN physical sectors without adding layout/flux/ID metadata that exceeds
the selected raw-only representation. Treating logical-sector index as a
duplicate-CHRN selection test would be a false claim.

This invokes the proposal's stop condition. T376 cannot close, and the final
Model-339 audit cannot be admitted, until the owner chooses one of these
bounded paths:

1. retain pure raw IMG and explicitly exclude duplicate-CHRN selection from
   T376 and the selected Model-339 media contract; or
2. admit a separately scoped non-raw/physical-sector representation task,
   with its own source, persistence, compatibility and migration decision.

The existing physical-sector search, rotation/index, CRC, weak-bit/flux and
board-level timing transfers remain for the final Model-339 audit; they are
not repaired or accepted here.
