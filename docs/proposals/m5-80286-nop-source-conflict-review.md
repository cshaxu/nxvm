# M5 80286 NOP Timing Source Conflict Review

## Purpose

Resolve the retained primary-source conflict for 80286 `NOP`: the selected
1987 Appendix-B opcode table says three clocks while prose describes two. The
task decides only the formal source precedence and its complete consumer
consequence; it does not use an emulator, benchmark, or averaged value.

## Scope

Establish the edition/page/context of both statements, determine whether they
describe the same execution condition, and inspect every 80286 timing ledger,
classifier branch, focused smoke, scheduler/device consumer, and downstream
corpus record. Retain the formal Appendix-B value or revise it only with a
primary-source decision and a focused regression.

## Boundaries and completion standard

No unrelated instruction timing, bus wait, device latency, source import, or
cycle-exact claim is in scope. The completion record must name the chosen
authority, the semantic context of the other statement, every changed or
unchanged consumer, and the residual physical-timing boundary. This review
must finish before the complete instruction-timing corpus treats 80286 timing
as source-settled.
