# M5 T445 S1 VM Native Display Resource Lifetime

T445 S1 repairs the two admitted native display lifecycle defects without
changing guest display production, copied-frame ownership, or a Core boundary.

## Owner And Mechanism

`w32adisp_context` is the sole owner of its persistent window DC.  It retains
the `HWND` received at initialization and pairs its one `GetDC` with one
`ReleaseDC` in `w32adispFinal`.  `win32app_display_renderer_finalize` is the
single display-thread cleanup operation: normal window closure invokes it
from `WM_CLOSE`, and pre-message-loop cancellation invokes it before
`DestroyWindow`.  The outer run-handle finalizer no longer frees the renderer
or writes its state.

`linuxcon_display_thread` owns curses for its whole lifetime.  Failed
initialization already rolls back with `endwin` in that thread; successful
initialization now finalizes before the thread returns.  The cross-thread
`terminal_initialized` flag and the outer-finalizer curses path are deleted.

## Similar-Issue Sweep

The following tracked-source sweep was run after the repair:

```powershell
rg -n "GetDC\(|ReleaseDC\(|initscr\(|endwin\(|newterm\(|delscreen\(" src tests CMakeLists.txt cmake docs
```

Production dispositions:

| Hit | Disposition |
| --- | --- |
| `src/vm/platform/win32/w32adisp.c` | One persistent DC owner; T445 pairs its sole acquisition and release. |
| `src/vm/platform/linux/linuxcon.c` | One curses initializer, one initializer-failure rollback, and one normal display-thread finalization. |
| `tests/platform/vm_w32_text_palette_smoke.c` | Independent test-owned DC pair; it is not the renderer resource. |
| `newterm` / `delscreen` | No tracked hit. |

The mechanically detectable rule is retained by
`cmake/verify_t445_vm_native_display_resource_lifetime.cmake`: it rejects an
unpaired Win32 owner, renderer cleanup in the outer finalizer, cross-thread
Linux terminal state, and curses cleanup outside the display thread.

## Verification

- Fresh Windows/MinGW configuration in `build/t445-clean` built
  `vm-0-5-0445` from 103 compile/link steps and passed the T445 lifecycle
  verifier.
- The normal build produced `build/output/nxvm_0_5_0445.exe`, SHA-256
  `2D369C04936536F987ECD59826037949E70DF6BCC33241C131530E4BA724808D`.
- Focused Win32 CTest coverage passed 2/2:
  `current.vm-platform-surface-context-smoke` and
  `current.vm-w32-text-palette-smoke`.
- `current-fast-smokes-gcc` completed, and the final `current-gates-gcc`
  execution completed its 292 `current-gate` CTests without a failure marker;
  all 70 specialized gates passed, including
  `M5:T445:S1:NATIVE-DISPLAY-RESOURCE-LIFETIME:OK`, documentation governance,
  and the T382 process-tree self-test.  An earlier concurrent T382 attempt
  missed its PID marker; its isolated rerun passed before the final clean gate.
- The current host is Windows/MinGW.  It compiles and runs the Win32 adapter,
  but cannot execute the Linux curses adapter.  Linux verification is limited
  here to the owner-local source gate and existing Linux contract gates; this
  records no Linux runtime-support claim.

## Minimalism Accounting

The source change removes three Linux lifecycle-state/cleanup lines and five
outer Win32 cleanup lines; it adds one narrowly named display-thread cleanup
operation, one stored native window handle needed for the API pair, and one
static recurrence check.  No renderer abstraction, callback, compatibility
path, or additional mutable state owner was introduced.
