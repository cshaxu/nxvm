# T311: CPL3 Paging And User/Kernel Boundary Admission

## Purpose

T311 follows the retained T258 CPL0 4 KiB paging baseline. It admits only the
remaining 80386 paging semantics required to make the same core machine path
correct across CPL3 and CPL0: U/S and R/W permission checks, CR0.WP, page-fault
error bits and CR2, and checked accesses that cross a 4 KiB boundary.

## Authority And References

Intel 80386 Programmer's Reference Manual Chapters 5, 6, and 9 are the
semantic authority. Bochs 2.6 paging/exception paths and PCjs 2.00.0 paging
paths may be inspected as read-only behavior references. No external source,
firmware, media, trace, or implementation is imported.

## Frozen Admission Matrix

| Area | T311 admission | Explicitly deferred |
| --- | --- | --- |
| Page walk | Existing 4 KiB PDE/PTE walk, present checks, A/D updates, CR2 | PAE, large pages, host-memory substitution |
| Privilege | CPL3 U/S and R/W permission checks; CPL0 CR0.WP behavior | Generic policy or product-owned access rules |
| Fault outcome | Intel 80386 #PF error-code bits and CR2 through the existing core diagnostic/delivery boundary | New reset, triple-fault, or product policy |
| Access spans | Instruction fetch, data, stack, and string/port-adjacent memory accesses crossing one page boundary | A new memory route or bulk host copy path |
| Translation cache | Only if the existing execution path requires explicit invalidation evidence | Speculative TLB redesign or unconditional INVLPG admission |

## Planned Batches

1. S2 audits the retained T258 walk and freezes the exact P/U, R/W, WP, CR2,
   error-code, A/D, and delivery observations for each access origin.
2. S3 implements and proves user/supervisor and read/write permission behavior
   with precise #PF outcomes and pre-fault nonpublication.
3. S4 implements and proves cross-page instruction, data, and stack access
   boundaries using existing checked core memory.
4. S5 admits INVLPG or explicit translation-cache behavior only if S2--S4
   demonstrate a concrete execution requirement; otherwise it records that no
   new interface or opcode is warranted.
5. S6 closes the admitted family with full gates and an artifact, without
   starting the next queue candidate.

Every implementation batch uses focused prepared-state probes. A product
checkpoint is observation only and never a build prerequisite. The next task
may not begin until T311 is accepted and closed.
