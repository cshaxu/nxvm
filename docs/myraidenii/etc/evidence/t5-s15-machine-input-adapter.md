# M1 T5 S15 — Machine Input Adapter

## Ownership

`myraidenii_machine_deliver_input()` is now the only KVM-to-Board adapter.
It accepts only `KVM_EVENT_KEY` and delegates each recognized transition to
the Board's existing semantic control operation.  The Board remains the sole
owner of active-low electrical state and register packing.  Emulator Session
remains the sole source-lifetime/pressed-key ledger: it emits the balanced
key releases for input reset and source retirement before the Machine sees
them.  The Machine intentionally creates no second held-key state.

## Initial Host Policy

The following host defaults are **L1** product policy.  They are not asserted
as original-board key wiring or as manual-backed L3 facts:

| Host key | Board semantic control |
| --- | --- |
| Arrow keys | Player 1 directions |
| Z / X | Player 1 button 1 / 2 |
| 1 / 5 | Start 1 / Coin 1 |
| F2 | Service |

The semantic direction/two-button/start/service/coin board controls and their
active-low realization remain the existing **L2** board contract.  No primary
manual PDF supplying a host keyboard layout was inspected, so this task adopts
no L3 formula, constant or key rule.

## Regression

The existing Machine-driver smoke compiles the actual production driver and
now proves every mapping's make event, a button break event, unknown-key
ignore, and direct reset/retirement-event ignore.  The latter confirms the
driver does not duplicate Emulator Session's input-reset ledger.

Cached x64 and x86 builds each pass the focused driver test and the complete
repository-only MyRaidenII unit selection, 8/8. Documentation governance and
whitespace checks pass.  There is no Product executable yet, hence no artifact
refresh, desktop input, external-ROM, sound, Window or gameplay qualification
is claimed.
