# M5 T518: NXVM Host-Input, Capture And Status UX

The [active proposal](../proposals/m5-nxvm-host-input-capture-status-ux.md)
defines the owner-approved UX contract. This record is updated only with
accepted subtask outcomes and final closure evidence.

## Task Record

| Subtask | Scope | State |
| --- | --- | --- |
| S1 | Converge one Console lease, console text-Window-text transitions, multi-Window lifecycle and Window-close pause. | Accepted: P1 `8be4223c` |
| S2 | Converge host input and product action classification. | Accepted: P1 `586735fa` |
| S3 | Publish one lifecycle observation to platform presentation. | Accepted: P1 `e077de3e` |
| S4 | Implement Window-local native pointer capture and release behavior. | Accepted: P1 `960721de` |
| S5 | Run full unit closure, manual host proof and developer artifact delivery. | Planned |

## Accepted Outcomes

### S1

- The Console backend remains the sole holder of the process Console lease.
  Its graphics transition creates the shared per-session Window presenter and
  does not join, finalize or restart the Console runner. The reverse transition
  removes that presenter and restores the same Console route.
- Fixed `window` sessions and Console-session graphics use the one native
  Window presenter. Closing a Window reports the existing bounded pause event;
  it neither resets Core nor affects another session.
- The obsolete presentation-transition runner yield and every `auto` display
  spelling were removed. Session listing now reports the immutable selected
  policy rather than a temporary graphics presenter.
- Repository-only focused platform proof and the complete unit suite pass:
  302/302 in 19.13 seconds with `ctest --test-dir build/t518-s1 -L unit -j 4
  --output-on-failure`. Documentation governance and actual-diff review pass.

### S2

- One Win32 classifier now precedes the retained Core keyboard normalizer for
  both Console and Window adapters. It leaves delivered F1/F9 on the ordinary
  guest route and removes every F9-to-stop route, including the Linux Console
  equivalent.
- Ctrl+Alt+P, Ctrl+Alt+D and Ctrl+Alt+M each publish one bounded platform
  request. The runner maps the debugger request to the existing session pause
  boundary; capture release remains reserved for its native S4 owner.
- Ctrl/Alt transitions are held until the following key establishes whether
  they form a product chord. Reserved chords suppress their modifiers and
  make/break transitions; ordinary Ctrl+Alt input is flushed in order through
  the existing guest normalizer.
- The owner-local action smoke covers F1/F9 delivery, all three actions,
  reserved key-up suppression and ordinary Ctrl+Alt input. The prior
  cancellation and session-run integration tests now assert the same F9
  contract. Focused checks pass 3/3; repository-only unit passes 303/303 in
  17.84 seconds; documentation governance and actual-diff review pass.

### S3

- The session-control owner now exports one three-state lifecycle observation
  through the opaque execution transport. The former parallel `is_running`
  callback was removed; Running, Paused and Stopped have one disposition.
- The shared Window presenter derives its exact `NXVM (Running)` or
  `NXVM (Paused)` title, paint eligibility and all guest input eligibility
  from that observation. It does not retain a native pause acknowledgement.
- Window close retains only a presentation-local close-to-paused-to-resume
  transition: it hides after issuing the existing pause request, waits for the
  authoritative Paused observation, then shows again only after authoritative
  Running. That state is not a second session lifecycle owner.
- Focused lifecycle/title proof passes 2/2; repository-only unit passes
  303/303 in 17.64 seconds; documentation governance and actual-diff review
  pass.

### S4

- Each native Window presenter now solely owns its capture flag and native
  capture handle. A button-down on a Running, uncaptured Window acquires host
  capture; moves and buttons otherwise do not reach the existing Core mouse
  route.
- The presenter releases capture and clears relative-motion history on the
  existing Ctrl+Alt+M report, any non-Running lifecycle, close, destruction,
  capture loss or focus loss. The host action is consumed by that presenter's
  existing run handle; no Core/session pointer state or global focused-session
  registry exists.
- The shared input-eligibility proof covers captured/uncaptured Running and
  Paused states, while the existing host-action proof covers the release
  report. The complete repository-only unit suite passes 303/303 in 18.30
  seconds; documentation governance, static sweep and actual-diff review pass.
