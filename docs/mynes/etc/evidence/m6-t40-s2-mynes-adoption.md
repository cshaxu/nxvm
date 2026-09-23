# M6 T40 S2 MyNES Adjacent-INI Adoption Evidence

## Change

MyNES App now asks Lib Base for the actual executable directory and appends its
product-owned `mynes.ini` filename. The App no longer includes the Windows SDK
or calls `GetModuleFileNameA`; it directly links the `base` target that owns the
host query.

The executable-bearing MyNES change publishes the `0_0_0040` x64/x86 pair.
The prior `0_0_0039` pair is retired. The sole editable adjacent `mynes.ini`
remains owner-local and is deliberately excluded from this task P.

## Verification

On 2026-09-23:

- `mynes-app-config-smoke` and `mynes-0-0-0040` built in both
  `mynes-gcc-x64-release` and `mynes-gcc-x86-s4`.
- `ctest -R "^(mynes\\.app\\.config-smoke|library\\.process_directory)$"`
  passed 2/2 on x64 and 2/2 on x86.
- Static source sweep found no `GetModuleFileName` or `<windows.h>` occurrence
  in `src/app-mynes/product`.
- The published executable headers identify x64 as `0x8664` and x86 as
  `0x014c`.
- `Verify-DocumentationGovernance.ps1 -Product mynes` and `git diff --check`
  passed.
