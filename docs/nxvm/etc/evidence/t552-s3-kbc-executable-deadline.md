# T552 S3 KBC Executable Deadline Repair

## Contract

`x86_kbc8042_ticks_until_event()` must report the next time at which
`x86_kbc8042_advance()` can make progress that is observable through the KBC
output path. It must not advertise a zero-tick reply while a deliverable
keyboard serial byte still has a positive delivery delay. It must also not
hide a delayed reply merely because an existing FIFO byte is present when the
remaining FIFO capacity can accept that reply.

## Change

Two private predicates make the existing publication condition single-owned:

- `x86_kbc8042_keyboard_serial_delivery_pending()` identifies scan-enabled,
  delivery-enabled queued keyboard serial work;
- `x86_kbc8042_response_can_publish()` identifies a poll-ready reply that is
  not preempted by that serial work and fits in the FIFO.

`advance()` and `ticks_until_event()` consume the same response predicate.
The deadline path considers serial timing only when queued serial work can
actually drain into an empty FIFO. Thus stale serial timing with no queued
byte, or an output-consumption blocker, is not fabricated into a time wake.

No public API, state field, queue, board clock, scheduler branch or profile
condition changed.

## Regression Matrix

The KBC owner-local contract now proves:

| State | Expected outcome |
| --- | --- |
| Two queued translated scan bytes, serial delay 3, zero-delay `20h` reply | Query returns 3, then 1; scan bytes publish before `43h`; no false zero. |
| Same serial backlog, then keyboard `F5h` disables scanning | `FAh` reply publishes immediately; disabled serial input no longer blocks it. |
| Existing FIFO scan byte, no serial backlog, delayed `20h` reply | Query returns response delay and `43h` queues behind the scan byte. |
| FIFO filled to capacity, another delayed reply pending | Query returns `INVALID_STATE`; output consumption, not fabricated time, is required. |

The first case retains keyboard Set-2 to Set-1 translation (`1Ch/32h` becomes
`1Eh/30h`). Controller reset is exercised between cases while timing
configuration preservation remains covered by the existing configuration
contract.

## Caller Disposition And Verification

`board-base/board_deadline.c` already delegates the KBC value through its
declared KBC clock and treats zero as immediate. Since the chip now supplies a
truthful value, no board/scheduler production change is correct. S4 owns the
single cross-owner qualification route.

- Core manifest and corpus gates: pass.
- KBC controller, serial-cadence and board-controller focused units: 3/3 on
  x64 and 3/3 on x86.
- Eight PC artifacts: rebuilt from their original profile-specific Ninja
  directories; each build reported its expected architecture.

No external firmware boot or desktop interaction is claimed by S3.
