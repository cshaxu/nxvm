# T375 S3: Model-339 Non-retirement Time Inventory

## Result

`M5:T375:S3:MODEL339-NONRETIREMENT-TIME-INVENTORY:OK`

The current machine has exactly one normal elapsed-time publisher:
`core_machine_run()` calculates an instruction cost, increments
`machine->elapsed_ticks`, and invokes `core_machine_advance_scheduler()`.
Cold reset is the only other writer, and it resets the count to zero rather
than publishing elapsed time.  Every configured Model-339 clock-domain
consumer reaches the timeline only through that scheduler:

| Boundary | Current consumer | Result when no normal time is published |
| --- | --- | --- |
| arbitration | DMA, PIT, PIC refresh | no DMA grant/service, PIT count, or PIC refresh |
| readiness | FDC/HDC observation, RTC | no controller observation or RTC count |
| peripheral | KBC, VADP | no delayed keyboard or raster progression |
| provider | execution-provider `advance_time` | no provider time callback |

Reset reinitializes all domains and schedules the three callbacks at tick one;
it is correctly cancellation/reinitialization, not elapsed machine time. A
synchronous instruction fault publishes neither successful-retirement nor
device time, as the CPU timing rules require.

## HLT And Host-loop Finding

After `HLT` retires once, `core_machine_run()` returns
`CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`.  On the next call,
`core_machine_cpu_execution_refresh()` suppresses `ExecIns()` while the halt
flag remains set and performs only `ExecInt()`.  If no interrupt cleared the
halt flag, the enclosing run loop nevertheless falls through to the normal
instruction-cost and scheduler-publish block before testing `flagHalt` again.
It therefore charges the retained instruction state and advances all device
callbacks without a new successfully executed instruction.

The normal VM runner always supplies an instruction-only budget and, on this
stop reason, sleeps one host millisecond then calls `core_machine_run()` again.
The resulting halted-device cadence is consequently determined by repeated
host-loop calls, not by a source-labelled virtual-clock input or board event.
The current core has no separately classified external-unavailability time
publisher; any future such non-executing refresh must be prevented from
falling into the same shared cost publisher. DMA/HOLD itself is not a second
time publisher: it is consumed only from the arbitration callback after time
already advanced.

This disproves the prerequisite for treating the accepted Model-339 RTC/PIT
ratios as physical device progress in halted/non-retiring intervals. It does
not invalidate their rational conversion from a supplied nominal source tick.

## Owner And Next Repair

The shared owner is core-machine execution/time publication, not Model-339
profile code or a device. The next bounded implementation S must:

1. prevent a non-executing halted refresh from being represented as a retired
   instruction or implicitly advancing `elapsed_ticks`; and
2. introduce one explicit production virtual-time advancement boundary, with
   a bounded source-tick input, through which the timeline and every existing
   clock consumer advance exactly once.

That boundary must be usable by product composition and deterministic replay;
it must not be a test-only injector, a platform adapter mutation of guest
state, or an implicit conversion of host sleep duration into an IBM board
fact. A later S must select and validate the Model-339 composition policy that
supplies such source ticks, including paused, reset, HLT, interrupt-delivery
and DMA/HOLD replay. The repair belongs in T375 because it is the prerequisite
for its device-phase conclusion, while its shared core mechanism remains
reusable by later DeskPro and XT work.

No ROM, media, binary, third-party source or reference-runtime value was used.
This is a repository-state audit, not a device timing implementation or an L3
claim.
