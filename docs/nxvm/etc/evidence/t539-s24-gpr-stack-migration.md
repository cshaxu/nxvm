# T539 S24: GPR Stack Test Migration

Baseline a9ca9a6f2. Current owns admission and acceptance. This batch consumes
the three S24 entries in the [incremental inventory](t539-cpu-incremental-inventory.md),
not the whole CPU row. Implementation and verification are complete; coordinator
actual-commit acceptance is still pending.

## Original Coverage And Receivers

Counts are explicit instruction contexts, not CTest processes or assertion counts.
The original arrays, profile loops, sentinels and assertion conditions remain
the coverage authority. Sources are the three core_machine_*_smoke.c files
below test/app-nxvm/unit/core/devices.

| Original family | Contexts | Receiver |
| --- | --- | --- |
| gpr_push_pop: PUSH registers | 32 | CPU: four profiles times eight registers, including PUSH SP generation difference |
| gpr_push_pop: POP ESP-relative address | 1 | CPU: address evaluated using advanced ESP |
| gpr_push_pop: POP registers | 32 | CPU: four profiles times eight registers |
| gpr_push_pop: r/m forms | 26 | CPU: six forms on four profiles plus two 386 address forms |
| gpr_push_pop: rejections | 22 | CPU: nine legacy attributes, six LOCK forms, seven invalid POP extensions |
| gpr_push_pop: 386 attributes | 4 | CPU: operand/address-size combinations |
| gpr_push_pop: protected faults | 4 | CPU: full private cache rollback; board: real descriptors, copied registers/segments and physical-memory sentinels |
| gpr_push_pop: IRQ without shadow | 4 | Board: register/rm PUSH/POP, frame IP, stack image, PIC ISR/IRR |
| push_immediate: defaults | 8 | CPU: six supported and two 8086 rejection cases |
| push_immediate: attributes/LOCK | 17 | CPU: four success, nine legacy rejection, four LOCK rejection cases |
| push_immediate: protected faults | 2 | CPU and board: 16/32-bit stack writes rejected by expand-down limit, with owner-local cache assertions retained |
| push_immediate: IRQ | 2 | Board: both immediate encodings and sign-extension stack image |
| pusha_popa: defaults | 6 | CPU: three profiles times PUSH/POP |
| pusha_popa: attributes | 6 | CPU: six operand/address-size forms |
| pusha_popa: rejections | 28 | CPU: two 8086, eighteen legacy attribute and eight LOCK cases |
| pusha_popa: protected PUSH/POP limits | 2 | CPU and board: register/cache rollback and complete original five/eight-slot memory checks |
| pusha_popa: IRQ | 2 | Board: complete push image or popped registers, frame IP, PIC ISR/IRR |

Total: 198 original contexts, with 190 CPU executions and 16 board executions.
Eight protected-fault contexts deliberately have complementary receivers:
the CPU receiver retains private cache fields absent from copied observations;
the board receiver retains actual descriptor loading, machine fault lifecycle,
physical RAM and PIC composition. This is 206 executions, not 206 original
contexts. No original context is removed.

## Boundary Review

The three original files mixed instruction invariants with a complete PC
machine and private executor_cpu mutation. CPU receivers now use the existing
CPU-owned bounded-memory fixture and link only x86-cpu. Board cases use existing
debug register patches and copied CPU snapshots. Guest code loads the GDT and
enters protected mode; a paused register patch reloads the descriptor needed
for the target fault. No board test accesses private CPU fields. Complete
private segment-cache comparisons, including validity/type fields absent from
the copied contract, remain at the CPU owner.

The original gpr_push_pop sentinel at physical 0x47ffe is retained as written;
it is not silently changed to 0xbffe. Faulted-state physical RAM inspection
remains board-owned, as in S22/S23. No new product API is needed for it.

Production handlers and all Shared corpora stay unchanged. S38 still owns
opaque allocation and S39 the physical CPU move. No new artifact is justified
unless executable inputs change; current 0539 pairs remain the baseline.

## Actual-Difference Review

The original seven register/cache comparison helpers remain unchanged in the
CPU receivers. Of 83 original constant byte-array declarations, 82 remain
unchanged after whitespace normalization across the receiver pair. The sole
removed declaration is the one-byte HLT used only by protected-mode fixture
setup in pusha_popa: a ten-instruction execution budget now stops before it,
so no private unhalt operation is needed. IRQ-path HLT instructions remain.
Original instruction matrices, malformed encodings, profile loops, memory
sentinels and full five/eight-slot PUSHA/POPA expectations remain.

An initial board-fixture attempt loaded the deliberately invalid SS limit
during bootstrap. All three board receivers failed before the instruction
under test, while the CPU receivers passed. Inspection located the failure
at MOV SS (IP 0015h), after seven bootstrap instructions: the existing CPU
checks ESP after each instruction. The original tests instead corrupted the
cache after bootstrap. The final board setup therefore boots with valid
descriptors, stops at the existing instruction budget, then reloads the
test descriptor through the existing paused-debug register API. This retains
the original target precondition without changing production behavior, adding
an API, or weakening the target fault assertions. Temporary diagnostics were
removed. This batch does not claim a new MOV SS architectural qualification.

## Verification And Sweep

- Fresh full builds pass in both retained NXVM trees.
- Full unit suites run sequentially across widths: x64 379/379 (27.87 s),
  x86 379/379 (28.28 s). Each contains the three additional CPU receivers.
- verify-current-specialized-gates passes: 66 checks; its final invocation
  also performed six CPU-target rebuild/link steps. T344 classifies 104
  inventoried and 111 total constructor sources; the strict matrix has
  378 rows (345 strict, 33 unchanged deferred).
- All six Shared source/test manifests pass without corpus changes.
- The CPU authority negative test now rejects four private-access forms in
  each of eight migrated board tests (32 controls), alongside its existing
  72 CPU and five board-production controls.
- Documentation governance and git diff --check pass; final delivery checks
  are repeated after the evidence update.

The inventory query is
`rg -l 'executor_cpu\.|executor_cpu_instructions|support/core_machine_cpu_fixture.h' test/app-nxvm/unit`.
It finds 92 remaining original consumers and one deliberate negative-verifier
string owner. All three S24 board files are absent; S25-S37 retain the other
receivers. Build lists, UD-owner registration, constructor/lifecycle checks
and the negative fixture move together. No production, Shared, MyNES, INI or
asset input changed. Retained build trees and the recovery patch remain needed
by subsequent CPU packages; test-only changes require no new EXE.

## Size And Ownership

Git numstat over the eleven test/build paths (five CMake/boundary files and
six C receivers, including the three new files) gives 1,692 added and 1,232
removed lines, net +460; documents and artifacts are excluded. The increase
retains eight complementary CPU-cache regressions and replaces implicit
private board mutation with explicit existing public operations. It adds no
production layer or duplicate CPU implementation. The original board files
now contain only protected-fault and PIC integration; instruction matrices
have one CPU-local home. No generic framework or new fixture API was added.
