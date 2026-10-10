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
