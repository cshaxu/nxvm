# T329 S1: 16-bit TSS Direct-Jump Evidence

## Boundary

This record covers only protected far `JMP` transfers to an available 16-bit
TSS on 80286 and 80386.  It does not promote task gates, far `CALL` task
switches, nested-task `IRET`, 32-bit TSS images, LDT task images, or task
paging to this slice.

## Owner Smoke

`core-machine-task-switch-smoke` retains the T261 regression markers and adds
`M5:T329:S1:TSS16-JMP:OK`.  Its one local fixture uses an old busy 16-bit TSS
and an available target 16-bit TSS.  The successful transition proves outgoing
IP/AX save, incoming register/cache/TR state, old/target busy access bytes,
and `CR0.TS`; target code then writes its marker and halts.

The admitted form matrix is:

- 80286 and 80386 default direct far `EA ptr16:16` and memory-indirect
  `FF /5 m16:16` transfers;
- 80386 `66h` direct and indirect `ptr16:32`/`m16:32`, `67h` indirect
  `m16:16`, and combined `66h 67h` indirect routing;
- invalid selector, non-present target, busy target, short target TSS, and
  stack-limit failure boundaries, plus 80386 `LOCK` rejection; and
- a pending IRQ0 installed before a successful direct task jump whose incoming
  16-bit TSS sets IF.  The IRQ is accepted immediately after the switch: the
  handler halts at its IDT target, PIC ISR is set and IRR clear, the new task's
  stack moved by the 16-bit frame size, and its normal target-body marker
  remains untouched.  T320 remains the owner of general protected IRQ frame
  layout and cross-privilege delivery semantics.

No task-switch production change was required.  The direct route audit is
limited to `_e_jmp_far` and `_ser_jmp_far_tss`, every 16-bit TSS field access,
and target/old TSS descriptor writes.

`_e_jmp_far` reaches `_ser_jmp_far_tss` only in its protected-mode branch.
Thus real mode and ordinary VM86 classify the same far-JMP encodings as their
already-owned ordinary/VM86 transfer paths, not as a task switch.  T303 and
T320 retain behavioral ownership of those non-task transfer and VM86 delivery
routes; S1 neither changes nor claims them.

## Transfer

The later task/LDT/debug/VM86 package retains 32-bit TSS, task gate, far CALL
to TSS, NT return, non-null LDT task state, task paging/CR3, and broader task
fault/interrupt composition.  This record is form evidence only; the Queue
remains the allocation authority.
