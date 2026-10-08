# T546 S16 Task-Transition Source And Owner Design

## Sources

The primary sources are Intel's *iAPX 286 Programmer's Reference Manual*,
210253-006, chapter 8, Table 8-1 and Table 8-2, SHA-256
`9C6067E777AE694D5F71D8ADE23014558BE4E9D156041C778AED84C1246D8538`,
and Intel's *80386DX Programmer's Reference Manual* (1990), chapter 7,
Table 7-1 and Table 7-2, and sections 9.9.10--9.9.14.1, SHA-256
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`.
The PDFs remain in the external manual archive; no manual bytes are copied
into this repository.

Both manuals separate task-switch processing into two architectural phases.
The outgoing context owns access checks through incoming-TSS presence and
limit, then saves the outgoing dynamic image.  The processor loads TR, marks
the incoming task busy, applies the transition's busy/link/NT semantics, and
loads the incoming image.  Invalid incoming LDT, code, stack, data, or page
references detected thereafter are delivered in the incoming task context.
The 386 manual explicitly says that the first instruction of the incoming task
appears not to have executed.  The 286 table gives the same division after its
third pre-switch step.

The manuals also require the new CPL to come from incoming CS.RPL, not from
the outgoing CPL.  The task-specific selector checks are: nonconforming CS
DPL equals CS.RPL; conforming CS DPL is no greater than the incoming CPL; SS
is writable data with RPL and DPL equal to that CPL; DS/ES and, on 386, FS/GS
are readable and satisfy the corresponding privilege rule.  A direct TSS or
task-gate access check uses the caller's effective privilege; an interrupt,
exception, or nested IRET has its source-defined exception to that rule.

Table 8-2 / 7-2 gives the transition kind semantics: JMP sets incoming busy,
clears outgoing busy and clears incoming NT; CALL/interrupt keeps outgoing
busy, sets incoming busy, stores the old TR selector in the incoming backlink
and sets incoming NT; nested IRET leaves the returning task busy, clears the
outgoing busy bit, and leaves the returning NT image unchanged.

## Current Owner Audit

`_ser_task_transition_tss` remains the one external task-transition entry.
The retired 80286-only construction body is removed: one private plan now
selects the explicit 16-bit or 32-bit TSS image layout on both processors.

The predecessor prevalidated every incoming LDT/CS/SS/data selector, target
instruction byte and target stack before the first outgoing-TSS write.  It
also required the old CPL, incoming selector RPL, code DPL, stack DPL and data
DPL all to be zero; it required DS/ES/FS/GS to be writable data; and its
non-nested path did not clear an NT bit loaded from the incoming image.  Those
are not source-defined restrictions.  They suppressed the manual's
incoming-task exception context and left two publication mechanisms to
maintain.

The existing `instruction_task_switched` and
`instruction_task_checkpoint` are already the sole late-fault bridge.  The
normal instruction finalizer selects that checkpoint for an exception raised
after task publication.  S16 will use that existing mechanism, not add a
second finalizer, diagnostic state or public API.

## One Transition Boundary

The repair will retain explicit 286 and 386 TSS layouts, but replace the two
construction bodies with one private transition plan parameterized by:

- outgoing and incoming image width;
- transition kind: non-nested JMP, nested CALL/interrupt, or nested IRET
  return; and
- origin privilege policy already established by the direct-TSS, task-gate,
  IDT-gate, and IRET callers.

The plan has these ordered stages:

1. Validate the entry cause, source-task TR, incoming TSS descriptor
   availability/presence/limit, and outgoing-save accesses in the outgoing
   context.  Materialize the source-defined outgoing image.
2. Save the outgoing dynamic image, update busy descriptors/backlink according
   to transition kind, publish the incoming TR and task checkpoint, and set
   CR0.TS.  No old-task rollback is permitted after this boundary.
3. Load the incoming TSS image and incoming CR3 where applicable, then load
   and check its LDT, CS, SS and data selectors in manual order.  A fault here
   uses the existing task checkpoint and ordinary exception delivery in the
   incoming context.  Raw loaded selector state is never replaced with a
   second cache owner.
4. Clear local debug enables on 386 and deliver an enabled TSS debug trap only
   after incoming publication, as the existing checkpoint-aware route already
   does.

The 80386 table says that register loading is step 4 and begins its named
selector exceptions at step 5.  Therefore an incoming TSS CR3 image is copied
as a register image; the former project-only reserved-bit `#TS` precheck is
removed.  Existing paging references continue to use the existing architectural
page-directory-base mask; this does not add a TLB, page-walk policy or timing
model.

## Implemented Disposition

- Direct TSS CALL/JMP retains the source caller's TSS-DPL admission; GDT/IDT
  task gates and nested IRET pass their already-defined entry policy instead
  of pretending that every entry has a direct-TSS privilege check.
- The shared plan writes the outgoing image and descriptor transition once,
  publishes the incoming raw register/selector image, checkpoints it, then
  validates and materializes LDT, CS, SS and data caches in Intel order.
  `instruction_task_checkpoint` is the existing late-fault owner.
- Incoming CPL is CS.RPL.  CS, SS and readable DS/ES/FS/GS checks now use the
  documented conforming/nonconforming and RPL/DPL relationships; null data
  selectors remain valid unavailable data caches.
- Two-stage 286 and 386 regressions enter a Ring-3 task through their normal
  DPL-0 TSS, then execute a direct far JMP/CALL with an RPL-3 selector or a
  DPL-3 task-gate CALL to a DPL-3 TSS.  The JMP case proves the second switch
  uses the current incoming CPL and clears the non-nested source TSS busy bit.
  The direct and gate CALL cases prove the same admission while retaining both
  busy descriptors, publishing the backlink and setting NT.  A direct switch
  retains its RPL-3 target selector in TR; task-gate entry loads the gate's
  descriptor target selector.  Neither route retains the retired CPL-0-only
  admission rule.
- The 16-bit image path preserves the old 80386 high-half fill for a 386
  16-bit TSS, but no longer applies it to a real 80286 register file.
- Owner-local regressions now cover both 286 and 386 nonzero incoming CPL,
  readable-code data caches, null 386 data caches, raw 80386 CR3 image load,
  and task-gate delivery of late selector/LDT/stack faults.  No public API,
  board, VM, profile, timing value or second finalizer is introduced.

The implementation may retain small owner-local image readers/writers and
selector validators because their layouts and 286/386 register sets differ.
It must remove the retired integrated 16-bit transition body and must not
preflight post-publication selector or target-stack errors merely to preserve
old rollback behavior.

## Required Regression Matrix

The existing 16-bit, 32-bit, cross-width and decode fixtures remain receivers.
S16 adds or corrects direct CPU-owner cases for:

- 286 16-bit and 386 16/32-bit non-nested JMP, direct CALL, GDT task gate,
  IDT task gate and nested IRET, including busy/backlink/NT publication;
- incoming CPL 0 and nonzero CPL with valid code/stack/data selector rules;
- readable data and readable code in DS/ES/FS/GS, null data selectors, and
  invalid/nonpresent LDT, CS, SS and data selectors;
- late `#TS`, `#NP`, `#SS` and relevant page-fault delivery after incoming TR
  and descriptor publication, versus descriptor/presence/limit rejection in
  the outgoing context; and
- 16-to-32, 32-to-16 and 32-to-32 images, CR3/LDT/DR local-enable/debug-trap
  behavior without inventing a task-switch clock value.

Each row must observe CPU state, TR, descriptor busy bytes, outgoing TSS
image, backlink/NT and fault context through existing owner-local fixtures.
No board, VM or product test substitutes for this proof.

## Qualification Record

- `ctest --test-dir build/t546-s10-my5160-x64/test/x86 --output-on-failure -j 8`:
  182/182 passed.
- `ctest --test-dir build/t546-s10-my5160-x86/test/x86 --output-on-failure -j 8`:
  182/182 passed.
- Both runs include the complete x86 unit corpus, x86 ownership/Types/negative
  gates, and both manifests.  No desktop or external-media integration run is
  claimed by this packet.
