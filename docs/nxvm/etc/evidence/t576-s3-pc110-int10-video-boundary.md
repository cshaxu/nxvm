# T576 S3: PC110 INT 10 video boundary (superseded map conclusion)

> **Corrected by T576 S4.** This record preserves the bounded INT observation,
> but not its initial C000 bank conclusion. Stronger live-map evidence in the
> already-admitted Open-Source-PC110 corpus identifies bank 2 at E000 and C000
> as the reset-visible shadow destination. S4 restores that topology and repeats
> the same bounded checkpoint.

## Scope

This record closes the bounded control-transfer source gate after T576 S2.
It uses the admitted external TFT firmware from the external PC110 asset bundle
only at runtime.  No firmware byte, third-party source, IVT/BDA mutation,
BIOS interception, QEMU patch, or Core compatibility policy is introduced.

## Initial App-owned mapping hypothesis

The 256 KiB TFT image contains distinct 64 KiB banks. The immutable image at
offset zero begins `55 AA 00 EB`; bank 2 begins `55 AA 47 EB` and contains the
`VIDEO` / `IBM VGA Compatible BIOS` identification. S3 initially treated the
second fact as a C000 route correction. That conclusion is not retained: the
subsequent source review proves that bank 2 belongs at E000 and C000 is its
runtime shadow destination.

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
  widths for the then-current interim map.
- The optional INT checkpoint confirms the same control-transfer fact on both
  widths.
- The complete repository-only unit suite passes **661/661** on x64 and x86
  through the persistent Ninja/ccache graphs.

## S4 corrected-map replay

S4 preloads C000 from flash bank 2 at reset, exposes E000 directly from the
same raw bank and preserves F000 as bank 3.  Both widths prove that C000 and
E000 expose the bank-2 `55 AA 47 EB` header after reset; the existing EC/ED
write/relock route remains the only later mutable-shadow transition.

The repeated bounded `--int10` checkpoint reaches the same `F000:4CA9` `CD 10`
with a zero vector on both widths.  Its retirement counters are zero for both
`C0000–CFFFF` and `E0000–EFFFF`.  Thus the corrected video-map owner is
verified, but it is not on the execution path before this terminal interrupt.
The remaining missing video/board initialization source is unchanged.

This is not a POST, Setup, DOS boot, display, or release qualification.
