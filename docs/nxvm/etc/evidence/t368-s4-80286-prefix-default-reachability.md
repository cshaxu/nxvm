# T368 S4: 80286 Prefix and Default Reachability Ledger

## Boundary and method

This is a classifier reachability ledger, not an allocation of a fallback
clock.  It reads the sole post-refresh publisher and the ordered 80286 chain
in `machine.c`, then compares its gates with the selected decoder metadata and
the accepted T359/T366 owners.  A row is a candidate only after an instruction
successfully retires; decoder rejection and synchronous delivery do not reach
this ledger.

Intel 210498-005 Appendix B remains the first authority for later successful
form allocation.  Where it does not distinguish a prefix/context, the source
policy requires an explicitly documented same-profile reference-model step;
this S allocates neither such a value nor physical time.

## Exact residual routes

| Classifier condition | Successful candidates remaining after prior owners | Not a candidate | Next receiver |
| --- | --- | --- | --- |
| `prefixes >= oplen` | None. | A byte stream containing only prefixes cannot retire an instruction. | None. |
| `prefixes != 0u` before the local opcode switch | Legal non-string, non-I/O prefix combinations that primary/control ownership does not already accept: in particular legal 80286 LOCK context and any successfully retired non-string REP or segment-prefix context outside those owners. | Defined string/repeat and ordinary-I/O paths are owned first by `core_machine_string_io_source_instruction_cost`; one legacy segment override on a primary or control/stack form is owned by its respective classifier. `64/65/66/67` are below-profile rejection, and faulting/illegal prefix combinations do not publish. | S6 prefix-context execution matrix and Appendix-B/reference-model disposition. |
| `0F 00 /2,/3` | Protected CPL0 successful `LLDT` and `LTR`; they are decoder-valid but are not among the local `SLDT/STR` or `VERR/VERW` predicates. | Real-mode/CPL violation, selector/table fault, task-transition/delivery and reserved `/6,/7` forms. | S5 80286 system-form authority/context reconciliation. |
| `0F 01 /0`--`/3` | Successful `SGDT`, `SIDT`, `LGDT`, `LIDT` in their real/protected CPL0/memory contexts. | Register encodings, VM86/CPL rejection, descriptor-memory fault and delivery. | S5. |
| `0F 01 /4,/6` | None: SMSW/LMSW already have the accepted successful `2/3` and `3/6` routes; S3 proves the protected LMSW fixture state. | Faulting forms. | Existing owner. |
| `0F 02`, `0F 03`, `0F 00 /0,/1,/4,/5` | None: LAR/LSL, SLDT/STR and VERR/VERW have existing successful routes. | Faulting selector/table/memory outcomes. | Existing owner. |
| `0F 06` | Successful real-address or protected-CPL0 `CLTS`. | CPL violation and any delivery path. | S5. |
| Explicit local invalid-form fallbacks (`BOUND` register form, real-mode ARPL, invalid segment-register ModR/M, register LES/LDS, Group-2 `/6`) | None; these are decoder/semantic rejection paths and must remain zero-publication. | All forms listed at left. | S7 negative boundary confirmation. |
| Final default after `B0`--`BF` | 80286 `WAIT` and `ESC D8`--`DF` require an explicit x87/interface disposition; they cannot inherit an integer fallback. | Reserved/undefined primary slots and below-profile instructions. | S7 x87/default closure audit. |
| Lookup fallback | None in the accepted primary/control table entries: T366 S10 mechanically proves every extracted control/stack form appears in the 80286 ledger. | Null/non-80286 helper paths. | Static guard retained; S7 re-runs it. |

## Ordered implementation sequence

1. **S5:** audit and, only where Appendix B is exact and the completed context
   is capturable, admit the five residual system families: `LLDT/LTR`,
   `SGDT/SIDT/LGDT/LIDT`, and `CLTS`.
2. **S6:** prove the legal 80286 prefix matrix at successful retirement,
   allocate only authority-backed rows, and retain source-undefined legal
   contexts as one named transfer.
3. **S7:** prove the invalid forms are non-retiring, decide the explicit
   x87/WAIT interface receiver, rerun the whole lookup sweep, and perform the
   T368 CPU-ledger closure audit.

## Verification and non-claim

The inspected source anchors are `core_machine_instruction_prefix_count`,
`core_machine_string_io_source_instruction_cost`,
`core_machine_primary_source_instruction_cost`,
`core_machine_control_stack_source_instruction_cost`,
`core_machine_80286_source_instruction_cost`, and
`verify_t366_s10_80286_unallocated_publishers.cmake`.  The ledger retains the
existing one publisher and makes no decoder, executor, bus, device, profile or
physical-clock change.  It does not claim complete 80286 timing or IBM 5170
L3.
