# ui-base

`ui-base` is the shared UI foundation. It depends only on `types` and provides
copied frame/input values, source-local hotkey matching, and private mailbox
mechanics to `ui-window` and `ui-console`.

Its internal component emission helper may let a leaf filter matcher output,
but source attribution and chord matching always remain in this component.
This permits a frozen Window to discard ordinary content input while forwarding
registered hotkeys without creating a second matcher path.
Each instance owns its ledger; modifier snapshots may satisfy chord modifiers
without corresponding makes in that instance. This accepted behavior does not
merge ledgers or require a second modifier-state table.

`worker_interface.h`, `mailbox_interface.h`, `mailbox_wake_interface.h` and
the root `input_interface.h` are shared leaf-support contracts, not
application entry points. Each leaf owns its own state; Window capture is
Window-local. Outstanding suppressed keys survive subsequent matched chords
until their breaks. One ordered held-key ledger gives every physical key one
pending, delivered or consumed disposition. Repeats reuse it, releases remove
it, and storage growth failure is terminal rather than dropping key state.

No leaf includes anything under `ui-base/win32` or `ui-base/linux`. The root
input declarations expose copied input normalization with common surrogate,
recovery and delivery state. Same-shape platform operations decode raw keys and
text layout; Linux terminal text uses TEXT rather than inventing physical keys.
Window message decoding and key-state queries belong only to ui-window.
Mailbox wake operations remain selected by the build. Frame damage accumulates
until successful consumption even when intermediate complete pixel frames are
replaced. Capture copies without consuming; acknowledgement clears pending only
if that publication is still latest. Failed output keeps it pending without
restoring a stale copy over newer content or signalling another retry.

Mouse and close events never flush a keyboard prefix. Mismatch/keyboard release
replays pending keyboard events in order; keyboard/mouse interleaving is not
buffered. A rejected input sink closes the source and clears pending state;
only the worker's quiesced exit reports failure and retirement. STOP/fault closes
frame and control admission by taking the independent frame then control locks;
ordinary control never waits for frame copying. FIFO processing up to
STOP is unchanged. Window's sole final filter discards ordinary frozen input,
not registered hotkeys or lifetime events.

Physical key identity is scan code + EXTENDED flag, with logical key fallback
only when the scan is absent. A held key retains its original logical key across
lock/layout changes. Both leaves share this ledger. Ordinary keys and modifiers follow the same lifetime: a
delivered make always retains its break and never becomes consumed later.
The ledger is released at retirement; destroy also handles workerless cleanup.

Both leaves submit copied keyboard records through ui_keyboard_submit_record:
Window supplies separate transitions/characters, Console combined records.
The common entry chooses physical versus UTF-16 input. An accepted physical
transition must not also be translated to a character by the native caller.
Only UNMAPPED asks the Window adapter for character translation; combined
Console records consume their attached text when a physical key is available.
Independent character records always enter UTF-16; there is no timing or
character-credit deduplication table. Text-only breaks do not produce text.
Text synthesis reads the same matcher's held-key ledger and releases only the
keys it introduces. Existing keys (including either modifier side) remain held;
a held trigger receives a repeat make with its original physical identity.
The ledger borrow is used synchronously on the same input owner as submission.

Native adapters preserve repeat counts in copied records. This normalizer alone
expands physical makes (breaks stay single), or complete decoded characters.
Zero count means one. UTF-16 halves must have matching counts; malformed pairs
are rejected and cleared. A new high replaces an unfinished high; a valid BMP
unit is processed even after a malformed prefix. Both layout-map failure and
unrepresentable physical mapping fall back to the same TEXT event. A rejected
delivery ends a batch immediately without retry.

A held make captures leaf-supplied allow_replay once. False prevents its cached
ordinary make from being replayed, without changing chord matching or release
semantics. Repeats cannot upgrade the original permission. Default component
delivery permits replay; a filtering leaf supplies its own permission.
