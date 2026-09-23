# M5 T346 S4: RTC And Storage-Observation Readiness

## Owner And Timing Contract

S4 moves the remaining RTC advancement and FDC media-generation observation
out of the post-retirement batch scheduler.  `core_machine_timeline` now owns
one reset-safe readiness callback per due tick.  The cold-reset path schedules
the established arbitration callback first and the readiness callback second.
At a shared due tick the observable order is therefore:

```text
DMA -> PIT -> PIC -> FDC media observation -> ATA refresh -> RTC advance
```

The order is deliberate: DMA/PIT sources are arbitrated immediately by the
S3 callback; a readiness callback can publish an RTC IRQ source only for the
next PIC due tick.  This gives a bounded, testable visibility boundary without
claiming a sub-instruction event or host-time behavior.

`rtc_clock` is a distinct machine clock domain.  It defaults to the existing
identity ratio, but no longer shares the retained external-provider domain.
This prevents an RTC configuration from implicitly choosing host/provider
advance behavior.

## Proof

`core-machine-rtc-storage-s4-smoke` configures a one-tick RTC and executes a
two-tick instruction retirement.  It proves two complete due-time chains,
each carrying full-width `timeline_ticks`, RTC second progression, two pending
successor callbacks, and cold-reset replacement by exactly one arbitration and
one readiness callback.  The registered marker is:

```text
M5:T346:S4:RTC-STORAGE-READINESS:OK
```

The retained S2 and S3 timeline smokes prove the original event ordering after
the second callback was added.  S3 still owns the internal `DMA -> PIT -> PIC`
chain; its smoke now filters that chain from the added readiness records rather
than falsely treating it as the whole scheduler.

The CMake pure owner-test cohort rises from 120 to 121; the new smoke receives
the established target-local GCC strict options through that verified cohort.

## Controller-Path Sweep And Transfer

| Path | Disposition |
| --- | --- |
| RTC `core_machine_rtc_advance` | Migrated through `rtc_clock` and the readiness callback. |
| RTC/CMOS NMI | The index-port mask is retained, but RTC periodic/update events route through IRQ8; no PC/AT parity or I/O-channel NMI producer exists in core and it transfers to TODO. |
| FDC `core_machine_fdc_refresh` | Migrated as deterministic media-generation/DIR observation only. Existing media-change smoke remains the controller behavior proof. |
| HDC `core_machine_hdc_refresh` | Called at the readiness boundary; it is currently a no-op and has no command completion behavior to claim. |
| FDC final-command execution | Still executes synchronously from the data-port path and may immediately assert DMA/IRQ. |
| ATA command execution and subsequent sector readiness | Still executes synchronously from the command/data-port paths and may immediately publish DRQ/IRQ. |

The last two rows are not mechanical L3 timing.  Neither current controller
has an admitted hardware service-duration source, and substituting arbitrary
tick delays would create a false PC/AT claim.  `TODO(High)` transfers one
future storage-service task: it must establish per-command validation,
pending/abort/reset, DRQ/DMA/IRQ publication, media-provider failure, and
hardware/probe timing evidence before changing either controller state machine.

## Verification

Fresh configuration, S4 focused smoke, retained S2 timeline and S3 arbitration
smokes, RTC/FDC/HDC focused regressions, documentation governance, diff check,
and the current gate are required before S4 acceptance.  No hardware source,
guest image, host clock, Bochs/PCjs source, or provider ABI is introduced.

Promotion: retain through T346 S6, then merge this timing/transfer disposition
into the T346 history and Windows readiness handoff.
