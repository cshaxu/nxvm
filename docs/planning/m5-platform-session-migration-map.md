# M5 Platform Session Migration Map

## Baseline

Runtime baseline: `20821e3` (T78). The FDD DOS-prompt gate is mandatory after
each runtime cutover. An earlier uncommitted combined migration faulted with
`0xC0000005` and was discarded.

| Cutover | Current entry and implicit state | Required explicit owner | Gate |
| --- | --- | --- | --- |
| C1 | `win32con` kernel/display/input threads use global execution and keyboard sinks. | A Console run context passed as each thread `LPVOID`; it references one composition-owned transport set. | Console command smoke, keyboard transport smoke, FDD prompt. |
| C2 | `win32app` window procedure uses globals and thread statics. | A window run context stored in `GWLP_USERDATA` and passed to kernel/display threads. | Window launch/close, keyboard input, FDD display. |
| C3 | Linux Console follows the same global execution/input shape. | The matching POSIX run context passed to pthread entry points. | Static ownership scan; deferred POSIX runtime gate. |
| C4 | `core_platform_display_frame` is one process-global mailbox. | One VM presentation mailbox owned by the platform run context; composition publishes to an explicit callback. | Two-context mailbox isolation, FDD display. |
| C5 | `platform.flagMode`, Win32 Console handles, and window handles are globals. | Console/window run contexts own their handles and mode; product composition owns their lifetime. | No cross-context overwrite, retained NXVM mode command. |
| C6 | `core_product_wait_bind` is lifecycle-global. | Keep it out of machine lifecycle; each host thread enters its own scoped wait provider only after C1/C2 owns all entry points. | Core wait scope smoke, FDD prompt. |

## Ordering Rules

- One cutover at a time, beginning with C1. Do not change the window path while
  proving Console.
- A context contains adapters and host-owned state only; it never copies CPU,
  RAM, device, or profile state.
- The legacy path remains until the active cutover passes its focused and FDD
  gates; then remove only the replaced global calls.
