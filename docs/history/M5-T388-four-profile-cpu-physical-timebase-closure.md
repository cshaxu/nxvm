# M5 T388: Four-Profile CPU Physical-Timebase Closure

## Task Record

T388 owns the shared prerequisite discovered and transferred by T366, T368 and
T387: a successful-retirement tick may not enter a selected machine physical
clock domain unless its unit is source-accounted. It precedes all remaining
DeskPro physical-device timing and Model-40 L3 audit work.

## Accepted Progress

### S1: Publisher And Consumer Inventory

S1 establishes the sole successful-retirement elapsed publisher, terminal
classifiers for all four CPU profiles, explicit successful sentinel paths and
all present scheduler/provider clock consumers. It distinguishes non-retiring
paths and the waiting-only host virtual-time route, so neither can be used to
mask the active-retirement mixed-unit boundary. The retained
[physical-timebase inventory](../etc/evidence/t388-s1-physical-timebase-inventory.md)
adds a static recurrence check and names T388 S2 as the only immediate
receiver: it must classify each reachable successful sentinel route as an
exact primary row/formula, qualified secondary observation, or prohibition.

Coordinator review accepted P1 `79fe997a` against the active packet, source
and ownership boundaries, actual change set, focused static check,
documentation governance, specialized current-gate verification and the full
281-target current smoke gate. No runtime/profile ABI, physical scalar,
artifact revision or L3 claim is introduced.
