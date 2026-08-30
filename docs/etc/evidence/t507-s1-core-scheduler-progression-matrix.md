# T507 S1: Core Scheduler Progression Matrix

M5:T507:S1:SCHEDULER-PROGRESSION-MATRIX:OK

## Method and fixed universe

The universe is the complete current progression surface found by tracing the
sole elapsed-tick publisher, its scheduler, every time-observation query, and
both VM waiting calls. It contains the thirteen Core rows below plus four VM
boundary rows. Controller ledgers remain the authority for timing facts.

Read-only local 86Box, Bochs, MAME and PCjs review corroborates architecture
only: their timers are subordinate to CPU or virtual-time progress and select
the earliest armed event. PCjs has browser-host policy and supplies no NXVM
timing fact. T507 accepts the one-clock/earliest-event lesson and rejects
imported timer frameworks, device-local queues, host-generated cycles and
compatibility shortcuts.

## Frozen Core matrix

| Row | Current owner and route | Next change / lifecycle | Disposition | S2 receiver and regression owner |
| --- | --- | --- | --- | --- |
| C1 | CPU successful retirement -> sole elapsed publisher | Selected instruction and declared bus cost; faulting round publishes none. | Completed Core delta. | Retain the only writer; split only at existing earliest deadline. instruction-timing smoke. |
| C2 | CPU external wait | Bus-ready change or declared remaining wait; reset cancels pending retirement. | Existing tick-by-tick Core route, not HLT fast path. | Retain until board contract supplies a deadline. competition smokes. |
| C3 | Timeline | Earliest token; cancellation removes it; equal due values preserve insertion order. | Exact Core-private deadline. | Retain sole ordered token owner. timeline smoke. |
| C4 | PIT / auxiliary PIT | Any armed counter next OUT edge; reset/control write replaces state. | Qualified or labelled-ratio deadline through copied clock. | Call only when due; retain arbitration-before-readiness order. time and auxiliary PIT smokes. |
| C5 | RTC/CMOS | Periodic, update or alarm IRQ edge; write/reset re-arms or cancels it. | Qualified or labelled-ratio deadline through copied clock. | Call only when due. RTC owner smokes. |
| C6 | DMA | Qualified pending DREQ next service phase; withdrawal/reset clears it. | Model-339 deadline; other plans explicit L1/L2 boundaries. | Due-only qualified service; retain one bounded receiver otherwise. plan smoke. |
| C7 | PIC | Source mutation, EOI, acknowledge or cascade publication; reset clears it. | Immediate same-tick settlement, no clock. | Settle after owning producer; never poll PIC alone. cascade smokes. |
| C8 | FDC 8272A | Existing reset, seek, byte, NDMA and result due tick; reset/media change cancel it. | Owner absolute deadline or already-due transition. | Call only at due/current boundary; do not classify pending command/result as L1. FDC smokes. |
| C9 | HDC | Command/read/write phase; reset/cancel returns idle. | L1 causal receiver; no sourced service duration. | Keep one Core bounded path; no deadline, guessed latency or second HDC path. HDC smokes. |
| C10 | KBC / XT keyboard | Minimum serial, response, typematic or BAT count; input/reset changes state. | Existing clock-domain deadline. | Due-only owner advancement, never VM timer. keyboard smokes. |
| C11 | VADP | No current sourced guest-observable raster deadline; port/memory write is immediate state mutation. | Idle delta consumer. | Do not invent deadline; skip during HLT advance. VADP capture smokes. |
| C12 | Model-40 D4 refresh hold | One logical hold transaction; reset/cancel clears pending state. | L1 causal receiver, no proved duration. | Retain Core bounded path until queued D4 task changes source disposition. D4 smoke. |
| C13 | Optional execution provider | Completed provider-clock delta; reset separate. | No product provider advances time. | Preserve completed-delta callback after Core settlement; no recurring maintenance. external-time smoke. |

## VM boundary matrix

| Row | Owner and direction | Required retained rule | S3 regression |
| --- | --- | --- | --- |
| V1 | Standard pacing | Compare completed copied Core progress with monotonic-host budget; wait only when Core is ahead. | speed-policy smoke |
| V2 | Turbo | Do not wait; ask same Core deadline/L1 capability and never pass a tick count. | speed-policy smoke |
| V3 | Runner HLT | Call the single waiting operation, then yield only if Core reports no progress. | runner/display cadence smoke |
| V4 | Session reset/speed change | Reset only host comparison origin; Core reset remains Core-owned; speed cannot reconfigure devices. | session lifecycle and console smokes |

## Implementation predicate

S2 may replace the present three unconditional group calls only if every same
guest tick retains this order: timeline settlement; arbitration (D4/DMA/PIT);
readiness (FDC/HDC/RTC); peripheral (keyboard/VADP); completed provider delta.
A row is skipped only when its own query proves no transition is due. C2, C9
and C12 remain explicit receivers; no fast path may hide them.

S3 may change Standard/Turbo only after S2 proves the sole Core progression
route. The modes retain identical Core event order: Standard may wait after
completed progress or before Core selected deadline, Turbo does not wait.
Neither may write guest time from host elapsed time.

The predicate is exhausted only when C1--C13 and V1--V4 have one implemented
path or named lower-tier receiver, with reset, withdrawal and same-tick
behavior proved by its regression owner.
