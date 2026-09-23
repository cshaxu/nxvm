# T501 S5: Model-40 Boot Universe

`T501-S5-MODEL40-BOOT-UNIVERSE`

## Frozen Row

The T501 Model-40 row is DeskPro 386 Model 40 / 80386 / 1.2-MB 5.25-inch
DOS media.  Its accepted terminals are the task's existing `dos-prompt`,
`date-input`, or `installer-ready` states.

Available original material establishes the 1.2-MB option but not the fitted
drive count.  The read-only PCjs DeskPro configuration declares two 1.2-MB
mechanisms and mounts media only in A:.  That construction fact is retained as
External-L2, not as a claimed Compaq manual fact.  No external source code or
asset is imported.

## Repair And Observation

The previous D4 compatibility buffer duplicated immutable firmware ownership
at E0000h--FFFFFh.  During protected-mode entry, a legal GDT Accessed-bit write
was routed to that duplicate buffer and faulted.  The repair removes the
buffer and its replacement mappings.  The immutable firmware mapping is the
sole ROM owner; D4 retains only its own diagnostic, parity and independent
setup-bank responsibilities.

The frozen profile now declares two fitted 1.2-MB mechanisms but binds one
medium only to A:.  Core FDC remains the sole owner of selection, status, DMA,
IRQ and media observation.  The normal Core KBC route delivers F1 only after
the visible firmware `RESUME` prompt; this is an ordinary guest input event,
not a firmware escape.

The owner-managed ROM/media replay reaches the `A:\\>` DOS prompt.  The replay
also passes the earlier protected-mode transition, FDC command/DMA/IRQ path and
firmware continuation without a profile scheduler, ROM, CMOS or media bypass.

## Verification

- Complete repository-only unit suite: 312/312 pass.
- Complete owner-managed integration suite: 20/20 pass.
- Current stripped Release artifact is rebuilt as `nxvm_0_5_0501.exe`.

## Retained Boundary

The original sources still do not quantify Model-40 mechanical READY/change
timing or its physical board conversion.  That limited physical relation
remains source-gated; it was neither estimated nor needed to manufacture the
observed DOS terminal.

`T501-S5-MODEL40-REPAIR-DISPOSITION`
