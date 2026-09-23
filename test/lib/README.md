# Shared Lib tests

Transfer `src/lib`, `src/common`, `test/lib`, and `test/common` unchanged.
This suite requires only `src/lib` and `test/lib`, a C11 compiler and system
libraries. No product target, firmware, INI or media is required.

```text
cmake -S test/lib -B build/lib-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/lib-tests
ctest --test-dir build/lib-tests --output-on-failure -LE desktop
```

The suite owns test registration, fixtures, failure injection and its manifest.
Production verifiers remain in src/lib. Platform probes are selected explicitly
by CMake; Linux fakes test algorithms, not native Linux runtime availability.
Assertions remain enabled in Release. Scratch files stay in the build directory.
The shared file cleanup helper belongs to this suite's storage probes.

The `desktop` label marks real native Window/Console tests, including tests
that hide a window only after creating it. Run these explicitly with `-L desktop
-j 1` when the desktop is reserved for testing. Mocked input/rendering tests
remain in the background suite. An unfiltered CTest run includes both groups.

Coverage: Types/atomic/clock and vocabulary; logical Console event/output gates;
Console broker, display and reader failure; Base sync lifetime and Linux waits; Storage
binary writer, medium and consumed-close failure; KVM physical/text input,
matcher/frozen admission, frame damage, geometry/motion, control FIFO,
capture, modal wake, failures and retirement. Native GUI smoke does not prove
every terminal host or interactive product scenario.
