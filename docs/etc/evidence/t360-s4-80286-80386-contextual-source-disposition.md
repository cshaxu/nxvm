# T360 S4: 80286/80386 Contextual Source-Row Disposition

## Decision and method

This record reconciles only the successful-retirement rows selected by T359.
It does not turn Intel's prefetched/no-wait processor numbers into PC/AT bus,
device, exception, task-switch, or interrupt-delivery clocks.  The primary
authorities are the 1987 Intel *80286 and 80287 Programmer's Reference Manual*,
Appendix B, and the original Intel *80386 Programmer's Reference Manual*,
section 17.2.2.3 and the instruction pages cited by the T359 ledgers.

The audit searched all tracked timing owners in `src/core/machine/machine.c`
for `m`, `component_count`, `byte_count`, `prefix`, `flagLock`, `flagMem`,
`effective_address`, `odd_word`, `protected`, `VM`, `dpl`, `descriptor`, and
`lexeme`.  `core_machine_instruction_cost()` remains the one post-refresh
publisher.  Each private classifier consumes completed decoder state plus the
old CPU state; none writes decoder, CPU, selector, descriptor, transaction, or
device state.  A failed or rejected instruction never reaches the publisher.

## 80286 selected context

| T359-selected 80286 receiver | Current capture and source disposition | S4 result |
| --- | --- | --- |
| `70`--`7F` conditional branch | Old `EIP`, completed `EIP`, and the 16-bit fallthrough select taken/not-taken. | Exact selected row; no following-byte assumption. |
| `88`--`8B` and `A0`--`A3` selected MOV | Completed decoder memory shape selects the row; the existing 80286 EA helper and odd-word helper supply only the Appendix-B additions already retained by T357/T359. | Exact selected row. |
| Selected calls, returns, stack, loop and same-level control rows | `core_machine_control_stack_source_instruction_cost()` uses `lexeme.byte_count` only for the Appendix-B next-instruction term, and separately rejects unsupported prefix/mode/delivery contexts. | Exact selected row; task, outer-privilege, exception and IRQ paths remain transfers. |
| `63`, `0F 00 /0`--`/5`, `0F 01 /0`--`/6`, `0F 02`, `0F 03`, `0F 06` | Appendix B has form-specific protected-state, effective-address, next-instruction, and descriptor conditions.  The current 80286 fallback deliberately has no system-form classifier, and the completed decoder capture has no dedicated nonpublishing descriptor-granularity result for `LSL`. | Transferred, without borrowing a 80386 row, to the next complete instruction-timing corpus candidate.  Its admission must first create a profile-local form/context table and prove that all required conditions are capturable before retirement. |

The transfer is intentional: assigning a number now would either duplicate the
80386 privileged classifier or hide an unproven descriptor/transition context.
The retained source fallback is an explicit unallocated marker, not a clock
claim.

## 80386 selected context

| T359 receiver | Context captured at the sole publisher | S4 result |
| --- | --- | --- |
| String, repeat and ordinary I/O | Opcode, REP class, operand/address attributes, repeat continuity, profile-local I/O permission outcome, and successful primitive publication. | Exact only for the rows in the S4 string/I/O ledger; provider latency, wait, or failed permission remains external. |
| Dynamic `IMUL` | Completed source operands and the explicit 80386 formula path. | Exact selected formula; no 8086/80186 range is reused. |
| Near conditional branch, bit, double-shift, scan and MOVX forms | Completed decoder capture chooses register/memory and legal source prefixes; near taken Jcc and the documented `m` term use `lexeme.component_count`. | Exact selected rows. |
| Primary data/ALU and Group-3 forms | Completed decoder memory shape, legal prefix gate and selected source formula/fixed row. | Exact selected rows; unresolved legacy ranges remain T360 S3 debt. |
| Control and stack forms | Old/new state selects the documented same-level outcome; `lexeme.component_count` is the sole `m`/next-instruction capture. | Exact selected success only; task switch, outer privilege, VM86 frame, exception and IRQ delivery are cycle-exact transfers. |
| `63`, `0F 00`, `0F 01`, `0F 02`, `0F 06`, CR/DR/TR, FS/GS and full-pointer forms | `core_machine_80386_privileged_source_instruction_cost()` checks legal source prefixes and LOCK, ModRM memory shape, protected/VM state, and CPL before returning only the selected successful row. | Exact rows listed in the T359 S6 ledger. |
| `0F 03 LSL` with byte/page descriptor-limit distinction | The source row differs by descriptor granularity, while retirement has no dedicated nonpublishing descriptor-granularity capture. | Transferred to the next complete instruction-timing corpus candidate; no guessed row. |
| Other lexical prefix, paging, selector, VM86, privilege, fault or delivery combination | The 80386 manual's successful no-wait row does not prove that combination. | Transferred to the named physical, delivery, VME/PVI, or cycle-exact receiver in T359 S6/S7. |

`SMSW`, `SGDT`, and `SIDT` are not silently treated as faults in ordinary
VM86: the existing S6 classifier assigns only their documented successful
forms, while rejected or faulting forms publish no retirement tick.  This is a
mode-specific source disposition, not a general VM86 timing rule.

## Similar-issue and ownership sweep

The production sweep found only the seven private timing classifiers called by
`core_machine_instruction_cost()` and its compatibility fallback.  There is
no handler-local elapsed-tick writer, second publisher, or source value in the
decoder, selector, descriptor, delivery, memory, port, or device owners.
Existing T359 timing smokes and the T360 inventory verifier cover the retained
owners.  No reproducible source-row mismatch was found, so S4 makes no runtime
or CMake change.

## Exact transfers and L3 boundary

The next complete instruction-timing corpus task receives the unallocated
80286 system-form/descriptor contexts and 80386 descriptor-granular `LSL`.
The bus-timed, device-service, and cycle-exact Queue candidates respectively
receive physical wait/arbitration, controller latency, and delivery/transition
cycles.  The two source ambiguities already retained in `TODO.md` (80286 NOP
table/prose and 8086/80186 dynamic arithmetic ranges) remain separately owned;
this S neither changes nor resolves them.

Accordingly, T360 S4 proves source-context truthfulness at retirement.  It
does not claim L3 or cycle-exact timing.
