# T539 S33: first INC/DEC through DIV/IDIV test ownership

## Original-case receiving map

All 19 original first-group functions have one receiver. The CPU receiver
links only `x86-cpu`; the board receiver uses public machine construction,
debug snapshots/register patches and memory reads. No production algorithm,
public ABI, input or firmware changed. The S34/S35 functions remain in the
original mixed-owner file for their assigned migrations.

| Original group | CPU instruction executions | Public board contexts |
| --- | ---: | ---: |
| INC/DEC (register, r/m, address/profile; protected fault) | 122 | 2 |
| NOT/NEG (forms, address/profile; protected faults) | 15 | 4 |
| TEST (forms, accumulator profiles, address/profile; protected fault) | 24 | 1 |
| MUL/IMUL (forms, sign-extension profiles, address/profile; protected faults) | 24 | 2 |
| DIV/IDIV (forms, attribute/profile; real #DE delivery and protected faults) | 15 | 14 |
| **Total** | **200** | **23** |

The 200 CPU executions were counted by temporarily instrumenting the sole
test-local execution call, recording each of the 13 CPU function totals, then
removing that instrumentation. The board cases are 11 protected descriptor/
access faults and 12 real-mode divide deliveries (six opcode forms across
zero-divisor and quotient-overflow classes). Original flags, register, memory
nonpublication, profile and 16/32-bit exception-frame assertions are retained.
The board receiver has no `executor_cpu` or private machine-memory access.
The remaining 32 original S34/S35 test functions retain their existing path;
their private-access inventory remains open until S35.
The historical T316/T401 first-group success banners move with the executing
CPU tests, rather than remaining in the original test after its cases moved.

## Verification

Full x64 and x86 builds pass. Full repository-only unit suites pass 401/401
on each width, versus 399/399 before S33 plus the two receivers. All 66
current specialized gates pass per width, including T317, T332, T337, T344,
CPU/PIC authority and the 400-row direct compilation matrix. The six Shared
source/test manifests remain unchanged and pass. `git diff --check` passes.
No tracked product binary changes: S33 only reassigns test ownership and
registers the two new test executables.

Actual pushed P1 `6ca7f61ac` has exactly six scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. It adds
1,099/removes 1,041 lines overall; excluding the 43-line evidence report,
the code/test/build/gate change adds 1,056/removes 1,041 (net +15). There is
no tracked product binary. S34/S35 remain explicitly pending.
