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

## S2 Owner And Path Audit

The sole paging owner is the core CPU executor in
`src/core/machine/cpu_instructions.c`.  `_kma_linear_logical` produces the
segmentation-checked linear span; `_kma_read_linear`, `_kma_write_linear`, and
`_kma_test_linear` split a span at 4 KiB boundaries before calling
`_kma_physical_linear` for each page.  The latter is the one PDE/PTE walk,
permission, A/D, CR2, and `#PF` producer.  Instruction fetch, ordinary data,
stack primitives, strings, and test/preflight accesses all enter through this
route; no VM, platform, or host-memory translation owner participates.

| Access rule | Current disposition | S2+ admission/probe boundary |
| --- | --- | --- |
| PDE/PTE present | Implemented: bit 0 of the error code is zero and CR2 receives the faulting linear address. | Retain T258 present-fault evidence; S2 records fetch/data/stack first-fault and unchanged destination/stack state. |
| CPL3 U/S | Implemented for both PDE and PTE: a supervisor entry produces present+user, with write added for writes. | S3 proves user fetch/read/write at both levels, exact code 5/7, CR2, and no register/stack/destination publication. |
| CPL3 R/W | Implemented for both PDE and PTE writes. | S3 proves present+write+user code 7 and read success; A/D effects are asserted only after successful access. |
| CPL0 R/W and CR0.WP | Missing distinction: supervisor writes do not currently reject a non-writable entry when WP is set. | S3 adds only the 80386 WP condition and probes WP clear success versus WP set code 3, at PDE and PTE. |
| CR2/error code | CR2 is written in each present/U/S/R/W fault branch; existing code uses Intel's P/W/U bits. Reserved-bit and instruction-fetch bits are outside 80386 scope. | S2 freezes all six admitted P/W/U combinations and diagnostic preservation. |
| A/D and atomicity | PDE A may be written before later PTE permission/fault evaluation; PTE A/D are written after its local checks. | S2 decides the preflight/commit boundary per complete access; no partial A/D publication is accepted for a failed admitted access. |
| Cross-page access | Read/write/test helpers split at 4 KiB; the leaf translation rejects an unsplit span. | S4 proves fetch, data read/write, and stack spans crossing one boundary, including second-page failure and zero publication before commit. |
| Translation cache / INVLPG | No architectural TLB state or `INVLPG` decoder/handler is found. CR3 writes already use the retained control route. | S5 is evidence-only unless S2--S4 show a stale-translation execution path; no current consumer admits `INVLPG`. |

## Fault Delivery Intersection And Batches

The executor records `#PF` through its existing first-fault diagnostic. T308
closed same/outer delivery only for the admitted `#GP/#NP/#SS/#TS` producers;
page-fault error-frame delivery was expressly deferred. Therefore S2 and S3
may prove exact producer diagnostics and terminal preservation, but may not
claim a 32-bit `#PF` IDT frame. If an admitted CPL3 probe requires that frame,
the work stops for a separately admitted delivery packet rather than changing
T311's planner implicitly.

1. **S2 - walk, diagnostics, and commit audit.** Focused prepared state for
   PDE/PTE present, U/S, R/W, CR2, P/W/U codes, A/D timing, and producer
   nonpublication. It may make only a demonstrated shared-walk correction.
2. **S3 - CPL3 and WP permissions.** The minimal U/S, R/W, and CR0.WP
   correction and probes for user fetch/data/stack and supervisor WP cases.
   It stops at a missing `#PF` delivery frame.
3. **S4 - cross-page checked spans.** Existing split access only: one-boundary
   fetch/data/stack success and second-page failure preflight/commit evidence.
4. **S5 - cache disposition.** Record no-op closure unless a focused S2--S4
   trace proves translation caching; only then admit a narrow cache/invalidation
   packet with a concrete consumer.
5. **S6 - family closure.** Full gates and artifact, with no PAE, large-page,
   host-memory, task/V86, debug/test, or product-policy expansion.

Intel 80386 PRM Chapters 5, 6, and 9 remain authoritative. The read-only
comparison identities are Bochs 2.6 `cpu/paging.cc`, `cpu/exception.cc`, and
`cpu/fetchdecode.cc`, plus PCjs 2.00.0 paging and fault behavior in
`machines/pcx86/modules/v2/x86.js`, `x86ops.js`, and `x86help.js`. They are
only review paths; no source is copied. The retained T258 paging smoke and
T305--T308 delivery probes are intersections, not substitutes for the new
focused prepared-state probes.
