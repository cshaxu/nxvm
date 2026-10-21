# M1 T6 S2 — Product Monitor And Hotkey Policy

## Delivered Boundary

MyRaidenII Product now owns exactly one monitor-provider adapter.  It injects
only structured content into Emulator Product; Emulator remains the sole owner
of fixed command parsing, help layout, result framing, prompt handling and
lifecycle request dispatch.

The one registered host hotkey is:

```text
Esc -> pause-toggle -> Emulator Session lifecycle request
```

The adapter calls the existing Session pause-toggle selection and the existing
Emulator Product lifecycle admission helper.  It does not call Machine pause
or resume directly and does not maintain another machine-state mirror.

The Window/raw-console help content documents the initial existing Machine
input policy:

```text
Arrow keys  move
Z           fire
X           bomb
1           start
5           coin
F2          service
Esc         pause or resume
```

Only Escape is a registry hotkey.  The other rows describe ordinary KVM key
events, which continue through Session to Machine's already existing semantic
control adapter.  This avoids a second guest-input injection path.

## Capability And Grade Boundary

Save, load and debug are still handled by the shared fixed command grammar,
but MyRaidenII installs no private execution callback for them.  They therefore
retain the canonical `Feature not implemented.` result instead of pretending
that a Board snapshot/debug protocol exists.

The host keyboard choices are explicitly **L1** product experience policy.
They are not represented as board-wiring or manual-derived L3 facts.  The
standing rule remains that only an inspected primary-manual PDF with an
explicit applicable formula, constant or rule can establish L3; external or
range-only evidence is L2.

## Verification

- The existing composition smoke now checks the one generic provider, all
  seven structured rows, exact Escape registration, and both directions of
  pause-toggle lifecycle selection.
- x64 `ctest -L myraidenii`: **12/12** passed.
- x86 `ctest -L myraidenii`: **12/12** passed.
- Both stable Ninja graphs rebuilt the affected Product library/test.
- No second `pause-toggle`, direct Machine lifecycle call, raw help formatter
  or competing monitor provider exists under MyRaidenII Product/Machine.

No external ROM, desktop/RDP, host-audio endpoint, application EXE or gameplay
qualification occurred.  The next Product increment must add a real entry and
release route only after its asset/configuration boundary is explicitly ready.
