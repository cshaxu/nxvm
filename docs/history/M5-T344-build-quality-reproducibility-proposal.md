# M5 Build-Quality Reproducibility

## Purpose

Repair the code-quality governance defects exposed by the full-tree baseline
audit before the Windows 3.x readiness map consumes the current build and
current-gate surfaces.  The work is limited to reproducible configuration,
truthful direct-source warning coverage, unique current-gate registration, and
test-only fixture-shape governance.

## Dependencies

This candidate follows the completed four-profile CPU program.  It precedes
the Windows 3.x readiness map because a readiness conclusion requires a clean,
reproducible configured build and a truthful account of compiler quality
coverage.

## Ordered Subtasks

### S1 - Reconcile Clean Configure And T337 `#UD` Inventory

Classify `core-machine-vm86-delivery-smoke` in the T337 current-gate `#UD`
inventory using its actual real/protected/VM86 evidence.  A fresh out-of-tree
GCC configure must succeed and preserve the source-sensitive inventory gate.
Do not weaken the gate, remove the smoke from current-gate, or change CPU
behavior.

### S2 - Strict-Compilation Coverage Matrix

Derive a target/source matrix from actual Ninja direct compile commands.  For
every configured production and current-gate target, record retained strict,
newly strict, or deferred status; deferred rows must identify inherited or
mixed source ownership and a precise admission condition.  Add narrow
mechanical verification that distinguishes a target's direct compilation from
its linked dependencies.  Apply strict flags only where the audit proves a
new/repository-owned target is safe.  Do not globally add `-Werror`, weaken
diagnostics, or silently classify inherited sources as strict.

### S3 - Current-Gate Registration Integrity

Make the canonical current-gate target list unique at its definition boundary
and add a narrow configure-time verifier for duplicate or missing targets.
Preserve target membership, labels, media arguments, and test behavior.  Do
not collapse distinct media lists or create a second test-registration path.

### S4 - Historical Fixture-Shape Classification

Create a fixed source-to-shape inventory for the remaining historical
machine/device/protected smokes.  Compare setup phases, provider lifetimes,
GDT/IDT state, and reset/freeze order.  Extract only a setup helper whose
equivalence is proven and which remains under `tests/support`; otherwise
record each retained variant and its future admission condition.  No test-only
public interface, product-visible state, or generic fixture framework is
permitted.

### S5 - Strict-Declaration Uniqueness Corrective

Remove the reproduced duplicate target-local strict declaration and verify
configured target properties so a target cannot receive any member of the
strict GCC option set twice. This is a de-duplication check only: it does not
promote deferred targets, make a global warning policy, or infer direct-source
coverage through linked libraries.

## Shared Completion Standard

Every S must self-review and provide exact changed surface, fresh configure
proof, applicable direct compilation/CTest evidence, documentation governance,
and diff check.  The task closes only when the audit report is reconciled:
clean configuration succeeds, strictness is truthful and mechanically checked,
the raw current-gate list cannot drift through duplicates, and all candidate
fixture shapes have one evidence-backed disposition.

## Explicit Transfers

The following remain outside this candidate: inherited executor warning
cleanup, xasm/debug internal capacity redesign, any CPU semantic change,
device behavior, Windows compatibility work, and a fixture abstraction for
non-equivalent setup.  Each retains its existing TODO or receives an exact
debt record rather than a broad exemption.
