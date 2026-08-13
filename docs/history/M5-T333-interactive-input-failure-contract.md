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

## Boundary

The existing xasm capacity and failure-semantics TODO remains open. The
source-specific test allocator redirection exists only in its two smoke targets;
the ordinary production libraries retain the normal allocator path.
