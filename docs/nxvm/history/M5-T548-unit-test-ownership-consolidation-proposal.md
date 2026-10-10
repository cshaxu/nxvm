# Global Unit-Test Ownership And Coverage Consolidation

## Request And Admission

Owner requested this implementation-task proposal at the head of the NXVM
queue on 2026-10-09, following the global static ownership audit. It remains an
unnumbered M5 candidate until implementation admission assigns a numeric T and
its active packet. This proposal does not replace the active T547 packet.

## Goal

Give each tested behavior one canonical component-owned unit-test home. Move
misowned tests to their production owners, consolidate redundant assertions,
and retain higher-layer tests only for their own directly relevant contracts.
Higher-layer units may use real lower components; they need not repeat the
lower component's correctness matrix. Preserve unique regression conditions,
failure cases, CPU families, machine configurations and wiring evidence.

The owner's preference that each component/file be covered once means one
canonical owner for each behavior and context, not prohibiting execution of a
source file by multiple tests. Source-file presence or identical function calls
alone cannot establish equivalent coverage. Different failure conditions,
parameter ranges, bus traces and composition contexts remain distinct evidence.

## Baseline And Initial Findings

The initial read-only audit observed committed baseline `9f966695e` and ongoing
uncommitted T547 changes. Admission must freeze the accepted source/test/build
baseline and repeat the inventory against it; this candidate authorizes no
change to those unrelated working-tree changes.

The initial scan found 512 C entry-file candidates outside `integration`
directories: Lib 37, Emulator 16, Product 9, Core 335, MyNES 44 and four PC Apps
71. These counts include runners/probes and are not CTest counts or measured
coverage. Among the PC App entries, 53 include Core Machine's private header:
NXVM 29/43, My5160 1/1, My5170 3/3 and MyDeskPro386 20/24. Private dependency
is a review trigger, not proof that a test is redundant.

Confirmed and candidate batches from source inspection:

| Surface | Initial finding | Required disposition |
| --- | --- | --- |
| `test/app-nxvm/unit/product/nxvm_ini_smoke.c` | Tests factory null-request rejection/output clearing, not INI; `test/core/product/factory_smoke.c` already covers that condition. | Remove the duplicated condition or replace it with a genuine App-binding contract, with exact receiving assertion evidence. |
| `test/app-nxvm/unit/machine` | Cancellation, reconfiguration, worker lifecycle, speed, error propagation, isolation, input and Debug mapping primarily exercise `core/machine`. | Move canonical adapter assertions and neutral fixtures to `test/core/machine`; preserve actual profile-specific increments. |
| App FDC/HDC/EGA/RTC tests | Mix chip algorithms, Board port/IRQ/DMA routing and selected-profile values; corresponding chip and Board suites already exist. | Compare case by case, consolidate generic logic into Core, and keep App selection/wiring increments. |
| Four App `unit/board` CPU timing runners | 14,731 lines include CPU retirement/form timing as well as machine contexts; some are qualification tools rather than passing unit registrations. | Separate CPU/Core, Board and profile evidence; preserve timing-ledger consumers and distinguish tools from registered units. |
| App `core_machine_port_assembly`, `machine_time`, `machine_competition` | Generic transactions/time/competition are mixed with DeskPro D4 attachment or deadlines. | Split canonical Core tests from App-owned D4 increments without losing transaction/failure coverage. |
| NXVM `nxvm_ibm_5170_direct_plan_smoke.c` | Contains another App's fixed profile contract. | Assign 5170 assertions to My5170 and default assertions to NXVM. |
| Model40 CECG/controller/media tests | Generic mechanisms overlap Core tests; D4 decode, fixed geometry, CMOS seeds and selected routes are genuine App contracts. | Split mixed assertions; retain Model40-specific proof. |
| PC Surface and MyNES Product tests | Shared lifecycle wording, command state matrices and help formatting are repeated above Emulator Product. | Keep the full shared matrix in Emulator Product; retain delegation, preflight, extension, Debug and snapshot increments in consumers. |
| MyNES `unit/core` | CPU/PPU/APU/mapper implementations genuinely live in `src/app-mynes/core`. | Preserve their owner; distinguish mathematical-result matrices from unique bus traces before deduplication. |
| `test/verify_test_boundaries.cmake` | Checks include/link boundaries, not assertion ownership; build inspection does not recursively inspect included registration files. | Review against the actual component graph; strengthen finite registration/fixture checks without claiming semantic ownership can be proven by include checks. |

The audit found no byte-identical duplicate entry files. It did not establish
semantic uniqueness of all assertions or runtime source/branch coverage.

## Scope And Targets

Inventory all repository unit tests, including C tests, executable test scripts,
negative gates, registration aliases, fixtures and qualification runners.
Separate registered units from integration, native desktop, benchmark and
research/probe tools before counting or assigning ownership. Integration is a
regression consumer and must not silently be deleted or relabeled as unit.

Declared implementation targets are Shared, NXVM and MyNES. Shared changes cover
Lib, Emulator and Product test/build contracts; NXVM changes cover Core and the
four PC Apps; MyNES changes cover its App-owned tests and registrations. Each
admitted S declares its actual targets, all consumers and separate P boundaries.
SoftPC is a read-only downstream consumer/reference; sibling edits or imports
require their own explicit authorization and provenance.

Expected changed surfaces are `test/`, corresponding CMake registrations,
fixture support, manifests, finite boundary verifiers and task evidence.
Production behavior, public runtime ABI, firmware, media, user INIs, snapshots,
hardware timing and machine identities are non-goals. Do not modify production
solely to make tests easier to relocate. A necessary production repair requires
separate admission or an explicitly approved scope revision.

## Convergence Ledger

Before implementation, freeze one durable task ledger under task evidence and
link it from the active packet. Its universe contains every tracked production
implementation file, test entry and supporting fixture/registration in scope.
Record headers/build-only owners separately rather than inventing executable
coverage requirements for them.

For each behavior/case or cohesive assertion group, record:

- production component and source file/function;
- present test path, registered names, labels, targets, build variants and fixtures;
- asserted behavior, failure predicate, input range and required execution context;
- canonical receiving test and any directly relevant higher-layer increment;
- disposition: retain, move, split, merge/remove duplicate, non-unit tool,
  coverage gap, or explicitly deferred with reason and named receiver;
- before/after assertion mapping, verification command/result and review evidence.

For each production file, record its mapped tests and unresolved coverage gaps.
Use `not measured` for execution coverage unless instrumented results establish
it. Missing ownership evidence cannot become `covered` through naming or linking.
Every deletion must identify an equal-or-stronger same-context receiving case.
Existing unresolved qualification work remains linked to its original ledger.

## Proposed Sequential Batches

1. Freeze full inventory, registration graph, assertion-owner ledger and exact
   baseline suite counts. Finish semantic review beyond the initial examples.
2. Consolidate Core Product and generic Machine tests/fixtures; retire the
   confirmed duplicate factory condition and fix wrong-App profile ownership.
3. Split chip/Board/App controller and display tests, preserving fixed-profile
   values and actual IRQ/DRQ/memory/clock connections.
4. Rehome CPU timing tools and split mixed time/transaction/D4 evidence. Update
   every ledger, script and build consumer of the moved paths.
5. Consolidate shared Emulator Product contracts and consumer increments;
   audit remaining Lib, Product tools and MyNES cases with the same ledger.
6. Perform complete qualification and actual-change/assertion-preservation review.

These are planning batches, not preallocated S identifiers. Each admitted S
consumes a complete finite ledger batch with its own exit criteria.

## Verification And Exit

Before each batch, capture registrations/counts and affected case coverage.
After it, build/run affected owners on x64 and x86, verify their manifests and
dependency gates, and prove the receiving package builds without an unintended
peer-App test dependency. Shared fixtures belong to the narrowest actual owner;
do not move everything into one global fixture library.

Final qualification runs the complete registered repository-only unit suite on
x64 and x86, including all affected product consumers. Compare exact names,
counts and assertion mappings before/after; reduced counts are acceptable only
when every retired case has its documented receiver. Run affected integration
and desktop gates when available; explicitly report unavailable or unrun gates.
Do not claim artifact rebuild/publication merely from test-only qualification;
any required artifact action must be named in its admitted packet.

Completion requires disposition of every frozen ledger member, correct canonical
ownership, no unexplained registration aliases, no unreviewed loss of unique
assertions, and preserved higher-level configuration/transaction/lifecycle proof.
Coverage gaps may be repaired within admitted test scope or explicitly transferred
to approved receivers; transfer does not establish runtime coverage. Report added,
removed and net source/test lines and counted changed paths per execution rules.

Stop for a missing equivalent receiver, weakened failure condition, changed
hardware/firmware expectation, an unexpected behavioral failure, protected-source
issue or a required undeclared production/sibling change. Preserve the original
case until the ledger and admission resolve the issue.

## Authorities

Read the [NXVM guide](../README.md), applicable
[MyNES guide](../../mynes/README.md), current packets and
[Execution rules](../../rules/EXECUTION.md). Apply the selected products'
architecture, coding and source policies, shared rules and documentation gates.
This proposal specifies prospective work; Current alone owns active task status.
