# M6 T43 S10 Registration And Byte Boolean

## Scope And Review

Owner accepted S9 and admitted S10 on 2026-09-25. Reference f91686808.
Shared owns registration and Types; NXVM/MyNES own their receiving artifacts
and tests. No sibling, INI, title policy, firmware or media was modified.

- `test/lib/register.cmake` moved unchanged to `test/register.cmake` (100%
  rename). Three CMake entry points now include that sole helper. Each still
  selects its own sources, dependencies and tests; no combined runner added.
- Transfer all required corpus directories **and `test/register.cmake`** from
  the same commit. All three test READMEs state this prerequisite. The helper
  SHA-256 is `28DA71E009EFEC16E2A27E23CA6E7F9856552409FC8097238930A93A2943BF34`.
  Six manifests retain exact root-local inventories; the root helper is a
  separate versioned Shared build input, not an unlisted Lib file.
- `lib_bool` is now `lib_u8`, retaining canonical false=0 and true=1. A C11
  type assertion prevents regression to a signed/wider alias. This is a copied
  value and callback ABI change, not a stable binary ABI. Rebuild every receiver.
- Review covered Shared bool return expressions, native input/mouse/render and
  event boundaries, atomics, Common driver callbacks and both App adapters.
  Native SDK results remain native-width; success checks normalize before the
  neutral bool boundary. Atomics and statuses remain their existing widths.
  No production adaptation or cast was required beyond the single typedef.
- Strict compilation exposed two Lib input callbacks and two NXVM composition
  test substitutes returning `lib_i32`. They now declare the actual `lib_bool`
  contract, without unsafe casts. Existing frame-copy/layout checks were updated:
  text 16084, Window text 24276, Console text 17108, Window frame 984080,
  Common frame 985108 bytes. Both architectures verify these values.
- MyNES snapshot bools already encode/decode explicit one-byte 0/1 values;
  debug transport likewise writes explicit byte offsets. Their wire formats
  are unchanged. Original xasm instruction tables are untouched.

## Verification And Failed Attempts

All six manifest checks pass at revision `shared-m6-t43-s10-p1`.
Independent standalone suites pass Lib 49/49, Common 18/18, x86 10/10.
Full MyNES configured suites pass x64 130/130 (146.28 seconds), x86 130/130
(136.26 seconds), including snapshot restore and native presentation tests.
Full NXVM units pass 335/335 on both widths (18.96/27.15 seconds).

Initial compile failures were the four mistyped test callbacks above. Initial
runtime failures identified the remaining old Common frame-size assertion.
The first MyNES x64 run also exposed an existing test synchronization defect:
reset accepted from Paused can still report the old Paused state while reset
is pending; sleeping 5ms before debug is not completion proof. Twenty isolated
repeats passed, but the test was corrected to wait for its existing
RESET_COMPLETED sink event after both reset requests. No Common lifecycle or
product runtime workaround was introduced. Both complete suites then passed.

A standalone Common manifest run overlapped the assertion edit and failed;
after final manifest generation its entire suite passed. An NXVM native modal
test failed while other build trees also used the desktop. Final receiving
verification runs one NXVM architecture at a time without another desktop
suite; no timeout/assertion was weakened and no test was excluded.

## Simplicity And Delivery

Actual-diff review retains one registration implementation and one Types owner.
`git diff --numstat f91686808 -- src test`, excluding README/manifests and
recognizing the unchanged rename: +41/-22, net +19. Shared is +18/-15 (net +3),
NXVM +2/-2, MyNES +21/-5. The positive delta is a type assertion and explicit
test completion synchronization, not new production machinery.

Both product documentation gates and `git diff --check` pass. Build trees/logs
are retained for active T43 verification. T43 stays open; S10 delivery awaits
owner manual acceptance. External-ROM integration is not a claimed T closure.

Shared P1 `75099c178` and NXVM P2 `c5b17f671` are pushed. The latter records
the eight rebuilt 0535 EXEs and their hashes in NXVM's receiving evidence.
This MyNES P3 contains its reviewed lifecycle-test synchronization, governance
and the rebuilt 0043 pair against Shared P1. Production inputs match that commit;
MyNES production sources are unchanged by S10. PE architecture and absence of
compiler debug sections pass. Existing Running/Paused title policy is retained.

| Artifact | Bytes | PE | SHA-256 |
| --- | ---: | --- | --- |
| `assets/mynes/mynes_0_0_0043_x64.exe` | 245774 | 8664 | `A78E5305AC3970247201AD0F4B62BDCE8A44B12251C0CBCF77E3F8F5A0E2CB03` |
| `assets/mynes/mynes_0_0_0043_x86.exe` | 237070 | 014C | `837E0BB0FF554EB6F6560AB55AA8AC5A819253F4AD4EB1B93D2C55FAB5F2F2C2` |
