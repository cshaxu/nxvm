# T580 S28: Post-delay execution boundary

## Scope

This was a diagnostic-only investigation of the admitted TFT BIOS after the
existing post-C000 `F000:EA84` return.  It changed no production route, ROM,
external media, Shared/Core contract, or product artifact.

## Reproduction

The existing real-firmware diagnostic first established that port `61h` is not
stuck: its Core tick rises between reads and bit 4 alternates.  The x64 trace
observed `23h` and `33h` values at `F000:EA86`; its first inner counter was
`CX=040Bh`.

The immediate `EA84h` successor is not a hardware receiver.  The exact BIOS
code returns through `EA55h`: it decrements `BL` from `70h`, repeatedly calls
the existing `EA80h` port-61 delay with `CX=040Bh`, and then reaches
`F000:3F6Ch`.  The temporary copied retirement ring captured this path on both
host widths.  The focused diagnostic passed in 34.18 seconds on x64 and 41.63
seconds on x86.

After that point the BIOS executes already-owned routes, including EC Block-B
`35EAh/35EBh`, VL82C420 SCAMP `74h/76h`, and CRT status `3DAh`.  The source
tree's `Open-Source-PC110/Discovery/Chipset/readme.md` additionally identifies
`8Bh` as a repurposed passive boot/IPL latch, but does not supply a reset value
for the selected BIOS branch.  A longer 100,000,000-retirement containment run
did not reach that branch before its bounded host-time window; it was stopped
without treating the ceiling as guest progress.

## Disposition

S28 does not select a new receiver.  In particular, it does not invent an
`8Bh` reset value, attach an FDC/MCU model, or change port-61 timing.  The
temporary trace target, test registration, enlarged ceiling, runner script and
captured build output were removed after the two-width observation.

This is not POST, Setup, storage, panel, guest-boot, or release qualification.
