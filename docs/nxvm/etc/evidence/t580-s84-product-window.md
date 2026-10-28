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
