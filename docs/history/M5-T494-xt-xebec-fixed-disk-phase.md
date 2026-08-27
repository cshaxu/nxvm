# M5 T494 IBM 5160 Xebec Fixed-Disk Controller Phase Contract

T494 is the independent IBM 5160 Xebec fixed-disk-controller unit. It
validates original IBM sources, freezes complete function/timing and current
code-gap ledgers, then consumes one coherent owner-local HDC/media repair
batch. It must retain one Core controller/media owner and must not create an
ATA alias, a duplicate CHS/cache path or VM-owned controller state.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: visual verification of IBM 6139790 establishes the finite original-source basis for List 1, retains OCR as discovery-only and excludes ATA/WD1003/ESDI substitution, code and protected material. [Evidence](../etc/evidence/t494-s1-xebec-original-source-ledger.md). |
| S2 | Accepted: all 18 IBM Xebec command/port/state/timing source families have one Manual-L3, bounded Other-L3, L1 or unsupported disposition; no external timer or guessed status is promoted. [List 1](../etc/evidence/t494-s2-xebec-function-timing-list-1.md). |
| S3 | Accepted: all 18 rows map to one Core HDC/media/DMA route; R1 is the sole complete repair batch, fixing Read Data's ignored block count without new state or path. [List 2](../etc/evidence/t494-s3-xebec-current-code-gap-list-2.md). |
| S4 | Submitted for acceptance: Read Data now consumes its positive DCB block count through the existing DMA3/CHS/media owner; focused two-sector proof passes. T-level closure audit remains pending. [Evidence](../etc/evidence/t494-s4-xebec-read-block-count-repair.md). |
