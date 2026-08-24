# M5 T447 S2 Session Boundary

S2 consumes Td S125 B/I, Td S127 C, and Td S129 A. The complete S2 inventory
searched tracked source, tests, CMake, and governance records for `SESSION`,
`open_with_options`, `borrow_selected`, `C_VOID **`, mutable command/open
token views, and selected-session casts.

## Disposition

- `core_product_session_manager_borrow_selected` is deleted. The manager owns
  the selected object and offers either one synchronous operation callback or
  one copied selected snapshot. A VM pointer is used only within the adapter's
  callback invocation; it is never returned or retained.
- `src/core/product/session/command.c` is the only reachable
  `SESSION LIST/OPEN/SELECT/CLOSE` grammar. The VM console retains only its
  profile chooser: it freezes its selected profile into a read-only token view
  and calls the same Core `OPEN` command. Core preserves the prior console
  list format and no-selected CLOSE diagnostic.
- Command and open tokens are `const C_CHAR *const *`. The provider may parse
  them but cannot change caller token storage.
- Manager, profile, multi-window, and conditional startup-failure smokes no
  longer receive a selected raw pointer. The startup-failure assertions run
  synchronously through the manager callback; VM-local tests construct VM
  sessions locally when that is their actual subject.

`C_VOID **` remains only at the separate provider-factory creation boundary
where a provider creates an owned opaque session. It is not a selected-session
borrow and is unchanged by this S.

## Proof

- `M5:T447:S2:SESSION-BOUNDARY:OK` rejects the old public borrow, mutable
  session token view, and VM duplicate command dispatch.
- `M5:T205:S1:CONSOLE-ADAPTER-CLOSURE:OK` rejects the former adapter borrow
  helper and proves its single manager operation route. The retained public
  raw-borrow closure also passes.
- Focused manager command-invalid-payload and no-selected cases, manager,
  profile, multi-window, console, and lifecycle smokes pass. The dedicated
  stage-1 Windows startup-failure smoke builds and exits successfully.
- The full `current-gate` run completed 293 tests without a failed-test log
  (`115.12` seconds). All 150 configured specialized-gate build steps pass,
  including documentation governance.
- Current artifact `nxvm_0_5_0447.exe` has SHA-256
  `BEA8A10910776C8E23090A6CDCAB6AAB174B7382A71CDCC421DA416F82265BA0`.

## Minimalism And Review

The duplicate VM grammar, output-borrow API, and three cross-owner test
escapes are deleted. The one necessary addition is a synchronous capability:
each machine action executes while the manager owns the selected session, so
there is neither a second state owner nor a retained raw pointer. The actual
implementation/build/test diff is +438/-272 lines before governance evidence;
the increase is explicit lifetime enforcement rather than a parallel path.
Review found no compatibility parser, forwarding-only public API, mutable
session token contract, or selected-session output pointer remaining.
