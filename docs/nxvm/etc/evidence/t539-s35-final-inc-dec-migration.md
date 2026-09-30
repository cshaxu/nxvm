# T539 S35: final INC/DEC mixed-test retirement

## Original-case receiving map

All 19 remaining functions in `core_machine_inc_dec_smoke.c` have one
receiver. Instruction observations moved to the CPU-only test; protected
faults and real divide delivery moved to the public board test.

| Original group | CPU instruction executions | Public board contexts |
| --- | ---: | ---: |
| OR | 28 | 2 |
| AND | 31 | 2 |
| SUB | 34 | 2 |
| XOR | 28 | 2 |
| CMP | 34 | 2 |
| Decimal adjust | 7 | 1 |
| XLAT | 3 | 1 |
| Group-1 matrix | 160 | 0 |
| **Total** | **325** | **12** |

The CPU execution counts were measured temporarily at the test-local
execution call for each function, then the instrumentation was removed.
The CPU receiver links only `x86-cpu`. The ten logical/CMP protected fault
contexts use the existing public board-fault fixture; the AAM zero-divisor
context uses a new test-only public divide-delivery fixture, now shared with
S33's twelve DIV/IDIV contexts. XLAT's protected fault is checked through
public machine operations. Historical T316/T401 final-group success markers
moved with the corresponding CPU cases. The original mixed source and its
private-test inventory entry are removed; the inventory now records 71
pending consumers for S36-S42.

No production CPU/timing algorithm, public ABI, firmware, INI, media,
Shared corpus, MyNES or executable input changed.

## Verification

Full x64 and x86 builds pass. The complete repository-only unit suite passes
404/404 on each width, with architectures run sequentially to avoid host
resource contention. All 66 current specialized gates pass per width,
including T317/T332/T337/T344, CPU/PIC authority, documentation governance
and T344's 403-row direct strict-compilation matrix (370 strict, 33 deferred).
The six unchanged Shared manifests verify. `git diff --check` is clean.
There is no tracked product binary change; S35 only reassigns test owners.

Actual pushed P1 `0fc194460` has ten scoped paths, passes `git show --check`,
equals `origin/master` at independent review and adds 508/removes 733 lines
(net -225). Excluding this 45-line evidence report, code/test/build changes
add 463/remove 733 lines (net -270). P2 acceptance is recorded in the active
packet. S36-S45 remain explicitly pending.
