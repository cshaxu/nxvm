# T500 S10: Existing-Test Parallel Isolation

T500 remains an existing-test reorganization task.  It adds no test, fixture
class, coverage target, or product behavior.

The fixed-write unit-fixture sweep found two unowned CTest working directories:
the debugger recording smoke writes its recording file and the XT profile smoke
writes synthetic ROM and media files.  Both now use the existing per-test
build-tree working-directory mechanism already used by the Console fixtures.
There is one CTest ownership mechanism, no global serial setting, and no
second fixture path.

The integration keyboard-input sweep found that the Windows checkpoint sent
direct Core native bytes without a complete key lifecycle.  It now uses the
existing VM Win32 host-input producer for `Enter`, `C`, and `:` transitions.
This removes its Core-input bypass and gives each key an explicit press and
release through the product's one mapper path.

Verification after the repair:

- `cmake --build build/mingw-gcc-x64 --target run-unit-tests --parallel 4`:
  312/312 passed in 16.57 seconds.
- `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -L integration -j 4`:
  20/20 passed; `integration.vm-windows31-checkpoint` passed under the same
  run.
- `cmake --build build/mingw-gcc-x64 --target verify-documentation-governance
  --parallel 4` and `git diff --check` passed before final documentation.

Tracked code change, excluding documentation and generated files: 35 added,
21 removed, net +14 lines.  The increase replaces the direct incomplete byte
sequence with the required host-path press/release transitions; no production
path or test inventory changed.
