# M5 T347: PC/AT Storage Controller Service Timing

## Task Record

T347 consumes T346's deterministic due-event foundation and completes the
retained uPD765/8272A-compatible FDC and primary ATA PIO command-service
surfaces. It replaces same-transaction completion publication with one
deterministic readiness-service boundary without creating a second scheduler,
host-time model, media ABI, or Windows compatibility claim.

## Completed Subtasks

| Subtask | Closed outcome |
| --- | --- |
| S1 | Established the cross-controller lifecycle inventory, sole readiness owner, pending/publication rule, cancellation sweep, and S2--S4 migration plan. |
| S2 | Moved retained FDC final-command and final-transfer completion through pending command/completion states; only readiness service publishes result, DMA/IRQ-visible completion. |
| S3 | Moved retained ATA READ/WRITE/IDENTIFY issue and final-sector transitions through captured-task-file pending states; readiness service publishes DRQ/status/IRQ follow-up. |
| S4 | Reconciled the combined FDC then ATA readiness order, trace proof, retained cancellation paths, and explicit later L3/device transfers. |

## Closure Audit

The owner requested high-value PC/AT device completeness and L3 timing before
Windows-startup testing. T347 supplies that result for its bounded storage
surface: accepted command issue shows pending/busy state; a later deterministic
readiness callback performs controller service; and output becomes eligible for
the following arbitration pass. The sole production owner is
`core_machine_readiness_tick`, with fixed order `FDC advance -> FDC refresh ->
ATA advance -> ATA refresh -> RTC advance`.

Each retained FDC and ATA command/final-data path now has one service transition
and one cancellation boundary. The task-file snapshot prevents ATA command
register rewrites from changing queued service. DOR/reset/media paths for FDC
and SRST/control/media paths for ATA prevent stale pending publication. The
S4 trace smoke proves two complete readiness chains; retained FDC media,
read-track, ATA PIO DOS, and HDD boot probes prove controller behavior at this
shared boundary.

Fresh configuration, focused storage/timeline probes, documentation governance,
`git diff --check`, and the 222-test parallel current-gate passed. The retained
proposal accompanies this record in history; detailed S1--S4 evidence remains
indexed in `docs/etc/README.md`.

## Explicit Transfers

T347 does not model controller command durations, rotational mechanics,
bus-master IDE, LBA48, ATAPI, cache policy, host-async media, generic DMA,
or physical bus wait states. The ordered Queue assigns DMA, PIC, platform
signals, KBC/AUX, digital video, port topology, and bus-timing convergence to
separate bounded candidates. Windows readiness consumes those results; it is
not evidence supplied by this task.
