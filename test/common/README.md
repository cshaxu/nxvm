# Shared Common tests

This suite travels with `src/common`, `src/lib`, and `test/lib` in the same
four-directory layout. It has no importing product source/build dependency.

```text
cmake -S test/common -B build/common-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/common-tests
ctest --test-dir build/common-tests --output-on-failure
```

Coverage: session FIFO/wake, derived presentation/state matrix, input admission
and retirement, source-local physical-key identity, machine input FIFO,
machine/executor/debug lease contracts with a fake driver, xasm32 byte/text
contracts, and independent source manifest/DAG negative probes. Common sync
tests use Host; the existing native-thread machine fake is Windows-only and
does not imply Linux execution coverage. Production Common stays platform-free.

CLI bindings, original machine execution, cooked/raw product presentation,
firmware and media integration remain in the importing product's test suite.
They are not copied or weakened to turn them into Common unit tests.
