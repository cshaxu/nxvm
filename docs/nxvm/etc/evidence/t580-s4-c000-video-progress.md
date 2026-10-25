# T580 S4 — TFT C000 Video-ROM Progress

## Scope

The real TFT BIOS selects the C&T video bank and transfers to `C000:0003`.
S4 corrects only the observed route and port contracts needed for that normal
execution path.  It does not claim POST, panel presentation, Easy-Setup or
guest boot.

## Repairs

1. MyPC110 keeps its writable C000 shadow as the ordinary data mapping.  Core
   now gives a provider an optional instruction-read source, so the PC110
   firmware map supplies the immutable selected C&T bank only for instruction
   fetch/prefetch and retains shadow data reads/writes for the BIOS's copied
   tables.  Before the repair, the selected C&T image began `55 AA 47 EB` but
   later C000 execution could consume overwritten shadow bytes.
2. F65535 supplies the finite VGA reads actually reached by the image:
   sequencer index (`3C4h`), DAC write index (`3C8h`), graphics index
   (`3CEh`), Input Status 0 (`3C2h = 00h`) and an alternating Input Status 1
   retrace bit (`3DAh`).  The read-only `3CCh` and `3DAh` writes seen in the
   image are accepted as no-ops.  These are L2 compatibility facts; they do
   not model panel timing.
3. MyPC110 declares the sequencer, graphics, CRTC and F65535 extension
   index/data pairs as two-byte latch banks.  This matches the Core port-bank
   contract for the BIOS's `OUT DX,AX` accesses.
4. The trace exposed two generic Core port defects: a bank lane received an
   unmasked shifted value, and a narrow output could retain upper bits from a
   prior wider latch.  Core now supplies one byte per lane and zero-extends
   every narrow latch write.  Both repairs are independent of PC110 and have
   Core regressions.

## Evidence And Limits

`mypc110-real-firmware-post-diagnostic --c000` runs the ordinary executor
until it crosses the actual former `C000:6000` port boundary, without a
firmware-address shortcut.  The x64 and x86 CTest runs report respectively:

```text
status=0 ... executed=10171897 ... c000-header=55aa47eb
exceptions=0 ... c000=0/1943

status=0 ... executed=10170105 ... c000-header=55aa47eb
exceptions=0 ... c000=0/151
```

The first C000 counter is pre-reset and the second is post-reset.  Thus both
host widths cross the two real BIOS instructions that previously exposed the
Core `OUT DX,AX` lane and narrow-latch defects, with no first Core fault.  The
evidence proves only that bounded progress point.

After the repair, both x64 and x86 reran the same external-firmware CTest
(`mypc110.diagnostic/real-firmware-c000`) successfully.  The current runs took
28.67 seconds and 39.05 seconds respectively.  The complete repository-only
unit suites also passed on both widths (663 selected tests per width, in the
stable Ninja trees).  During that qualification, Lib's Types-layout self-test
showed that an interrupted negative probe could leave an owned scratch fixture
behind; it now removes and recreates only that fixture root before testing.
That test-only cleanup has no production behavior or component-edge effect.

PC110-EMU is retained as L2 research for its VGA register readback behavior;
Open-Source-PC110 documents the observed `3CCh = 67h` miscellaneous-output
value.  No external source code or protected firmware bytes are imported.
