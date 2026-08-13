# T338 S4: 80186 Primary-Extension Profile Matrix

## Scope And Mechanism Sweep

This record closes the S1-allocated 80186-only primary forms. The sweep covers
the metadata/primary-dispatch profile gate, `PUSHA`/`POPA`, `BOUND`, immediate
`PUSH`/`IMUL`, `INS`/`OUTS`, `INS_C0`/`INS_C1`, `ENTER`/`LEAVE`, and their
stack, arithmetic, string, port, prefix, and final-delivery collaborators.
`F0` remains T328's single legacy bus-prefix policy owner; this S neither adds
a 80386 whitelist nor treats an invalid form as valid.

## Form/Owner Matrix

| Forms | 8086 disposition | 80186 proof owner | Defined boundary |
| --- | --- | --- | --- |
| `60/61` | `#UD` before stack publication | `core-machine-pusha-popa-smoke` | Original SP image, POPA skipped slot, all word registers, and stack image. |
| `62 /r` | `#UD` before source read | `core-machine-bound-s54-smoke` | Memory-only signed bounds, 80186 real `#BR` vector-5 restart frame, and no publication. |
| `68`, `6A` | `#UD` before immediate/stack publication | `core-machine-push-immediate-smoke` | Word immediate and sign-extended imm8 stack images. |
| `69 /r`, `6B /r` | `#UD` before source/publication | `core-machine-imul-immediate-s56-smoke` | Register alias plus direct r/m16 source, signed product, CF/OF, and high-word retention. |
| `6C`--`6F` | `#UD` before port or memory effect | `core-machine-port-strings-smoke` | All single and `REP` byte/word input/output forms, count/index, port callbacks, and source/destination memory. |
| `C0/C1 /0`--`/5`, `/7`; `/6` | `#UD` before operand access | `core-machine-rotate-smoke` | 80186 byte/word immediate-count rotate/shift results; `/6` terminal `#UD` with GPR/FLAGS nonpublication. |
| `C8/C9` | `#UD` before stack publication | `core-machine-enter-leave-smoke` | Allocation, nesting display, `imm8 & 31`, frame construction, and 16-bit LEAVE stack route. |

## Defect And Similar-Issue Disposition

The new 80186 `INS`/`OUTS` vectors reproduced one repeated defect: all four
local handlers retained a 80386 guard although metadata and architecture define
them from 80186. The four profile guards now uniformly use 80186. No shared
port, REP, prefix, or delivery helper changed. Existing 80286 and 80386
vectors remain regression evidence; protected I/O permission, VM86, and 80386
width paths transfer to later candidates.

The matrix adds direct 80186 `BOUND` real `#BR` delivery, all byte/word
`C0/C1` extensions, and `69/6B` r/m16 evidence. It does not claim
Intel-undefined arithmetic FLAGS.

## Verification

The seven matrix owners passed as an exact current-gate selection. The full
current-gate suite passed 218/218 tests; documentation governance and
`git diff --check` passed. The rebuilt `vm-0-5-0338` developer artifact is
SHA-256 `7D79417889821695DB4993DFEA5134B01E5B16D69007C20A6F1E3CBB8C75C05F`.
