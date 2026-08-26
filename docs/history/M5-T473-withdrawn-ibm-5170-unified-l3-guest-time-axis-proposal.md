# M5 IBM 5170 Unified L3 Guest-Time Axis

## Purpose

Close the selected IBM 5170 Model-339 machine's complete L3 guest-time chain:
CPU execution, transactions, board fabric, controllers, media and display use
one Core-owned guest-time axis. VM supplies only copied immutable profile
values and provenance; Standard limits completed Core progress against a
monotonic host budget, while Turbo takes the same Core path without that wait.

This is a correctness task, not a pacing workaround. A fixed host sleep, a VM
tick writer, a second event queue, device-local host clocks, or a profile
callback that advances Core state is prohibited.

## Scope And Claim Boundary

The selected target is the existing `ibm-5170-model-339` configuration. It
must have one finite coverage ledger for the CPU, memory and I/O transactions,
DMA/refresh/arbitration, PIC, PIT, RTC/CMOS, KBC/NMI, 8272A FDC and media,
the selected fixed-disk boundary, VADP/CGA/EGA, and every selected board route.
Each entry must end as either:

- **Manual/Other L3:** a source-backed constant, formula or deterministic
  bounded selection, implemented on the shared Core axis; or
- **L2 fallback:** a named functional/proportional input with a bounded source
  gap and profile-visible provenance.

Exact integer or rational conversion of an L2 estimate remains L2. It may be
consumed as deterministic Core data, but it must never be relabelled L3 merely
because the conversion is exact. L1/L0 is not accepted for a selected 5170
mechanism at closure; it must be implemented, explicitly removed from the
selected topology, or be a documented stop condition requiring owner review.

This task targets Project A L3, not L4 electrical reproduction. It does not
claim universal PC compatibility, import external emulator code, distribute
firmware/media, or manufacture undocumented physical facts.

## Architecture

```text
profile source facts / approved L2 ratios
                 |
                 v
      copied immutable Core machine plan
                 |
                 v
CPU + bus + controllers + deadlines --> Core sole guest-time axis
                 |                              |
                 | copied observation            | Core-only advance
                 v                              v
       VM Standard host budget             VM Turbo no host wait
```

Core owns the counter, event order, deadline composition, execution/transaction
costs and device state. A resolved VM profile may select only neutral copied
values, rule IDs and provenance; it never writes elapsed time or holds a Core
device pointer. Core exposes one copied observation containing current elapsed
time, the earliest composable deadline, axis qualification/rate and retained
L2/L3 provenance. It exposes bounded Core-owned operations only; no raw
scheduler, device, RAM, CPU or machine layout leaks through the interface.

## T473 Subtask Plan

1. **S1 - frozen 5170 whole-machine time ledger.** Reconcile the selected
   5170 board/manual corpus and existing controller ledgers into one row-complete
   CPU-to-presentation/IRQ/DRQ/refresh universe. Cross-check read-only 86Box,
   MAME, PCjs, Bochs and QEMU where applicable; classify every line Manual L3,
   Other L3 or L2 fallback and identify every missing input.
2. **S2 - one Core physical-axis and profile-plan contract.** Consolidate the
   copied Core time-axis/deadline/provenance contract. Remove duplicate or
   transitional time paths; validate construction-only L3 values and L2 ratios.
3. **S3 - CPU retirement, exception and transaction time.** Implement the
   selected 80286 instruction, interrupt/fault, memory and I/O transaction
   costs on the axis, including documented wait/arbitration selection.
4. **S4 - 5170 board fabric.** Implement source-backed CPU/bus, DMA, refresh,
   HOLD/HLDA, PIC route and availability relationships; make all residual
   ratios copied L2 values with retained provenance.
5. **S5 - timer, RTC and keyboard deadlines.** Complete PIT, RTC/CMOS,
   KBC/NMI and speaker/selectable board-clock deadlines on the shared axis.
6. **S6 - DMA, FDC and media phases.** Complete 8237A and 8272A transfer,
   DRQ/IRQ, motor/media and cancellation/reset timing; retain only explicit
   L2 board/media assumptions.
7. **S7 - fixed disk and display phases.** Complete the selected storage route
   and VADP/CGA/EGA port, memory, scan/status and snapshot timing boundaries
   without a parallel video or media clock.
8. **S8 - composed deadline and L2 materialization.** Compose every eligible
   owner deadline at Core, block fast advance where a declared selected
   mechanism cannot safely compose, and prove no VM or host source advances
   guest time.
9. **S9 - selected 5170 profile closure.** Bind only the frozen Model-339
   source facts/L2 exceptions through the resolved copied plan; reject invalid
   or unproven combinations at construction.
10. **S10 - Standard/Turbo runtime proof.** Standard may wait only when
    completed Core progress leads its verified profile budget; Turbo omits only
    that wait. Prove HLT, deadlines, reset, pause, debugger and cancellation.
11. **S11 - whole-machine closure audit.** Reconcile every ledger row and
    source/code path, run the full current gate plus targeted system workload,
    publish stripped Release 0473, and retain every remaining L2 exception.

## Exit Criteria

The selected Model-339 machine has exactly one Core guest-time axis and one
profile-to-Core construction path. Every selected capability is L3 or carries
an explicit, owner-visible L2 exception with source, input, provenance,
regression and earliest receiver. Standard and Turbo use the same guest
execution/deadline semantics; only Standard's host wait differs. No fixed HLT
poll, host-to-Core tick injection, duplicate clock, mutable profile setter or
unowned deadline path remains. Closure requires a row-complete ledger, focused
regressions, full current gate, documentation governance, stripped Release
artifact and a source-to-runtime closure audit.

## Stop Conditions

Stop for owner direction if a required selected mechanism has neither primary
source nor defensible L2 input, if a requested model fact requires protected
firmware/media or third-party source import, or if the complete selected
topology cannot be represented without violating single-Core ownership. Record
the exact row and blocked evidence; do not silently substitute a generic clock.
