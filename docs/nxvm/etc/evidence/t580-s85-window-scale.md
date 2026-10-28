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

## Follow-up targeted review

The P1 mock checked the snap restore rectangle, but not the next frame. A
new assertion for that next frame failed against P1 on x64. The correction
uses the existing snap-state transition to record the restored scale; the
same controlled scenario then passes on x64 and x86. The fixture covers top
and side state, restores with only WM_SIZE, eight simulated WM_SIZING edges,
and next-frame geometry. Its messages and snap-state responses are mocked;
this is not a recording of physical Windows snap/drag ordering. In an actual
task-local PC110 product run, the native startup sequence produced 8x1,
320x350 and 640x400 rather than the earlier 9600x6000. Actual snap and
restore on a live desktop remain unverified.

The Lib Types self-test owns its fixture under CMAKE_CURRENT_BINARY_DIR in
standalone and parent builds. Concurrent invocations sharing that directory
could collide; a process-scoped CMake file lock now serializes them. Two real
simultaneous invocations against the same task-owned directory both completed
successfully, at 26 and 54 seconds. Two successive x64 runs and one x86 run
passed with the prior fixture cleanup; an injected stale probe was removed.
A separate x86 replay was interrupted and is not counted. Windows junctions
placed as the fixture itself or nested inside a task-owned test fixture were
removed without deleting the separate target marker. This is not an assurance
for arbitrary untrusted TEST_WORK_DIR values or all filesystem types.

The timeout changes predate S85. T580 S32 recorded fixture checks exceeding
30 seconds and an Emulator standalone run at 32.14 seconds; T556 records
parallel 30-second and 60-second failures; T564 records a concurrent Lib
30-second timeout. S85 cached eight-worker full runs measured the two Lib
checks at 6.73--13.56 seconds and the Emulator check at 18.37--18.91 seconds.
No fresh cold-cache measurement was made; these facts justify extra headroom
but do not establish that exactly 300 seconds is necessary. The owner selected
60 seconds for each check based on approximately three times the observed
19-second maximum. Earlier 30/60-second parallel timeout evidence remains
recorded; the smaller budget is not proof that all contention will fit.
After regenerating the x64 tree, the three checks passed concurrently under
the new 60-second bounds in 4.43, 6.44 and 10.26 seconds. This was a cached
run, while the separate x86 full suite was active; it is not a cold-cache run.
After regenerating the x86 tree, the same checks passed in 2.89, 4.17 and
7.39 seconds. The complete repository test run after the restore correction
passed 760/760 on each width including MIDI under RDP. That full run began
before the new timeout values were regenerated; the three subsequent runs
are the evidence specifically for the revised bounds.

The rebuilt TFT x64 product's native client measured 640x400 before the
probe's manual adjustment. The later Debug handshake failed and the probe
exited 1, so this observation does not qualify PC110 guest boot or colours.
Physical top/left/right snapping and dragging were not exercised, and no
native message sequence for those gestures was captured. The controlled
fixture tests the permitted state transitions and eight injected edge values,
not Windows shell input or an independently confirmed live message order.
