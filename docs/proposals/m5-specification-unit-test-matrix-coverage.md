# M5 Specification-Driven Unit-Test Matrix Coverage

## Purpose

Establish a finite, source-traceable unit-test matrix for every currently
supported CPU, controller, controller personality and machine/profile contract
before the Pre-Windows L3 admission audit.  The outcome is not a higher test
count: it is one owner-local regression route for every claimed manual or
reference-derived behavior, with repetition consolidated into table-driven
matrices where the inputs and semantics are genuinely shared.

The task covers the five CPU profiles (8086, 8088, 80186, 80286 and 80386),
their admitted instruction, state, exception and timing behavior; every
selected controller/personality (PIC, DMA, PIT, RTC/CMOS, KBC, FDC, VADP CGA
and EGA, and ATA/WD1003/Compaq-WD/Xebec HDC); and the selected VM board/profile
contracts for IBM 5160, IBM 5170, DeskPro 386 Model 40 and `default-pc-at`.

## Authority And Classification

Original CPU, controller and machine technical manuals are the normative
source for each test claim.  The coverage ledger records each row's source
locator, applicable variant/context, state owner, test owner and precision:

- **Manual-L3** only when the manual gives an exact value or executable
  formula;
- **L2** for a manual range, a source-qualified external-emulator model, or a
  proportional macro model; and
- **unsupported/deferred** only with its named earliest receiver.

86Box, MAME, PCjs, Bochs and QEMU are read-only cross-checks.  They may inform
an L2 model or expose a manual/code discrepancy; they never override a manual
or turn an unproven value into L3.  Undefined or reserved architectural state
is tested only for its defined constraints, never a fabricated exact value.

## Boundary And Design

Core remains the sole owner of CPU, bus, controller, device, timing and guest
state.  VM remains the sole owner of profile composition, immutable board
selection, session policy and host-facing behavior.  Tests observe the public
or owner-local test seam of that owner; they do not create profile-side device
models, duplicate Core state, second clock paths, firmware shortcuts or
cross-layer white-box contracts.

All repository-only tests stay below `test/`, mirroring source ownership:
`test/core/cpu`, `test/core/controller`, `test/core/bus`, `test/core/machine`,
`test/vm/profile`, `test/vm/session`, and narrowly shared `test/support`.
Core--VM composition tests remain repository-only large unit tests.  BIOS,
ROM, DOS floppy/HDD and Windows checkpoints stay solely in
`test/integration/`; they are not substitutes for owner-local conformance
coverage and are never copied into fixtures.

One reusable table harness is allowed only where rows share the same owner,
setup lifecycle, failure boundary and assertion semantics.  It must delete
the duplicated fixture/test path it replaces.  Similar-looking behavior with
different owners, reset rules or failure semantics stays in adjacent focused
tests rather than being hidden behind a generic framework.

## Cross-Cutting Regression Rule

Every admitted Core or profile repair first records its finite affected slice;
the direct reproducer alone is never sufficient coverage.  The slice contains
the changed state owner, every supported variant of that owner, the reset and
construction route, each upstream producer and downstream consumer, and every
CPU or profile whose selected composition can reach the changed contract.
The resulting test rows distinguish a shared-Core mechanism from a
profile-specific selection: a Core mapping, CPU, bus or controller correction
must prove all applicable CPU/profile consumers; a profile mapping or
personality correction must prove its selected profile and explicit absence
from the other profiles.  This applies equally to memory-provider precedence,
ROM aliases, port personalities, interrupt/exception frames, reset state,
signal wiring, deadline publication and media/service phase transitions.

The ledger records the direct case and its finite sibling/negative cases in
the same batch.  A table-driven product is used only when the owner, lifecycle
and assertion are genuinely identical; otherwise owner-local adjacent tests
make the boundary visible.  Integration success may confirm the batch but
never replaces its repository-only unit rows.

## Durable Convergence Ledger

Before any implementation batch, this task creates one durable coverage ledger
that maps every in-scope source row to exactly one disposition:

- direct existing owner-local test;
- missing/incorrect test with its implementation batch;
- non-applicable variant with source-backed reason; or
- deferred/unsupported behavior with a named receiver.

Each accepted row names its source locator, precision, owner, variant/context,
test target and assertion class (function, state, reset, fault, signal/order,
timing, mapping or negative-isolation).  It also names the affected-slice
reason: direct behavior, same-owner sibling, producer/consumer, CPU variant or
profile/personality isolation.  Matrix products are finite: only admitted
CPU/profile/controller variants and their documented contexts belong to the
universe.  A test passing at one DOS/ROM checkpoint is integration evidence,
not a substitute for a ledger row.

## Subtask Plan

1. **S1 - source and coverage-universe admission.** Reconcile existing
   CPU/controller/board List 1 material with the selected manuals; freeze the
   finite unit-test universe, source quality limits and L3/L2 definitions.
2. **S2 - existing test audit.** Classify every current repository-only test
   by owner, source row(s), input contract, duplicate/obsolete disposition and
   reusable matrix candidate.  For every existing regression, identify its
   finite same-owner, CPU/profile, producer/consumer and negative-isolation
   cases; no test is deleted merely because it appears redundant.
3. **S3 - CPU matrix design.** Map all five CPU instruction, state, delivery,
   FLAGS, prefix, reset and timing rows to owner-local test modules and finite
   parameter products; every admitted ModRM instruction covers its supported
   register and addressing variants, so one AX form cannot mask a different
   register-selection or write-back defect.  Cover real-mode `INT` frame
   creation and `IRET` round trips across all five CPUs, then the applicable
   80286/80386 protected-mode fault, segment and atomicity cases; identify
   shared test mechanisms that can replace duplicate fixtures.
4. **S4 - controller matrix design.** Do the same for every admitted PIC,
   DMA, PIT, RTC/CMOS, KBC, FDC, VADP and HDC personality row, including
   signal chains, reset, state transitions and L3/L2 timing assertions.  For
   every personality-specific register or phase repair, add its positive
   personality row plus unaffected sibling-personality rows and the complete
   upstream/downstream signal or deadline path.
5. **S5 - VM and board matrix design.** Map IBM 5160, IBM 5170, Model 40 and
   `default-pc-at` board/profile/CMOS/ROM mapping, ports, IRQ/DMA wiring,
   device presence and session contracts to VM-owned unit tests.  Explicitly
   cover RAM/device/ROM/fallback precedence, immutable alias source/target and
   write protection, selected board mappings and their absence from unrelated
   profiles, plus every selected CPU/profile construction combination.
6. **S6 - matrix-harness consolidation.** Implement only the accepted common
   table mechanisms and retire proven duplicate fixtures/registration paths.
7. **S7 - CPU implementation and sweep.** Add or correct every accepted CPU
   matrix row, then run the complete five-profile CPU sweep.
8. **S8 - controller implementation and sweep.** Add or correct every
   controller/personality row at its existing Core owner, then run the whole
   controller matrix and all affected CPU/VM regressions.
9. **S9 - VM/board implementation and sweep.** Add or correct every
   profile/board composition row at its existing VM owner, then run the
   complete profile matrix.
10. **S10 - closure.** Reconcile the entire ledger, remove remaining proven
    duplicate test paths, run complete repository-only unit and owner-managed
    integration gates, and transfer every non-eligible row explicitly before
    Pre-Windows L3 audit admission.

No implementation subtask begins before S1--S5 establish the complete ledger.
Each S closes with the complete repository-only unit suite; T closure also
runs the integration suite.  A focused selection is transient to its S and is
never retained as a third test tier or fixed manifest.

## Exit Criteria

- Every claimed supported CPU, controller/personality and VM board/profile
  behavior has exactly one ledger disposition and one regression owner.
- Every Manual-L3 and L2 claim has a source-qualified owner-local assertion;
  no L3 claim is inferred from an external emulator or integration success.
- Every repair-class row has its complete finite affected slice: direct case,
  same-owner variants, applicable producers/consumers, selected CPU/profile
  combinations and explicit unaffected or absent variants.
- Reusable finite matrices replace only real duplicate fixtures and do not add
  a second model, state owner, parser, scheduler or profile path.
- All retained unit tests are repository-only under `test/`; external assets
  are isolated to `test/integration/`.
- Complete unit and integration gates pass, and all remaining unsupported or
  non-eligible rows have a named receiver before the Pre-Windows audit begins.
