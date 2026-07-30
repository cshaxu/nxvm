# M0 T6 S1 Runtime CLI Contract Review

## Scope

Reviewed the future `ntvdm64 run` product contract against the canonical
architecture, roadmap, source policy, redistributability policy, and project
status.

## Result

The command contract is linked from the README, architecture, and M3 roadmap.
It preserves the owned DOS and non-invasive default path, keeps Microsoft and
WineVDM work outside the product runtime, assigns Windows-specific display,
Console, input, and filesystem enforcement to Platform, and records no runtime
implementation claim. M1 remains eligible but inactive.
