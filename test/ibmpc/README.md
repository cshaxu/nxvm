# Shared IBM PC tests

This suite owns board-common, board-xt, board-at, Machine adapter/media and
PC Product regressions. It consumes src/ibmpc, src/x86, Common and Lib without
App source, external firmware, INI or disk inputs. Fixtures contain their own
bytes. Cross-domain Core fixtures are borrowed explicitly from test/x86/core;
there is no copied executor or relaxed assertion.

```text
cmake -S test/ibmpc -B build/ibmpc-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ibmpc-tests
ctest --test-dir build/ibmpc-tests --output-on-failure
```

shared-ibmpc-tests builds every suite-owned executable. CTest registers the
original assertions and independent source/test manifest, DAG and negative
probes. test/x86 remains the owner of chip, neutral Core and Debug/xasm32
regressions; its retained PC-composition tests explicitly consume this package.
No product boot coverage is substituted by these units.
