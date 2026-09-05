# M5 SoftPC Platform-Library Adoption And Win32 UX Convergence

## Purpose

Determine whether the owner-provided sibling `softpc/src/lib` can become the
one NXVM host-platform implementation, replacing the current
`src/core/platform/win32` implementation without introducing a second host
state, presentation path, or input path.  This is an admission candidate, not
permission to copy source.

## Required Outcome

If admitted evidence proves the import is permitted and the interfaces can be
made coherent, NXVM has one retained platform library that supplies the Win32
window, console lease, host-input routing, mouse capture, frame presentation,
and event-queue mechanics.  NXVM retains ownership of sessions, Core/VADP
guest state, guest-key normalization, debugger semantics, and product command
authority.  The old NXVM Win32 implementation is removed in the same change;
there may be no compatibility adapter, dual event loop, duplicate renderer,
or parallel input/capture state.

## S1 - Research And Gap Disposition

Read the sibling library, its build metadata and provenance record,
and NXVM's current platform, composition, UX and build surfaces.  Produce a
finite interface ledger for every `softpc/src/lib` public owner and every
current NXVM Win32 owner, including lifecycle, thread/event ownership, copied
frame flow, console lease, native/RDP input, mouse capture/release, pause and
window-close semantics, multi-session behavior, diagnostics, build and x64/x86
requirements.

S1 must explicitly dispose of these admission gates:

1. the owner's 2026-09-05 migration authorization recorded with the exact
   source inventory, plus a check that no individual file carries an
   independent third-party notice requiring separate treatment;
2. a source-only import plan that excludes SoftPC runtime assets, build trees,
   generated files and unrelated product code;
3. one-way NXVM ownership: the library is a host adapter and cannot own Core,
   VM, profile, session, guest memory, VADP state or firmware policy;
4. a complete mapping from existing NXVM UX contracts to the candidate library
   with no lost behavior, especially one Console lease plus independent Window
   sessions; and
5. a build/test route that preserves self-contained unit tests, external-asset
   integration tests, and stripped x64/x86 artifacts.

Any independent third-party notice requiring separate treatment, incompatible
ownership boundary, required retained parallel path, unsupported required UX
behavior, or unbounded rewrite is a gap.  S1 stops after recording the
evidence and reports the gap to the owner; S2 is not admitted in that case.

## S2 - One-Time Integration And Retirement

Only after S1 records no unresolved admission gap and the owner approves the
S1 disposition, import the exact accepted library source, normalize it to
NXVM's coding and public-boundary rules, connect it at the existing Core-to-VM
platform frame/input boundary, and remove the superseded NXVM Win32 platform
source and build declarations.  Do not wrap both implementations behind a new
generic facade.  Retain the narrowest interface already required by NXVM, and
make the library's state lifetime session-local where the current product
requires it.

The integration must preserve the user-visible T518 contract: explicit
console-versus-window session behavior, one Console lease, multiple independent
window sessions, click-to-capture, explicit release/pause, host shortcuts
before guest input, and ordinary guest keyboard delivery.  It must also retain
VADP as the sole guest-video owner and copied snapshots as the only
presentation input.

## Evidence And Verification

S1 records the exact SoftPC revision, source-file inventory, authorization and
notice disposition, API/lifecycle ledger, NXVM deletions, and every gap.  S2
records provenance for each imported or substantially derived unit, actual
before/after owner paths, counted source/test delta, and a similar-issue sweep
across all platform backends and session modes.

Every S runs the complete repository-only unit suite.  S2 additionally runs
the complete external-YAML integration suite, relevant Win32 host acceptance,
documentation governance, clean x64/x86 Release builds, and verifies stripped
task artifacts.  The task cannot close while a required UX behavior survives
only through the retired NXVM Win32 path.

## Non-goals And Stop Conditions

This candidate does not import SoftPC's emulator, guest devices, profiles,
firmware, media, product command layer, test fixtures, build output, or
assets.  It does not change Core/VADP guest semantics merely to fit a host
library.  It does not create a cross-repository source dependency.

Stop and report before source import if S1 finds an independently licensed
component requiring separate treatment, a required NXVM UX contract lacks a
single-owner mapping, or the candidate would require a permanent parallel host
path.  A failed gate is a useful S1 outcome, not a reason to copy a partial
subset.
