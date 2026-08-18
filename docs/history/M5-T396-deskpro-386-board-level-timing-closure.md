# M5 T396: DeskPro 386 Board-Level Timing Closure

## Task Record

T396 establishes the source-backed board-clock and visibility contract required
before any selected DeskPro 386 Model 40 physical timing publication. It
consumes T394's CPU qualification boundary and T395's multi-route prerequisite,
but does not itself accept firmware, media, device-service or L3 timing.

## S1 Source And Owner Ledger

S1 records D3PE hardware facts and current Core/VM owners without converting
them into elapsed ticks. The [ledger](../etc/evidence/t396-s1-board-timing-source-owner-ledger.md)
confirms that C0 qualification alone does not establish a retirement-to-16-MHz
unit or phase, so the Model-40 neutral plan remains mandatory.

The completed publisher/consumer sweep makes the boundary finite: Core has six
neutral rational domains and three ordered due-tick callbacks, while VM's two
Model-40 construction routes select deterministic time. It retains logical
order and reset behavior, but transfers every physical value or phase to a
future source-backed board-clock or physical-device/firmware receiver.