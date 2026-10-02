# T540 S73: board-owned refresh PIT initialization

Baseline: S72 P2 `58f149940`. This consumes the complete reset/configuration
port-write class in the pre-relocation ledger: one helper, three writes and
three callers (planar parity configuration, D4 configuration and after-PIT
cold reset). It does not consume firmware/attachment or physical relocation.

## Source proof and refined decision

S72 correctly withheld direct chip substitution until Core scratch effects
were inspected. The old port helper writes ioDWord and sets access_bytes to
one; its final count high byte is zero. The PIT route dispatch then calls
x86_pit_write_register on the board's own device. Addresses 43h/41h map to
selectors 3/1; the exact control and reload bytes remain 74h/12h/00h.

Board configuration is admitted only while INITIALIZED, not frozen and not
inside a firmware operation. Public bus/debug access cannot read its scratch
there. Firmware configuring context cannot perform port I/O. Execution needs
STOPPED/PAUSED, reached through cold reset; configuration cannot reopen after
the execution provider is frozen, including failed firmware reset.

Cold reset zeroes all Core port data before the board callback. The old final
ioDWord remains zero, so direct chip initialization leaves the same observable
value. Only access_bytes differs until the next bus operation; every CPU,
debug, public bus and firmware port access explicitly sets its width before
dispatch. The scratch layout is not a public observation. No trace was emitted
by the old raw helper. Processor-only reset does not call this board routine.
No provider can override the installed PIT writer; Core rejects route conflict.

Therefore no new Core construction-I/O API is necessary. Use the existing
PIT public contract for board initialization, retaining one chip algorithm;
guest/debug/firmware bus I/O stays with Core's existing dispatcher. A synthetic
private fixture that changes scratch while INITIALIZED is not an exposed
guest behavior. Unit proof verifies count 18 before/after reset, public bus
reprogramming and unchanged zero cold-reset scratch for both planar and D4.

## Verification and boundaries

Expand the existing same-owner port-assembly matrix rather than adding a new
test framework or external unit input. Full units and gates run on both widths;
independent neutral proofs, eight optimized stripped 0540 EXEs and one actual
boot per row precede P1. Shared/MyNES/INI/master inputs are unchanged.
Legacy raw port helpers used by direct private tests retain their distinct
same-owner fixture purpose until the measured test-classification receiver.
The complete receiving results below satisfy this intake.

## Implementation checkpoint

All three writes now call x86_pit_write_register with valid constructed device
and constant selectors; the existing API can fail only for NULL device or an
invalid selector, neither admitted by these callers. As with the old raw void
helper, no synthetic error branch or new lifecycle operation is introduced.
Board/plan source now contains no executor_port access. The existing authority
gate checks both memory and port private state at this composition boundary.

The existing planar/D4 failure matrix also verifies its successful retry:
one chip input cycle commits reload 18 during construction; freezing and cold
reset leave Core scratch zero and reload 18; public bus writes reprogram the
counter, then another cold reset restores the same result. Existing focused
port-assembly markers pass on x64. No external unit configuration is used.

Git numstat for the three tracked source/test/gate paths is +29/-8, net +21:
production is -3, existing matrix coverage +24 and prevention gate net zero.
The positive test delta proves all three callers and the retained public bus
path; production gains no mechanism or state. Both full unit suites pass
470/470: x64 271.80 seconds and x86 78.21 seconds. Both specialized gates
pass, including 402 strict-compilation rows (377 retained, 25 declared deferred).
The original dual-width unit/gate and Release receiving jobs returned zero, using
build/s73-{build,unit,gates}-<width>.log and
build/s73-<profile>-<width>-{build,neutral,boot}.log. No completed job was
restarted after observation timeout.

All eight matching probes and independent Core executions pass; the latter
retain M5:T540:S69:NEUTRAL-LINK:OK and the actual-source sixteen-file proof
without PC board archives. This does not replace physical relocation.
Each boot row was run once: default x64/x86 returns zero at dos-prompt;
XT, AT and Model40 x64/x86 return zero at installer-running. These headless
INI/Core/firmware/media checkpoints do not claim native KVM/audio desktop
acceptance or indefinite absence of intermittent faults.

## Artifact identity and review boundary

Eight optimized Release 0540 EXEs were rebuilt. Post-build and objdump checks
confirm x64 pei-x86-64 and x86 pei-i386, with no .debug sections. SHA-256:

| Product | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | BF45BD5B0354A5DD76FD45EED05454B2009717583C68800188605A737E6DD9FE |
| default | x86 | 638CC3E1CE70B6B81E866A3804FD2B2A37A3EDEBC252C0924A8C21B77FA2C0EB |
| XT | x64 | 062C1FEB2A25043DAD37B7A215D5F0F19ED9029361A7142FB1ECB98B52F10F57 |
| XT | x86 | D7319B80AD4B2D20E869BF679F79759F3F2474F2EAA91C8A4DF1C76EA50851AA |
| AT | x64 | 5473B6906E41132ACD2152B10D8D061BB9BAB8A893CCFCBD7263C7CFABA426AB |
| AT | x86 | 5AC70A647761F76C27FD265AC5AF4D8EECCA435BA5367CD73F75AE8C271BD6FB |
| Model40 | x64 | 3F223E2B360F646D02DC55EB840CF11F3E1A6F7152E94A01F61DACE993565A76 |
| Model40 | x86 | 690B830DA726AFD5EA85426C6C5C5CC402B9E82CE006030D102E03700DBF4966 |

Executor review checks all three source/test/gate diffs, task documents and
eight artifacts against the sixteen-field packet. Source inspection confirms
the chip-write inputs cannot fail; the single chip algorithm, output callbacks
and guest bus dispatch remain unchanged. All production raw port read/write
calls outside port.c are gone; same-owner private fixtures retain their explicit
test-classification receiver. Shared, MyNES and owner INIs have no diff.
Documentation governance and diff whitespace checks pass. Firmware/attachment,
test-owner classification and physical relocation remain open under T540.
