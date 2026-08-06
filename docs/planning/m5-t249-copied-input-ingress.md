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
