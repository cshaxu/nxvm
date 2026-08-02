# M5 T60 S1 Platform Migration Decision

## Decision

No platform source file is moved in T60.

The T59 audit found that the policy-free shared mechanisms are already in
`src/core/platform`: copied display snapshots and host sleep. The remaining
Win32/Linux files mix host mechanics with VM execution, input, display,
threading, Console, and guest-control policy. Moving them now would create
false ownership clarity while preserving hidden VM coupling.

## Preserved Names

The symmetric host-surface names remain canonical:

- `win32app`: Win32 window/app host surface.
- `win32con`: Win32 console host surface.
- `linuxapp`: future Linux window/app host surface.
- `linuxcon`: Linux console host surface.

These names may appear under `core/platform/<os>` only after a future task
extracts policy-free host-surface mechanisms with explicit context objects.
Current VM adapters remain under `src/vm/platform/<os>`.

## Deferred Extraction Candidates

Future work may extract:

- Win32 raw keyboard probing, once VM scancode mapping and stop semantics are
  separated.
- Win32 console/window drawing primitives, once VM display globals and
  lifecycle callbacks are replaced by explicit host-surface context.
- Linux ncurses console drawing and polling primitives, once VM execution/input
  callbacks are separated.

Each extraction must keep VM lifecycle and guest-control policy in
`src/vm/platform` or future VDM policy in `src/vdm/platform`.

## Verification

- No `src/vm/platform` or `src/core/platform` file is moved in T60.
- `build/output/nxvm_0_5_0060.exe` is the task artifact.
- Retained Console smoke and source-DAG gates remain required.
