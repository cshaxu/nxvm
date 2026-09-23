# T360 S1: Four-Profile Timing Source-Authority And Consumer Inventory

## Decision boundary

This is an authority and consumer inventory, not a replacement timing table.
It makes T359's allocations reviewable before T360 changes any number. Intel
primary manuals remain authority; Bochs, PCjs, host measurement, and existing
NXVM behavior may suggest a probe but cannot settle a value or conflict.

## Profile source baseline

| Profile | Primary source edition and stable location | Current consumer rule | Confidence / open issue |
| --- | --- | --- | --- |
| 8086 | Intel, *The 8086 Family User's Manual*, October 1979, [9800722-03 scan](https://bitsavers.org/components/intel/8086/9800722-03_The_8086_Family_Users_Manual_Oct79.pdf), Tables 2-20/2-21 | Base row plus explicit EA, segment-override and odd-word additions where the table defines them | Table source is identified; range-form policy remains T360 work. |
| 80186 | Intel, *iAPX 86/88, 186/188 User's Manual*, 1985, [210912-001 scan](https://www.bitsavers.org/components/intel/8086/210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf), Table 1-16 | Parenthesized 80186 rows under the manual's stated assumptions | Range rows and edition/table interpretation remain T360 work. |
| 80286 | Intel, *80286 and 80287 Programmer's Reference Manual*, 1987, [210498-005 scan](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf), Appendix B | Exact selected Appendix-B rows, plus documented following-byte/EA/odd-word rules | Formal NOP row `3` conflicts with prose `2`; privileged Appendix-B normalization remains open. |
| 80386DX | Intel, *80386 Programmer's Reference Manual*, 1986, [230985-001 scan](https://www.bitsavers.org/components/intel/80386/230985-001_80386_Programmers_Reference_Manual_1986.pdf), section 17.2.2.3 and instruction pages | Prefetched/no-wait/no-HOLD/aligned successful instruction rows; `m` is next-instruction component count | Selected rows are source-backed; unselected mode/prefix and delivery forms remain separate. |

Every source's processor and bus assumptions are retained as a boundary, not
silently applied to PC/AT service time. The 80386 manual expressly assumes a
prefetched/decoded instruction, no wait states, no local HOLD delay, no
exception, and aligned memory operands; it therefore cannot be used to
allocate fault, delivery, or bus timing.

## Current timing-consumer inventory

| Consumer in `machine.c` | Profile/form class | Current source record | T360 disposition |
| --- | --- | --- | --- |
| `core_machine_8086_source_instruction_cost` | 8086 selected primary forms and additions | T359 S2/S3/S4 ledgers | S3 rechecks range and table-form boundaries; fixed selected rows remain unchanged until evidence says otherwise. |
| `core_machine_80186_source_instruction_cost` | 80186 selected primary forms | T359 S2/S3/S4 ledgers | S3 resolves parenthesized/range interpretation per form. |
| `core_machine_80286_source_instruction_cost` | 80286 selected primary forms, following-byte and memory additions | T357 S6 and T359 ledgers | S3 normalizes Appendix-B row, following-byte, protected state and NOP conflict. |
| `core_machine_80386_source_instruction_cost` | retained fixed primary rows | T359 S2/S3 and 80386 PRM | S4 performs form/prefix/mode citation sweep; no later-profile row may backfill earlier profiles. |
| `core_machine_primary_source_instruction_cost` | ALU/data/Group-3/SETcc | T359 S2 ledger | S2/S4 classify formula/range and lexical variants. |
| `core_machine_control_stack_source_instruction_cost` | calls/returns/stacks/loops/HLT/software interrupt | T359 S3 ledger | S3/S4 separate source rows from transition/delivery timing. |
| `core_machine_string_io_source_instruction_cost` | strings, repeat and ordinary I/O | T359 S4 ledger | S3/S4 preserve `REP LODS`, permission and physical-service transfers. |
| `core_machine_80386_dynamic_multiply_cost` | source-defined 80386 multiplier formula | T359 S2 ledger | S4 confirms exact edition/formula applicability; no 8086/80186 range is mapped onto it. |
| `core_machine_80386_secondary_source_instruction_cost` and `core_machine_80386_privileged_source_instruction_cost` | 80386 `0F`, system/selector/control/debug forms | T359 S5/S6 ledgers | S3/S4 validate source form, `m`, descriptor-granularity, mode and privilege boundaries. |

`core_machine_instruction_cost()` is the one post-refresh publisher. Its
visible `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` fallback is a transfer marker;
T360 may remove a successful form from that marker only with an exact source
disposition and focused regression.

## Conflict and uncertainty ledger

| Class | Exact observation | Interim state | Required receiver |
| --- | --- | --- |
| 80286 NOP conflict | Appendix B formal opcode table says `NOP = 3`; prose describes two clocks | T357/T359 retain the formal table row `3` under the documented provisional precedence; this S neither reaffirms nor changes it | T360 S2 establishes edition/page context and a primary-source precedence record, then sweeps every `NOP` consumer. |
| 8086/80186 range forms | Selected arithmetic/multiply/divide forms are published as ranges without an input-to-clock rule | No midpoint is allocated | T360 S3 determines whether bounded primary evidence yields a formula; otherwise retains a precise transfer/TODO. |
| 80286 Appendix-B context | Some rows require following-instruction bytes or protected/EA/odd-word distinctions | T359 selects only captured, stated rows | T360 S4 normalizes each permitted state into nonpublishing capture or preserves transfer. |
| 80386 form context | `m`, register/memory split, permission, lexical prefix, descriptor granularity and mode distinctions vary by opcode page | T359 selected only the exact rows it could classify | T360 S5 validates every selected 80386 citation and assigns unresolved variants to the correct later receiver. |
| Physical/delivery conditions | Wait/HOLD, device service, faults, exceptions, IRQ/NMI, task and VM86 frames are not instruction retirement | No instruction source row is synthesized | Queue bus-timed, service-timing and cycle-exact candidates own them. |

## Bounded T360 execution sequence

1. **S2 - authority editions and 80286 NOP precedence.** Establish a
   page/edition comparison record, decide or retain an explicit primary-source
   ambiguity for NOP, and sweep every current NOP consumer. No unrelated clock
   allocation.
2. **S3 - 8086/80186 range-form disposition.** Audit all dynamic/range forms
   against their exact manual wording; implement only a primary-defined formula
   with focused proof, otherwise create one precise retained TODO transfer.
3. **S4 - 80286 Appendix-B and 80386 contextual row normalization.** Reconcile
   following-byte, EA/odd-word, privilege, descriptor-granularity, `m`, and
   prefix/mode consumers into truthful nonpublishing capture or exact transfer.
4. **S5 - four-profile source and consumer closure audit.** Reconcile every
   changed consumer, rerun all focused/current gates, and close T360 without
   claiming bus, device, delivery, or cycle-exact work.

No S may use an implementation from another emulator as an authority, merge
profiles, or change source clocks as a side effect of a fixture repair.
