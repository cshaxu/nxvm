# T580 S38 — Observer-Free Complete Delay Boundary

## Method

S38 temporarily extended the existing MyPC110 real-firmware diagnostic with
one execute watchpoint at `F000:5405h`.  Unlike S36, the mode deliberately
installed neither its retirement observer nor its trace provider.  It used the
existing Core run loop and a fixed 25,000,000-instruction containment only.

The temporary mode built on both retained Ninja graphs:

- `build/.ninja/gcc-x64-release`
- `build/.ninja/gcc-x86-release`

## Result

The x64 run did not reach the `F000:5405h` watchpoint during its 180-second
wall-clock containment and was explicitly interrupted.  It emitted no normal
completion record because the existing diagnostic reports only after its run
loop returns.  No x86 repetition was run: a second unbounded-equivalent wait
would add no new semantic fact after the x64 no-hit result.

This establishes only that removing the diagnostic observer is insufficient to
make the selected BIOS's full port-61 delay a practical progress boundary.
It does not establish a BIOS non-return, a missing I/O receiver, a host-width
difference, POST completion, or guest boot.

## Disposition

The temporary diagnostic mode was removed before this record.  No production
source, public interface, firmware, media, artifact or App behavior changed.
The retained T580 source-gated boundary remains unchanged: future work needs
a separately bounded, evidence-backed receiver or an explicitly admitted L2
model, not a longer undifferentiated BIOS run.
