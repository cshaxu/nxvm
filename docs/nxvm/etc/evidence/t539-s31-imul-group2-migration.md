# T539 S31: immediate IMUL and Group-2 test ownership

## Scope and case map

S31 changes only NXVM test sources, test registration, and fixture gates. The
original two mixed suites contain 335 execution contexts. Every original
profile/program/branch has one current receiver:

| Original suite | CPU-only receiver | Board receiver |
| --- | ---: | ---: |
| Immediate IMUL: 58 contexts | 55 | 3 |
| Rotate/shift Group 2: 277 contexts | 269 | 8 |
| Total | 324 | 11 |

IMUL CPU contexts retain both 8086 #UD encodings, default/profile/LOCK and
operand-size forms, memory and segment selectors, 67/SIB/SS, VM86, and the
synthetic short-SS-cache fault. The original short SS limit was written
directly into `executor_cpu.data.ss.limit` after bootstrap; it was not a
board-loaded descriptor. The board receiver retains the real DS descriptor
limit fault and both PIC IRQ/no-shadow programs, including pushed IP/FLAGS,
ISR and IRR assertions. Its interrupt-frame checks intentionally exclude ESP,
which the interrupt must change.

Group-2 CPU contexts retain rotate forms (72), zero counts (24), multi-counts
(12), two profile cases, CL/profile matrix (32), shift forms (54), boundaries
(19), six shift profile cases, 8086 immediate rejection (16), and 80186/80286
immediate extensions (32). The board receiver retains two rotate and six
shift protected access faults (descriptor limit or non-writable descriptor).
CPU #UD tests bind the same terminal IVT condition formerly supplied by the
board preflight, so fault assertions do not depend on an empty vector entry.

The CPU receivers link only `x86-cpu`; board receivers use the public
Core-machine operations. No production/API, firmware, INI, media, Shared
corpus, or MyNES source changed. The test-local memory fixture holds the
original highest segment address `0x60010` without shortening the case.

## Verification

Complete x64 and x86 builds pass. Repository-only unit suites pass 397/397
on each width, from the S30 baseline of 395/395 plus two CPU receivers.
T317 strict coverage, T332 fixture lifecycle, T344 historical shape and unit
registration, CPU/PIC authority, and T344 direct strict-compilation matrix
pass on both widths; the matrix has 396 rows, with 363 strict and 33 deferred.
Both new CPU receivers compile with warnings treated as errors. The six
Shared source/test manifests remain unchanged and pass within the unit suite.
S31 is test-only; the x64 executable changed incidentally during a gate build
and was restored to its clean pre-build tracked version.

Actual pushed P1 `38bc5b10c` has exactly nine scoped paths, passes
`git show --check`, and matches `origin/master` at review. Its code/test/build
and gate delta adds 1,289/removes 1,468 lines (net -179), excluding the
49-line P1 evidence draft. No tracked binary is in the commit. S31 exits here;
S32-S45 remain unaccepted.
