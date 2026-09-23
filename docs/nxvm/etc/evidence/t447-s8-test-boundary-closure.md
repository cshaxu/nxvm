# T447 S8 cross-owner test boundary closure

## Classification

The S7 private-layout sweep found 98 direct test includes: 93 session, three
default-PC/AT profile, one FDD, and one HDD.  Their complete disposition is:

| Class | Count | Disposition |
| --- | ---: | --- |
| VM composition mechanism tests | 96 | Reside in `tests/machine`: 92 session, two PC/AT profile, one FDD, and one HDD private include. They link `vm-composition` and use private state only to set up or verify the composing owner's lifecycle, devices, firmware, or Core plan. Eighteen were moved from `tests/products` or `tests/platform`; target names and assertions are unchanged. |
| Default-PC/AT descriptor owner test | 1 | Remains in `tests/firmware`; it verifies profile-owned descriptor data. |
| CPU adapter fixture | 1 | Remains in `tests/adapters/support`; it owns its test-only session storage and exposes only copied CPU captures to its consumer. |
| Cross-owner product/platform test | 0 | The new static guard rejects direct VM private-layout includes in those directories. |

The 57 T345 residual records are separately classified mixed/inherited or
embedded production compilation rows.  They are not test-private-header
escapes and remain exactly reconciled by T345; S8 neither deletes nor widens
them.

## Result

Product and platform test locations now contain behavioral tests only; VM
composition white-box tests are named and located as owner tests.  No
production API, compatibility fixture, or forwarding path was added.  The
session-layout gate now fails if a product or platform test includes a VM
session, profile, Model-40, FDD, or HDD private layout.

## Minimalism accounting

The 18 test-file moves preserve their contents.  Excluding documentation, the
S8 diff is 32 added and 20 removed lines: CMake source-path corrections and
the ten-line cross-owner include guard.  No production source line is added.

## Verification

- The strengthened session-layout gate passes and emits
  `M5 T447 S8 cross-owner test private-layout closure: OK`.
- The relocated session, platform-run-handle, and Model-40 focused regressions
  pass without target or assertion changes.
- T345 retains its exact 237-row ownership reconciliation and 57 residual
  production rows; no new direct production-test compilation is introduced.
- Full `cmake --build build/mingw-gcc-x64 -j 4` and all 76 specialized gates
  pass.
- Full `ctest --test-dir build/mingw-gcc-x64 --output-on-failure` completes:
  292 `Test Passed`, 0 `Test Failed`.
- Current `vm-0-5-0447` artifact SHA-256 remains
  `CAA87452E18E90C5E72114D8C8B6C04FD830920BB45DA160CA6562787AA3B920`.
