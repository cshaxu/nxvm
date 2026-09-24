# M5 Persistence And Release Baseline

## Outcome

Provide safe cartridge battery persistence, controlled configuration saving and a reproducible dual-architecture release baseline. Generic atomic replacement belongs in Lib; cartridge RAM/dirty state belongs in Core; save/load policy belongs in App.

## Acceptance

Fault-injection tests prove old save/config data survives partial write, flush, close and replacement failures. Identity mismatch, missing/damaged save, dirty shutdown, paused save and replacement behavior are deterministic. Release builds produce versioned x64/x86 artifacts, one editable INI and a checked manifest under the declared reproducibility criterion.
