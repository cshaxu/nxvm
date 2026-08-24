# T447 S10 Core machine coordinator closure

## Result

Td S125 A identified `core/machine/machine.c` as a mixed coordinator of
timing, plan validation, event scheduling, firmware, board-device wiring,
display materialization, diagnostics, lifecycle, and fault publication. This
change restores each already-distinct mechanism to one existing owner-local
implementation. `machine.c` now owns only the unique machine aggregate and
the create, reset, run, stop, reconfigure, and destroy coordination path.

No new object, framework, public ABI, VM dependency, parallel execution
route, or transaction route was introduced. The private `machine.h` remains
the aggregate's definition; its internal declarations identify the few
cooperating owner operations, not an exported contract.

## Mechanism and caller inventory

| Mechanism | Prior caller/owner | Single resulting owner | Retained caller seam |
| --- | --- | --- | --- |
| CPU timing, external-cycle accounting, transaction trace | `machine.c` | `cpu_timing_model.c` | creation installs the same CPU/transaction callbacks; run invalidates the same timing state. |
| Timing-plan validation and copied topology materialization | `machine.c` | `machine_plan.c` | creation and plan entry points call the owner directly. |
| Event callbacks, clocks, DMA/refresh arbitration, elapsed-tick publication | `machine.c` | `machine_scheduler.c` | cold reset schedules and run/advance-time publish through the same one timeline. |
| Firmware callback context, ROM mapping, status conversion and rollback | `machine.c` | `machine_firmware.c` | reset/run invoke the sole firmware operation boundary. |
| Display port/VADP setup and copied display snapshot | `machine.c` | `machine_display.c` | topology materialization and public capture retain their existing one route. |
| RTC, DMA, PC/AT parity/speaker, D4, absent-memory, FDC and HDC board setup | `machine.c` | `machine_board.c` | topology materialization invokes the same public Core configuration operations; cold reset calls the board's two phase-accurate hooks. |
| CPU diagnostic capture and ordered copied observation | `machine.c` | existing `debug.c` | creation installs the one diagnostic provider and observation capture delegates to it. |
| Instance lifecycle, one aggregate, execution-provider freeze, reset/run/stop/destroy and fault result publication | `machine.c` | `machine.c` | unchanged single production route. |

The resulting dependency flow is `machine.c` (aggregate/lifecycle coordinator)
to the neutral owner-local Core mechanisms. The mechanisms retain pointers to
the one Core aggregate only for their own callback state; none can create,
replace, or retain a second machine instance, and no owner depends on VM or
VDM.

## Failure and rollback preservation

- `machine_firmware.c` retains the original immutable-ROM bind rollback:
  failed binding rolls mappings back and clears provider/context before the
  original status is returned.
- `machine_display.c`, `machine_board.c` RTC/parity/D4 setup, and FDC/HDC
  setup retain their pre-existing port-registration checkpoint/rollback
  pairs. There is still one checkpoint per configuration attempt.
- `machine_plan.c` continues to validate before applying copied topology;
  failed plan materialization reaches the existing create cleanup path.
- Board cold reset remains phase ordered: generic devices reset first, then
  the shared PIT resets, then board port-61h/refresh/failsafe wiring is
  re-established. The two board hooks replace no behavior and introduce no
  reset path.
- CPU diagnostics still report the first fault through the one lifecycle
  fault publisher; their copied readout remains unavailable while initialized
  or running.

## Size and minimalism accounting

The audited coordinator was 6,786 lines. After the decomposition it is 1,090
lines before documentation changes. The split is by existing mechanism,
not line count: it deletes the former co-location without duplicating state or
copying behavior. The only new source owners are the six cohesive files that
had no existing owner-local home: timing model, plan, scheduler, firmware,
display, and board configuration. Diagnostics join existing `debug.c`.

The implementation diff changes 337 tracked lines and deletes 6,095 tracked
lines before the new owner-local files are counted. The deleted coordinator
code is relocated as one copy, not reimplemented; the net source accounting
and actual diff review are recorded with P1.

## Verification and review

- Strict full MinGW build passed after the `cpu_timing.h` self-containment
  correction. This caught and corrected the only compilation defect exposed by
  splitting its timing-model declaration.
- All 77 specialized gates passed. Updated guards retain the same assertions,
  but inspect their actual owner-local source: board configuration, display
  materialization, scheduler, firmware and CPU timing model rather than the
  retired monolithic location.
- Full CTest passed 292/292 with zero failures (300.98 seconds). The CTest
  wrapper left its completed process resident; `LastTest.log` records the
  authoritative completed 292/292 result and the owned process was then
  stopped without terminating a test.
- The rebuilt `vm-0-5-0447` artifact SHA-256 is
  `8279CA97F977C40EDDF014D03DB21E1A89839E602E81F7D4829D6A14E1BF2601`.
- `git diff --check` passes. Manual actual-diff review confirms one aggregate,
  one execution/transaction path, one rollback sequence per materializer, and
  no Core-to-VM/VDM include edge.
