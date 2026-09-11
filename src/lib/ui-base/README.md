# ui-base

`ui-base` is the shared UI foundation. It depends only on `types` and provides
copied frame/input values, source-local hotkey matching, and private mailbox
mechanics to `ui-window` and `ui-console`.

Its internal component emission helper may let a leaf filter matcher output,
but source attribution and chord matching always remain in this component.
This permits a frozen Window to discard ordinary content input while forwarding
registered hotkeys without creating a second matcher path.
