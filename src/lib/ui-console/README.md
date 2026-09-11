# ui-console

`ui-console` depends on `types`, `console`, and `ui-base` only. It owns one raw Console lifecycle,
creates its logical Console object, and publishes copied text frames through
that object. It never opens, registers, or renders native Console I/O.
The public component contract is cross-platform. This corpus currently has a
supported Win32 implementation only; the Linux leaf is an intentional
`LIB_STATUS_UNSUPPORTED` placeholder, not a claimed Linux presenter.
