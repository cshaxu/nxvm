# M4 Supported-Game Acceptance

## Outcome

Complete the declared Dr. Mario and Super Mario Bros. 2 play scenarios on both
KVM backends with sound, speed and management behavior.

## Acceptance

Each owner-local input completes the M4 ten-minute bounded protocol on x64 and
x86. Project-owned regressions, local permitted observations and release
artifacts agree; any observed defect is repaired in its owning mechanism.

## T25 S2 Interaction Prerequisite

Before treating gameplay observations as acceptance evidence, MyNes aligns its
App interaction with the read-only SoftPC App baseline: mynes.ini uses
display=window|console; one lifecycle transition owns its deferred outcome and
prompt; raw Console gameplay suppresses management output while Window gameplay
returns it through the cooked Console. MyNes retains its NES-specific rom and
nested debug grammar. The implementation stays entirely in MyNes App and its
tests; SoftPC is neither edited nor made a runtime dependency.
