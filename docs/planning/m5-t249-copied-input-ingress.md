# M5 T249: Copied Input Source And Composition Ingress

**Status:** S1 active.

## Goal

Move only the neutral host-input value and source/sink vocabulary to
`core/platform`, while keeping the ingress queue, ordering, profile mapping,
and guest delivery in composition/VM. The change must preserve current NXVM
Console and window keyboard/mouse behavior.

## Subtasks

### S1: Contract And Call-Site Inventory

Define one copied `core_platform_input_event` value, event-kind set, source
call boundary, callback-thread rule, and lifetime/freeze rule. Inventory every
Win32/Linux Console/window producer and every current VM consumer. The source
may only submit copied data; it must never touch KBC/AUX, BIOS/BDA, guest RAM,
or a session control flag directly.

#### S1 Contract

`core/platform/input_interface.h` will define a copied
`core_platform_input_event` with only two admitted kinds:

- `CORE_PLATFORM_INPUT_KEY`: `scan_code`, `virtual_key`, and `pressed`;
- `CORE_PLATFORM_INPUT_RELATIVE_MOUSE`: `delta_x`, `delta_y`, and `buttons`.

`core_platform_input_source_submit()` receives the value by pointer but copies
no caller-owned storage into retained state. Its sink receives the event only
for the duration of that call. `core_platform_input_source_stop()` is
synchronous: after it returns, no source callback remains in flight and later
submissions are rejected. The source owns neither a queue nor guest state.

Composition binds a single sink whose only effect is to convert the copied
event into the existing `vm_platform_request` and enqueue it in
`vm_platform_request_transport`. The existing command-boundary observer
remains the sole dequeue and guest-delivery point.

#### S1 Producer And Consumer Inventory

| Path | Role | Disposition |
| --- | --- | --- |
| `vm/platform/win32/win32.c` | Shared Win32 key/mouse producer; F9 additionally reports the retained VM stop event. | Submit a copied core event; retain the VM-only F9 run-handle policy. |
| `vm/platform/win32/win32con.c` | Console adapter producer. | Submit through the shared Win32 producer. |
| `vm/platform/win32/win32app.c` | Window adapter producer. | Submit through the shared Win32 producer. |
| `vm/platform/linux/linuxcon.c` | Linux Console producer. | Submit copied key events through the core source; no host queue. |
| `vm/composition/session/lifecycle.c` | Current keyboard/mouse sink and ingress producer. | Replace separate sinks with one composition input sink; it only enqueues copied `vm_platform_request` values. |
| `vm/platform/vm_request_transport.[ch]` | Session-owned ordered ingress and execution-boundary consumer. | Remains VM/composition-owned; no source move. |

The active source is created by `vm_session`, bound after its request transport
has a consumer, stopped before that transport is closed, and destroyed as
session storage is finalized. This is the only lifecycle ordering permitted by
T249.

**S1 marker:** `M5:T249:S1:INPUT-CONTRACT:OK`.

### S2: Move The Neutral Vocabulary

Implement the core/platform input value and source/sink contract, adapt the
existing VM transports to it, and keep the existing composition-owned request
queue as the sole execution-boundary consumer. Do not add a host queue or
change keyboard mapping, chord ordering, mouse packets, or product input mode.

### S3: Verify The Boundary

Add focused copy/lifetime/source-stop coverage for Console and window adapters;
retain keyboard, mouse AUX/IRQ12, DOS typing, CGA/EGA, FDD/HDD boot,
Console/debugger, current GCC/CTest, and artifact verification. Produce the
next developer artifact only if S2 changes runnable source.

## Risks And Stops

Stop for owner direction if the proposed core contract requires a host capture
policy, a window/Console decision, direct guest mutation, raw guest memory,
layout/scan-code policy, or a second queue/run loop. `core/platform` owns only
the copied value and neutral source call; composition owns enqueue/dequeue
order and VM/profile owns interpretation.

## Applicable Evidence

Use the T248 admission record, module layout, contracts, coding/source policy,
execution policy, and M5 closure checklist. The expected S1 marker is
`M5:T249:S1:INPUT-CONTRACT:OK`.
