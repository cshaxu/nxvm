# M1 T7 S4 — Selected-Set Control Diagnostic

## Scope and containment

This record covers only the existing `--coin-start-held` option of the
repository's bounded selected-set probe.  The option asserts Coin 1 and
Start 1 directly through the public Board-control API before execution; it is
not a Window/KVM event, a physical controller test, or proof that the title
accepts a credit/start transition.

The unchanged, owner-supplied archive was read from
`O:\assets\raidenii\raiden2.zip`.  Its already recorded SHA-256 is
`9fedc8ddac2208408c3b05201316e3f2db43bdfb104551ba17337bb55db13d71`.
The existing wrapper staged only the twelve roles consumed by the current
Machine input transaction beneath
`O:\assets\raidenii\myraidenii-diagnostics\`; the directory had no child
items after both runs.  No protected input, dump, trace, screenshot or
executable was retained in the repository.

## Commands and result

The existing x64 and x86 probe targets were rebuilt through the stable Ninja
trees, then the existing wrapper was invoked once per width with
`-CoinStartHeld`.

Both widths produced the same bounded result:

```text
MYRAIDENII:STARTUP-PROBE:OK reason=1 detail=0 fault-valid=0
cop-operation=0 control-valid=0 coin-start-held=1
executed=2000000 ticks=33301143 frame-width=320 frame-height=240
nonblack=74795 frame-digest=a06b9c216e1ce034 pcm-frames=4096
pcm-dropped=95807 pcm-nonzero=0 pcm-digest=1d00c08067be8383
```

`reason=1` remains the probe's normal instruction-budget stop.  Relative to
S1, every observed execution/frame/PCM field is unchanged except the
explicitly requested `coin-start-held=1` marker.  In particular, the Board
reported no unowned control-window access and no CPU/COP terminal fault.

## Evidence grade and disposition

This is **L2**: it is a bounded observation using an external, owner-supplied
set.  It is not derived from an inspected primary manual PDF with an explicit
applicable rule, formula or constant, so it is not L3.  It does not establish
host input delivery, debouncing, title/attract behavior, credit acceptance,
gameplay, COPX correctness, rendering fidelity, audio fidelity or release
readiness.

The archive's `copx` role remains deliberately outside this probe's twelve
Machine roles.  An inspected external emulator reference identifies a
separate 0x40000 COPX image but describes its lookup use only tentatively;
that is an L2 receiving-boundary fact, not authority to invent a table or
declare the current COP model complete.  A later packet must establish an
independently implementable Board contract before admitting that image.
