# ux-base

`ux-base` is the shared UX foundation. It depends only on `base` and provides
copied frame/input values, source-local hotkey matching, and private mailbox
mechanics to `ux-window` and `ux-console`.

Its internal component emission helper may let a leaf filter matcher output,
but source attribution and chord matching always remain in this component.
This permits a frozen Window to discard ordinary content input while forwarding
registered hotkeys without creating a second matcher path.
