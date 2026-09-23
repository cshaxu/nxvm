# T504 S1 L1 No-Deadline Inventory

## Scope and method

This inventory follows every condition that currently prevents the Core
scheduler from publishing a next deadline in
`src/core/machine/machine_scheduler.c`, then follows the owner through its
normal advancement and lifecycle paths. It also checks the VM runner and
waiting boundary. It is an implementation inventory, not a new timing source.

## Finite disposition

| State at `core_machine_capture_time_observation_private()` | Owner and normal wake | Evidence class and disposition |
| --- | --- | --- |
| A primary or secondary DMA request/service phase is pending. | The existing DMA owner advances one service primitive through `core_machine_arbitration_advance()` after its configured DMA clock conversion; reset clears DMA state and DRQ owners may withdraw requests. | L1 causal service when no source-qualified DMA service deadline exists. Candidate for the shared Turbo rule, after known work is settled; no controller-specific path. |
| HDC phase is pending command, read sector, or write sector. | The sole HDC owner consumes its existing phase in `core_machine_readiness_advance()`; normal command/result/IRQ and reset paths remain owner-local. | L1 causal service: current code has no HDC next-due query or sourced service duration. Candidate only for the shared rule; it remains L1. |
| D4 refresh hold is pending. | The board refresh owner consumes the one existing transaction in `core_machine_d4_refresh_hold_advance()` and normal reset clears it. | L1 board-causal hold. Candidate for the shared rule; no physical refresh duration may be inferred. |
| Slave PIC has IRR while the master cascade request has not yet been published. | Existing `core_machine_pic_refresh()` publishes the cascade during arbitration; normal PIC acknowledge/EOI/reset retain their current owner. | L1 ordered publication. Candidate for the shared rule, with IRQ selection/stop boundaries re-evaluated after each step. |
| FDC pending command or pending complete. | `core_machine_fdc_next_due_tick()` reports the current FDC elapsed tick and `core_machine_fdc_advance_at()` performs its existing transition. | Not L1 and not eligible for compatibility progression. This is already-due Core work. The scheduler's early global block hides it, so S2/S3 must preserve and settle it before considering any L1 quantum. |
| FDC reset, seek, DMA-byte, or NDMA-byte due event. | The FDC owner reports an absolute due tick and advances through its existing state machine. | Source-qualified controller deadline when its configured timing rule supports it; never an L1 escape candidate. |
| Timeline, PIT, RTC, KBC, or XT keyboard event. | Existing owner-specific due query and normal controller advance. | Source-qualified deadline; never an L1 escape candidate. |
| CPU retirement external bus-ready wait. | `core_machine_run()` itself remains active and publishes one existing external-wait tick; reset, stop and fault checks run at its normal loop boundary. | Not a `STOP_WAITING_FOR_INTERRUPT` state and not a VM waiting/escape candidate. It needs its own board-timing evidence, not a Turbo path. |
| HLT with no Core owner, deadline, or pending ordered event. | Only external input/interrupt, stop, reset, debugger/pause, or a later normal Core event can change it. | External wait, not an internal L1 owner. Turbo must yield and must not manufacture guest time. |

## Boundary conclusion

The four candidate classes are DMA, HDC, D4 refresh and unpublished slave
PIC cascade. They share exactly one property: their owner has already declared
a causally necessary state transition but supplies no source-qualified time
for it. They may therefore use one bounded Core-owned compatibility
progression only after known deadlines and already-due FDC work have been
settled.

The FDC finding is an in-scope correctness constraint, not a deferral. A
future T504 implementation must repair the scheduler ordering so that an
already-due FDC transition is not hidden behind a broad L1 block. It must not
misclassify it as a Turbo-only compatibility case.

VM remains outside this ownership: `vm_session_waiting_advance()` can call
only `core_machine_advance_to_next_deadline()` today, and `runner.c` yields
when the copied observation has no deadline. The subsequent copied Core result
may expose a narrow disposition, but VM must not select ticks or mutate device
state.

## Gate corrections discovered during S1

The first complete unit replay exposed four pre-existing owner-path failures.
They are closed here because S1 cannot claim an all-green baseline otherwise.

| Finding | Sole correction |
| --- | --- |
| A non-D4 DMA machine lost its PIT1-to-DMA0 refresh attachment when PIT reset cleared the active output state. | `core_machine_board_after_pit_reset()` restores the already configured DMA callback only when D4 has not selected its distinct refresh consumer. |
| The CGA port test wrote ordinary RAM instead of VADP-owned CGA VRAM. | The test now configures the existing VADP CGA memory provider. |
| The DMA cascade test carried a previous fixture byte count, and the refresh test observed the software-request field rather than the hardware DRQ status field. | The tests reset their own fixture and inspect the existing DMA hardware-DRQ owner state. |
| T503 made high-resolution CGA palette selection visible and made disabled EGA output publish a blank copied snapshot; older test expectations still asserted the former behavior. | The tests now assert the one VADP snapshot contract. High-resolution capture also retains `color_select` in its existing capture comparison, so a visible palette change marks the copied frame changed. |

No new owner, timing source, scheduler, profile workaround, or VM mutation was
introduced. These corrections are independent of the later Turbo policy but
are a necessary green-gate baseline for it.
