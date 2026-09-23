# M5 T316: Ordinary Execution Closure

## Accepted Boundary

T316 closes its declared Intel 80386 ordinary-execution slices, from the
S2--S21 primary groups through S22--S65 bounded form families. It does not
close every wider parent instruction family and does not claim complete 80386,
80387, devices, timing, or Windows readiness. The authoritative reconciliation
is the [T316 ordinary-execution matrix](../etc/evidence/t316-ordinary-execution-matrix.md),
including its S66 residual-transfer table.

## Audit Result

The audit read the matrix's primary and `0F` route inventory, the retained
S2--S65 evidence sections and markers, the owner smoke registrations in
`PROJECT_CURRENT_SMOKE_TARGETS`, `QUEUE.md`, `TODO.md`, and the active task
artifact record. No declared T316 slice is left missing or unclassified.
Broader parent rows remain explicitly **Partial** where appropriate; they are
not silently upgraded. Each residual is transferred to exactly one later Queue
package, the legacy LOCK TODO, or the external-coprocessor boundary.

The final slices close shared ordinary prefix/attribute semantics (S64) and the
CPU-side WAIT/ESC/CR0/#NM interface (S65). S65 corrects only the narrow #NM
vector-7 delivery used by `WAIT` and `FPU_ESCAPE`; no provider ABI or x87
implementation is implied.

## Verification And Artifact

- Artifact: `build/output/nxvm_0_5_0316.exe`.
- SHA-256:
  `672F11D9174B910836F9FF02BC31025C064DE9F7D5F1667A31443D1FEB96AB3E`.
- `verify-current-artifact-target` selected the 0316 artifact.
- The `current-gate` CTest label passed 194/194 tests.
- Documentation governance and `git diff --check` passed at S66 closure.

## Deferred Boundaries

The [S66 transfer table](../etc/evidence/t316-ordinary-execution-matrix.md#s66-residual-parent-family-transfers)
remains the binding list of residual parent-family work. It transfers ordinary
instruction/flag/string breadth, exception/control, protection, paging, and
task/V86/debug work to their named Queue packages; it retains the legacy LOCK
TODO and the external-coprocessor boundary. The next package must perform its
own Intel-form admission audit and cannot inherit a whole-family completion
claim from T316.
