# T376 S6: Raw-IMG Sidecar Fidelity Task Closure

`M5:T376:S6:TASK-CLOSURE:OK`

## Closure Matrix

| Requirement | Proof | Closure disposition |
| --- | --- | --- |
| Raw sector payload remains independently usable; sidecar is optional metadata only. | S2 parser/serializer and `vm-media-provider-smoke`. | Accepted. |
| Raw-only default marks, strict identity/geometry parsing, stale and malformed sidecar rejection, and failed persistence preserve publication boundaries. | S2 lifecycle evidence; S6 malformed one-byte JSON fixture rejects before `malformed_fdd` publication while the mounted FDD remains readable. | Accepted. |
| Address marks persist as part of the paired raw/sidecar lifecycle without disk-change generation mutation. | S2 lifecycle and S3 evidence. | Accepted. |
| Read/Write Deleted Data and Control Mark use one FDC transfer/DMA/IRQ/reset/timing owner. | S3 focused FDC smoke. | Accepted. |
| Equal, Low-or-Equal, High-or-Equal and SK Scan behavior use the same owner, with DMA/non-DMA cadence and cancellation. | S4 focused FDC smoke. | Accepted. |
| Duplicate-CHRN selection/physical occurrence. | Owner-selected pure raw IMG boundary; raw payload plus logical-position sidecar carries no CHRN/physical sector-ID record. | Explicitly unsupported, not a failed implementation claim. Any flux/ID/layout representation requires a separately admitted task. |

## Final Replay

```text
M5:T272:S2:VM-MEDIA-PROVIDER:OK
M5:T280:S2:ATOMIC-MEDIA:OK
M5:T283:S6:ATOMIC-SAVE:OK
M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK
M5:T376:S6:MALFORMED-SIDECAR:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T375:S20:FDC-DMA-CADENCE:OK
M5:T375:S21:FDC-SEEK-CADENCE:OK
M5:T375:S24:FDC-NDMA-CADENCE:OK
M5:T376:S3:8272A-DELETED-DATA:OK
M5:T376:S4:8272A-SCAN:OK
```

The current local developer artifact remains `nxvm_0_5_0376.exe`, SHA-256
`95E1EE62E4D9E4A960BE637AA3BEBF8FE7CD5CD313A0124068977320D215E0DF`, built
from the accepted T376 S4 source commit. The S6 test-only change does not
alter the runnable product artifact.

## Transfer

T376 closes only the raw-IMG address-mark and selected 8272A command-fidelity
scope. Physical CHRN duplication, sector search, rotational/index timing,
CRC, weak-bit/flux behavior and board-level grant timing are not raw-IMG
claims. The historical [IBM PC/AT 5170 final model-L3 audit](../../history/M5-T377-5170-final-l3-audit-proposal.md)
closed not ready; its queued repair and re-audit successors must make each
residual row visible in their next decision. This does not accept Model-339 L3.
