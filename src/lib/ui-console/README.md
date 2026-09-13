# ui-console

`ui-console` depends on `types`, `console`, and `ui-base` only. It owns one raw Console lifecycle,
creates its logical Console object, and publishes copied text frames through
that object. It never opens, registers, or renders native Console I/O.
The public component contract is cross-platform. This corpus currently has a
supported Win32 implementation only; the Linux leaf is an intentional
`LIB_STATUS_UNSUPPORTED` placeholder, not a claimed Linux presenter.

Combined raw key/UTF-16 records enter ui-base's shared record normalizer.
The Console adapter does not synthesize characters or decide text fallback.

Before a new native input stream starts, host synchronously delivers INPUT_RESET.
The callback clears the existing held-key ledger, unfinished UTF-16 and mouse
baseline; registrations and pending output survive. It neither renders nor
retires the source. The next mouse record establishes a zero-motion baseline.

Successful host binding delivers a neutral Console activation event. The callback
only signals the existing worker's wake; it neither renders nor reenters host.
The worker draws a pending mailbox frame, or does nothing if empty. NOT_CURRENT
does not acknowledge a frame: the next publication or activation wakes it again.
STOP retains priority over frame consumption. No separate activation renderer,
polling retry or application republish requirement exists.
