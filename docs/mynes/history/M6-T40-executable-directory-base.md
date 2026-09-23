# M6 T40 Executable-Directory Base Capability

## Purpose

This record owns the Shared Lib Base capability and its MyNES/NXVM adoption.
The owner approved the task after identifying MyNES's direct Win32 call and
NXVM's `argv[0]` inference as divergent adjacent-INI paths.

## Delivered P Boundaries

1. Shared `4913410fb`: Base contract, Win32/Linux implementations and Lib tests.
2. MyNES `246e45a6c`: replace direct `GetModuleFileNameA` startup lookup,
   declare the direct Base dependency, test and publish `0_0_0040` x64/x86.
3. NXVM `fb461ee21`: replace `argv[0]`/working-directory fallback with a
   testable App startup module, declare the direct Base dependency and rebuild
   current x64/x86 artifacts.

## Final Result

Lib Base is the single owner of actual executable-directory discovery. Each
product owns only the composition of its fixed adjacent configuration filename:
MyNES appends `mynes.ini`; NXVM appends `NXVM.ini`. Neither product treats a
launcher's `argv[0]` spelling or the current working directory as configuration
authority. SoftPC has no repository change in this task and can import the same
canonical Lib capability independently.

Focused Base and product path regressions passed on x64 and x86 for both
adoptions. MyNES's current artifact pair is `0_0_0040`; NXVM's existing current
artifact version remains `0.5.0535` and was rebuilt through its own deploy flow.
