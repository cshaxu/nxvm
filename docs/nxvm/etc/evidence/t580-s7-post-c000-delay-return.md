# T580 S7 — Post-C000 delay return

## Question

After S6 proved that port `61h` bit 4 changes during the real TFT BIOS's
post-C000 delay, determine whether the BIOS can actually complete that named
subroutine. This prevents treating a normal polling delay as a missing PC110
device.

## Method

`mypc110.diagnostic/real-firmware-c000-port61-return` reuses the normal Core
executor and the admitted external TFT BIOS. At the first `F000:EA86` entry
after the C000 return, it retains `ECX`; it then requires execution of the
delay routine's `F000:EA92` `ret` and records the first returned instruction.
The initial counter (`040Bh`) supplies the finite containment rationale; the
pass condition is the semantic return, not the instruction count.

The diagnostic adds no port intercept, firmware-address behavior, extra clock
or production device model.

## Result

Both host widths complete the same path:

```text
first delay ECX: 0000040b
after ret:       F000:EA84
```

The returned location is the caller's `pop ax; ret` sequence. The surrounding
real BIOS bytes show that it calls the `EA86h` delay routine with `CX=040Bh`;
therefore the sampled polling loop is a completed BIOS delay subroutine, not a
stalled port-61/PIT/Core/F65535 receiver.

## Disposition

No hardware repair is justified by this path. It does not qualify panel
output, Easy-Setup, storage, a guest OS, or the unavailable original-DSTN
variant. The next T580 receiver must be selected from a source-backed
qualification domain rather than extending this completed delay trace.

## Verification

- `mypc110.diagnostic/real-firmware-c000-port61-return` passed on x64 in
  35.79 seconds and on x86 in 40.14 seconds.
- The diagnostic rebuilt from the two persistent Ninja/ccache trees. The x86
  Ninja metadata emitted its pre-existing recovery warning but completed with
  a zero task result.
- The complete repository-only unit suite passed on both widths: 663/663 on
  x86 and 663/663 on x64. The first x64 `-j 8` run timed out three static
  CMake gates under host contention; each passed serially, and the complete
  `-j 4` rerun then passed without a timeout.
