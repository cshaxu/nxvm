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

### S2: Successful Sentinel Matrix

S2 maps every direct one-tick successful fallback to its exact form/context,
primary status and physical-time disposition. It confirms that the current four
profile axes remain mixed-unit: an unallocated success retains deterministic
progress but cannot represent oscillator time. The retained
[successful-sentinel matrix](../etc/evidence/t388-s2-successful-sentinel-matrix.md)
separates legal fallback routes from non-retiring failures and transfers only
one shared mechanism to S3: a Core-owned eligibility boundary at the sole
publisher, without a second scheduler or a VM reclassification route.

Coordinator review accepted P1 `c9971e9a` against the S2 packet, T359/T360/T361/
T363/T366/T368 retained evidence, actual static-gate changes and the full
281-target current smoke gate. It adds no runtime/profile ABI, clock scalar,
artifact revision or L3 claim.
