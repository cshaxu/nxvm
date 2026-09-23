# M3 T18: Console Presentation Qualification

T18 consumes the final functional M3 candidate. It proves that the same
self-authored NROM fixture used for Window qualification is playable through a
real Win32 Console without moving NES conversion or policy into Common or Lib.

## S1 Native Console Receiver

`mynes.integration.native-console-smoke` owns a hidden test Console and creates
the production Core driver with `text_output` enabled. It uses the production
App command provider and Common Session/UI wiring, then runs the ordinary
`rom -> machine -> Core text frame -> Common route -> Lib Console` path.

The test opens `CONOUT$` only after the raw KVM binding selects its alternate
screen buffer. It reads the active 80x25 native cells with
`ReadConsoleOutputW`, injects a real K down event with `WriteConsoleInputW`,
and requires a changed presented text image before sending K up. This protects
the controller's serial poll from a synthetic down/up race and proves the
standard K=A mapping through the actual Console input reader.

The same receiver sends native F5 and F12 keys, requires each corresponding
paused transition, returns to running through the cooked monitor command path,
then sends `quit` and verifies session, UI, machine and driver teardown. It
does not alter `src/lib`, `src/common`, `test/lib` or `test/common`.

## Evidence And Transfer

The targeted native receiver passes on x64 and x86. Fresh full JUnit suites
report 103 tests, zero failures and zero disabled tests on both architectures.
T18 accepts the Console portion of R04/R11/R12 and the Console presentation,
input and lifecycle members assigned by the M3 delivery plan. It transfers the
finite all-ledger reconciliation and milestone closure to the remaining final
candidate; no new emulator feature is implied by this acceptance.
