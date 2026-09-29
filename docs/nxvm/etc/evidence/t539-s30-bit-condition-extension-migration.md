# T539 S30 Bit/Condition/Extension Test Ownership

Baseline is accepted S29 P2 `3ebd70b97`. This S moves assertions from six
mixed NXVM tests; it changes no production source, public ABI, Shared corpus,
MyNES source, firmware, INI, media or executable input.

## Original-case receiving map

| Original suite and exact contexts | CPU-only receiver | Board receiver |
| --- | --- | --- |
| Bit scan: 16 opcode/width/memory/zero forms, two older-profile #UD cases, two protected read-limit faults (20) | `cpu_bit_scan_smoke.c`: 18 | `core_machine_bit_scan_smoke.c`: 2 |
| Bit test: eight register forms, four immediate groups, five offset/memory forms, eight indexed/immediate memory destinations, three #UD rejections, two protected access faults (30) | `cpu_bit_test_smoke.c`: 28 | `core_machine_bit_test_smoke.c`: 2 |
| SHLD/SHRD: 376 direction/width/location/count-source/count forms, two zero counts, two older-profile #UD cases, two protected access faults (382) | `cpu_double_shift_smoke.c`: 380 | `core_machine_double_shift_smoke.c`: 2 |
| Two-operand IMUL: eight width/location/overflow forms, two older-profile #UD cases, one protected read-limit fault (11) | `cpu_imul2_smoke.c`: 10 | `core_machine_imul2_smoke.c`: 1 |
| SETcc: 32 register and 32 memory condition/truth forms, two prefix forms, one 286 #UD and one protected limit fault (68) | `cpu_setcc_smoke.c`: 67 | `core_machine_setcc_smoke.c`: 1 |
| Sign extension: 16 default profile/opcode/sign, four 32-bit, two address-prefix, twelve rejected-prefix, two LOCK #UD and two IRQ forms (38) | `cpu_sign_extend_smoke.c`: 36 | `core_machine_sign_extend_smoke.c`: 2 |

All 549 original execution contexts retain one receiver: 539 CPU-owned and ten
board-owned. The original program byte arrays, profile sets and assertion
matrix are preserved. CPU-only receivers link only `x86-cpu`; no parallel
Core-machine execution remains for those instruction assertions. The six board
receivers observe actual descriptor loading/exception delivery or PIC IRQ,
frame and ISR effects through a single Core-machine path.

Five protected-limit suites share one test-local public machine constructor,
`support/cpu_board_limit_fixture.h`; the IRQ suite keeps its distinct board
configuration. The helper uses create, freeze, reset, register patch, real
descriptor/bootstrap bytes and `core_machine_run`. It owns no CPU-private
register/cache pointer. The two profile #UD probe suites and bit-test
rejections share `support/cpu_operand_probe_fixture.h`, which counts operand
transactions at the original `0x5000` address without counting instruction
fetch. T337 assigns their terminal #UD assertions to CPU-only targets.

Across the 19 changed code/test/build paths, the migration adds 1,429 lines
and removes 1,506 (net -77); the two small test-local fixtures replace five
copied descriptor bootstraps and three copied operand probes.

The original faulted-machine physical-memory assertions remain board-owned.
After a fault, ordinary `core_machine_memory_read` rejects the FAULTED state;
the board tests use existing `core_machine_debug_read_real` instead. No new
production read path or lifecycle exception was introduced.

T332 now verifies the public helper's lifecycle and five actual callers. T344
classifies 108 direct constructors and five shared-helper callers rather than
silently lowering its inventory count; the sixth IRQ receiver is an explicit
direct board constructor. The CPU/PIC authority gate checks all six board
receivers, and the negative fixture injects four forbidden private accesses
into each (96 migrated-board controls).

## Verification and remaining boundary

Complete repository-only unit suites pass 395/395 on x64 and x86 (from the
389/389 baseline plus six CPU receivers). The 12 focused CPU/board tests pass.
All 66 specialized gates pass in both widths, including T332/T344, T337, the
CPU/PIC authority check and the strict direct-compilation matrix. Six unchanged
Shared source/test corpus manifests and documentation governance pass. Actual
pushed P1 `442088410` contains exactly 23 NXVM-scoped paths, passes
`git show --check`, and matches `origin/master` at review; S30 is accepted.

The original direct-private `.c` search has 81 entries at S29 HEAD and 75 now;
including the still-live shared fixture header gives the S29 recorded 82 and
the current 76 pending private-test consumers. S31-S42 retain those cases;
S43-S45 still own lifetime, physical relocation and whole-CPU acceptance.
No executable or ROM artifact is required for this test-only S.
