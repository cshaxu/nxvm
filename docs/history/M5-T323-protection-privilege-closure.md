# M5 T323: 80386DX Protection And Privilege-Transfer Closure

## Scope And Result

T323 closes the Queue's bounded non-task, non-VM86 80386DX protection and
privilege-transfer program. It complements the retained T301, T303, T306,
T307, T320, and T321 evidence with focused 16-bit protected coverage:

- S1: direct protected far `CALL`/`JMP`;
- S2: loaded data-segment access rights and limits;
- S3--S5: 16-bit same-CPL software/external gates and outer TSS16/TSS32 entry;
- S6: 16-bit outer `IRET`; and
- S7: 16-bit call-gate entry, including declared parameter-word copying.

The task-level [closure audit](../etc/evidence/t323-protection-privilege-closure-audit.md)
maps every assigned condition to the current serializer and focused evidence.
The corresponding [80386DX closure-map row](../etc/evidence/80386-closure-map.md)
is now complete for this bounded state composition.

## Focused Production Result

S7 corrected the retained 16-bit call-gate serializer: a nonzero parameter
count is no longer rejected. It preflights every old-stack parameter word and
all target stack slots before changing privilege state, then pushes the copied
words in architectural order. The change is local to
`_ser_call_far_call_gate`; descriptor, TSS, stack, PIC, and generic
far-transfer helpers remain unchanged.

The current `vm-0-5-0323` artifact SHA-256 after that source correction is
`0C901AC5E883FDB26D70B317814E580BB9000CE50899BBCA2AF273912DF1F785`.

## Transfers

Task gates/switching, LDT/debug and VM86 instruction breadth are owned by the
Queue's task/LDT/debug/VM86 package. Paging is owned by the paging/translation
package. Legacy 80186/80286 LOCK legality and numerical x87 execution retain
their existing separate boundaries. T323 makes no whole-80386 claim.

## Verification And Acceptance

The S7 owner marker, exact current-gate registration, documentation
governance, and `git diff --check` passed. The full current gate passed
209/209 tests. S8 independently re-read the affected serializers, retained
evidence, closure map, Queue, and task history before task acceptance.
