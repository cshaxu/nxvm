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
| S3 | In progress: map all 18 rows to the actual Xebec HDC/media/DMA/PIC/profile paths and freeze one complete implementation batch. |
