# T527 S6 - Debug Source-Preservation Correction

## Result

`common/debug/command.c` remains the one DOS Debug grammar and retains its
original table-driven command-handler layout.  The correction does not add a
parser, a Debug queue, a file-service callback, or a Core print path.

| Concern | Sole route after S6 | Evidence |
| --- | --- | --- |
| Assembly/disassembly result | `common/xasm32` returns the existing explicit `LIB_STATUS_INVALID_ARGUMENT`, `LIB_STATUS_UNSUPPORTED`, or `LIB_STATUS_LIMIT_EXCEEDED` classification | Contract smoke exercises invalid, unsupported and capacity cases; no `LIB_STATUS_FAULT` alias was added. |
| Host diagnostics | copied Debug result text | `aasm32.c`/`dasm32.c` have no direct `printf` or enabled trace object; parser error-flow macros remain part of the retained table implementation. |
| `N/L/W` bytes | public `lib/storage` | `L` opens one readonly medium; `W` uses the one truncate/append byte writer.  The writer's existing mode selects `wb` or `ab`; it accepts opaque bytes and exports no handle. |
| Common C vocabulary | public `lib/types` | Common uses Lib memory/text/allocation/bounded-formatting and atomic vocabulary; it includes neither the old root `type.h` nor a direct ISO C header. |
| Segment/control detail | `common/debug -> common/machine -> vm/machine -> core/machine` | one bounded copied CPU snapshot supplies selector/base/limit/access fields and CR0--CR4 values only while the paused lease is valid. |

The Core endpoint copies its CPU state into its local snapshot.  The VM adapter
maps that copied value into the neutral common snapshot.  Common Debug neither
includes Core headers nor receives a Core/VM pointer.

## Focused Proof

- `common-debug-smoke`: table command continuation, exact XSREG/XCREG text,
  direct `N/L/W` byte round-trip and paused target operations.
- `common-xasm32-contract-smoke`: retained xasm32 public contract and
  classification behavior.
- `storage-file-smoke`: binary truncate/append writer behavior, including
  NUL and `FFh` bytes.
- `types-smoke`: memory, text, bounded formatting/append, and allocation
  behavior through the sole public type vocabulary.

## Closure Verification

- x64 Release full unit suite: **297/297 passed** in 17.17 seconds.
- `lib-verify-manifest`, `verify-c-facade-headers`, Core Debug-boundary,
  debugger-boundary, recorder-lifecycle, bounded-formatting and documentation
  governance gates: passed.
