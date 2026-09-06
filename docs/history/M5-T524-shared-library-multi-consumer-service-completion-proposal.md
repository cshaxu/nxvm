# M5 Shared-Library Multi-Consumer Service Completion

## Purpose

Make `src/lib` a complete, source-identical, independently buildable host-side
library that NXVM and SoftPC can copy byte-for-byte. It contains only neutral
capabilities. Product binding, machine execution, firmware, profiles and
guest state remain outside it.

## P0: Corpus And Publish Boundary

`MANIFEST.sha256` is the complete fixed-revision corpus authority. It must
include every tracked file below `src/lib/`, including public and private
headers and both Windows/Linux implementations. A standalone verifier compares
the tree to the manifest, detects missing and extra files, and is registered in
CI so a changed library file cannot pass with a stale manifest.

The repository supplies a lib-only CMake build/verify entry point. A consumer
does not recreate NXVM's root source list: the selected Windows or Linux corpus
is provided by the library build itself. The target also runs neutral
conformance tests using public library headers only.

## Retained Boundaries

- **UX** owns copied frame/input values, mailbox, actions, configurable routing
  policy, native console/window presentation and capture. Its contract defines
  copied frame ABI; mailbox producer/consumer/generation/wake rules; presenter
  lifetime/result; keyboard/Unicode/mouse/wheel/absolute/relative events; and
  action identity, capacity and conflict results. It never owns DIBs, product
  hotkey meaning, guest keyboard handling, renderer state or machine pointers.
- **Host** owns clock, event, task, cancellation, join and, where two consumers
  require it, a neutral wait-any operation. Event reset behavior and
  signal/timeout/cancel precedence are explicit and parity-tested on Windows
  and Linux. It never owns guest time, timer ticks, devices or callbacks.
- **Session** owns only lifecycle requests, wake, cancellation and acknowledgements.
  A product retains every executor turn, safe point and transition action. A
  callback driver is admitted only if two real product bindings need it; it
  names `enter`, `run_until_safe_point`, acknowledgement and `leave` solely as
  product callbacks, never as machine ownership.
- **Storage** owns opaque byte-medium leases. Readonly and direct media retain
  a file-backed byte route; overlay retains its readonly base plus sparse
  4-KiB dirty pages and is discarded. It owns replacement, offset I/O, flush
  and defined exclusive-lease destruction behavior. It does not know media
  topology, CHS, drives, firmware, profiles or guest policy.
- **Observability** owns bounded copied outcomes. A bounded diagnostic event is
  admitted only after two consumers require sequence, status/severity and fixed
  copied message/metadata; paths, CLI rendering, machine traces and log policy
  remain product-local.

`lib` itself has no `softpc`, `mvdm`, `ccpu`, `cvid`, `bios`, `rom`, `guest`,
`controller`, `profile` or `vm` concept in its source or public ABI. A
machine-neutral use of an otherwise ambiguous word is documented at the one
owner; it does not create a product dependency.

## Subtask Plan

1. **S1 - complete-corpus disposition.** Freeze every tracked library file,
   manifest entry, CMake source-list entry, public header, platform variant and
   current conformance consumer. Record P0 omissions, forbidden vocabulary and
   one disposition per capability. No API or behavior change.
2. **S2 - manifest and standalone build.** Make the manifest exhaustive and
   fixed-revision, add its standalone verifier and lib-only CMake entry, and
   ensure CI rejects drift. Delete duplicate root source lists rather than
   maintain parallel inventories.
3. **S3 - UX contract closure.** Remove host-to-x86 key translation and all
   product/machine vocabulary from `lib`; define mailbox lifetime/concurrency,
   frame validity, event and action conflict/capacity semantics; then add two
   neutral UX consumers and platform conformance coverage.
4. **S4 - host contract closure.** Specify and prove event reset behavior,
   cancellation/timeout precedence, task destruction and wait-any behavior.
   Supply Windows/Linux parity tests; do not add a timer or product callback.
5. **S5 - session disposition.** Prove two neutral consumers of the existing
   lifecycle state. Admit an executor driver only if both need the same
   callback lifecycle; otherwise remove the one-product generic facade.
6. **S6 - storage disposition.** Reconcile the existing byte-buffer modes
   with a real image lease, safe replacement and commit/discard contract.
   Implement only the neutral shared ownership semantics established by two
   consumers; no media topology enters the library.
7. **S7 - observability disposition.** Prove two neutral consumers of copied
   outcomes. Add a bounded diagnostic event only if both require the same
   fixed copied facts; otherwise retain product diagnostics outside `lib`.
8. **S8 - portable closure.** Run the lib-only Windows build/test path, strict
   Linux source syntax, the full NXVM unit and integration gates, full
   vocabulary/source sweeps and actual-diff review. Record the exact source
   revision and manifest hash used for later SoftPC adoption.
9. **S9 - Model-40 integration-gate disposition.** Under explicit owner
   direction, reproduce the external boot row without competing diagnostics;
   repair only a demonstrated FDC/board/product defect, then rerun the
   complete integration gate. It may not add a library compatibility route or
   product vocabulary.
10. **S10 - file-backed storage repair.** Correct the false equivalence between
   an owned RAM byte image and a direct file medium. Add one opaque neutral
   byte-medium contract with offset read/write/flush and direct, readonly and
   sparse 4-KiB dirty-page overlay modes. The overlay reads its immutable base
   file on demand and allocates only changed pages; it always discards at media
   removal and has no commit operation. Port NXVM FDD/HDD file insertion to that contract while
   retaining their topology and offset calculation locally. Do not import
   SoftPC controller code or add controller vocabulary to `lib`.
11. **S11 - lifecycle neutrality repair.** Remove single-step requests,
   numerical pause reasons and `flip` from the shared lifecycle state. Keep
   only start, stop, reset, generic pause request/acknowledgement and resume in
   `lib/session`. NXVM `session/control` exclusively owns debugger-specific
   single-step and pause-reason state; its runner consumes the step flag after
   one instruction and requests a normal lifecycle pause. Delete `flip` in
   both layers after confirming it has no consumer. Build stripped x64/x86
   artifacts for owner manual testing.
12. **S12 - storage API dead-surface cleanup.** Delete unconsumed exclusive
   creation, existence, replacement, removal and line-reader helpers, together
   with their native-only implementations. Delete `medium_mode_of` and `medium_flush`;
   tests prove Direct persistence by reopen behavior rather than a redundant
   public flush. Retain one pointer-clearing `medium_destroy` because FDD/HDD
   eject uses it to destroy and null the sole lease. Direct `write_at` remains
   the one
   immediate-persistence boundary. No overlay commit or delayed-write policy
   is admitted. Repair the discovered stale Linux platform-contract gate to
   inspect its current `linux.c` host-sync/UX lifecycle, without changing
   Linux runtime behavior. The retained neutral text writer exposes one open
   parameter for truncate or append; product record formats, reset timing and
   write cadence remain outside `lib`.
13. **S13 - UX owner and surface-lifecycle cleanup.**
   Apply the completed whole-`src/lib` quality audit without creating a second
   renderer, presentation state, or product policy route. A presenter has one
   active render surface: either the Console text buffer or one native-window
   DIB. A Console-mode session whose copied frame becomes graphical closes its
   Console presenter and returns the Console to the product controller before
   presenting only through the window DIB. On a stable return to text it
   destroys the window DIB/window, then reacquires the Console text surface.
   Window-mode sessions always use the window surface. The router remains the
   one target-selection owner and the copied mailbox remains the only frame
   source; this change does not create a second video, keyboard, mouse, or
   session-state owner.

   The Win32 window binds its one heap context to its `HWND` through
   `GWLP_USERDATA`, eliminating the thread-local current-context and its
   forwarding macro layer. One owner-local cleanup path releases a partially
   or fully initialized native surface, including selected DIB restoration,
   DCs, bitmaps, frame and window lifetime. Console input/output handles are
   always closed by their opener, while `FreeConsole` remains limited to a
   Console allocated by this presenter. Remove the mailbox `active` mirror,
   the ignored presenter modifiers parameter, the duplicated mouse
   cursor-hidden state, and the stopped-state pause request that cannot survive
   `start`. Linux mailbox creation must prove nonblocking fd setup and retain
   its wait reference as a native internal value rather than an fd-to-`void *`
   encoding.

   The S consumes the complete UX/native/lifecycle finding batch: Win32
   Console handles, Console-to-window release, Win32 surface creation and
   teardown, HWND context dispatch, common mailbox lifetime, Linux mailbox
   wake setup, presenter action ABI, mouse state and stopped lifecycle pause.
   It sweeps all `src/lib`, `src/vm`, `test`, and CMake callers for the retired
   fields/parameters/TLS access and native-resource paths. It adds owner-local
   regression coverage for router transitions and lifecycle semantics, runs
   the complete repository-only unit suite plus lib-only build/CTest and both
   static platform gates, and builds the required stripped x64/x86 T524
   artifacts for owner Windows-host validation. It neither requires a Linux
   runtime nor changes guest, Core, profile, firmware, or media semantics.
14. **S14 - Console viewport and explicit presentation-lifecycle repair.** A Win32
   Console text presenter must establish both its screen buffer and its visible
   viewport for the copied text frame; growing only the buffer leaves a
   physically present but hidden lower screen. Do this at the Console surface
   owner, with no second frame or display-state copy. Separately, remove the
   run-handle's start-time `window_display` mirror. NXVM is the sole
   presentation-lifecycle authority: it writes only the latest `CONSOLE`,
   `WINDOW`, or `NONE` request to the router and wakes the existing mailbox;
   no event queue or polling loop is introduced. The active Console or Window
   thread releases its own native surface only after that request. The run context remains the
   sole presentation-mode owner; lib never infers a switch or destruction from
   lifecycle state. In particular,
   Ctrl+Alt+P pauses a Window-mode session in place, releases capture and
   updates its title, but neither stops the session nor destroys its window.
   Add focused repository-only proof for live mode observation and retain the
   complete unit suite, lib-only build/CTest, platform gates, governance and
   dual stripped artifacts. No guest, Core, profile, router-policy or external
   asset change is admitted.
15. **S15 - retire thin session and outcome library roots.** `session-lifecycle`
   and `observability-outcome` have one real product consumer, NXVM, and add no
   reusable policy or mechanism. Move lifecycle state to its owning NXVM platform
   layer and keep the sole start-result record directly on the NXVM session,
   update every product and test caller,
   remove their public lib headers, source roots, manifest entries, standalone
   tests and neutral-consumer assertions, and remove their CMake targets. Do not
   retain compatibility wrappers or product logic in `src/lib`. Retain one
   NXVM-owned lifecycle state and one NXVM-owned start-outcome record. Sweep all
   build declarations, includes and tests for the retired names; verify focused
   lifecycle/startup behavior, full repository-only unit, the reduced lib-only
   corpus and dual stripped artifacts before P closure.

## Acceptance

- Every tracked `src/lib` file is manifest-listed exactly once; a verifier,
  standalone CMake entry and CI gate reject drift.
- Each retained root has at least two neutral, public-API consumer/conformance
  proofs; a one-product capability remains outside `lib`.
- Public library headers and the full library tree contain no product or
  machine ownership concept, native SDK type, raw product pointer or implicit
  product policy.
- Windows lib-only build/test evidence and strict Linux source syntax pass,
  and the full NXVM unit/integration gates remain green. A remote Linux CI
  runtime is not required.
- The later SoftPC task imports the recorded manifest unchanged and supplies
  only a product binding.
- Direct and readonly media retain an opened backing file and perform bounded
  offset I/O; they never first materialize an entire file merely to call them
  direct. Overlay retains the readonly base and only changed 4-KiB pages in
  memory; it is always discarded and never materializes a replacement.
- Shared lifecycle headers expose neither stepping, a pause reason nor `flip`;
  NXVM retains each debugger-specific fact exactly once in `session/control`.

## Non-goals

Do not import or modify SoftPC, Bochs, third-party code, firmware, media or
guest assets. Do not add a generic emulator, machine thread, controller,
BIOS/ROM service, guest input protocol, product router policy, logging system
or compatibility wrapper.
