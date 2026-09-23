# Shared Corpus Import And Independent Build

## Goal And Prerequisites

Create the reviewed four-root baseline and a working independent test/build gate.
Requires owner task admission after review of the [M1 context](../etc/evidence/m1-retired/first-emulator-increment-planning.md),
and a complete per-unit authorization/notice ledger. Unresolved rights stop copying
the affected corpus; do not silently reduce the four-root commitment.

## Scope

Map src/lib and src/common unchanged in location; map test/lib and test/common to
src/test/lib and src/test/common. The pinned source universe contains 187 tracked
files including build/readme/manifests. Review notices and transitive test inputs;
do not import recovered VM/Compat/MVDM, firmware, ROMs or whole product CMake files.
Record original path, destination, source hash, effective license, modification
class and destination hash for every unit. Retain original hashes when updating
corpus manifests; a regenerated manifest must not erase provenance.

Combine import and minimal build portability in one bounded task: replace fixed
test-relative source discovery with explicit root inputs, remove product build
assumptions, enforce C11 and the shared warning profile, supply both architecture
presets and standalone entry points. No fake product executable. Keep all
compatible names/formatting. Record path/build/C11 deltas separately from source
snapshot identity. Initial x86 debug/xasm source may remain pending the next
package only if its licensing is cleared; it is not neutral-corpus acceptance.

## Acceptance And Stop Conditions

All four inventories accounted for; independent Lib/Common production targets and
test entry points configure, compile and pass on x64/x86 without App/Core, sibling
paths or guest assets. Run all applicable inherited tests, manifest and dependency
checks; classify host-only tests explicitly and run the admitted Windows cases.
Document inherited failures rather than blessing a red baseline. If required
build corrections exceed portability/C11 scope, report and revise the package
before expanding implementation. No source-only commit is a completed delivery.

Hand off accepted commit/manifests, build commands, test inventory and x86 removal
inventory to [neutral contracts](../etc/evidence/m1-retired/shared-neutral-contracts.md). This task does not
implement NES, remove useful generic mechanisms or claim unchanged downstream use.
