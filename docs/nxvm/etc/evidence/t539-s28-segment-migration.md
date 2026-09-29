# T539 S28 Segment Selector and SREG MOV Migration

## Boundary and original cases

Baseline: accepted S27 P2 `5c93dbf8d`. The two original board smokes held 244
execution contexts and three instruction-metadata queries. Their original C
files have no C/H includers. This delivery changes NXVM tests and their build
and boundary checks; production, public ABI, Shared sources, MyNES, firmware,
INI and executable inputs are unchanged.

| Original group | Contexts | Receiver |
| --- | ---: | --- |
| Selector real loads | 4 | CPU |
| 80286 protected legal forms | 18 | CPU |
| 80286 protected cache rejection | 4 | CPU cache assertion and board exception delivery |
| LXS memory-only forms | 30 | CPU |
| LXS fault atomicity | 5 | CPU rollback and board fault outcome |
| Real segment-register loads | 15 | CPU |
| Protected segment-register success | 15 | CPU |
| Protected segment-register failures | 6 | CPU cache rollback and board fault outcome |
| Protected selector forms | 6 | CPU |
| Selector query edges | 15 | CPU |
| Rejected selector forms | 19 | CPU terminal UD |
| POP fault atomicity | 1 | CPU rollback and board fault outcome |
| VERR/VERW/reserved metadata | 3 queries | CPU |
| SREG MOV real profile forms | 56 | CPU |
| SREG MOV 386 extensions | 8 | CPU |
| SREG MOV rejection and attribute forms | 24 | CPU terminal UD and register/memory effects |
| SREG MOV protected forms | 15 | CPU; six faults also have board receivers |
| SREG MOV IRQ shadow | 3 | Board PIC, frame IP, shadow and segment result |

This yields 241 CPU executions and 25 board executions. Twenty-two faults
have complementary CPU and board receivers, while three IRQ contexts have
only the board receiver: 241 + 25 - 22 = 244 original contexts. The three
metadata queries remain CPU-owned. Original program tables, profile loops and
private cache assertions stay with the CPU owner. Board tests retain actual
80286 delivery, 80386 machine fault results and PIC acknowledgement.

## Defect corrected during migration

The original 80286 cache-rejection helper returned one on success and zero on
failure, while `main` interpreted one as failure. Its assertions could fail
and still yield a passing executable. Both receiving tests now return zero
for success and one for failure. The same helper compared EAX after executing
`MOV AX,imm16` to EAX captured *before* that instruction. It now compares to
the immediate encoded in each original program. The corrected four contexts
pass on both widths. No CPU production behavior changed.

## Ownership and structural review

Both CPU receivers link only `x86-cpu` and use the existing instruction
fixture. Its RAM fixture grows from 128 to 256 KiB so the original
`DS=3333h` memory forms still address their original physical locations.
Protected bootstrap runs a bounded instruction count before the temporary
HLT; it does not alter the guest programs being tested. CPU-local tests keep
complete private selector/cache and accessed-bit assertions. No new public
helper or cache accessor is added.

Both board receivers now create/freeze/reset through the existing public
machine operations and use public register patches and copied CPU snapshots.
The original SREG MOV IRQ context preserves its registers, segment selectors,
FLAGS and PIC/vector setup. Board faults assert public outcome and copied
rollback. Descriptor bytes after a terminal machine fault remain checked by
the CPU receiver: public machine memory reads require stopped/paused state and
are unavailable in `FAULTED` lifecycle. The board does not borrow RAM or CPU
storage to make that post-fault assertion.

The T337 terminal-UD classification moves to the CPU test targets. T332
classifies the two board receivers as public setup; T344 counts 111 inventoried
and 118 total direct constructors, with 20 shared tails and 91 explicit
shapes. CPU/PIC boundary negatives cover both new board receivers, rejecting
four private-access mutations each. The source inventory drops from 86 to 84
remaining original private consumers assigned S29-S37. Opaque lifetime and
physical Shared relocation remain S38 and S39.

## Verification and delivery

Full x64 and x86 builds succeed. Complete units pass 387/387 on x64 (29.35
seconds) and x86 (29.70 seconds).
All 66 specialized gates pass, including the 386-row direct compilation
matrix (353 strict, 33 deferred). Six unchanged corpus manifests, documentation
governance, diff checks and target-scope review complete before P1. This is
an implementation record awaiting actual-commit coordinator review; it does
not close S28 or the CPU/T539 ledger by itself.

The ten test/build paths add 2,065 lines and remove 1,530, net +535. No
production, firmware, INI, executable or protected asset file changes.
