# T7 S49 Selected-Set Progress Diagnostic

## Scope And Asset Boundary

This receiver reran the repository's existing selected-set startup probe after
the completed S45--S48 local Sound CPU batches.  The wrapper verified the
external owner archive against the recorded inventory, staged only the
required images below `O:\assets\raidenii\myraidenii-diagnostics`, and removed
its generated child directory in `finally`.  No archive member, guest byte,
guest address, instruction trace, or third-party source was retained here.

## Bounded Result

The x64 and x86 probes both completed their fixed 2,000,000-instruction budget
with a zero wrapper exit status.  Each reports the budget terminal rather than
a decoder, map, control, COP, or CPU fault.  Both runs produced active
320-by-240 frames with multiple sampled frame transitions, periodic video
events, Sound-command acknowledgements and interrupts, and device traffic.

The input mode was intentionally neutral.  No coin/start transition was
scheduled or observed, so this result establishes only that the program can
continue through this bounded boot/attract interval.  It does not establish a
title-screen, coin/start, gameplay, audio-output, desktop, RDP, or release
qualification.

## Receiving Boundary

The previous finite `DECODER` terminal is no longer observed under this
bounded run.  Therefore the next receiver is not another speculative CPU
opcode patch.  It must first exercise the existing controlled coin/start input
mode and compare its two-width finite result; any later repair must still be a
complete owner-valid family selected from that observation.

## Verification

Both existing Ninja trees rebuilt the existing probe target, then ran the
existing external wrapper once.  The wrapper returned success on both widths,
and the diagnostics root contained no generated staging child afterward.
No production, test, build, manifest, public interface, release artifact, ROM
or media file changed in this S.
