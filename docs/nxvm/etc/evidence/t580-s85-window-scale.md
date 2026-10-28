# T580 S85 — Window scale ownership

The shared Win32 Window preserves its existing rational scale separately from
the actual native client. Initial automatic fitting cannot turn a native
minimum into an enlargement. WM_SIZE updates geometry but does not select a
scale. A supported WM_SIZING operation selects a scale when its size/move
loop exits; the existing explicit maximize and natural-size paths continue to
select their result. Snapped-frame sizing uses the current snap region without
overwriting the ordinary scale. This does not claim to identify the origin of
every Windows resize.

The existing Window contract fixture now checks the 8x1/native 120x1 case,
ordinary WM_SIZE without a scale change, an explicit user resize, snap frame
changes and saved-scale preservation. Both x86 and x64 pass the focused test
and both Lib manifests. Existing fit-down, maximize, restore and failure-path
assertions remain. The final production code passed 759/759 registered tests
on each width excluding the native MIDI case; the last added snap-scale test
assertion was separately rerun successfully on both widths. Both build trees
subsequently reported no work.

The native MIDI case failed in both earlier full runs and separate reruns; a
direct WinMM call returned error 1. After the owner connected RDP, the native
MIDI case passed independently on each width. These are separate runs, not a
single 760-test pass. All 16 receiving EXEs were rebuilt, and their PE machine
fields matched the named architecture. PC110 TFT x64 product copy hash matched
the deployed artifact. In the RDP background probe it produced a 640x400
client before the probe's manual resize. Its later debug handshake failed and
the probe exited 1; this is a geometry observation, not boot or colour
qualification. No owner INI, disk image or firmware was changed.

The user's real drag, Windows snap and physical desktop experience have not
been independently exercised; current validation is the existing simulated
Win32 contract and the measured product startup. S86 retains S84's PC110
guest setup-colour and boot observations for after owner acceptance of S85.
