# M1 T7 S1 — Selected-Set Startup Diagnostic

## Input and containment

The existing inventory verifier accepted the owner-supplied, read-only external
archive `O:\assets\raidenii\raiden2.zip` with the retained archive SHA-256:

`9fedc8ddac2208408c3b05201316e3f2db43bdfb104551ba17337bb55db13d71`

It reports 13 runtime images and one non-runtime JED entry.  The existing
startup wrapper stages only the 12 paths currently consumed by the Machine
configuration transaction.  Staging took place under
`O:\assets\raidenii\myraidenii-diagnostics\` and was empty after each run.
No ROM bytes, source material, screenshots, raw traces, samples or executable
were added to this repository.

## Commands

Both existing targets were already present in the stable Ninja graphs:

```text
cmake --build build/.ninja/gcc-x64-release --target myraidenii-selected-set-startup-probe-build --parallel 8
cmake --build build/.ninja/gcc-x86-release --target myraidenii-selected-set-startup-probe-build --parallel 8
```

The existing inventory verifier was invoked once, followed by the existing
`Invoke-SelectedSetStartupProbe.ps1` wrapper once for each probe width.

## Neutral results

Both x64 and x86 produced the same bounded result:

```text
reason=1 detail=0
fault-valid=0 delivered-exceptions=0
cop-operation=0 control-valid=0
executed=2000000 ticks=33301143
frame-width=320 frame-height=240 nonblack=74795
frame-digest=a06b9c216e1ce034
pcm-frames=4096 pcm-dropped=95807 pcm-nonzero=0
pcm-digest=1d00c08067be8383
```

`reason=1` is the probe's normal budget stop.  The result establishes only
that the current selected-set route can execute the bounded amount on both
host widths without the diagnostic reporting a CPU, COP or control terminal
fault.  The zero PCM observation and dropped-frame count are observations for
later audio/pacing qualification, not proof that sound is absent or broken.

## Grade and disposition

This is **L2** external-input behavior evidence.  It contains no inspected
primary-manual formula, constant or rule and therefore cannot be L3.  It does
not prove a title screen, input response, full rendering, audio fidelity,
protection correctness, stage progression, desktop presentation or release
readiness.  The 13th runtime `copx` archive role is not currently a Machine
image input; that ownership/disposition must be reviewed explicitly before a
later packet changes configuration or Board behavior.
