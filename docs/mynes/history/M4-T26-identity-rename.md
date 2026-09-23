# M4 T26 MyNes Identity Rename

## Outcome

MyNes is now the sole visible product spelling and mynes the sole technical
identifier across the tracked product, CMake/test graph, executable artifacts,
tooling, documentation and retained project records. The emulator's behavior,
version and ownership boundaries did not change.

## Delivered Work

- Renamed the CMake project, targets, presets, toolchain variables, test names,
  fixture labels and packaged executables to mynes.
- Renamed visible title, help text, configuration comments and cooked prompt to
  MyNes, including the previously missed character-built prompt.
- Updated the documentation-governance verifier and its fixtures for the longer
  technical prefix. Its artifact-stem calculation now derives the suffix from
  the complete mynes target prefix.
- Repaired the Lib type-layout verifier so portable code still rejects external
  vocabulary while a Win32 or Linux platform leaf may translate its native API.
  The existing type-layout negative self-test passes on both Windows targets.
- Rebuilt the retained x64/x86 executable pair. Owner ROMs remain untracked and
  ignored by the existing root assets policy.

## Verification

- Tracked text and tracked paths: no retired project identifier remained after
  the case-sensitive and technical-prefix sweeps. Protocol terms such as iNES,
  NES and external project names were retained.
- Documentation governance and its self-test passed for mynes-0-1-0011.
- x64: configured, rebuilt and completed the 110-test suite. The direct
  command-smoke test confirms the MyNes prompt.
- x86: reconfigured after an obsolete generated build file was found, rebuilt,
  completed the prior 110-test suite except its then-failing type-layout
  self-test, and passed that repaired self-test afterward. No other x86 test
  failed in the complete run.
- assets/roms has no tracked path; Git reports it ignored by .gitignore line 1.

## Delivery

- P1 admission: bb92ee0.
- P2 implementation: e8ea217.
- P3 closure: recorded by this commit.
- No Git remote is configured. The approved remote-aware delivery rule therefore
  requires local commits and makes push unavailable.
