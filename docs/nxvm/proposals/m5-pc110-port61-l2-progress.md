# M5 T579 — PC110 port-61 L2 progress

## Decision

The admitted TFT firmware makes an ordinary 8042 reset, then polls port 61h
bit 4 at `F000:EA86--F000:EA90` before it has programmed PIT channel 1.  The
previous PC110 plan selected the directly wired PIT1 output, so that status
bit cannot provide the observed edge at this point.

MyPC110 will instead select the existing Board-AT elapsed-tick toggle source.
It remains on the Core's sole guest tick axis and changes only the existing
port-61 read result.  This is an App-selected L2 execution-compatibility model,
not a claim about PC110 oscillator frequency or physical refresh circuitry.

## Scope

- Change only the MyPC110 board-plan selection and its App tests/diagnostic.
- Pick a nonzero finite period proven to let the real image leave the loop.
- Preserve zero parity-memory bytes and existing speaker/latch behavior.
- Record external PC110-EMU's toggled-read behavior as L2 research only.

## Excluded

- No Lib, Emulator, X86 Core, X86-NXVM or ABI change.
- No second clock, polling thread, BIOS interception, guest-address route,
  borrowed PC/AT device or protected firmware input.
- No POST, video, Setup, DOS boot, runnable artifact or qualification claim.

## Evidence and completion

The diagnostic must run the actual external TFT BIOS through its actual reset
and show a post-reset retirement outside `F000:EA86--F000:EA90` on x64 and
x86.  App tests prove the selected plan and its port-61 state source.  If the
firmware does not advance under the bounded experiment, retain the finding and
stop rather than broadening the model.

## S1 result

The selected three-tick source passes the new dual-width real-firmware
progression diagnostic.  Both runs enter the real port-61 loop and later
retire at `F000:DD05`, with no delivered exception.  This resolves only the
unprogrammed-PIT1 deadlock.  S2 retains the next live boundary as diagnosis;
it does not presume that the BIOS now completes POST.

## S2 result

The first path beyond the port-61 loop reaches the BIOS's `3BCh--3BEh` status
probe.  With unowned ports reading zero, the BIOS repeatedly writes `21h` to
`3BEh` and waits for status bit 6 at `3BDh`.  A small App-local LPT placeholder
now retains that control byte and supplies `90h` normally or `D0h` during the
admitted probe.  This is an independently implemented L2 behavior based on
PC110-EMU research; it is neither a physical-printer claim nor a shared board
model.  Both widths retire past the probe to the next finite BIOS work at
`F000:55B2`.
