# M5 T348: PC/AT 8237A DMA Completeness

## Task Record

T348 completes the bounded dual-8237A mechanism after T346 established one
deterministic machine due-event order and T347 moved retained storage service
through its readiness boundary. It covers the PC/AT primary/secondary port and
page layout, byte/word address forms, channel-4 cascade, request selection,
terminal ownership, checked-memory failure atomicity, and the storage-consumer
boundary created by authentic same-page 64 KiB wrapping.

## Completed Subtasks

| Subtask | Closed outcome |
| --- | --- |
| S1 | Built the dual-controller contract and finite owner/proof ledger, including the reproduced invalid page-carry defect and exact S2--S4 receivers. |
| S2 | Repaired shared byte/word same-page address progression and secondary page-port ownership; permanent owner proof covers the PC/AT sparse page and word-channel layout. |
| S3 | Unified logical DREQ/software-request selection, fixed/rotating priority, channel-4 cascade, controller-disable isolation, and binding-scoped ordinary EOP. |
| S4 | Added validation-before-publication for every adopted memory route, M2M terminal/auto-init lifecycle ownership, deterministic consumer reconciliation, and a default-FDC 64 KiB crossing bounce path. |

## Closure Audit

The owner requested a stable, comprehensive deterministic-L3 core-machine
foundation before Windows testing. T348 supplies that bounded DMA result.
Every physical DMA data operation now goes through its checked preflight before
provider, latch, address/count, request, or terminal publication. The sole
M2M completion owner preserves the real two-channel split: channel 1 owns the
count/TC outcome, while both participating current-register and binding states
receive their explicit terminal or auto-init disposition.

The PC/AT page latch remains programming state, not a carry extension. The
default FDC firmware therefore uses its reserved `9fc0:0000` bounce page for
crossing INT 13h buffers. Its fixed `f000:0c00--dfff` ROM interval is protected
from sequential BIOS materialization; the owner smoke proves both caller
fragments, the 639 KiB BDA boundary, and channel-2 page retention. Reconciled
HDC port evidence advances the accepted T347 pending command/data phases, and
the T346 input/display trace now explicitly retains FDC/HDC advance events.

Fresh configuration, focused DMA/FDC/HDC/ROM/timeline owners, documentation
governance, `git diff --check`, and the parallel 223-test current-gate passed.
Detailed S1--S4 evidence remains indexed in `docs/etc/README.md`; the retained
proposal accompanies this task record.

## Explicit Transfers

T348 does not model DREQ/DACK/EOP electrical waveform or polarity, READY or
compressed duration, generic bus wait states, host DMA, clone-specific timing,
or whole-board Windows compatibility. The ordered Queue assigns PIC, platform
signals, KBC/AUX, video, port topology, and the separate L3 bus-timing package
to their own bounded tasks. No transfer reintroduces page carry or a second DMA
scheduler.
