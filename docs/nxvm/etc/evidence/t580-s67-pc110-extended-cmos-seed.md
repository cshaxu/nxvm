# M5 T580 S67 — PC110 extended CMOS-seed receiver

## Source and asset boundary

S61 correctly rejected synthesis of an extended seed from incomplete current
research files.  This S instead recovered the cited, historical live capture
from the locally retained Open-Source-PC110 commit
`b0a5a3bcf88079dd53c13eb06136fcac04c95be6` (`Discovery/Live-Dump/raw/ports_indexed.json`).
That commit is research evidence only; no external source code is imported.

The exact 128 bytes of the `CMOS_RTC_70_71` capture are materialized unchanged
only in the owner-managed BYOB archive:

```text
nxvm-assets/profiles-nxvm/ibm-pc110-tft/cmos/pc110-tft-live-2026-07-02.cmos
SHA-256 87275449615d5027094e7e7491aa88a8fdba4a876dede0eaada75ecf08940509
```

The payload is a representative live-system configuration capture (L2), not a
claim about a universal power-on state.  It remains outside the repository.
The original DSTN variant deliberately receives no TFT seed.

## Receiver

The existing single Machine seed route now accepts only a 64-byte classic or
128-byte extended image.  It preserves that input length, copies only
configuration addresses `0Eh` through the final supplied byte into Board RTC
defaults, and rejects the 128-byte form when the selected Board does not
declare extended registers.  RTC time/calendar/status registers remain Core
owned; this does not add a second RTC image, a BIOS hook, or a PC110-specific
runtime path.

The Board default capacity is the real maximum writable configuration range:
`128 - 0Eh = 114` bytes.  The generic PC110 construction finisher is now used
by MyPC110 as intended, so the embedded CMOS asset reaches the sole Machine
receiver.  MyPC110's separate 1 MiB Kanji ROM remains App-owned and is not
mistaken for the generic 8 KiB text-generator asset.

## Direct proof

Focused dual-width unit tests cover both accepted seed sizes, malformed
lengths, and actual PC110 Board port delivery at `0Eh`, `3Fh`, `40h`, and
`7Fh`.  The unchanged TFT BIOS replay with the S66 scratch-only Easy-Setup
oracle reaches these real-firmware checkpoints:

```text
PC110:SETUP:entry=1 target=5000:00000000 bytes=232185 matched=1
PC110:SETUP:frame status=0 produced=1 graphics=1
PC110:SETUP:image width=640 height=480 stride=640
PC110:SETUP:enter-delivered count=1
PC110:SETUP:enter-delivered count=2
PC110:SETUP:processor-reset=1 pc=fffffff0 resets=5
```

The x64 run reached the handoff at 109,719,228 retired instructions and the
x86 run at 109,672,053.  Both published the first Setup frame, consumed both
ordinary Enter make/break pairs, and reached the fifth firmware reset: x64 at
144,096,630 instructions and x86 at 144,068,624.  Both retain the established
four cold-prefix exceptions and report no Core fault.

## Disposition

S67 closes the source-backed extended-seed receiver only.  Direct Board tests
prove the complete captured address range reaches the ordinary Board route;
the unchanged selected firmware replay proves that installing the capture does
not disturb its established handoff.  It does not prove which captured bytes
that firmware consumes, F1 branch selection, durable Setup persistence,
authentic SunDisk/CF boot, PCIC behavior, panel identity, desktop/RDP
behavior, DSTN behavior, or a publishable PC110 product.

## Receiving regression boundary

The complete x64 unit suite passes (669/669), as does x86 (669/669).  The
selected x64 product integration groups pass, and the x86 XT and AT groups
pass.  The existing x86 DeskPro floppy boot matrix did not reach its terminal
within its established 300,000 ms guest budget on two ordinary runs; a
diagnostic run with a 330,000 ms guest budget also did not reach a terminal.
Its FDC progress and instruction location varied between runs.  This is the
already recorded Model40 x86 external-throughput qualification concern, not a
new PC110 seed assertion: the Model40 has no selected CMOS seed and S67 does
not change its profile selection.  It remains a failed receiving integration,
not a passing result or a reason to alter its budget in this S.

## Receiving artifacts

The eight current receiving-PC artifacts were rebuilt through the stable Ninja
graphs and verified as PE x64/x86 as named. Their final SHA-256 values are:

| Artifact | SHA-256 |
| --- | --- |
| `my5160_0_5_0580_x64.exe` | `A1528A91F1769A279D38EDD8FDF0FE9BDAA9BF5AF0CBCE5AD622BA0A3AA6031F` |
| `my5160_0_5_0580_x86.exe` | `006E56435964E3D69B855E16098387312068CF559839370A05A410573585CD32` |
| `my5170_0_5_0580_x64.exe` | `A31C5C5AE5D6DB65AC8CFF0A61D3CAFF79D6C533E6D32B9CD92B6F1AA5F7A19A` |
| `my5170_0_5_0580_x86.exe` | `6A404EDDDADFD8C89BA12CAF6294FABC25D47EABA59848AE494CCAD213669DFA` |
| `mydeskpro386_0_5_0580_x64.exe` | `22EDA7C0A27B4D534AA8E5E2A85B14DC750EDE8E5062F8183105D96220F8328A` |
| `mydeskpro386_0_5_0580_x86.exe` | `0F9427A2C67243151149FAA836215A1AC8A6243E72D4482CEFB58E4D9ABC945B` |
| `nxvm_0_5_0580_x64.exe` | `E5F3FA3DE0B811E29CB3BF7C82A58740B826843894F7940363822F9C37184460` |
| `nxvm_0_5_0580_x86.exe` | `0ED9340049D63E2788E1013B6BD4E6529D9BF05635E20FDF76A9E1B5A4C00D8D` |

The disposable Easy-Setup oracle, extracted bytes and task-owned output were
removed through `tools/shared/Clear-Build.ps1` after hash review.
