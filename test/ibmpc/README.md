# Shared IBM PC tests

This suite owns board-common, board-xt, board-at, Machine adapter/media and
PC Product regressions. It consumes src/ibmpc, src/x86, Common and Lib without
App source, external firmware, INI or disk inputs. Fixtures contain their own
bytes. PC-composition cases formerly under test/x86/core are owned here.
Fixtures are local; no other test package is a build or include dependency.
Production executors are linked from src/x86, never copied into fixtures.

```text
cmake -S test/ibmpc -B build/ibmpc-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ibmpc-tests
ctest --test-dir build/ibmpc-tests --output-on-failure
```

shared-ibmpc-tests builds every suite-owned executable. CTest registers the
original assertions and independent source/test manifest, DAG and negative
probes. test/x86 remains the owner of chip, neutral Core and Debug/xasm32
regressions and does not consume this outer package. Protected IRET assertions
remain there once; this suite retains its distinct real-mode/PIC composition.
Distinct board exception/paging regressions remain registered. Pure decoder
inventories, neutral Core entry and CPU/FPU interface tests belong to test/x86.
This suite directly covers neutral Machine memory replacement/media lifecycle
and shared AT descriptor/CPU-contract and ROM-candidate preparation.
Unused helper clusters, two historical timing
generators lacking their catalog, and an obsolete trace-API test are removed;
current trace-plan coverage remains in machine/debug_budget_smoke.c.
The frozen import/disposition ledgers are in the importing project's T85
S7 evidence and S10 proposal; these are historical import records, not current
test ownership. Decoder JSON now belongs to the x86 suite build directory.
No product boot coverage is substituted by these units.
