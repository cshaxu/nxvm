# T531 S6 Common input and failure closure

## Result

Common Session remains the only copied-fact FIFO.  The batch closes the input
and asynchronous-failure routes that were still split between Lib callbacks,
Common UI and NXVM product output.

`common_ui_input()` now translates the Lib input contract correctly: a Common
sink returning `LIB_STATUS_OK` becomes a nonzero Lib acceptance result.  The
old direct return treated every successful input as failed delivery.

All nonterminal UI delivery failures now return as copied Session facts:

`Lib component/broker callback -> Common UI failure sink -> NXVM binding ->
Common Session UI-delivery-failed fact -> Session plan -> product monitor text`.

The binding only copies the fact; it neither formats it nor owns a second
failure state.  Cooked monitor-line delivery now uses the same path.  UI
actions retain their requested action identity even when their operation
fails, so the reducer clears its in-flight action and can reconcile a retry.
Mouse/frame application failures use the same copied failure fact rather than
being ignored.

## S6 matrix

| Source/family | Disposition and owner | Regression/proof |
| --- | --- | --- |
| Key press/release and source retirement | Session owns the bounded held-key ledger by `source_identity`; retirement releases only that source while running, then removes every retained key even when paused/stopped. | `common-session-smoke` exercises press, retirement and exactly one synthesized release. |
| Keyboard, mouse and text input | Common UI copies Lib input into Session. Session stamps the current run at ingress and rejects later mismatched facts in its reducer; only running state reaches the injected machine sink. | `common-ui-smoke` proves accepted Lib input; `common-session-smoke` proves run stamping and lifecycle dispatch. |
| Registered hotkeys | Lib emits copied UI hotkey records; Session transports them and NXVM product policy maps pause, CAD, Alt+Enter and mouse release. No native or direct guest path is added. | Existing product console/unit coverage plus the Common input acceptance smoke. |
| Mouse capture/release | Session lifecycle facts produce capturable/release plan fields; Common UI alone invokes Lib Window operations. Paused/stopped force noncapturable plus release. | Existing Session lifecycle smoke and S3 UI action coverage. |
| Stale frame/completion/input | Frame and UI completion retain run identifiers; input now does too. Reducer rejects a nonzero stale run before it can change presentation or guest state. | Session smoke stale-machine and current-run input assertions. |
| Full Session FIFO | A required fact that cannot fit latches terminal `LIB_STATUS_LIMIT_EXCEEDED`; `take()` exposes it rather than silently dropping an input. The product stops its drain and reports delivery failure; no invented replacement queue exists. | Session smoke fills all 64 slots and observes the terminal result. |
| Window/Console component and broker action failure | A failed action reports its original action identity through copied UI completion, clearing the in-flight reducer state before retry reconciliation. | Common UI smoke injects create/claim failures; Session smoke reduces failed completion and retries. |
| Asynchronous leaf/broker/monitor failure | Common UI converts each callback failure to one copied Session delivery-failure fact. | Strict compile covers real Console host; Session smoke reduces the copied failure fact. |

## Two-adapter boundary

Read-only SoftPC `app/control.c` has the corresponding source-retirement,
pressed-key, run-generation and latched-delivery-failure semantics.  Its
current product binding remains the explicit S8 receiver; no sibling source was
modified and this S does not claim final two-consumer integration.

## Verification

- Strict C11 compilation with `-Wall -Wextra -Wpedantic -Werror` passed for
  changed Common Session/UI, NXVM product binding and owner tests.
- Directly linked `common-session-smoke` and `common-ui-smoke` pass with the
  changed sources, including accepted Lib input, copied failure reduction,
  failed UI completion identity, source retirement and queue saturation.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`:
  **299/299 passed** in 16.33 seconds.
- Common manifest/corpus verification, documentation governance and actual
  diff review are required before acceptance.

The aggregate CMake/Ninja focused build again stalled before launching a
compiler.  The affected sources and direct linked owner smokes above provide
the changed-source proof; the complete unit replay is recorded separately.

## Change accounting

Tracked production paths add 65 and remove 15 lines (net +50); owner tests add
49 and remove 5 lines (net +44).  The added code is one copied failure fact,
one status translation and one retained failure callback boundary.  It removes
the successful-input-as-failure bug and the product-side direct UI error print;
no queue, machine state, native API or product policy owner is duplicated.
