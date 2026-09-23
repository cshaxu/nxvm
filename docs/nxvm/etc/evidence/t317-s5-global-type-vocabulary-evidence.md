# T317 S5 Global Fixed-Width Vocabulary Evidence

## Baseline And Scope

The owner-approved baseline in the active S5 packet is 240 tracked code/script
files and 4,138 direct standard fixed-width spellings: 124 `src/` files / 2,085
tokens, 114 `tests/` files / 2,044 tokens, and two build/validation scripts /
nine tokens. The scan family is exact signed/unsigned 8/16/32/64-bit names,
their least/fast variants, maximum-width names, and pointer-width names.

Every baseline use was mechanically migrated to the existing project facade:
`type_unsigned_8/16/32/64`, `type_signed_8/16/32/64`, or the new foundational
`type_unsigned_pointer`/`type_signed_pointer` wrapper aliases. The pointer
aliases are defined directly from the standard pointer-width types in
`src/type.h`, so the atomic token and host interop layout remain unchanged.
No least/fast/max spelling occurred in the approved baseline, but the verifier
checks all of them.

## Exceptions And Equivalence Review

Only two lexical exceptions remain:

| File | Tokens | Reason |
| --- | ---: | --- |
| `src/type.h` | 16 | The foundational include and typedef aliases that implement the project facade, including pointer-width wrappers. Public wrapper declarations use project aliases. |
| `cmake/fixtures/t317-test-type-vocabulary-forbidden.txt` | 28 | Controlled negative-only validation fixture; it contains every scanned family member and is not product or test source. |

There are no deferred unsafe files. The migration is token-for-token only:
every changed declaration, cast, return type, parameter, and local variable
uses an alias with the same underlying standard type. The GCC rebuild of all
affected targets and the current-gate run are the source/ABI equivalence proof;
no structure field, function contract, expression, calling convention, or
runtime marker changed. `rg -n "tests/support" src` returned no matches.
An independent normalized comparison rebuilt the expected content of all 237
changed `src/`/`tests/` C and header files by applying only the approved token
map to the pre-change source; it found zero mismatches.

## Global Verifier

`verify-global-fixed-width-vocabulary` replaces the former inventory-only
scanner. It scans all tracked `src/`, `tests/`, `cmake/`, and `tools/` C,
header, CMake, and PowerShell files; dynamically constructs the forbidden
fragments so its implementation contains no direct forbidden spelling. It
accepts only the two files above, verifies a clean positive fixture, and
requires every family member in the named negative fixture. The retained
`verify-t317-test-type-vocabulary` target now depends on this global verifier.

Post-migration the global scan covered 467 code/script files and reported zero
forbidden direct spellings outside the 16 foundational aliases and 28 negative
fixture tokens.

## Artifact And Verification

- Fresh `mingw-gcc-x64` configure completed.
- `current-gcc` built `vm-0-5-0317` and copied
  `build/output/nxvm_0_5_0317.exe`; SHA-256:
  `A7EC7165730E2B037C24693E1E6B0EBFA6C67B9126F9CD5D5863A62DCA963F24`.
- The global verifier, retained T317 vocabulary target, and 47-command strict
  Ninja audit passed.
- Documentation governance, `git diff --check`, and the production support
  boundary check passed.
- `current-gates-gcc` passed all 54 static/governance targets and 194/194
  CTests.
