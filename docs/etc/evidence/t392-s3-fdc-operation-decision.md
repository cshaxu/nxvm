# T392 S3: FDC Operation Decision

`M5:T392:S3:FDC-OPERATION-DECISION:OK`

## Project-owned operation evidence

The Core 8272A owner already models read-command execution, media validation,
DMA byte service, IRQ assertion, result phase, normal completion and abnormal
status. Existing FDC topology/controller smokes exercise read success and
absent/error paths. Model40 composition binds that generic FDC, DMA channel and
IRQ route; it does not own a duplicate FDC state machine.

## Decision matrix

| Candidate | Current owner/state | Why it cannot be used now | Transfer |
| --- | --- | --- | --- |
| Read-data successful FDC result | Core FDC private phase/result/status/DMA/IRQ state | No immutable published operation snapshot; exposing `core_machine_fdc` layout or a VM pointer would violate ownership. | Future generic Core device-operation snapshot contract, if separately admitted. |
| FDC IRQ assertion | Core PIC/FDC route | IRQ can describe reset, seek, error and completion; alone does not prove a boot-sector read. | Same operation contract must bind result and request identity. |
| DMA terminal | Core DMA/FDC state | Completion may be normal or abnormal and has no selected firmware-consumer meaning. | Same operation contract plus selected consumer semantics. |
| FDD media generation | VM FDD media owner | Media replacement is not command completion. | Remains non-semantic. |

Local read-only 86Box/PCjs inspection is consistent only at the high level:
floppy reads are controller operations with completion/error paths. It is
secondary design context, not semantic or timing authority and no code is
imported.

## Transfer

FDC read completion is a plausible *future* device operation, but T392 cannot
select it as C1 endpoint because no primary firmware/consumer contract says
which exact successful operation means boot transfer, and the required generic
immutable Core operation snapshot is not an existing interface. The earliest
receiver is a separately admitted Core-device/VM-session observability design
that fixes operation identity, copied result fields, lifetime, reset/failure,
consumer and regression corpus before implementation. T392 records no CPU,
firmware, board or physical-time defect; physical-retirement qualification
remains blocked.