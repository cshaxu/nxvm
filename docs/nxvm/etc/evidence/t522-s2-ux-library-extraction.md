# T522 S2 Shared UX Library Extraction

## Delivered Surface

`src/lib/platform/win32` no longer exists.  The admitted source is now split
by responsibility:

| Surface | Owner and boundary |
| --- | --- |
| `src/lib/ux/` | Platform-neutral copied `ux_frame`, `ux_event`, action registry, router, binding and mailbox. Root headers name no Core/VM/product type or native SDK type. |
| `src/lib/ux/win32/` | Native Win32 input translation, geometry, mouse capture, Console/Window loops and the one `ux_win32_run` selector. The Console lease is the only process-global UX state; each Window run allocates one private context. |
| `src/lib/ux/linux/` | ncurses Console presenter consuming the same binding, frame mailbox and events. It has its own pointer-free Console lease. There is deliberately no Linux Window backend. |

The Core copied display mailbox remains Core's producer boundary. A product
will perform its sole representation conversion into `ux_mailbox`; the library
does not obtain a Core, VM, profile, executor, device, memory or session
pointer. The old Win32-native mailbox and unreferenced event queue were
deleted rather than retained as parallel owners. `ux_mailbox` is the one
latest-copied-frame owner in the library and assigns its own generation.

## Native Routing

`ux_win32_run` is the only native Console/Window selector.  It consumes a
validated `ux_binding`, uses the root router, and transitions only on typed
results. Console and Window callbacks emit the one `ux_event` type and use
the one root action registry. Window rendering, input normalization, mouse
capture, title and GDI resources reside in a heap `ux_win32_window_context`;
the thread-local dispatch pointer is not state storage and exists only while
that window's loop is active.

NXVM's present VM platform remains unchanged in this S. This is intentional:
S4 must bind the product to this surface and delete the old production loops
in the same delivery; no compatibility selector exists.

## Provenance And Verification

The S1 source provenance remains the owner-provided SoftPC revision
`ff3b21a36180ff8d3b7efa0feac7bc068c7224fc`; this S derives a new NXVM
layout and API, so it is not represented as a byte-identical SoftPC subtree.
`src/lib/MANIFEST.sha256` records every current library source after CRLF
normalization. `unit.ux-library-manifest` verifies it.

On the Windows x64 Debug host:

- `ux-contract`, `ux-win32-native`, and `ux-contract-smoke` compile.
- focused `unit.ux-contract-smoke` and `unit.ux-library-manifest` pass.
- the complete repository-only unit suite passes: `306/306`.
- `git diff --check` passes.

No Linux compiler/runtime is configured on this Windows host, so the Linux
source is not claimed as locally compiled or accepted at runtime. Its native
build target is selected only on non-Windows CMake configurations with the
same required Curses dependency as the retained NXVM Linux platform.
