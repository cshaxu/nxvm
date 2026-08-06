# M5 T249: Copied Input Source And Composition Ingress

**Status:** Complete.

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

#### S2 Implementation Evidence

- `src/vm/platform/input.[ch]` was moved with Git history to
  `src/core/platform/input.[ch]` and renamed as the public
  `input_interface.h` contract.
- `vm_session` now owns one `core_platform_input_source`; its single sink in
  `lifecycle.c` converts a copied event into the unchanged
  `vm_platform_request_transport` ingress queue.
- Win32 Console/window and Linux Console producers submit core input values.
  The retained F9 run-handle report remains VM product policy. VM-only Console
  buffer flush moved to `vm/platform/input_flush.[ch]`.
- The source is initialized only after the request transport consumer is
  bound, and is synchronously stopped before transport close.
- `core-platform-input-source-smoke` checks value routing and post-stop
  rejection. Existing keyboard, AUX mouse, DOS typing, and host-cancellation
  coverage now uses the same source.
- `current-gates-gcc` passed with 85/85 CTest cases and updated static markers
  `M5:T249:S2:INPUT-INGRESS-BOUNDARY:OK`,
  `M5:T249:S2:INPUT-TRANSPORT-SURFACE:OK`, and
  `M5:T249:S2:AUX-MOUSE-BOUNDARY:OK`.

#### Similar-Issue Sweep

**Class:** VM-specific keyboard/mouse transport facades crossing a reusable
host-input boundary. **Scope:** tracked `src`, `tests`, CMake, and relevant
static gates. Query:
`rg -n "vm/platform/input\\.h|vm_platform_(keyboard|mouse)_(receive|transport)|keyboard_transport|mouse_transport" src tests cmake CMakeLists.txt`.

All production hits were either migrated to `core_platform_input_source`, kept
as the VM-only console flush, or were static-gate names. No VM-specific input
transport remains in a production path; KBC/AUX and request transport remain
intentionally VM-owned.

### S3: Verify The Boundary

Add focused copy/lifetime/source-stop coverage for Console and window adapters;
retain keyboard, mouse AUX/IRQ12, DOS typing, CGA/EGA, FDD/HDD boot,
Console/debugger, current GCC/CTest, and artifact verification. Produce the
next developer artifact only if S2 changes runnable source.

S3 allocates developer artifact revision `0.5.0248`, updates the configured
current artifact target, reruns the full gate, records its SHA-256 and banner,
then closes T249 without changing product behavior.

#### S3 Closure Evidence

- `vm-0-5-0248` builds the current source with GCC `-Werror` and copies
  `build/output/nxvm_0_5_0248.exe`.
- SHA-256:
  `6B0CAD6A3DB97D794963E052E3A70C08C2080614047AB7AC5355CCE6E07E8F35`.
- Runtime identity is the configured NXVM developer banner for version
  `0.5.0248`; no Console, debugger, boot, keyboard, or mouse UX policy changed.
- `current-gates-gcc` passed against `vm-0-5-0248`: all static ownership gates
  passed and 85/85 CTest cases passed, including Console/window adapter,
  KBC/AUX, DOS prompt, graphics, FDD/HDD, debugger, and product-session
  coverage.
- The S2 similar-issue query has no production occurrence of the retired
  VM keyboard/mouse transport API. The remaining VM input code is the explicit
  Console flush policy and does not cross into core.

**Closure marker:** `M5:T249:S3:INPUT-BOUNDARY-CLOSED:OK`.

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
