# M5 T333 Interactive Input Failure Contract

## Outcome

T333 closed the retained VM Console and core debugger host-input failure
mechanism. Failed reads now stop the relevant interaction before parsing,
execution, scanning, buffer indexing, or guest mutation. Argument allocation is
preflight, and normal/failed main-loop exits clear the owned context pointer.

The complete 44-reader inventory found 43 debugger readers and one VM Console
reader. Debugger readers converge through one private owner helper; Console
retains its distinct owner-local loop check. No public API, ABI, host policy, or
guest execution path changed.

## Delivery And Evidence

- S1 inventory: `2ce2194f`; [inventory evidence](../etc/evidence/t333-s1-interactive-input-inventory.md).
- S2 repair: `2a59ab52`; [repair evidence](../etc/evidence/t333-s2-interactive-input-repair.md).
- S3 proof: `eafeca40`; [focused-proof design](../etc/evidence/t333-s3-interactive-input-proof.md).
- Two owner-separated smoke targets prove Console EOF/reuse/allocation failure
  and debugger main/nested EOF/reuse/allocation failure without register
  mutation. Their markers are `M5:T333:S3:CONSOLE-INPUT:OK` and
  `M5:T333:S3:DEBUG-INPUT:OK`.
- `current-gates-gcc` passed 214/214 current tests; real time was 35.84 seconds.
- Documentation governance and `git diff --check` passed.

## S4 Corrective Artifact Closure

The later quality audit found that T333's runnable source change had closed
without the task-specific developer artifact required by the execution policy.
Corrective S4 therefore changed only the current artifact target and preset to
`vm-0-5-0333` / `0.5.0333`; it made no production-source, ABI, test, or input
behavior change.

- Accepted artifact-target source commit: `0619a824`.
- Local developer artifact: `build/output/nxvm_0_5_0333.exe`.
- SHA-256: `EB89A9701036EFF6FEBFD211B12C92832FFB90CAE51672FEEF4241C6BE5A7A3E`.
- `verify-current-artifact-target` reports exactly
  `M5:T197:S1:CURRENT-ARTIFACT-TARGET:vm-0-5-0333:OK`; the compiled binary
  contains the CMake-supplied `0.5.0333` identity and retained product name.

## Boundary

The existing xasm capacity and failure-semantics TODO remains open. The
source-specific test allocator redirection exists only in its two smoke targets;
the ordinary production libraries retain the normal allocator path.
