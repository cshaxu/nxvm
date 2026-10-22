# T7 S74 — Pause-Boundary Control Release

The Session input ledger forwards synthesized source releases while running,
but deliberately does not deliver late releases to a paused machine. The
MyRaidenII driver previously retained its Board control levels across pause,
so a key released while paused could remain asserted after resume.

The initial uncommitted candidate at the heartbeat-disable boundary made the driver
release every Board control through the existing semantic control API before
disabling audio. Board remains the only control-level owner. A release error
sets the existing driver failure flag; audio shutdown is still attempted.
There is no per-source or per-key duplicate state. The same release also runs
on stop and internal worker pause boundaries. MyRaidenII has no snapshot
implementation; if one is added, its state-read input policy needs review.
Running source-retirement/focus-reset releases remain owned by Session; Board
reset already initializes its controls. This is not a desktop or RDP test.

The candidate's Machine unit exercised every control, repeated release, enabling
heartbeat, and failure as observed by the next `run`; both host widths rebuild
the Product and pass that test. Full repository-only units pass 697/697 on x64
(88.06s) and x86 (108.83s), but do not cover the queue race below.

Further source review found an unresolved queue ordering race: Machine drains
one input per executor callback, then enters pause and can continue draining
the input queue while paused. A pre-pause key-down still in that queue can
therefore reassert a control after the heartbeat release; a quick resume can
also precede queue exhaustion. The current local change is only a partial fix,
and the S must not be accepted as complete without a real queued-input test
and an ownership-correct resolution. No Shared change is made here.

## Queue ownership and candidate constraints

The race is not limited to events delivered during the paused wait. Input
admission checks RUNNING before acquiring the queue's mutex, so an enqueue
which passed that check can finish after a separate pause-side queue clear.
Clearing the queue alone therefore does not establish a reliable boundary.
The Session also accepts a late key release while paused for its own pressed
ledger but intentionally does not forward it to the Machine driver.

This FIFO is shared with the PC and MyNES drivers. Blindly discarding its
already accepted events would also discard key releases and other inputs for
those products; the PC heartbeat callback, for example, does not release its
guest keyboard state. An ownership-correct solution must define admission and
the handling of accepted events together, preserve product and state-read
semantics, and account for a producer racing the transition. Merely filtering
input inside the MyRaidenII driver while heartbeat is disabled is insufficient:
an old queued event can instead be delivered after heartbeat is re-enabled.

These are source-level ordering findings, not a reproduced desktop failure.
The existing driver-only smoke does not exercise the real Machine FIFO, so a
deterministic queued-input regression is still required before closure.

A temporary x64 probe subsequently reproduced the ordering against the real
Machine FIFO and executor using the existing scripted-wait fixture. It enqueued
two key-down events while running, requested pause, released a synthetic driver
control on heartbeat disable, and resumed immediately. The first executor
callback left the second accepted event queued and the control released; the
next callback delivered that old event and asserted the control again. The
probe exited successfully because its assertions described the observed defect.
This is deterministic Machine/driver-boundary evidence, not a real Board,
gameplay, desktop, RDP, or x86-host test; it is not a passing fix regression.
The temporary probe source SHA-256 was
`f8abb477970afe16b38c423f52c6e2d874ef1e82e901408e5fe4177cf5d32005`.

The incomplete candidate and its candidate-only assertions were removed after
the owner required an App-only solution: they do not resolve this FIFO race
and heartbeat disable is also used for internal state-read continuation. This
restores the prior driver behavior, not a fix for stale controls. S74 remains
open; the existing successful test runs do not qualify the reverted source.

An App-local FIFO fence was considered after the owner prohibited changes to
the Shared components. The existing `KVM_EVENT_INPUT_RESET` cannot serve as a
generic fence: its public contract identifies a particular source that can no
longer observe balanced releases, and Session owns its source-specific ledger
handling. Reusing it as an undocumented global resume marker would change that
meaning. Likewise, inventing a sentinel hotkey or invalid text scalar would be
a second hidden event protocol, not a clean application of the existing API.
Neither alternative is adopted.

The indistinguishability constraint is concrete: a queued old `KVM_EVENT_KEY`
make and a new make after resume contain the same public fields. The existing
Machine driver receives neither queue depth nor an event-generation value.
Discarding the first make after resume therefore also discards a legitimate
new press; treating every make as a pulse loses held-direction semantics.
An App-local solution must establish an ordered boundary using an event or
callback whose documented meaning actually permits that use. It cannot claim
to distinguish these two cases by examining the key event alone.

The existing UI public interface was also checked for a source-reset or
FIFO-drain operation. It exposes neither. The KVM component's reset operation
is a worker-internal API and emits a source-specific fact to Session; Session
cleans its ledger rather than forwarding that fact to a paused Machine.
Destroying and recreating the Window to force retirement would change the
visible pause experience and still does not supply a Machine FIFO boundary.

The unchanged baseline's potential stale-control scenario is distinct from
the candidate-specific FIFO reproduction: Session's existing
`control_reconciler_smoke` explicitly asserts that a release consumed while
paused is not delivered to the driver, while MyRaidenII's baseline driver
only changes a Board control on delivered key events. If the Board has already
received the make, a release in that paused interval therefore leaves that
control asserted. This conclusion combines existing source and test contracts;
it has not yet been observed in an end-to-end game or desktop test.

## App-only candidate under verification

The current candidate sends ordinary releases for the nine mapped host keys
when Session observes a PAUSED-to-RUNNING completion. The callback runs on
Session's control thread: previously submitted input is already in the single
Machine FIFO and later Session input is dispatched only after the callback.
The existing driver translates those releases to the Board's control API.
This adds no special KVM marker, Shared change, extra input ledger or second
FIFO. Rejection leaves an accepted prefix and requests a reported stop rather
than replaying or silently claiming full success. Machine owns one mapping
table used by both key translation and release generation.

Both architectures build and pass the existing Machine driver and Product
composition units. The driver test checks the nine keys in mapping order and
injects rejection at each position; the composition test checks lifecycle
selection and reported stop. These tests use stubs and do not establish the
actual FIFO ordering, resume race, held-input UX or desktop behavior. A real
FIFO regression and the S's remaining checks are still required.

A temporary x64 probe then linked the actual MyRaidenII release function to
the actual Emulator Machine FIFO and its existing scripted-wait fixture. Two
pre-pause makes, the nine production releases, and one later make were
delivered in FIFO order; the selected key's observed order was `++-+` and it
remained held by the new make. Probe source SHA-256:
`4e02177fcf72fa431bb7c7adb584243d8dcd5250b434205f0c2fc1e43c4aba0d`.
The delivery sink was synthetic and the Session completion was invoked by the
probe, so this is not an end-to-end Session, real Board, x86-host, desktop or
gameplay result. A maintained real-path regression is still required.

The complete consumer inventory also includes the PC Product's synthetic
hotkey sender, not only Session: both call `emulator_machine_enqueue_input()`.
Consequently, serializing only Session's producer would not close the Machine
boundary. Existing `request_lock` serializes several requests but does not
currently protect either input admission or the pause state transition.

An earlier Shared-side candidate was to serialize input admission with the worker's
pause boundary, finish delivering the accepted FIFO prefix in order, then
disable heartbeat and announce pause. It must not hold a shared mutex while
calling a product driver. This is not yet an implementation-ready contract:
publishing PAUSED early would make it observable before the driver is parked,
whereas rejecting a racing enqueue while Session still observes RUNNING may
be treated as a delivery fault. State-read continuation and direct Product
producers also need explicit behavior. A fix must resolve those cases rather
than assert that a lock or a queue clear alone solves them. This candidate was
not implemented; the owner excluded Shared changes.

The maintained `myraidenii.unit.machine.input-fifo` regression now drives the
actual Machine worker and FIFO with a gated synthetic driver. It verifies the
same ordered old makes, production reconciliation releases and later make on
both x64 and x86. The Product and Machine unit tests separately verify the
transition, all nine mappings and every enqueue failure position. These do
not replace an end-to-end Session, real Board, desktop or RDP test.

## Verification and delivery boundary

Both x64 and x86 built the MyRaidenII Product with the existing Ninja trees.
The three focused Machine/real-FIFO/Product tests passed on each width. The
complete repository `unit` label passed 698/698 on x64 (533.15 seconds,
serial) and x86 (112.08 seconds, eight workers). MyRaidenII documentation
governance and `git diff --check` passed. These are background test results;
no real-game, desktop, RDP, integration or playable-release qualification is
claimed. No Shared source or test was modified.
