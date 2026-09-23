# M4 T25 Supported-Game Acceptance

## Outcome

T25 closes M4's declared support acceptance for the two owner-local NTSC iNES
inputs: Dr. Mario (MMC1) and Super Mario Bros. 2 (MMC3). Both Window and
Core-generated text Console routes, default controls, audio, pacing, lifecycle
and cartridge management are accepted on x64 and x86.

## Evidence

The automated record at
[M4 T25 automated acceptance](../etc/evidence/m4-t25-automated-acceptance.md)
contains the bounded dual-architecture title execution, frame, PCM, text-frame,
native KVM and controller evidence. It records no ROM bytes or local paths.

The owner then tested the current executable, reported it satisfactory, and
explicitly authorized S and T closure on 2026-09-21. That owner-visible result
accepts the direct Window/Console game experience that automation could not
observe.

T25 S2 completed the requested SoftPC interaction convergence. Its evidence is
[M4 T25 S2 interaction convergence](../etc/evidence/m4-t25-s2-interaction.md):
the MyNes-only App session state now holds lifecycle outcomes until the monitor
is current, uses display=window|console, keeps raw Console gameplay free of
monitor text, and preserves one blank line before a prompt.

## Delivery And Verification

T25's implementation sequence is P1 through P13 under S1, followed by S2 P1
admission, S2 P2 implementation, and S2 P3 interaction-sweep completion. S2
P2 is local commit 61b1e0e; no remote was configured, so it was not pushed.

The final S2 x64 and x86 focused regression set passed:

- App configuration and command smoke tests;
- App debug and media command integration smokes; and
- native Window and native Console smokes.

Both versioned executable artifacts and their editable adjacent mynes.ini were
rebuilt in assets/binary. The final configuration key is display, with window
and console as its accepted values.

## Scope Review

S2 changed ten tracked source/test files by +193/-66 lines (net +127), measured
from its admission commit to its implementation commit. The retained production
path is the single App command session in src/app/command.c; it owns transition
readiness, deferred monitor wording and prompt readiness. The increase replaces
the former one-bit pending command state with explicit ownership required by the
two presenter routes. No Common, Lib, Core or SoftPC source changed.

The similar-issue sweep classified the retired video configuration key, direct
lifecycle prompt timing, Window/Console broker handoff, active F5/F12 wording,
and all affected templates/tests. Active product records now use display and
Esc; historical evidence retains earlier terminology.

## Remaining Boundary

M4 has no active implementation task after T25. A future milestone may broaden
game compatibility only through its own admitted support scope; this closure
does not claim other ROMs, mappers, regions, expansion audio, persistence,
additional controllers or save states.
