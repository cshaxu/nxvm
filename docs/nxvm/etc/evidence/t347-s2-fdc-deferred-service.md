# M5 T347 S2: FDC Deferred Command and Completion Service

## Implemented Lifecycle

The FDC keeps command interpretation, media geometry, byte transfer, result
encoding, DMA request control, and IRQ6 ownership local to `fdc.c`.  It now
adds two local phases between the retained command, execution, and result
phases:

| Boundary | State after the issuing transaction | Next readiness service action | Published effect |
| --- | --- | --- | --- |
| Last command byte | `PENDING_COMMAND`; MSR has `CB`, without `RQM`, `DIO`, DRQ/DMA, or IRQ | Execute the retained command decoder once | Execution DRQ/DMA, an immediate command result, or a pending transfer result, as the command requires |
| Last DMA or non-DMA transfer byte | `PENDING_COMPLETE`; MSR has `CB`, DMA is deasserted, and IRQ6 is absent | Encode the retained seven-byte result and raise IRQ6 once | Result phase, `RQM|DIO|CB`, and IRQ6 |

`core_machine_readiness_tick` advances the FDC before its existing media/DIR
observation and before ATA/RTC work.  T346's due-event order consequently
remains `DMA -> PIT -> PIC -> FDC service/observation -> ATA -> RTC -> KBC ->
VADP`: a command or final transfer made visible by FDC service cannot be seen
by the already-completed equal-tick arbitration callback.  No controller owns
a timeline pointer, private scheduler, or host-duration constant.

The retained command sweep covers `SPECIFY`, `SENSE DRIVE STATUS`,
`RECALIBRATE`, `SENSE INTERRUPT`, `SEEK`, `VERSION`, `READ ID`, `READ DATA`,
`READ TRACK`, `WRITE DATA`, and `FORMAT TRACK`.  All command bytes now enter
the pending-command state.  Transfer-capable commands publish execution only
from service; every transfer/error completion enters pending-complete first.
Simple seek/recalibrate and retained immediate-result commands publish their
existing controller-specific result only while the pending command is serviced.

## Cancellation and Caller Sweep

`core_machine_fdc_cancel_execution`, controller reset, DOR reset/disable,
drive-loss cancellation, and finalization clear pending state and deassert DMA
before another readiness service can publish a result or IRQ.  The only
periodic production caller is the existing readiness callback.  The command
data-port writer, non-DMA data reader/writer, DMA byte callbacks, DMA terminal
callback, DOR writer, reset, media observation, result reader, and
finalization paths were reviewed; no completion-side IRQ/DMA publication
remains in an issuing port or final-byte transfer transaction.

## Proof

`core-machine-fdc-smoke` directly proves command-pending busy observation,
the next service transition to non-DMA read execution, final-byte
pending-complete without IRQ, and the next service transition to result/IRQ.
It emits `M5:T347:S2:FDC-SERVICE:OK`.  The retained media-change smoke proves
DMA DRQ publication only after command service and cancellation by DOR disable
and reset.  The retained topology, controller-authority, VM port, and T242
read-track probes exercise DMA, non-DMA, result, media, port, and VM-facing
paths after their helper boundaries were moved to service transitions.

Focused FDC/VM regressions, a fresh GCC configuration, documentation
governance, whitespace validation, and the complete current gate are recorded
with the S2 implementation commit.  ATA is unchanged and remains T347 S3's
separate admitted migration.
