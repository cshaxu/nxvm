# T576 S3: PC110 INT 10 video boundary

## Scope

This record closes the bounded control-transfer source gate after T576 S2.
It uses the admitted external TFT firmware from the external PC110 asset bundle
only at runtime.  No firmware byte, third-party source, IVT/BDA mutation,
BIOS interception, QEMU patch, or Core compatibility policy is introduced.

## Corrected App-owned mapping

The 256 KiB TFT image contains distinct 64 KiB banks.  The immutable image at
offset zero begins `55 AA 00 EB`; it is not the C-segment video firmware.  Its
third bank begins `55 AA 47 EB` and contains the `VIDEO` / `IBM VGA Compatible
BIOS` identification.  MyPC110 now owns the existing `C0000-CBFFF` replacement
route and reads that bank while its observed EC/ED shadow is invisible.  The
existing writable shadow remains the only route after the guest unlocks and
fills it; reset discards that shadow.  This is a source-map correction inside
the existing App firmware provider, not a generic Core alias.

## Ordered dual-width result

The existing real-firmware diagnostic was extended with an optional bounded
`--int10` checkpoint.  It waits for the ordinary reset boundary, then uses the
existing execute watchpoint to observe the next real `CD 10` instruction.  On
both x64 and x86 it reports:

```
point=f000:00004ca9 linear=000f4ca9 bytes=cd 10
vector=00000000
```

The execute watchpoint stops after ordinary INT dispatch changes execution to
the all-zero vector.  Therefore neither a protected far-control transfer nor a
stack receiver is the first remaining failure.  Core has performed the traced
INT operation; the guest video interrupt vector was never initialized.

## Source disposition

PC110-QEMU is L2 research only.  Its README explains that QEMU supplies its
own standard VGA BIOS and that its completion path explicitly injects a real
`INT 10h AX=0003` through that different BIOS.  That is evidence that video
initialization is a separate platform concern, not a transferable Core rule.
It is not an implementation input here.

The admitted PC110 asset bundle contains the TFT system image, fonts and
controller inputs, but no separately qualified F65535/C&T video BIOS or
source-defined initialization contract.  Implementing a synthetic IVT entry,
borrowing an IBM PC/AT video ROM, or installing QEMU's injection would all
fabricate the missing owner behavior.  S3 therefore stops at this finite,
reproducible boundary.

## Verification and limits

- MyPC110's seven unit/diagnostic registrations pass on both persistent Ninja
  widths after the map correction.
- The optional INT checkpoint confirms the same control-transfer fact on both
  widths.
- The complete repository-only unit suite passes **661/661** on x64 and x86
  through the persistent Ninja/ccache graphs.

This is not a POST, Setup, DOS boot, display, or release qualification.  The
next receiver must first admit a lawful PC110 video-firmware or board
initialization source, with an explicit App-owned contract.
