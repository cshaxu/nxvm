# kvm-window

Text cells select primary/secondary bitmaps through explicit glyph-bank values;
foreground/background palette indices are independent, with no device bit layout.

`kvm-window` depends on `types` and `kvm-base` only. It owns one Window lifecycle and sends
copied KVM events to the application queue entry. It never includes `console-broker` or
makes product decisions.

`kvm_window_frame` contains either common text fields with two bitmap-font banks,
or Window-owned complete indexed graphics and palette. It contains no
Console character map. Copies include only the active text or graphics extent.
Window compares resolved colours with its existing RGB surface on consumption,
updates changed pixels and invalidates their enclosing rectangle. Skipped frames
need no damage history. Both text and graphics decode colours directly into the
same pixel comparison helper, without an intermediate buffer. First frames
and recreated surfaces require full invalidation; same-size mode transitions
compare actual pixels. Cursor position/shape/visibility changes invalidate the
old/new overlay rectangles separately. Stride and palette come from each frame.
Native invalidation accumulates until paint; exposure redraws the surface.
Successful acknowledgement is generation-bound. No extra previous-frame cache.

Win32 mailbox notifications use SendNotifyMessage to the owned Window, not an
Event-to-message bridge. Notifications coalesce; requests stay in the private
control FIFO/latest-frame mailbox. Native move/size/menu loops dispatch the
same notification. The worker-thread consumer prevents recursive drains and
stops before further controls/frames on terminal failure. STOP cancels native
modal interaction before the worker's existing destruction/retirement tail.
This changes only leaf-support implementation, not application APIs.

The Window owns only host presentation mechanics: it draws a content text cursor
from copied position/shape/enabled frame fields and toggles that drawing every
250 ms while unfrozen, using one owned native Window timer. No outer-loop blink
timeout remains; native move/size/menu loops dispatch the same timer message.
Timer startup/transition failures use the existing failure path. Native Window
timer delivery advances the original 250ms phase grid, including missed periods;
it does not restart a new 250ms interval at every delayed message. Text glyphs
and cursor geometry use the same frame font height (8/14/16 rows, 16-row storage
stride), so a short glyph is not drawn above an unrelated 16-row cursor cell.
Native Window destruction removes its timer; stale queued ticks cannot bypass
frozen/due checks.
Checked destroy uses the shared 5000 ms join contract; a failed join retains the
component and all callback dependencies, never freeing a live worker.
`freeze()` submits one control; the Window worker prevents capture, releases any
capture, and holds the cursor at its current drawn state; `unfreeze()` resumes
the blink but waits for a later client-area click before it captures. A real
frozen-to-unfrozen transition requests foreground/focus once, subject to host
policy. Repeated unfreeze changes neither focus nor blink phase/deadline. Native
Raw Console cursor blinking remains outside this component.

Frozen Window keyboard transitions still enter the shared registered-hotkey
matcher. A matched hotkey is delivered to the application sink; ordinary
key/text/mouse input is silently consumed and is never retained for a later
unfreeze. The Window does not interpret a hotkey identifier.

Relative mouse scaling retains signed integer remainders per axis. Capture,
release and scale changes reset those remainders; copied event deltas remain
integers. While captured, only WM_INPUT produces motion: relative records supply
deltas directly; absolute records are mapped from 0..65535 to desktop units and
differenced, with the first sample establishing a baseline. Repeated absolute
positions produce no motion. Device, coordinate-space and geometry changes
reset the baseline. No pointer recentering or edge thresholds are used.
Legacy messages still supply buttons, but never duplicate motion. Native relative
motion bypasses system pointer acceleration; absolute input is still limited by
the positions supplied by its device. Neither path changes system mouse settings.
Capture acquires process-wide raw mouse registration only when none exists;
release removes only its own registration. Applications must serialize other
raw mouse registration changes with capture/release; an existing registration
causes capture to be declined, never stolen. Capture/clip loss, deactivation or
input failure releases capture; reentry requires another click.
All post-start worker exits stop input, release capture, close the
Window and retire the source once quiescent, before releasing storage. First
failure is reported on the detecting thread, not delayed until this cleanup.
Failed native disposal retains ownership and returns failure from join/destroy.
Paint acquisition, pixel transfer, cursor inversion and redraw-request failures
use that same terminal path, without retry loops. A successful BeginPaint is
always paired with EndPaint even if drawing fails. Empty/minimized content
does not require drawing. Missing transparent-cursor resources fail creation
rather than silently changing capture visibility.

The public component contract is cross-platform. This corpus currently has a
supported Win32 implementation only; the Linux leaf is an intentional
`LIB_STATUS_UNSUPPORTED` placeholder, not a claimed Linux presenter.

Pure rectangle/cursor calculations, pixel conversion and relative-motion scaling
live at the component root. Platform code only obtains native values, marshals
rectangles and performs native drawing, capture and message operations.
