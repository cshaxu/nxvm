# T363 S3: control and stack reconciliation

## Result

The successful control/stack mechanism remains completely source-backed by the
accepted T359 S3 ledger; S3 finds no uncovered row or runtime change.
`core_machine_control_stack_source_instruction_cost()` is the sole owner after
successful refresh and before the legacy fallback. It normalizes opcode,
extension, memory, prefix legality, old/new mode, privilege outcome, branch
outcome and next-instruction term without changing guest state.

| Form partition | Disposition |
| --- | --- |
| near/far CALL and JMP, RET, PUSH/POP, PUSHA/POPA, PUSHF/POPF, ENTER/LEAVE | Four-profile Intel rows plus existing EA/odd-word or documented next-term helper. |
| LOOP/LOOPE/LOOPNE/JCXZ, HLT, INT3/INT, INTO, same-level real IRET | Successful outcome rows selected at the one publisher; 80286 next-byte and 80386 `m` terms use nonpublishing lexeme preview. |
| rejected prefix/LOCK, failed stack access, fault/restart | No successful-retirement tick; delivery stays outside S3. |
| protected far transfers, protected INT/INTO/IRET, call gates, cross-privilege/task/VM86 and NMI/IRQ/exception delivery | Explicit S6/physical/cycle-exact receiver; no source row is borrowed. |

The 80386 protected same-level `INT` row remains a transfer: its 59 clocks
exceed the 46-clock existing source preflight ceiling, so S3 does not raise a
global ceiling for a context it cannot fully own.

## Verification

`core-machine-t359-s3-timing-smoke` passed on 2026-08-14. It covers direct,
register and memory transfer shapes; stack variants; PUSHA/POPA/ENTER/LEAVE;
taken/not-taken LOOP/JCXZ; HLT; real-mode INT/INT3/INTO/IRET; elapsed-provider
publication and retained fault/reset nonpublication. Static owner scanning
finds no handler-local elapsed tick or second publisher. No runnable path
changed, so S3 creates no T363 developer artifact. S4 receives only string,
repeat and ordinary I/O timing.
