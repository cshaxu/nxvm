# M6 T40 Executable-Directory Base Capability

## Purpose

This record owns the Shared Lib Base capability and its MyNES/NXVM adoption.
The owner approved the task after identifying MyNES's direct Win32 call and
NXVM's `argv[0]` inference as divergent adjacent-INI paths.

## Planned P Boundaries

1. Shared: Base contract, platform implementations and Lib tests.
2. MyNES: replace direct `GetModuleFileNameA` startup lookup and test it.
3. NXVM: replace `argv[0]` startup lookup and test it.
