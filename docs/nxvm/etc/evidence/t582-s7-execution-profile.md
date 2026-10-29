# T582 S7: post-optimization execution profile

The current x64 PC110 public-factory probe sampled early host seconds 5--25
and late seconds 220--240 using task-owned copied CMOS and overlay media.
The run exited successfully. It recorded 6,512 in-image observations, 18,229
outside-image observations and 10,749 failed context reads; the sampler includes
multiple threads, and observations are not CPU-time percentages.

The largest individual function bucket was CPU execution refresh (226 early,
232 late). Memory route resolution was 164 early and 74 late. PIC selection
was 127 late; keyboard input collection 104 early/108 late. CPU timing
selection was 78 early/98 late. These are indicative counts, not proof of
which work can be eliminated. Previously optimized board DMA advance still
appeared (59 early/66 late), and no cost estimate is inferred from samples.

PIC source has a pure pending-request predicate inside each of two priority
scans. A private candidate returns the existing no-request result before the
scan if all pending requests are masked or absent; neither pending calculation
nor the scan mutates state. A matched 10M-instruction dual-width benchmark is
running. The empty-mask path must preserve invalid-state and delayed-unmask,
cascade, priority, SFNM and edge-trigger semantics; existing PIC tests are the
regression owner. No production code has been changed.

## PIC empty-request candidate: not landed

The 10-million-instruction private comparison ended at identical 79,964,294
guest ticks and PC 0003dbc5. x64 baseline/candidate were 23,396/20,749 ms;
x86 were 26,676/29,893 ms. Each is a single sequential pair; the opposite
directions prevent claiming a stable cross-architecture improvement. The
prototype is not admitted to production. The existing PIC contract, command,
priority, cascade and lifecycle tests remain owners of any future repair.

## KBC no-repeat candidate

The Board keyboard-input snapshot asks keyboard_get_signals for scanning and
repeat_bytes, then always asks ticks_until_repeat. The latter returns invalid
state if repeat_count is zero or scanning is disabled; exactly those conditions
make repeat_bytes zero. A private candidate calls ticks_until_repeat only when
repeat_bytes is nonzero. The designated initializer already zeros repeat_ticks
and repeat_pending, matching the existing invalid-state path. It adds no API,
state or timer and preserves the existing repeat route when active.

At 10M instructions and identical 79,964,294 ticks / PC 0003dbc5, x64
baseline/candidate were 21,678/21,084 ms, x86 25,258/21,019 ms. These are
single ordered startup pairs and do not establish sustained or whole-product
gains. No production change is made during S7. A following S should examine
startup, scanning enable/disable, key-repeat deadlines, BAT and FIFO behavior
using existing tests and qualify all receivers before any delivery.
