# M5 NXVM Host-Input, Capture And Status UX

## Goal

Make NXVM's Console and Window presentations have one predictable host-input
contract: an operating-system-delivered function key reaches the guest, three
reserved `Ctrl+Alt` chords invoke product actions, and a Window session has
explicit running/paused and mouse-capture behavior.  The required user-visible
window titles are exactly `NXVM (Running)` and `NXVM (Paused)`.

## Problem And Boundary

T514 correctly converged Win32 native, virtual-key and Unicode input through
one normalizer, but the last platform hop still sends every key and every
mouse event straight to the guest.  Window creation also takes focus at once;
there is no capture state, product shortcut classifier, or title derived from
session state.  `VK_F9` is an isolated stop special case and must return to
ordinary guest-key delivery.

This task repairs the product boundary, not the guest keyboard controller:

`native Console/Window event -> one host-action classifier -> either product
action or existing normalizer -> existing VM request transport -> Core KBC or
mouse owner`.

The classifier is a VM/platform product capability.  Core retains keyboard,
mouse, device state and guest timing.  The session control owner retains the
running/paused transition.  The Window adapter owns only native focus, cursor
and capture mechanics; Console has no fictitious mouse-capture implementation.
Neither presentation holds a second lifecycle flag or maps a reserved chord on
its own.

## Function Keys And Fn

`Fn` is normally consumed or translated by laptop firmware/Windows and is not
a Win32 virtual key that NXVM can reliably see.  The contract is therefore
precise: when the host delivers `VK_F1` with its native scan information (as
ordinary F1 or an Fn layer that Windows exposes as F1), both Console and Window
must send one correct guest F1 make/break sequence through the existing
normalizer.  If firmware never delivers F1, NXVM must not invent a separate
Fn-to-F1 mapper.  Native Windows and RDP/soft-keyboard evidence both exercise
the delivered-key contract.

## Product Actions

The following chords are reserved before guest keyboard mapping, in both
Console and Window presentations:

| Chord | Product action | Guest effect |
| --- | --- | --- |
| `Ctrl+Alt+P` | Toggle the session's existing explicit pause/continue authority. Pause releases Window mouse capture and freezes presentation/input admission. | Never delivered as guest Ctrl/Alt/P input. |
| `Ctrl+Alt+D` | Request the existing runtime debugger entry/pause boundary. | Never delivered as guest Ctrl/Alt/D input. |
| `Ctrl+Alt+M` | Release Window mouse capture; it is idempotent when already released or on Console. | Never delivered as guest Ctrl/Alt/M input. |

F9 is removed from the product action surface and follows the same normalizer
and guest mapping as every non-reserved function key.  The task does not add
an action parser per surface, an additional command-language route, or a
second debugger/pause implementation.

## Window State Machine

The session control state is the sole source for the title.  The platform
samples/copies that state only to update its native title and to gate host
input; the Window must not independently decide that it is paused.

| Current state | Event | Next state | Required effect |
| --- | --- | --- | --- |
| Running, released | Left click in guest window | Running, captured | Capture/clip the host pointer and begin guest relative-mouse delivery. |
| Running, captured | `Ctrl+Alt+M` | Running, released | Release/unclip the host pointer and stop guest mouse delivery. |
| Running, any capture state | `Ctrl+Alt+P` | Paused, released | Request the existing session pause; when it is acknowledged, release pointer, stop presentation updates and reject guest keyboard/mouse admission. |
| Paused, released | Window click or guest input | Paused, released | Do not recapture or enqueue guest input; the displayed guest frame and guest cursor remain frozen. |
| Paused, released | `Ctrl+Alt+P` | Running, released | Resume through the existing session-control authority; a later click is required to capture again. |

"Freeze cursor" means the guest presentation/cursor and guest input stream
are frozen.  The released host pointer remains free for the user; the task
must not attempt to immobilize the operating system cursor after release.
While running, title is `NXVM (Running)`; while pause is acknowledged, it is
`NXVM (Paused)`.  The debugger action uses its existing pause semantics and
therefore gets the same paused title/capture release once the session reports
paused.

## Planned Subtasks

1. **S1 - host-event and action-boundary audit.** Trace Console, Window,
   RDP/Unicode recovery, F9, debugger commands, session control and request
   transport. Define one finite key/action disposition matrix including F1,
   `Ctrl+Alt+P/D/M`, ordinary F9, key-up behavior and host-unreported Fn. Record all
   current direct host-to-guest and product-action paths before code changes.
2. **S2 - one action and lifecycle publication path.** Introduce one
   host-action classifier and one copied session lifecycle observation suitable
   for platform use. Route pause/resume and debugger entry through their
   existing authorities, remove the isolated F9 stop path, and ensure only admitted
   guest input reaches the existing normalizer/request transport.
3. **S3 - Win32 presentation mechanics.** Make both Win32 surfaces call S2;
   implement Window click-to-capture, release/unclip, cursor visibility and
   paused input/presentation gating through the one lifecycle observation.
   Set the exact title strings. Preserve Console's lack of mouse capture and
   do not add a Window-only keyboard mapping path.
4. **S4 - regression and closure.** Add owner-local repository-only tests for
   the action matrix, normal F1/F9 delivery, no duplicate make/break, paused
   input rejection, title derivation and capture transitions. Run the full
   unit suite after each S and at T closure. The owner-approved external-ROM/
   media integration exception applies because the task changes no guest
   Windows lifecycle or external-asset contract. Perform a manual Windows-host
   and RDP proof for every key
   Windows actually delivers; record host-firmware Fn non-delivery as a host
   limitation, not an NXVM fallback.

## Non-goals

- Do not make Core aware of `Ctrl+Alt`, Fn, Windows messages, cursor APIs or
  native window handles.
- Do not add a second keyboard mapper, mouse queue, pause flag, debugger entry
  route, input transport or presenter frame cache.
- Do not promise a physical Fn key that the host firmware/OS consumes before
  NXVM receives it.
- Do not change guest scan-set semantics, profile input mappings, YAML,
  firmware, media, Core timing, or `build/output` configuration.

## Completion Standard

The complete Console/Window action matrix has exactly one classified product
route or existing guest-input route per row; delivered F1 and F9 reach the
guest in both modes; all three chords bypass guest injection; Window capture is
click-entered and product-released; paused Window interaction neither admits
input nor changes the guest frame/cursor; titles exactly reflect the sole
session lifecycle state; no surface-specific shortcut or state mirror remains.
The task closes only after full repository-only unit tests pass. The
owner-approved external-ROM/media integration exception is recorded above;
Windows/RDP manual evidence is limited to observable host input.
