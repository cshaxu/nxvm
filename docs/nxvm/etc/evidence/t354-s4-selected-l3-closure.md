# T354 S4: Selected L3 lifecycle and closure audit

## Closure result

T354 closes **selected L3**: a deterministic, traceable guest-time model with
one validation-to-commit boundary for the selected CPU/DMA memory and CPU-port
actions.  It does not claim cycle-exact PC/AT timing, pin waveforms, or
Windows installation readiness.

## Ledger disposition

| S1 selected family | Final disposition | Lifecycle / consumer proof |
| --- | --- | --- |
| CPU fetch/data memory, A20 and ROM | Immediate checked CPU transaction; mapping validation remains memory-owned and commit/cancel is traced | S2 owner smoke; S3 competition order; S4 reset-continuity smoke. |
| CPU ordinary/string I/O | Immediate checked CPU port transaction; permission/provider failure cancels before architectural publication | S2 owner smoke and retained port/firmware/DOS current-gate consumers. |
| 8237A ordinary and M2M | Existing request/grant/priority/cascade/EOP state owns selection; checked DMA memory transaction owns selected memory publication | T348 retained proof, S2 transaction smoke, S3 competition smoke, FDC-DMA boundary consumer. |
| PIC request, refresh, acknowledgement | Device source lifecycle and scheduled refresh remain PIC-owned; CPU successful architectural delivery owns logical acknowledgement | T349 retained proof, S3 order proof, retained CLI/STI and hardware-delivery tests. Physical INTA is transferred. |
| PIT, RTC, KBC, VADP | Existing deterministic timeline callbacks own event advance/visibility; no selected common bus reservation was established | T346/T350--T352 accepted current-gate proofs; reset re-arms their clocks/timeline. |
| FDC and ATA/HDC | Retained command service owns readiness; FDC DRQ binds to 8237A, ATA is retained PIO | T347/T348 proof, S3 order, `vm-fdc-read-track-dos-smoke` and `vm-ata-pio-dos-smoke`. |
| Firmware/profile/session consumers | Session/profile reset reaches core cold reset after topology is frozen; consumers observe only committed state | `core-machine-firmware-capability-smoke`, `vm-timer-firmware-smoke`, and current DOS prompt/keyboard/video/storage smokes. |
| Trace and reset | Machine owns transaction trace adaptation; reset cancels active state before reset event, clears counters, retains its trace binding, resets clocks/timeline, and re-arms callbacks | `core-machine-transaction-lifecycle-s4-smoke`; retained trace/timeline/session-reset smokes. |

## Direct lifecycle proof

`core-machine-transaction-lifecycle-s4-smoke` deliberately opens the
machine-owned transaction state, performs cold reset, and proves the trace
order `begin < cancel < reset`.  It then executes a reset-vector instruction
and proves a fresh CPU memory transaction's `begin < commit < retire` ordering.
The state is idle and its counters are zero after reset.  This proves the
new owner cannot leave a reservation or stale accounting across reset while
the trace binding remains usable.

## Consumer verification boundary

The full current gate retains the firmware capability, timer-firmware, DOS
prompt, keyboard, video-port, FDC read-track, and ATA PIO consumers.  Their
green result is a regression consumer of selected L3, not an authority for an
unmodeled timing value and not a Windows-readiness claim.

## Precise transfers

- Exact CPU bus-cycle and wait-state tables; prefetch, pipeline, cache, DRAM
  refresh, and electrical contention.
- Physical 8237A DREQ/DACK/EOP/HOLD/HLDA and 8259A INTA/spurious waveforms.
- Analog video/audio, host wall-clock timing, and non-selected device
  readiness/latency.
- A Windows 3.x installation or boot milestone.  It remains a future product
  consumer after selected device and timing scope is independently admitted.
