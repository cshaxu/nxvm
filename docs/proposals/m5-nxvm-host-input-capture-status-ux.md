# M5 NXVM Host-Input, Console Lease And Window UX

## Goal

Make NXVM a multi-session product with one process Console lease and multiple
independent guest Windows. Each session selects one presentation policy:
`console` or `window`. A `console` session owns the one Windows Console while
it runs, including after guest graphics creates its Window; a `window` session
never owns that Console. Session pause, resume and stop remain session-local.

The task also provides one host-input action boundary, OS-delivered function
keys, `Ctrl+Alt+P/D/M`, exact `NXVM (Running)` / `NXVM (Paused)` titles, and
click-to-capture pointer behavior. F9 is a normal guest function key.

## Presentation Model

There is one process-wide Console surface and any number of independent Window
surfaces. The Console surface has two mutually exclusive uses: NXVM command
entry when no running Console session holds its lease, or guest text/blank
presentation while one Console session holds it. It is never a second
command-Console mirror while leased to a guest. A graphics Window belonging to
that Console session is an additional presenter only: it neither returns the
Console to NXVM nor changes the session's Console ownership.

| Session policy and guest state | Console lease | Guest presenter | Command Console |
| --- | --- | --- | --- |
| `console`, running text | Held by this session; text is guest output/input. | None. | Unavailable. |
| `console`, running graphics/full-screen | Still held by this session; Console is intentionally blank. | One Window for this same session. | Unavailable. |
| `window`, running text or graphics | Not held. | One Window for this session. | Available unless another Console session holds the lease. |
| Any policy, paused/stopped | Released when this session had held it. | Paused Window freezes or is closed by its one platform lifecycle. | Available unless another Console session holds the lease. |

Consequences are deliberate:

- A graphics transition of a Console session is an additional Window presenter,
  not a transfer to a Window-runner and not a release/reacquisition of the
  Console lease.
- A later transition back to guest text closes that same session's additional
  Window and resumes text presentation in its already-held Console. It neither
  restarts the session nor returns the Console to NXVM command entry.
- Only one Console session may run at once. A second attempt fails clearly; it
  cannot steal, multiplex or silently redirect the Console surface.
- Multiple `window` sessions may run concurrently because each Window is
  owned by its own session. Their close, pause, capture and title changes are
  isolated.
- Pausing or stopping a Console session returns the actual NXVM command
  Console. Resuming it reacquires the Console lease before guest text can be
  presented again.

## Ownership And Data Flow

```text
native event
  -> one host-action classifier
  -> product action OR existing keyboard/mouse normalizer
  -> existing VM request transport
  -> Core KBC/mouse owner

session-control state
  -> copied platform lifecycle observation
  -> Console lease / per-session Window presenter
  -> native title, capture and paint behavior
```

- `vm_session_control` is the sole owner of one session's running, paused and
  stopped state. No Window, Console adapter or renderer invents another flag.
- VM composition owns selection of the immutable `console`/`window` policy and
  binds it to the session's one platform run context.
- The platform owns the one host Console lease and each Window's native focus
  and pointer capture mechanics. It reports bounded pause/stop/presentation
  events; it never mutates Core, chooses a session, or maintains a process-wide
  "focused session" registry.
- Core remains the sole owner of guest keyboard, mouse, display state and time.
- The NXVM command parser remains the only command authority. A released
  Console returns to that parser; no Window creates a command parser.

`auto` is removed as a display name. Its former console-first promotion
behavior is named `console` everywhere: YAML, session policy, platform enum,
tests and diagnostics. There is no compatibility alias.

## Input And Product Actions

The classifier runs before guest mapping in both presentation forms.

| Input | Required result |
| --- | --- |
| OS-delivered `F1` through native, virtual-key or RDP recovery input | One normal guest F1 make/break sequence for the running native presenter that received the event. Laptop Fn itself is not an application-visible key and is never fabricated. |
| F9 | Ordinary guest F9 make/break sequence. It is not a product stop action. |
| `Ctrl+Alt+P` | Pause only the session whose Console or Window received the chord, through its existing pause authority. A Console session pause releases its Console lease and returns NXVM command entry; its existing command path resumes it. A paused Window freezes guest display/cursor and rejects guest input. |
| `Ctrl+Alt+D` | Request that presenter's existing runtime-debugger pause/entry boundary. |
| `Ctrl+Alt+M` | Release only that Window's host pointer capture; idempotent when already released. |
| Window close | Pause only that session via the same pause authority; never stop/reset Core or another session. |

Window capture is explicit: a running Window receives guest pointer input only
after a click; `Ctrl+Alt+M`, pause, debugger pause, stop and close release it.
While paused, a Window neither recaptures on click nor submits keyboard/mouse
input, and its guest frame/cursor remain frozen. The host pointer remains free.

## Planned Subtasks

1. **S1 - session presentation lifecycle.** Separate a Console lease from a
   session's optional Window presenter. Converge Console text, retained blank
   Console plus graphics Window, graphics-to-text return, fixed Window,
   pause/release and resume into one session lifecycle. Remove the existing
   Console promotion join/restart path. Prove one lease, retained lease through
   both display transitions, release on Console-session pause, and independent
   multi-Window behavior.
2. **S2 - host event and action convergence.** Audit every Console/Window
   keyboard ingress, RDP/Unicode recovery, old F9 stop path, debugger command,
   platform pause event and request transport. Implement the one finite action
   classifier and delete every bypass or duplicate product-action route.
3. **S3 - lifecycle publication and native Window binding.** Publish one
   copied per-session lifecycle observation to platform adapters. Derive exact
   titles, Window-close pause, paused paint/input gating and Console command
   return from it without native-state mirrors.
4. **S4 - Window-local pointer capture.** Implement click capture, release,
   cursor mechanics and `Ctrl+Alt+M` at the native Window owner; verify native
   event delivery cannot route product actions or guest input to another
   session. There is no global focused-session state.
5. **S5 - closure.** Run full repository-only unit tests after every S and at
   T closure, conduct Windows-host/RDP manual proof for delivered keys and
   multi-session behavior, perform the similar-issue sweep, and build the
   required x64/x86 stripped developer artifacts. The owner-approved exception
   is that no external-ROM/media integration suite is a task acceptance gate;
   this task does not claim guest Windows support.

## Non-goals

- Do not add a Core dependency on Windows messages, host handles, modifiers,
  cursor APIs, Console leases or session pointers.
- Do not add a second keyboard mapper, request queue, debugger command parser,
  guest display snapshot, session lifecycle state, or Console surface.
- Do not make a physical Fn key work when firmware/Windows does not deliver F1.
- Do not change profile hardware, YAML asset loading, guest scan-set semantics,
  Core timing, external media, or user-managed `build/output` configuration.

## Completion Standard

The session-presentation matrix has one owner and one route per row: one
Console lease, a retained blank Console during a Console session's graphics
Window, graphics-to-text return without a restart, many independent Window
sessions, and release only when the owning Console session pauses/stops. Every
delivered F1/F9 and every reserved chord has exactly one disposition; Window
close/pause/capture cannot affect another session; titles and paused
input/presentation behavior derive from the single session-control state. No
`auto` display spelling, Console promotion restart, F9 stop shortcut, duplicate
host input route or lifecycle mirror remains.
