# Shared Lib tests

Transfer `src/lib`, `test/lib` and the shared CMake tools directly in `test/`.
This suite needs no other source or test package. Neutral products may also
adopt src/emulator and test/emulator independently.
Transfer all five shared CMake tools directly in `test/` with any test package.
This suite requires only `src/lib`, `test/lib`, those tools, a C11 compiler and system
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

Coverage: Types/atomic/clock, overlapping moves and text vocabulary; logical Console event/output gates;
Console broker, display and reader failure; Base sync lifetime and Linux waits; Storage
binary truncate/append writer, mode lock matrix, medium lease replacement/failure preservation and consumed-close
failure; KVM physical/text input,
matcher/frozen admission, frame damage, geometry/motion, control FIFO,
capture, component frame publication, terminal mailbox admission, modal wake,
failures and retirement; Audio queue, cancellation, partial delivery and backend lifetime. Native GUI smoke does not prove
every terminal host or interactive product scenario.
