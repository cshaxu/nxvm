# M5 T500 S4 Test Tree and Route Evidence

`test/` is the sole test source root. Repository-only tests are organized by
their `src` owner (`core`, `vm`, and `vdm`); tests requiring owner-managed ROM
or disk inputs reside only in `test/integration/dos` or `test/integration/hdd`.

CTest assigns each registered test exactly one route: `unit` or `integration`.
The `unit` route has 287 tests and reads no external media. The `integration`
route has 15 tests and receives its media only as command arguments. The route
registration verifier reported 302 registrations with no duplicate target.

Timing-manifest runners now write generated observations under the build tree
(`generated/test-results`), never into tracked documentation.

Verification on 2026-08-28:

- `cmake --build --preset unit-tests-gcc`: passed; 287/287 unit tests.
- `verify-t344-unit-registration`: passed; 302 registered tests.
- `verify-unit-separation`: passed; 287 unit, 15 integration.
