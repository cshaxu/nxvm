# T568 S4 — PC110 F65535 Register And VRAM Foundation

## Delivered boundary

S4 adds one standalone `x86-nxvm/chips/f65535` owner and one MyPC110 wiring
adapter.  The chip owns exactly the sourced state admitted by S3:

- 512 KiB of resettable logical VRAM;
- the F65535 extension index/data ports at `3D6h` and `3D7h`;
- seven-bit extension indexes; and
- the PC110-observed, read-only `XR00 = C1h` identity.

MyPC110 maps the controller's normal CPU aperture at `A0000h--BFFFFh` and
the two extension ports through Core's existing route registration.  Core
continues to own dispatch.  The App does not interpret controller registers,
maintain a second framebuffer, or select host presentation.

## Evidence grade and deliberate exclusions

The C&T F65535 Revision 2.0 manual directly establishes the extension port
pair, seven-bit index and standard 512 KiB display-memory minimum; the PC110
live dump establishes the selected `C1h` identity.  These are L3 facts.

This implementation intentionally does **not** claim a working VGA engine,
scanout, palette, panel timing, DSTN/TFT compensation, power sequence, BIOS
mode transition or guest boot.  The remaining controller register defaults
are not inferred from the manual's capability tables.  `x86-nxvm/chips/video`
and Board-Base VADP remain PC/AT owners and are not used as an F65535
personality.

## Direct proof

The chip contract test verifies exact VRAM bounds, reset clearing, port
ownership, index masking, `XR00` immutability and ordinary indexed-register
storage.  The MyPC110 route test verifies that Core owns the routes, the
aperture reaches controller VRAM, and adjacent ordinary memory remains outside
the controller route.

Both cached GCC Ninja configurations passed the direct proof:

| Configuration | Result |
| --- | --- |
| x64 | `x86-nxvm.chips/f65535/f65535_contract` plus five `mypc110.unit/*`: 6/6 |
| x86 | `x86-nxvm.chips/f65535/f65535_contract` plus five `mypc110.unit/*`: 6/6 |
| x64/x86 | `x86-nxvm-verify`: manifest and dependency/platform boundary gates pass |

The wider x64 X86-NXVM label ran 233 tests: 232 passed.  The sole failure was
the pre-existing `integration.session-ini-status-smoke`, whose asset INI
composition returns `OPEN-FAILED:1` before F65535 is involved.  It is recorded
as an integration-fixture baseline defect, not treated as an S4 pass or hidden
by changing the test.

## Next receiver

The next S must first audit the smallest truthful standard-VGA state and
capture contract needed to turn F65535 VRAM/register state into the existing
single copied-frame publication path.  Any external emulator behavior used
there is L2, and any selected panel value requires either selected-ROM proof
or an explicitly labelled model.  No Lib/Emulator change is admitted by S4.
