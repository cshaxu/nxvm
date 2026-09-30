# T539 S34: TEST, ADD, ADC and SBB test ownership

## Original-case receiving map

All 13 original second-group functions have one receiver. Two mixed functions
(`adc_attribute_profile_fault` and `sbb_boundaries`) were split at their
protected fault section: CPU instruction observations stay in the CPU test,
and the original board fault cases run through public machine operations.

| Original group | CPU instruction executions | Public board contexts |
| --- | ---: | ---: |
| TEST r/m-reg | 15 | 3 |
| ADD | 31 | 2 |
| ADC | 31 | 1 |
| SBB | 34 | 2 |
| **Total** | **111** | **8** |

The 111 CPU executions were counted temporarily at the sole test-local
execution call, with each function's count recorded before removing the
instrumentation. The eight board contexts retain the three TEST, two ADD,
one ADC and two SBB protected descriptor/access fault cases. The CPU receiver
links only `x86-cpu`; the board receiver uses only the public machine API.
The same board-fault setup and verification now live in one test-only helper
used by both S33 and S34, eliminating parallel implementations of the same
fault contract. S33's 23 cases still pass. Historical T316 second-group
success markers moved with their executing CPU cases.

Nineteen S35 functions remain in `core_machine_inc_dec_smoke.c`, including
its original private-test accesses; they are still assigned to S35. No
production CPU/timing algorithm, public ABI, firmware, INI, media, Shared
corpus, MyNES or executable input changed.

## Verification

Full x64 and x86 builds pass. Both repository-only complete unit suites pass
403/403, versus 401/401 at S33 acceptance plus two receivers. All 66 current
specialized gates pass on each width, including T317/T332/T337/T344,
CPU/PIC authority, documentation governance and T344's 402-row direct
strict-compilation matrix (369 strict, 33 deferred). The six Shared source/
test manifests remain unchanged and verify. `git diff --check` is clean.
There is no tracked product binary change; S34 only reassigns test owners.

The implementation and actual-commit review hashes are recorded in the S34
acceptance packet. S35-S45 remain explicitly pending.
