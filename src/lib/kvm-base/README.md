# kvm-base

`kvm-base` is the shared KVM foundation. It depends on `types` and `base`, providing
typed frame values, copied input events, source-local hotkey matching, and private mailbox
mechanics to `kvm-window` and `kvm-console`.

`kvm_text_frame` contains the common text fields with a fixed 80-cell row stride.
One `kvm_text_cell` array carries glyph index, glyph bank (0 primary, 1 secondary)
and foreground/background palette indices (0..15) together in each four-byte cell.
Cells have no padding; whole-array copying/comparison is valid. Colours and bank are independent;
visible cells are validated before publication. No packed device attribute is
interpreted here. Producers initialize the complete fixed-capacity value.
Fonts, character maps and graphics belong to the receiving leaf, not this base.
The public frame exchange circulates opaque storage through four slots
(produce/pending/processing/displayed) for both consumers. The producer
writes outside its lock, then commits a validated complete frame. A superseded
pending pointer becomes reusable; take pins processing until finish. No steady
allocation, payload copy, content callback or damage interpretation occurs.
One producer and one consumer use an exchange. Its owner destroys it only after
all consumers and observers stop; leaf destruction detaches but never frees it.
Bounded observations hold its lock, never retaining a pointer or drawing under
that lock. Invalidation drops publication without altering held input.

Control admission copies one record per call into the 32-slot ordinary FIFO.
Capacity rejection returns LIMIT_EXCEEDED without replacing any queued record.
STOP has a reserved slot, closes admission and is idempotent. The worker consumes
controls in order through STOP, then ignores later control/frame work. There is
no batch admission; frame publication remains independently locked, latest-wins.

The control envelope has an opaque nonzero consumer kind and 128 copied payload
bytes; only kind zero (STOP) is interpreted here. Window owns title/freeze/release
encoding and validation. Console currently has no ordinary controls. Consumers
reject invalid records through their existing fault path, rather than silently
accepting another component's commands. No native or application API exposes
the envelope; no per-control allocation or extra dispatch registry is involved.

Frame and control each use an independent Base blocking mutex.
The control lock only protects the short FIFO operation. STOP/fault acquires frame
then control; ordinary control remains independent of a contended frame observation.
Initialization creates both mutexes or releases the partial allocation and fails;
disposal releases them after all mailbox users have quiesced.

Each mailbox selects one notification implementation exactly once before caller publication.
Initialization allocates no wake. Console selects and creates the default wait
primitive (one Base automatic-reset Event); Window directly selects its native message notifier. A second selection is
rejected without changing the active notifier. Frame, control and
terminal fault all use that selected entry, outside mailbox locks. Context
remains valid through worker join; as with destruction, callers must quiesce
concurrent API use before releasing the component. A notification failure after
enqueue closes component admission and is not permission to replay.
The detecting thread reports the first failure through the existing failure
sink even if notification cannot wake the worker. Retirement is separate.

Its internal component emission helper may let a leaf filter matcher output,
but source attribution and chord matching always remain in this component.
This permits a frozen Window to discard ordinary content input while forwarding
registered hotkeys without creating a second matcher path.
Each instance owns its ledger; modifier snapshots may satisfy chord modifiers
without corresponding makes in that instance. This accepted behavior does not
merge ledgers or require a second modifier-state table.

`worker_interface.h`, `mailbox_interface.h` and
the root `input_interface.h` are shared leaf-support contracts, not
application entry points. Each leaf owns its own state; Window capture is
Window-local. Outstanding suppressed keys survive subsequent matched chords
until their breaks. One ordered held-key ledger gives every physical key one
pending, delivered or consumed disposition. Repeats reuse it, releases remove
it, and storage growth failure is terminal rather than dropping key state.

Base owns native keyboard decoding, surrogate state and layout queries; KVM
owns neutral delivery and text synthesis. Key values and input records come
directly from Base; there are no KVM key/modifier synonyms. Neutral submission
reports boolean delivery success; only native decoding distinguishes UNMAPPED.
Linux terminal text uses TEXT rather
than inventing physical keys. No KVM leaf decodes native key values itself.
Window message decoding and key-state queries belong only to kvm-window.
Base owns the default Event implementation; no KVM platform wake duplicate remains.
Take removes pending ownership and optionally exposes the previous displayed
input. Both inputs remain immutable through finish. Successful finish retires
the previous displayed input; failure preserves it.
An explicit redraw requeues held input only when no newer input is pending.
Failed output requeues only an
unsuperseded input, without replacing a newer pending frame or signalling retry.
The leaf mailbox binds that same exchange; it does not own another pending slot.

Mouse and close events never flush a keyboard prefix. Mismatch/matched keyboard release
replays pending keyboard events in order; keyboard/mouse interleaving is not
buffered. A rejected input sink closes the source and clears pending state;
the first failure is reported immediately; only quiesced exit reports retirement. STOP/fault closes
frame and control admission by taking the independent frame then control locks;
ordinary control never waits for rendering. FIFO processing up to
STOP is unchanged. Window's sole final filter discards ordinary frozen input,
not registered hotkeys or lifetime events.

Physical key identity is scan code + EXTENDED flag, with logical key fallback
only when the scan is absent. A held key retains its original logical key across
lock/layout changes. Both leaves share this ledger. Ordinary keys and modifiers follow the same lifetime: a
delivered make always retains its break and never becomes consumed later.
The ledger is released at retirement; destroy also handles workerless cleanup.
An unmatched break is consumed without delivery or pending-key replay: a new or
reset input source must not forward a release whose make it never accepted.
Destroy performs one bounded join. An unjoinable live worker is an importing
application terminal infrastructure failure; the library does not terminate or
add a fallback wake channel. A selected wake failure after a request has been
accepted reports through the failure sink; it does not roll back or replay it.

Window submits copied native records through kvm_keyboard_submit_record, which
delegates decoding to Base before the shared kvm_keyboard_submit_input entry.
Console's broker already decoded combined native records; Console goes straight
to that same neutral entry. Base chooses physical versus UTF-16 input. An accepted physical
transition must not also be translated to a character by the native caller.
Only UNMAPPED asks the Window adapter for character translation; combined
Console records consume their attached text when a physical key is available.
Independent character records always enter UTF-16; there is no timing or
character-credit deduplication table. Text-only breaks do not produce text.
Text synthesis reads the same matcher's held-key ledger and releases only the
keys it introduces. Existing keys (including either modifier side) remain held;
a held trigger receives a repeat make with its original physical identity.
The ledger borrow is used synchronously on the same input owner as submission.

Native adapters preserve repeat counts in copied records. KVM delivery alone
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
