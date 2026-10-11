# T552 S4 — KBC Caller Deadline Qualification

## Bounded Contract

The KBC owns whether a reply is executable and reports a device-clock
deadline. Board Base alone converts that deadline through the configured KBC
clock. Core x86 alone selects it as a machine deadline and advances the
registered Board Base peripheral callback. No App and no second scheduler
participate in this route.

## Route Reviewed

```
x86_kbc8042_ticks_until_event
  -> core_machine_kbc_ticks_until_event
  -> core_machine_board_deadline_observe
  -> core_machine_capture_time_observation
  -> core_machine_advance_to_next_deadline
  -> core_machine_board_peripheral_advance
  -> core_machine_kbc_advance
```

`board_deadline.c` preserves a positive converted KBC deadline and treats
zero as immediate only. `machine_scheduler.c` owns the absolute deadline and
calls the sole Board peripheral callback after advancing time.

## Regression

`machine_board_timing_qualification_smoke` now builds a physical-time AT
board, queues two native bytes behind a serial delay of three KBC ticks and
issues controller command `20h`. It proves:

1. KBC returns device deadline 3, not a fabricated immediate reply;
2. Board Base converts the same deadline through `kbc_clock`;
3. Core reports that converted value as the next machine deadline;
4. `advance_to_next_deadline()` invokes the actual peripheral route and
   makes the first translated scan byte (`1Eh`) readable.

This is cross-owner contract proof, not a duplicate controller algorithm
test. It passes on x64 and x86.
