# VM Runner Presentation Cadence

## Purpose

Remove the instruction-quantum-coupled full display snapshot path that slows
all runnable VM profiles, particularly graphical guest workloads.

## Scope

Retain the runner's bounded 256-instruction control quantum, but make display
capture and mailbox publication a VM-owned, host-clock presentation
cadence. A mode change and explicit lifecycle publication remain immediate.
The cadence must consume the existing Core display owner and copied-frame
boundary; it must not create a second display state, profile branch, or guest
time source.


## Exclusions

Do not change CPU instruction timing, Core device timing, guest clock ratios,
runtime debugger behavior, frame contents, Console grammar, or the default
real-time policy. An explicit turbo policy is separate work after this shared
presentation cost is measured.

## Acceptance

Default PC/AT, IBM 5170 Model 339 and Model-40 use the same runner cadence.
Normal execution no longer creates a maximum-size display snapshot once per
256 guest instructions; display mode/lifecycle publication remains immediate,
and a focused cadence regression plus the current gate pass. The rebuilt
stripped Release current artifact records a before/after DOS-start benchmark.
