# M5 NXVM Console Redesign And Frozen YAML Session Profiles

## Purpose

Replace the accumulated default-machine and mutable-session Console model with
an explicit zero-or-more-session model.  NXVM starts with no session and
preserves its existing copyright banner and `Type HELP` line, then enters the
same `SESSION OPEN` selection path that a user invokes later; it must never
silently construct a default PC/AT session.

At process startup NXVM discovers valid YAML profile files beside the NXVM
executable, snapshots them in memory, and exposes them through `SESSION OPEN`.
The menu displays the configuration **file name only**, in alphabetical order;
there is deliberately no schema `name` field and no separately authored menu
label.

This candidate is a prerequisite to DeskPro Model 40 work because the selected
machine must be explicit, reproducible, and inspectable before another
baseline profile is admitted.

## Required scope

- Start with zero sessions.  Startup invokes the ordinary `SESSION OPEN`
  chooser, after the retained banner and help line, instead of creating a
  default session.  Cancelling that chooser leaves the Console usable with
  zero sessions.  When no valid configurations exist, it reports that fact and
  remains in that same zero-session Console state.
- Discover only `*.yaml` and `*.yml` files in the executable directory,
  non-recursively.  Parse each file once before the Console begins; sort valid
  candidates by case-insensitive file name with a deterministic bytewise tie
  break.  A change to a file after startup has no effect until the next NXVM
  launch.
- Report an unreadable, syntactically invalid, or schema-invalid file outside
  the selection menu with a concise diagnostic, but do not make it selectable
  and do not prevent NXVM from starting.  The selection menu itself contains
  valid candidate file names only.  A valid file represents a frozen in-memory
  configuration snapshot, not a live file handle.
- Make `SESSION OPEN` select a numbered snapshot and create one session from
  it.  The selected file name and profile identity remain visible through
  `INFO`.  `SESSION LIST`, `SELECT`, and `CLOSE` retain their ordinary
  session-management roles and work when the session count is zero.
- Remove user-facing session construction and configuration mutation from the
  Console: no `SESSION CREATE`, direct `--profile`/CPU/FPU overrides, in-Console
  machine-setting edits, profile persistence, or YAML writing.  The user edits
  a file outside NXVM, then restarts NXVM to obtain a new snapshot.
- Permit only removable floppy media to change after session creation.  The
  Console may eject or insert a floppy while the machine is stopped; it updates
  the running session only and never writes YAML.  A reset preserves the
  currently inserted floppy.  Hard-disk media are fixed at session creation:
  they may be absent, but may not be attached, detached, or changed through
  the Console after the session is opened, including while stopped.
- Reject YAML fields that conflict with the selected profile's immutable
  topology.  In particular, the IBM 5170 Model 339 profile remains fixed at
  80286/8 MHz, 512 KiB, CGA, and no HDC; a YAML file cannot turn it into a
  generic AT, add a hard disk, or override its CPU or memory.

## YAML schema v1

The root is a mapping with exactly `schema`, `machine`, and `media`.  Unknown
keys are rejected.  The only accepted schema value is `nxvm-session/v1`.
Scalar values use the explicitly enumerated strings and non-negative integer
forms below; aliases, tags, merge keys, duplicate keys, environment expansion,
and implicit type coercion are not part of v1.

```yaml
schema: nxvm-session/v1
machine:
  profile: ibm-5170-model-339
  display: window
  boot: floppy
media:
  floppy:
    image: O:/assets/nxvm/fdd.img
  hard_disk: null
```

`machine.profile` is required and names a product-owned profile.  `display`
is required and is one of the retained NXVM presentation modes.  `boot` is
required and is one of `floppy`, `hard_disk`, or `rom`; the chosen non-ROM
device must be present in `media`.

`media.floppy` and `media.hard_disk` are each either `null` or a mapping whose
required `image` is a non-empty path.  Relative image paths resolve against
the YAML file's directory during startup, then the resolved value becomes part
of the frozen snapshot.  Image bytes are not copied or committed by this
feature.  Image-format behavior remains owned by the existing storage work;
this schema neither adds a format nor changes raw-IMG support.

For a profile that deliberately exposes configurable CPU state, `machine` may
also contain profile-declared fields such as `cpu`, `fpu`, and `memory_kib`.
Their allowed values and defaults are owned by that profile.  Fixed baseline
profiles do not admit those fields.  For example, a generic PC/AT profile may
use the following complete startup description:

```yaml
schema: nxvm-session/v1
machine:
  profile: default-pc-at
  cpu: 80386
  fpu: none
  memory_kib: 16384
  display: window
  boot: hard_disk
media:
  floppy: null
  hard_disk:
    image: O:/assets/nxvm/hdd.img
```

No `name` key exists.  Renaming a valid file changes its displayed menu text
and its relative-path base only at the next NXVM launch.

## Command-model disposition

This work defines one session-selection path rather than adding a menu beside
the old commands.  Session commands are global (`LIST`, `OPEN`, `SELECT`,
`CLOSE`), as are `HELP` and `EXIT`.  These are the only commands valid when
there is no selected session.  Machine, media, run-control, observation, and
debugger commands act only on the selected live session.  Every other command
in the zero-session state fails without side effects with the exact guidance
`No session selected. Use SESSION OPEN.`  The implementation must inventory
every retained Console command and publish its target, stopped/running
precondition, and error behavior before removing or migrating a legacy
spelling.

The startup interaction is consequently:

```text
[existing copyright banner]
Type HELP for help.

Available session profiles:
  1  5170-model-339.yaml
  2  default-at.yaml

Select profile [1-2, Enter to cancel]:
```

The chooser's candidate lines contain the ordinal and file name only.  They do
not expose a YAML-authored label or profile description.  On cancellation, or
when no valid files are found, NXVM writes a concise no-session diagnostic and
returns to the Console prompt.  A later `SESSION OPEN` invokes that same
chooser.  A successful open selects the newly opened session automatically.

The direct current-session commands may retain operational actions such as
`INFO`, `START`, reset/stop/resume, record, and debug.  The detailed spelling
is an implementation design decision, but it must not reintroduce a parallel
machine-construction path or a test-only Console API.  Floppy insertion and
ejection use the retained current-session media command surface; no new
generic `DEVICE` or `SET` machine-configuration spelling may be retained just
to configure YAML-owned state.

## Expected ownership surface

`src/vm/product/` is the primary owner of the Console grammar, startup chooser,
profile-catalog presentation, and no-selected-session rejection.  It is not
the whole expected changed surface.  `src/vm/composition/session/` owns
creating a selected session and binding its frozen profile/media values to the
machine-provider contract, and `src/vm/main.c` owns product assembly.  Product
and runnable-session tests, their CMake registrations, and only the narrow
platform path-discovery adapter proven necessary to locate the executable
directory may also change.  Core machine/device behavior and profile topology
are out of scope.  Owner approval on 2026-08-15 separately admits the sole
core exception: the generic product-session manager may represent zero
sessions, create no implicit default session, and close its last session.  It
may not acquire YAML, profile, Console, media, or machine behavior.

## Dependencies and verification

This candidate may be admitted only after the active T380 S2 corrective work
closes.  It is intentionally the first M5 candidate, before the DeskPro Model
40 profile/capability audit.  It must prove, through product-facing Console
tests, that no default session is created; startup and explicit open share one
selection mechanism; file ordering and invalid-file containment are
deterministic; YAML snapshots do not observe post-start edits; fixed profiles
reject topology conflicts; floppy mutability is bounded; and hard-disk
mutability is rejected.  Documentation governance and the full current gate
remain required.

## Non-goals and stop conditions

No DeskPro, XT, CPU, device-function, timing, ROM, firmware, disk-image
format, persistent save-state, GUI launcher, or Windows feature work is
included.  This candidate does not introduce external BIOS-ROM selection: that
requires the separate owner-approved provenance, mapping, validation, and
legal-boundary design required by the source policy.  Stop for owner direction
if parsing requires a third-party runtime dependency, a public process CLI,
guest-media import, a profile topology change, or a hard-disk hot-swap
exception.
