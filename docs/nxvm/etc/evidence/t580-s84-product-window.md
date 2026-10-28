# T580 S84 — deployed PC110 Window observation

The current TFT x64 deployed executable was copied to a task-owned directory
with a temporary 20 MiB, Window, console-control-enabled, no-media INI. The
existing background product probe sent `start`, observed the actual product's
Running Window, waited 240 seconds and then captured it and paused the machine.
The helper process exited successfully. This did not use or alter the owner's
INI or media. The probe reported `remote=1`, client `9600x6000`, and outer
`9616x6039`; its log SHA-256 is
`4e1f62b82ec03a38d02a50cee3638f251582697bbdc0fd78f5d39168433d29b4`.

The returned PrintWindow capture's SHA-256 is
`295ba64fff74e5d11beee2def9a33c3476408991419383578c0da270d6f28adf`.
Its 960x600 reduction sampled black at all 64,000 grid points. Given the
enormous off-screen window, this is not sufficient evidence of guest pixel
colour or proof that the firmware reached ERROR/Easy Setup; no colour
qualification is claimed. The configuration had no disk, so this is also
not evidence of a DOS or Personaware product boot.

Two independent observations identify the initial-scale failure mechanism:

1. A temporary print in the existing actual PC110 construction test showed
   its initially published text frame is one column, one row, one scanline;
   the temporary source instrumentation was removed immediately afterward.
2. A hidden native Win32 window using the same style and resize operation in
   the same session returned a 120x1 client after requesting 8x1. The current
   Window code records actual width / source width as the scale: 120/8 = 15.

The resulting 15x amplification matches the observed 9600x6000 client for a
640x400 source. The exact subsequent source-frame dimensions and the timing
of that scale selection have not yet been instrumented in the deployed EXE,
so this matching mechanism is a strong inference, not a complete execution
trace. The inferred issue is generic to native minimum window sizing, not a
PC110-only UI policy. No Shared source or public interface has been changed.

The PC110 initial-frame observation is consistent with the current F65535
source: reset zeroes the register file and snapshot derives columns as
CRTC[1]+1, scanlines as vertical-display-end+1, and character height as
(CRTC[9]&31)+1. These yield 1x1 at one scanline without proving that the
physical device would present a frame in reset. Investigating the chip's
actual display-enable and timing readiness is a separate hardware question;
arbitrarily rejecting small legal modes or assigning invented reset values
would not establish the correct behavior. The existing host resize test
already checks fit-down, user resizing, aspect and frame transitions, but its
native mock does not demonstrate the observed forced minimum for 8x1.

The C&T 65535 Register Reference revision 2.0, printed page 48, identifies
SR01 bit 5 as Screen Off: set disables video output. The current F65535
snapshot paths do not inspect that bit. This is an independently identifiable
display-behavior gap, not proof that SR01 is set in the observed incident:
the modeled register file is zero at reset, and no deployed-firmware trace of
this bit has yet been gathered. Any follow-up must establish how blank output
is represented while preserving geometry and normal output when re-enabled;
it must not substitute an invented frame-readiness test for the host scale fix.
The reset description on printed page 47 describes SR00 reset-control bits;
it does not by itself prove the initial value of every sequencer register.

For follow-up design, the existing Machine conversion resolves indexed
graphics through `palette_rgb`, and copies the text palette and cursor state
to both Window and Console frames. Thus an F65535-only change that merely
zeros the palette would hide the graphic pixels but would also need to handle
the text cursor, and cannot automatically guarantee blank output when a host
Console rejects palette changes. Screen Off must be tested at the chip and
actual receiver boundaries, with guest memory preserved and output restored
after clearing SR01 bit 5. No change to this behavior is made by S84.

A candidate minimal Shared fix is to avoid treating a native-enforced
minimum as a user-selected enlargement during the *initial automatic* resize:
record the smaller of actual client width and source width for the initial
scale, while preserving an actual smaller width when the initial image is
fitted to the monitor. Subsequent actual user resize continues to own the
scale. Before implementation, review the existing window resize tests and
add a regression for initial 8x1 forced to 120x1 followed by 640x400, plus
an initially fitted-down frame and a genuine user resize. This candidate
requires approval for Shared Lib changes and must be tested with the actual
product and both architectures; no code fix is claimed in this record.
