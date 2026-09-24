# M4 Lib Audio Refresh

## Outcome

Repair the MyNes PCM transport independently. Lib exposes a generic, bounded
signed-16-bit PCM stream with a bounded producer FIFO and output worker. Its
Win32 implementation uses native completion notifications to reclaim output
slots. Core retains NES APU production and nonblocking sample supply. The
result must serve MyNes now and remain a suitable host-audio component for
future x86 PC products.

## Boundaries

SoftPC is read-only and is neither copied from nor modified by this work. Lib
does not know NES timing, APU mixing, cartridges, emulator pacing, or x86 device
semantics. Core does not wait on host completion from its guest execution loop:
it supplies the Lib FIFO, while Lib's generic output worker publishes stable
1024-frame batches as native slots become writable. The task does not retune
APU mixing unless
transport evidence requires it. Human audible acceptance remains an owner test
after concrete executables are delivered.

## Acceptance

Lib provides `query` and completion-aware `wait_writable` semantics, a bounded
generic PCM FIFO, and an output worker; it does not embed a machine-specific
mixer or timing policy. Its Win32 callback performs only event signaling, while
the worker reclaims completed wave headers. Core supplies at most 512 frames at
a time and never waits for a host slot. Text-only controller input remains held
for one display-scale interval so guest polling can observe it. Lib contract
tests, Core regressions, the complete repository unit suite, x64/x86 builds and
rebuilt artifacts pass. The owner receives the commit and executable pair for
an RDP audible retest.
