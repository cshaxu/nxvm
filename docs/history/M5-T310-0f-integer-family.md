# M5 T310: 80386 0F Integer Family

## Accepted Closure

T310 S1--S9 completed the admitted 80386 `0F` integer bit/data family and
the 0310 artifact. The coordinator accepted and closed T310 before T311; this
record preserves that closure and does not activate a successor.

## Completed Scope

- S3: all `SETcc` byte destinations and condition forms.
- S4: `MOVZX`/`MOVSX`, including the corrected 16-bit B7/BF destination width.
- S5: `BT`/`BTS`/`BTR`/`BTC`, including immediate memory-element selection.
- S6: `SHLD`/`SHRD`, including masked-zero-count no-publication.
- S7: `BSF`/`BSR` focused scan evidence.
- S8: two-operand `IMUL`, with a defined host 64-bit intermediate product.

Intel 80386 PRM Chapter 4 remained authoritative.  The versioned Bochs 2.6
and PCjs 2.00.0 paths recorded in the [T310 evidence](../etc/evidence/t310-0f-integer-bit-data-admission.md)
were read-only behavior comparisons; no source was copied.  Every form uses
the existing core executor, ModRM, checked-memory, profile, and fault routes.
No public ABI, product UX, second executor, or alternate memory owner changed.

## Verification

- Artifact: `build/output/nxvm_0_5_0310.exe`.
- SHA-256:
  `C55AB35CB93132F0674A1DFBAC8AD8280585FDB70A7FDB5E35AFDC5B0E06F36C`.
- All T310 focused markers S3--S8 remain in `PROJECT_CURRENT_SMOKE_TARGETS`.
- `current-gates-gcc` passed 51 static/governance targets and 143/143 CTests.
- Documentation governance, current-artifact verification, and `git diff --check`
  passed.

## Deferred Boundaries

One-operand and immediate `IMUL`, later `0F` forms, paging, debug/test
registers, task/V86/system forms, and product observation remain outside T310.
They require separate admission; this closure does not imply their support.
