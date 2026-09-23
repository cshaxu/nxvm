# M5 T325: Intel 80386DX Paging And Translation Closure

## Scope And Result

T325 closes the CPU-native Intel 80386 non-PAE paging and translation boundary.
It completes three accepted slices:

- S1 corrected the CR0 model to the 80386 mutable controls and removed the
  incorrectly imported 80486 `CR0.WP` supervisor-write rule.
- S2 reconciled `MOV CR2/CR3`, selected-directory behavior, fault diagnostics,
  and the executor's functional no-persistent-translation-cache boundary.
- S3 reconciled the package form/state ledger and proved that the later-CPU
  `0F 01 /7` `INVLPG` encoding is `#UD` before effective-address decoding on
  the supported pre-486 profiles.

The detailed [S3 closure audit](../etc/evidence/t325-s3-paging-closure-audit.md)
maps every native paging requirement to its focused evidence and records the
strict boundaries below.

## Verification And Acceptance

Fresh configuration, the focused paging smoke, documentation governance,
`git diff --check`, the 47-target specialized verifier aggregate, and the
full 209-test current gate passed at S3 P1 acceptance. The paging smoke emits
`M5:T325:S1:CR0-PAGING-CONTROL:OK`,
`M5:T325:S2:CR2-CR3-TRANSLATION:OK`, and
`M5:T325:S3:PAGING-CLOSURE:OK` alongside retained T258/T311 markers.

No public ABI, host-memory route, persistent cache, or product behavior was
introduced. The only source change in the final S is an owner paging smoke;
S1's production correction is retained in its indexed evidence.

## Transfers

- Protected-mode vector-6 `#UD` delivery is a shared exception-finalizer gap,
  recorded as `TODO(High)` with its required producer sweep. It is not a
  paging-local repair.
- Task-switch CR3 loading and its fault contexts, plus VM86 paging-family
  breadth, transfer to the Queue's task/LDT/debug/VM86 system-state package.
- A persistent TLB, its CR3/task-switch invalidation, and TR6/TR7 behavior
  require a future dedicated cache/test-register admission. The present
  no-cache proof is not a hardware TLB claim.
- PAE, PSE, CR4, `INVLPG` implementation, and x87 numerical execution are
  outside the Intel 80386DX CPU-native paging closure.
