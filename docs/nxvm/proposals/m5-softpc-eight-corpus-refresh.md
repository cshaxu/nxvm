# SoftPC Eight-Corpus Audit And Refresh

## Approved Goal

The owner admits a new T to audit SoftPC's latest eight shared corpora and,
only if suitable, import them unchanged and connect existing receivers.
The eight roots are src/test of lib, common, x86 and ibmpc. This task precedes
the queued CPU gap repair without consuming or discarding its findings.

Baseline: NXVM 826eccc93. Source: owner-authored SoftPC sibling, read-only,
with committed HEAD and dirty paths recorded independently. Pin a reproducible
accepted source before copying; never silently combine snapshots or modify
SoftPC. Preserve MIT notices and record each root's identity/provenance.

## Scope And Constraints

- Audit every changed/new/removed file, manifest, public contract, dependency,
  registration and test relocation against current NXVM and MyNES consumers.
- Preserve CPU implementations, fixed PC compositions and original regression
  assertions. Test relocation must have a complete old-to-new receiver map;
  a missing file is not automatically permission to delete its coverage.
- Import the accepted eight source/test trees byte-for-byte. Required shared
  test-root helpers are explicit supporting inputs, not product dependencies.
- Adapt receiving build/App code at existing boundaries. No compatibility
  fork inside imported trees, second runtime or unrelated CPU gap repair.
- A quality, source, provenance, manifest or lost-coverage gap blocks that
  import; report the exact issue instead of repairing upstream without review.
- Shared, NXVM and MyNES are separately declared targets and separately
  committed. Owner INI/media/snapshot values and sibling files are untouched.

## S Plan

1. S1: complete eight-root difference and importability audit, including dirty
   versus committed source, root helper dependencies, whole test relocation
   map and actual contract/code review. Record an explicit import decision.
2. S2: if S1 accepts, pin/import exact corpora and supporting helpers; retire
   replaced paths with coverage proof, adapt existing receiver registrations,
   run manifests/boundaries and full units. No selective 'same enough' claim.
3. S3: complete clean owner-local Lib tests as specified below.
4. S4: complete clean owner-local Common tests as specified below.
5. S5: complete clean owner-local x86 tests as specified below.
6. S6: complete clean owner-local IBMPC tests as specified below.
7. S7: verify all receiving PC and MyNES products, original external integration
   and dual-width optimized artifacts; publish target-separated commits and
   close only after the full ledger is satisfied.

The owner adds S3-S6 after the accepted S2 import and the read-only test-owner
audit. The original, never-admitted S3 final qualification moves to S7; none of
its integration or artifact requirements is removed. These are planned briefs,
not active implementation packets.

## Owner-Approved Test Completion

The coverage unit is a component-owned contract and its applicable success,
failure, lifecycle and boundary cases, not an API-name hit or a test count.
Each S first inventories its complete public contracts and existing internal
mechanism tests, then records proof, non-applicability or a concrete gap for
each. A caller's test is not proof of the callee's complete contract. Maintain
an old-to-new assertion map for every relocation; preserve unique integration
and model assertions rather than deleting them as apparent duplicates.

Each package must build and run with its own src/test, required inward source
dependencies and declared neutral root test helpers, without another package's
test fixtures or App test/source dependencies. Reuse existing owner-local
fixtures and table-driven cases; do not create a generic device/test framework,
duplicate production algorithms, or promise unmeasured full branch coverage.

### S3: Lib

- Inventory Lib contracts and close missing direct owner-local proof, starting
  with medium replacement ownership, rejected replacement preserving both
  leases, byte-count observation and destruction/lifetime boundaries.
- Keep file/overlay mechanics, host synchronization, audio and KVM semantics
  tested in Lib; higher-layer tests retain only their own policy/wiring claims.
- Exit: every inventoried Lib contract has an explicit disposition; missing
  eligible cases are covered locally, standalone Lib and full units pass, and
  Types, dependency and manifest checks remain valid.

### S4: Common

- Cover actual Session construction, UI binding, destruction and public event
  ingress, not merely stack-created private state or a fake Session API.
- Exercise UI event kinds, runtime/frame completions, generation filtering,
  queue failure and lifecycle/input admission with neutral Machine/UI doubles.
  Reuse existing state/reconciler tests; do not retest native Lib presenters.
- Exit: the public-to-owner transition chains have direct Common proof;
  notification delivery and input admission remain distinct, standalone Common
  and full units pass, and its boundaries/manifests remain valid.

### S5: x86

- Separate neutral Core timeline ordering, cancellation, reset and nested
  scheduling assertions from IBMPC wiring scenarios; cover Core-owned port
  routes, explicit time and failure-atomic transactions at their actual owner.
- Move the three pure CPU decoder-inventory runners from IBMPC to x86, retaining
  their original output/count predicates and repairing receiving registrations.
  Audit remaining chip/CPU/Core contracts for foreign-test-only coverage.
- Exit: neutral chip/CPU/Core proofs are owned by test/x86; board signal-chain
  checks remain IBMPC. Standalone x86, full units, historical inventory consumers
  and boundary/manifest checks pass. This does not consume queued CPU repairs
  or claim a new manual-complete CPU qualification.

### S6: IBMPC

- Bring generic Machine memory-reconfiguration and floppy insertion/ejection
  lifecycle assertions out of App-dependent fixtures into neutral IBMPC tests.
- Cover shared AT descriptor/CPU-contract materialization and assembly policy
  locally where currently demonstrated only by a fixed App profile test.
  Reuse existing construction/media fixtures; preserve App-specific choices,
  firmware constraints and external boot checks at their existing owners.
- Exit: board/Machine/Product contracts have direct IBMPC proof without an App
  test dependency; generic relocations preserve assertions, standalone IBMPC
  and full units pass, and all original App/integration registrations survive.

### Common Batch Gates And Authority

Each S records its complete inventory, similar-issue sweep, assertion relocation
map, actual-diff review and code-size result. Run its standalone suite and full
repository units on x64/x86, plus applicable Types/corpus/manifest gates.
Source dependencies inward are allowed; borrowing another component's tests
is not. No external ROM, YAML, CMOS, font or media input enters a unit fixture.

The owner authorizes Shared test completion and necessary NXVM test/build
receiver changes, with separate target-correct commits. Shared production
repairs or new APIs discovered by tests require concrete owner review before
implementation. MyNES implementation, configuration, media and product tests
are outside these four batches. No sibling repository is modified.

The accepted S2 eight-tree import remains an exact historical baseline.
Owner-approved test changes produce new corpus revisions with refreshed
manifests; they must not be described as byte-identical to the old SoftPC pin.
Test/documentation-only batches do not rebuild current EXEs unless executable
inputs actually change; record the determination. S7 retains all 58 original
PC integration contexts and the receiving MyNES integration requirements,
once per group, with unchanged acceptance predicates and current artifacts.

Before each S, refine only its bounded affected surface and verification.
S1 is read-only for Shared and App implementation. New Shared repairs need
owner review; the conditional unchanged import is authorized by this request.

## Convergence And Exit

The task ledger maps all eight roots, supporting helpers, changed contracts,
removed/relocated tests, all five receiving Apps and their build/test/artifact
requirements to direct evidence. Record unchanged/non-applicable consumers
with dependency proof, not assumptions. S1 must establish this complete map.

Each numbered S runs full repository-only units at closure; each actual
Shared executable-input change rebuilds affected x64/x86 products and verifies
their sole existing paths before S acceptance. The four PC Apps use this T's
revision; MyNES keeps its admitted version unless separately approved. Tests
use code-owned fixtures; external firmware/media stays in product integration.
T closure preserves all original integration predicates and runs each group
once unless a documented failure requires diagnosis. No new EXE is required
for audit-only inputs. All eight manifests, exact S2 import evidence and the
subsequent owner-approved test revision/relocation record, Types/
dependency/corpus checks, code quality, documentation and actual-diff review
must pass. An unacceptable source leaves T open with the concrete blocker,
not an invented successful import or an unreviewed local corpus repair.
