# T564 S6: Lib Console and desktop isolation review

## Disposition

The historical T538 cooked Console failure after a failed raw activation was
not reproduced as a current defect. `console_broker_replace` prepares the new
binding, retires the old one, and attempts to reactivate the old binding if
the new activation fails; a failed restoration leaves the broker broken.
The Win32 backend saves and restores display metadata and checks the final
viewport. This does not prove preservation of cells outside the area that the
native host allows the integration test to observe.

Existing `console_broker_smoke` covers activation failure and rollback;
`console-broker/win32/console_smoke` covers display geometry and errors.
The native `console_broker_display` integration also injects reader failure,
compares restored metadata and visible cells, and exercises successful switches
and failure rollback. No additional production change or duplicate test was
justified by the review. This is not a claim that every possible host resize
or invisible history cell has been tested.

The native desktop tests use CTest `RESOURCE_LOCK native_desktop`, which
serializes within one CTest process, not between independently launched CTest
processes. Historical modal failures were not established as a current defect
or attributed to a particular cause. Execute desktop suites from different
build trees serially; no cross-process lock or new recovery mechanism was
added. RDP and arbitrary concurrent external desktop users were not qualified.

Lib's CMake enables `-Wall -Wextra -Wpedantic -Werror` for GNU/Clang; the
source and test trees contain no explicit `-w`, `-Wno-*`, or `-Wno-error`
option. This is a scoped configuration audit, not a claim about all platforms.
The separate X86 Core instruction-unit GCC warning exceptions are outside S6
and remain for the ordered Core review.

## Verification

- Existing x64 and x86 Release Ninja graphs: `build-unit-tests` and
  `verify-current-specialized-gates` succeeded; the direct strict-compilation
  matrix reports 486 retained rows and zero deferred for each architecture.
- Repository unit aggregate: x64 697/697, x86 697/697; run sequentially.
- Native desktop integration label: x64 5/5, then x86 5/5, including Lib
  Console display rollback and Window modal/retirement and MyNES native tests.
- The restricted-environment x64 aggregate attempt was interrupted after
  producing no result; only the subsequent real-desktop run is counted.
- No production, test, API, manifest, binary, INI, media, or sibling repository
  change. No RDP or inaccessible backing-buffer history verification claimed.
