# M5 Shared UX And Host Library Adoption

## Purpose

Adopt the owner-provided `softpc/src/lib` source as an NXVM in-repository library, then make the resulting contracts reusable by SoftPC and NTVDM64. The library supplies host mechanics and product-neutral values; it never becomes a machine emulator, a profile owner, or a second product/session implementation. It is copied source, not a cross-repository build or runtime dependency.

```text
src/lib/
  ux/
    frame.h event.h actions.h router.h mailbox.h presenter.h
    win32/   Win32 Console, Window, input, mouse, and presenter code
    linux/   Linux Console and future Window/input/mouse presenter code
  host/
    sync.h
    win32/sync.c
    linux/sync.c
  session/
    state.h command.h executor.h lifecycle.h
  storage/
    image.h overlay.h
  observability/
    trace.h outcome.h
```

`ux/` replaces the proposed `platform/` name: its responsibility is product-facing presentation and interaction, not generic platform policy. Platform-specific code appears only below `ux/win32`, `ux/linux`, `host/win32`, or `host/linux`. Root `ux` headers contain no `HWND`, `HANDLE`, POSIX fd, X11, Wayland, SDL, or platform SDK type. `mailbox` owns copied-frame sequence/data semantics. The native UX implementation owns its own Console/Window event loop and private wake mechanics.

The library follows the same discipline as `core/platform` and `vm/platform`: C11, `type.h` at NXVM boundaries, opaque handles, copied values, explicit create/start/request-stop/join/finalize operations, and one mutable-state owner per object. It does not import SoftPC machine policy, VM profiles, firmware, media, or global "current machine" state.

## Independent Library Components

The five top-level library components are peers, not layers: `ux`, `host`,
`session`, `storage`, and `observability` have no direct dependency on one
another. A native `ux/win32` or `ux/linux` implementation naturally depends on
the public `ux` value contracts above it, but never on `host`, `session`,
`storage`, or `observability`. It owns the full Console/Window event loop,
native input translation, presentation and local wake-up mechanics.

`host` owns only a separately consumable host synchronization capability.
`session` owns a deterministic lifecycle state machine and calls a bounded
product executor callback; it neither creates host threads nor depends on
`host/sync`. `storage` and `observability` are likewise independently useful
capabilities. A product composition root may combine any subset of these
components; no library component composes another.

## Architecture And Ownership

`core` remains host- and profile-neutral. VADP remains the sole guest video state, port, VRAM-mapping, and snapshot owner. The only display flow is:

```text
guest writes -> VADP state -> Core copied display frame -> product conversion
             -> lib/ux copied frame mailbox -> native presenter
```

The Core mailbox is retained: it is the neutral Core-to-product boundary. VM performs the sole representation conversion to `lib/ux`; the library owns only its latest copied presentation cache. Neither mailbox owns guest state.

`lib/session` is not `vm/session` and does not own a machine. It provides the serial lifecycle mechanism over an injected executor callback:

```text
product composition owns machine/profile/session facts
  -> lib/session serializes start/pause/resume/reset/stop requests
  -> injected executor performs product-owned work
  -> typed outcome returns to the product owner
```

NXVM, SoftPC, and NTVDM64 each remain the owner of their machine construction, profile/configuration, guest execution, user policy, and final session state. They may use the same lifecycle mechanism only through bounded calls and typed outcomes. `storage` similarly provides a generic image/overlay capability; a product remains the owner of its media topology, drive/controller meaning, and persistence policy. `observability` never controls execution or becomes an unbounded production trace sink.

This permits a later NTVDM64 adoption without making it a dependency of NXVM or importing NTVDM64 source into this task.

## S1 - Library Corpus, Neutral Contracts, And Migration Ledger

Inventory every staged owner-provided `src/lib` file and the current NXVM Win32/Linux presentation, lifecycle, storage, and diagnostic candidates. Create a finite migration ledger that gives each item exactly one disposition: admit into the declared library root, retain product-local with its reason, or exclude with a named receiver. Verify provenance and notices before source is moved.

Define the public `ux`, `host/sync`, `session`, `storage`, and `observability` contracts only where a current or explicitly identified cross-product capability exists. Do not create empty subsystem roots, speculative Linux Window backends, a generic machine interface, or a framework for a hypothetical consumer.

The S1 ledger must explicitly resolve these existing UX defects before any product cutover:

1. Window presentation state is per instance, allowing independent Window sessions.
2. The process Console lease is the only permitted process-global UX state and contains no session/machine pointer.
3. Window close requests product pause, retains a frozen presenter through the authoritative lifecycle transition, and only terminal state destroys it.

## S2 - Shared Library Restructure And Platform Parity

Move only ledger-admitted code into the declared `lib` roots. Extract the staged Win32 value contracts from native implementation into `lib/ux`; migrate the existing NXVM Linux Console route into `lib/ux/linux`; and implement `host/sync` as an independently consumable capability, not as an implicit UX or session dependency. Preserve the existing NXVM Linux behavior while removing its duplicated host mechanics.

All Window/Console input follows one event path. Native adapters convert only native input to `ux/event`; a product maps registered actions to its own pause, debugger, Ctrl+Alt+Del, Alt+Enter, and mouse-release decisions. This retains ordinary F-keys as guest input and avoids library knowledge of guest commands.

## S3 - Lifecycle, Storage, And Observability Consolidation

Admit the ledger's genuinely reusable lifecycle, storage, and diagnostic mechanisms. Integrate `lib/session` through NXVM composition using injected executor callbacks; remove the equivalent old lifecycle mechanism only if the library is its complete replacement. Do not make `lib/session` select a VM, profile, guest executor, or current session.

Likewise, consolidate only duplicate generic image/overlay and bounded outcome or trace mechanics. Controller-specific disk behavior, session media policy, and guest-visible debugging remain owned by their product/core boundaries.

## S4 - NXVM One-Path Cutover And Retirement

Bind NXVM's VM platform layer to the shared library. In the same delivery, delete superseded NXVM Win32/Linux presenter loops, duplicate native keyboard normalization, and any replaced lifecycle/host-sync route. No wrapper selects between old and new presenters, mailboxes, or lifecycle owners.

The retained UX contracts include one Console lease, independent Window sessions, Console lease retention while a Console-mode guest opens graphics, Window close-to-pause, click capture/release, and the existing product action semantics. Linux uses the same values and binding route; a new Linux graphics backend is explicitly out of scope unless the ledger finds an already-owned implementation suitable for migration.

## S5 - Closure

Run library and product repository-only unit tests, the full unit suite, the external-YAML integration suite, focused Win32 multi-window/Console-lease/close-pause/RDP acceptance, retained Linux presentation regression, source ownership sweep, documentation governance, and stripped x64/x86 artifact builds. Record provenance, the migration ledger, code-size result, and the single retained production path for each migrated capability.

## Non-goals And Stop Conditions

This task imports neither the SoftPC emulator, ROMs, media, product/app code, build tree, generated output, nor tests. It does not add a Linux Window backend merely to fill the directory tree. It does not change guest device, firmware, or controller behavior to fit a host library.

Stop and report if an independent third-party notice is found, a required UX behavior cannot retain one production route, a proposed common API must expose a raw Core/VM/CPU/RAM/device/executor/session pointer, or a supposedly shared mechanism is actually product policy. Do not retain parallel compatibility paths to avoid a difficult migration.
