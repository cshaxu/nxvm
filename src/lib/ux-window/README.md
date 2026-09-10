# ux-window

`ux-window` depends on `ux-base` and `base` only. It owns one Window lifecycle and sends
copied UX events to the application queue entry. It never includes `host` or
makes product decisions.

The Window owns only host presentation mechanics: it draws a content text cursor
from copied position/shape/enabled frame fields and toggles that drawing every
250 ms while unfrozen. `freeze()` atomically prevents capture, releases any
capture, and holds the cursor at its current drawn state; `unfreeze()` resumes
the blink but waits for a later client-area click before it captures. Native
Raw Console cursor blinking remains outside this component.

Frozen Window keyboard transitions still enter the shared registered-hotkey
matcher. A matched hotkey is delivered to the application sink; ordinary
key/text/mouse input is silently consumed and is never retained for a later
unfreeze. The Window does not interpret a hotkey identifier.

The public component contract is cross-platform. This corpus currently has a
supported Win32 implementation only; the Linux leaf is an intentional
`LIB_STATUS_UNSUPPORTED` placeholder, not a claimed Linux presenter.
