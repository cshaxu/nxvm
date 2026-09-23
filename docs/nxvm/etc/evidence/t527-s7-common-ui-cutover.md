# T527 S7: Common UI Cutover

## Result

`common/ui` is the sole generic binding from copied UI plans and facts to the
public Lib Console/Window components. It owns one process Console broker,
the active Console-or-Window leaf, the latest copied frame, and generic title
and mouse operations. It depends only on public Lib headers.

| Former path | Retained owner and route |
| --- | --- |
| `vm/presentation/presentation.*` | `common/ui/presentation.c` with `common_ui` opaque state and copied `common_ui_plan` |
| `vm/presentation/console_host.*` | `common/ui/console_host.c` with the one Lib host Console broker |
| `vm/presentation/frame.*` | `vm/machine/runtime/frame.*`, the NXVM-only Core-display to Lib-frame adaptation |
| NXVM hotkey registration, title and notice text | `vm/product/console.c` product policy and Console binding |

`common/ui` has no NXVM, Core, VM, platform or native-header dependency. Its
public interface exposes no native handle, machine/session pointer or product
text. `vm/product` converts the session-owned plan to the copied Common UI
plan and supplies NXVM's target policy, title and hotkey meanings.

Each native Lib leaf receives the required generic failure callback through
`common_ui`; NXVM's product binding reports that callback.  Thus a UI mailbox
or input-delivery failure has one explicit upward route rather than silently
preventing Console/Window creation.

## Similar-Route Sweep

- Repository source/test/CMake scan finds no `vm_presentation`,
  `vm/presentation` or `VM_PRESENTATION` production/test route.
- `src/common/ui` has no Core/VM include, legacy `type.h`, direct ISO-C or
  native-platform include.
- The dependency allowlist removes the retired presentation edges and records
  the moved VM display-adaptation edge.

## Verification

- Focused Common UI, frame-adapter and product-console tests passed.
- Full x64 Release repository-only unit suite passed: **298/298** in 16.03
  seconds.
- The full `verify-current-specialized-gates` aggregate passed, including
  dependency/header boundaries, route partition, direct-compilation ownership
  and current artifact checks.
- T344's canonical registration inventory now includes the pre-existing
  `types-smoke` route; T345's residual ledger no longer duplicates the
  strict `common-xasm32-contract-smoke` route.

`git diff --check`, documentation governance and the source-boundary sweeps
also passed. Implementation commit: `bcb8346a`.
