# M6 T43 S13 Stopped Startup

## Request And Boundary

Owner approves SoftPC-aligned initial STOPPED state on 2026-09-25, with explicit
start required, full dual-width verification, artifacts and commit/push. Baseline
is S12 c00ce0e41. Only MyNES changes; Shared, NXVM, INI and owner snapshots remain
untouched. T43 stays open for manual verification.

## Mechanism And Sweep

Common already initializes its machine STOPPED. MyNES composition passed
`initial_reset = cartridge_present`, and command open requested RESET, which
correctly ended PAUSED. Remove that product option and request. Composition now
publishes the authoritative initial STOPPED fact for both cartridge cases.
One existing pending-initial-fact flag gates prompt admission; no second machine
state, executor or lifecycle API is introduced.

Sweep: `rg -n "initial_reset|initial_state_pending|suppress_window_after_reset|report_suppressed_reset|app_command_initialize" src/app-mynes test/app-mynes`.
All initializer callers are migrated. No initial_reset production/test hit
remains. Explicit start still requests reset then resume; explicit reset and
interactive insertion still finish paused. The remaining reset suppression
flags serve those explicit commands and are not dead startup state. Snapshot
load remains stopped-to-paused; its schema and restore path are unchanged.
Live UI, architecture and App/Core startup descriptions are reconciled;
historical task evidence retains its historical behavior.

The command regression covers ROM absent/present times Console/Window: open
issues no lifecycle request, waits for initial STOPPED, then arms a clean prompt.
Native Console and Window tests now exercise explicit start from STOPPED;
Window absence before start, gameplay/input and later pause/resume remain checked.
Other initializer changes retain their debug/media/snapshot assertions.

## Verification

Both existing Release trees build with `cmake --build build/mynes-gcc-<arch>-release --parallel 8`.
Full verification uses `ctest --test-dir build/mynes-gcc-<arch>-release --output-on-failure -j 4`.
Final complete suites: x64 132/132 (101.47 seconds), x86 132/132 (89.20 seconds).
These include registered Shared gates, repository-only unit tests and generated-ROM
integration/native presenters; they do not claim a new external-game qualification.
MyNES documentation governance and git diff/check pass; all six Shared roots and
NXVM have zero changes. Current build trees are retained for the owner's immediate
verification/diagnosis; no new trace or external-ROM copy was produced.

The first x86 deployment met an existing running 0043 process lock; only that
verified MyNES process was stopped under standing owner authorization, then
deployment succeeded. The first x64 suite exposed a native Console test race:
RUNNING preceded raw-buffer takeover, so CONOUT$ could bind the cooked buffer.
The fixture now waits for the existing BROKER_COMPLETED raw-console event before
opening CONOUT$; no production delay or Shared modification was added. The
corrected native Console test and both subsequent complete suites pass.

Code count uses `git diff --numstat c00ce0e41 -- src/app-mynes test/app-mynes`:
production +10/-25 (net -15), tests +64/-21 (net +43), total +74/-46 (net +28).
Test growth proves the four startup variants and actual native handoff; production
removes the special path. Documentation/artifacts are excluded from those counts.

## Artifacts And Review

Current product identities remain MyNES 0.0.0043; artifacts are optimized stripped
Release with PE x64/x86 architecture and no .debug sections. Paths remain
`assets/mynes/mynes_0_0_0043_x64.exe` and `mynes_0_0_0043_x86.exe`.

- x64 SHA-256: `5DC59A8C259CED04C301119888538212F0E95807A360CAE8D334AC0B57833FBA`.
- x86 SHA-256: `2E4CB994451549FE9690AF1FF7DDEFC13715B42DDB6D2F792319FEE065804AE2`.

Implementation-P identity pins the source and both binaries. Coordinator review
must inspect actual changes against the original request before acceptance;
manual gameplay acceptance remains with the owner.
