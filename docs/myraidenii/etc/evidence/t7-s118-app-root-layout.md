# M1 T7 S118 — App-root layout normalization

## Disposition

MyRaidenII's product-owned source and test roots now use the repository-wide
`app-` filesystem convention:

```text
src/app-myraidenii/
test/app-myraidenii/
```

The change is a layout-only migration.  The logical `myraidenii_*` C symbols,
CMake target names, product identity, assets and runtime behavior remain
unchanged.

## Live owners updated

- Root CMake registration and the MyRaidenII test registration.
- Source-relative unit and integration compilation paths.
- Product-owned source/test/tools documentation and present-tense shared rules
  which name the App roots.

An old `test/myraidenii/integration` reference remains in the T2 V30 ledger.
It describes the historical layout at that task's time and is intentionally
preserved as a task record.

## Validation

| Check | Result |
| --- | --- |
| Live-root / old include search | Passed; only the preserved historical T2 ledger reference remains. |
| Documentation governance | Passed for `myraidenii`. |
| `git diff --check` | Passed. |
| x64 Ninja build and MyRaidenII unit suite | 13/13 passed. |
| x86 Ninja build and MyRaidenII unit suite | 13/13 passed. |
| x64/x86 selected integration executable compilation | Passed; the two existing selected-set integration targets build on both widths. |

The x86 configure invocation explicitly supplied the existing MinGW compiler
to restore the pre-existing stable `build/.ninja/gcc-x86-release` tree after
its cached compiler variable was absent.  No additional build tree was made.

No ROM-backed selected-set integration execution, desktop presentation, or
artifact deployment was needed or claimed: this task changes neither runtime
code nor release identity.
