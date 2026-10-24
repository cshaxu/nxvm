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
