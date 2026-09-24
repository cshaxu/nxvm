# M6 T41 S13: SoftPC Raw-CRT Reconciliation

## Scope and provenance

NXVM is the canonical six-component corpus. SoftPC commit
`2ef228f8ffe2487a6247604cbc8e7f2b6fe13561` was clean when inspected and is
project-owned MIT material with no independent notice. S13 imports only its
`test/x86/verify_negative.cmake` correction: the Raw-CRT negative probe now
uses `memcpy(0,0,0)`, which the x86 verifier must reject. The prior
`lib_memory_copy(0,0,0)` probe exercised an allowed Lib facade and could never
prove that the Raw-CRT rule worked.

`src/lib/README.md` also adopts the owner-approved SoftPC wording for the
already identical WASAPI implementation: its delivery worker owns COM
initialization, endpoint acquisition, endpoint-event waiting, release and COM
uninitialization. This is a documentation precision change only; it changes no
API or runtime path.

## Six-component comparison

The comparison used tracked file inventory plus SHA-256 of every common path
in `src/{lib,common,x86}` and `test/{lib,common,x86}`. Results after the S13
import are:

| Tree | NXVM-only files | SoftPC-only files | Content differences | Disposition |
| --- | ---: | ---: | --- | --- |
| `src/lib` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision. |
| `src/common` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision. |
| `src/x86` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision. |
| `test/lib` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision. |
| `test/common` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision. |
| `test/x86` | 0 | 0 | `MANIFEST.sha256` | NXVM canonical manifest revision after the imported fixture. |

There is no shared production C/H or behavior-test mismatch. The transferred
fixture and the approved Lib README wording are byte-identical to SoftPC. The
remaining six differences are intentionally local manifest revisions. The
reported Common “Raw CRT plus Noncanonical” issue does not exist in either
current corpus: both have the same Common fixture and the complete x64 and x86
unit gates execute it successfully. No speculative Common change was made.

## Verification

- `cmake -DX86_ROOT:PATH="$PWD/src/x86" -DPROBE_ROOT:PATH=<temporary> -P test/x86/verify_negative.cmake`: passed after the raw `memcpy` repair.
- All six `verify_manifest.cmake` invocations passed; `src/x86/verify_corpus.cmake` passed.
- x64: `ctest --test-dir build/mingw-gcc-x64-release -L unit --output-on-failure -j 8`: 335/335 passed in 18.90 seconds.
- x86: configured with `i686-w64-mingw32-gcc` in `build/s13-x86`; `ctest --test-dir build/s13-x86 -L unit --output-on-failure -j 8`: 335/335 passed in 24.15 seconds.
- `tools/shared/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Product nxvm`: passed.

The restricted sandbox stalled while launching the x86 compiler ABI probe; the
same configuration and complete test suite succeeded in the normal host
environment. This is a sandbox execution limitation, not a product failure.
