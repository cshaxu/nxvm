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

## Initial S Plan

1. S1: complete eight-root difference and importability audit, including dirty
   versus committed source, root helper dependencies, whole test relocation
   map and actual contract/code review. Record an explicit import decision.
2. S2: if S1 accepts, pin/import exact corpora and supporting helpers; retire
   replaced paths with coverage proof, adapt existing receiver registrations,
   run manifests/boundaries and full units. No selective 'same enough' claim.
3. S3: verify all receiving PC and MyNES products, original external integration
   and dual-width optimized artifacts; publish target-separated commits, review
   exact corpus equality and close only after the full ledger is satisfied.

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
for audit-only inputs. All eight manifests and exact imported bytes, Types/
dependency/corpus checks, code quality, documentation and actual-diff review
must pass. An unacceptable source leaves T open with the concrete blocker,
not an invented successful import or an unreviewed local corpus repair.
