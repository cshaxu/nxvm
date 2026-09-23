# T532 S2 Single-Product Layout Relocation

This evidence records execution of the S1 frozen layout ledger. The current
[Architecture](../../design/ARCHITECTURE.md) and
[Source Layout](../../design/CODING.md) remain the sole current authorities.

## Final Owner Map

| Previous root | Final disposition |
| --- | --- |
| `src/vm/app`, `src/vm/main.c`, `src/vm/request_interface.h` | `src/app` |
| generic `src/core/machine` | `src/core/core` |
| NXVM adapter `src/vm/machine` | `src/core/machine` |
| `src/vm/profile` | `src/core/profile` |
| `src/vdm` | deleted |
| repository-only `test/vm/{app,machine,profile,support}` | `test/{app,core/machine,core/profile}` |
| generic Core `test/core` sources and fixtures | `test/core/core` |
| `test/vdm` | deleted |
| `test/integration` | unchanged independent external-asset boundary |

The Git move review records 437 renames and 12 deletions. The remaining
deletions are the retired VDM source/test files and the VDM-only forwarding
gate; no VDM compatibility shell or forwarding include is retained.

## Path And Owner Proof

- A tracked-file sweep finds no `src/vm`, `src/vdm`, `test/vm`, or `test/vdm`
  consumer outside retained history/evidence. Empty filesystem directories have
  no tracked files and do not form a source or test component.
- `test/app`, `test/core/core`, `test/core/machine`, and `test/core/profile`
  now mirror their respective NXVM source owners. There are no flat tracked
  sources in `test/core`; `test/integration` was not moved.
- CMake source lists, direct include paths, static gates, dependency ledger and
  readiness ledger resolve only the final owner names. The readiness scan also
  excludes immutable `static ... const` decoder tables rather than misclassify
  them as mutable session state.

## Verification

The final S2 verification used the MinGW Makefiles configuration
`build/t532-s2-make`:

```text
cmake --build build/t532-s2-make --target vm-0-5-0531 --parallel 8
ctest --test-dir build/t532-s2-make -L unit -j 8 --output-on-failure
cmake --build build/t532-s2-make --target verify-current-specialized-gates --parallel 8
cmake --build build/t532-s2-make --target verify-documentation-governance --parallel 1
```

The product target builds successfully. The repository-only unit suite passes
336/336 in 17.12 seconds. Documentation governance, dependency-DAG and
session-readiness gates pass; the specialized aggregate passes after the
relocation ledgers are updated. No integration scenario was moved or
reclassified: its direct includes were mechanically repaired to the final
owners, while its external-asset result remains the current baseline's
separate T531 transfer.
