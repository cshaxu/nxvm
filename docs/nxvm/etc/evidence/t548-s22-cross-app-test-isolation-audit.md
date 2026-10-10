# T548 S22 Cross-App Test Isolation Audit

## Scope

This read-only audit reviews current non-integration C/H test dependencies
between `app-nxvm`, `app-my5160`, `app-my5170`, and
`app-mydeskpro386`.  A cross-App production or test-fixture include is a review
trigger: a selected-profile assertion may need data from a lower component, but
one App test package must not become the support owner for its peers.

## Confirmed Findings

### A. Default-PC board tests directly depend on DeskPro D4

The following Default-PC paths include the DeskPro D4 platform, or its test
fixture, and are linked against the DeskPro D4 target:

- `test/app-nxvm/unit/board/machine_time_smoke.c`;
- `test/app-nxvm/unit/board/machine_competition_smoke.c`; and
- `test/app-nxvm/unit/board/core_machine_port_assembly_smoke.c`.

They do not prove Default-PC profile behavior.  `machine_time_smoke` combines
generic Core time-axis validation with a D4 refresh-deadline ordering case.
`machine_competition_smoke` combines generic DMA arbitration/hold checks with
D4 attachment.  `core_machine_port_assembly_smoke` exercises generic
controller rollback plus optional D4 Port-B attachment.  Their existing paths
and targets therefore obscure three distinct owners.

Required receiver rule: move only the D4-specific cases to MyDeskPro386; move
generic Core board/machine cases to Core only after identifying an equal or
stronger Core execution context.  Do not transfer a mixed file wholesale or
delete any assertion before its receiver is recorded.

### B. `app-nxvm` incorrectly owns multi-App fixture support

`test/app-nxvm/unit/support/profile.h` includes all four PC App profile
construction interfaces and DeskPro private ROM state.  My5160, My5170 and
MyDeskPro386 unit tests include it or adjacent `app-nxvm` fixture trees.
Examples include My5160 `profile_smoke.c`, My5170 Model 339 composition/clock
tests, and DeskPro composition, media, CMOS and D4 tests.

This is not a Core fixture: it constructs App-owned profiles.  It is also not
a Default-PC fixture because other Apps use it.  The correct repair is to give
each App a local fixture for its own profile construction and to place only
neutral board/media observation helpers under their narrowest Core test owner.
Creating a new cross-App fixture layer would preserve the same reverse test
dependency and is not an acceptable substitute.

### C. Existing aggregate checks do not prove isolation

`verify-fixture-shapes` records direct constructor ownership, but it
intentionally lists the two mixed Default-PC paths as retained constructors.
It does not inspect cross-App source/test include edges.  `verify-unit-
registration` establishes CTest route existence, not semantic component
ownership.  Their current success does not contradict Findings A or B.

## Disposition

S22 establishes a real test-ownership repair batch, not a task-name cleanup.
The next implementation receiver must:

1. map every assertion in the three mixed Default-PC board files to Core or
   MyDeskPro386;
2. replace cross-App test-support includes with App-local or neutral Core
   fixtures, preserving fixture lifetime and failure paths;
3. update CMake registrations, manifests and finite ownership gates; and
4. run affected Core and all four PC App unit tests on x64 and x86.

No source, CMake registration, test assertion, artifact or external asset was
changed by this audit.

## Expanded repository-wide inventory

The current graph has 545 non-integration C test/support/fixture files and 45
integration C files. Fresh configured x64 and x86 graphs each expose 512
`unit` CTest routes with no duplicate CTest name. The registration verifier
reports 334 executable unit registrations; the remaining routes are static or
script-driven gates. Both widths pass the current registration and
fixture-shape verifiers.

Those results prove registration uniqueness, not semantic ownership.

### Additional confirmed topology defects

1. PC App unit routes are registered centrally by
   `cmake/nxvm/NxvmProduct.cmake`, rather than by independently selectable App
   packages. CTest has no unit labels for Default-PC, My5160, My5170 or
   MyDeskPro386; `-L unit` selects only the combined graph.
2. The cross-App helper issue is broader than `profile.h`: My5160 has one,
   My5170 has four, and DeskPro has nineteen unit/integration sources that
   consume `test/app-nxvm` support. Neutral Core board/machine fixtures and
   App-private profile/ROM construction support must be separated rather than
   moved wholesale into another App.
3. `nxvm_machine_initialization_atomicity_smoke.c`,
   `nxvm_timing_qualification_smoke.c`, and My5170's clock-contract smoke
   consume peer profile facts. They are family-qualification candidates, not
   Default-PC or My5170 unit ownership.

### Non-findings and limits

- No byte-identical C entry source and no duplicate CTest name was found.
- `test/core`, `test/lib`, `test/emulator`, and `test/product` already have
  standalone roots with inward production dependencies; their valid lower-layer
  use is not reclassified as a defect.
- Static verifier path references are evidence consumers, not duplicate CTest
  registrations.
- This inventory cannot prove semantic equivalence or branch coverage. A later
  deletion requires an equal-or-stronger receiving assertion in the same
  execution context.

## Bounded repair sequence

1. S23 rehomes support and registration topology without changing assertions.
2. S24 splits Core, fixed-profile and cross-profile assertion groups.
3. S25 resolves remaining semantic duplicate candidates from a behavior ledger.
4. S26 performs final component-ordered dual-width qualification and validates
   the resulting ownership boundaries.

## S23 implementation evidence

S23 moved generic boot, controller, CMOS, video, media, selection and machine
fixtures from `test/app-nxvm/unit/support` to their narrowest Core owner.  The
four PC Apps now retain only profile-specific construction or ROM helpers.
Six tests that compare PC profile facts now live under
`test/core/machine/qualification`; their source paths make their multi-profile
scope explicit.

The qualification-only Model 40 byte fixture owns the one shared consumer
case.  This removes the former Default-App integration dependency on DeskPro
test support without copying a fixture into a second App tree.

Each App boundary check now also scans its own C/H/CMake test sources and
rejects an include of another App's `support` tree.  Fresh x64 and x86 CMake
configuration passed all four App source and test-support boundaries.  CTest
labels make selectable owners visible: `app-nxvm` (42 registrations),
`app-my5160` (1), `app-my5170` (2), `app-mydeskpro386` (24), and
`pc-qualification` (6).  The remaining unit routes are Core or shared
component tests, not unlabelled App routes.

The complete Ninja graph and unit suite passed on both widths:

- x64: 1,098 build steps; 512/512 unit routes passed (242.54 seconds);
- x86: 1,337 build steps; 512/512 unit routes passed (45.08 seconds).

The `test/core` manifest and whitespace check also pass.  No C assertion,
production source, artifact input or integration route was removed.  S24 owns
the still-mixed assertion semantics identified in Finding A; S23 changed only
fixture/registration topology.
