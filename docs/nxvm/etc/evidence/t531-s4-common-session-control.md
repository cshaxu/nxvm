# T531 S4 Common session control proof

## Result

`common/session` is now the one reducer for the requested presentation target,
acknowledged Window/raw-Console/broker facts, and the one UI action in flight.
It does not treat an action request as completion.  Common UI applies one
mechanical action, then a copied completion or failure status returns through
the session FIFO before the next action can be selected.

The obsolete `active_target` mirror, `target_changed` plan value and Common UI
target helper route have been removed.  The retained values have distinct
owners: Session owns desired target and action-in-flight; Common UI owns actual
surface facts; the completion fact is the sole route from the latter to the
former.  Product code neither recreates a target translation nor owns another
surface-state copy.

Common Session now also owns the monitor endpoint used by the injected CLI:
the NXVM console requests a line and writes monitor text through Session, while
the existing injected CLI grammar and the distinct Core executor remain
unchanged.

## Transition and failure evidence

The owner-local Session smoke covers the full ordered topology in both
directions:

- `Window -> create raw Console -> bind raw Console -> destroy Window`;
- `raw Console -> bind cooked monitor -> destroy raw Console -> create Window`;
- `Window -> destroy Window` for `NONE`;
- an injected destroy failure reports one plan failure, preserves acknowledged
  facts, and requires an explicit later reconcile to retry.

The same smoke continues to cover lifecycle notices, injected CLI result and
prompt, machine observer, frame capture, input-source retirement, stale run
rejection and FIFO saturation.  There is no retry loop hidden in the reducer.

## Verification

- Strict source syntax: the changed Common Session/Common UI/NXVM integration
  and their owner tests compile with `-std=c11 -Wall -Wextra -Wpedantic
  -Werror`.
- The new Session smoke was directly linked with the production Lib archives
  and passed.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`:
  **299/299 passed** in 16.17 seconds.
- Common manifest, standalone Lib-only corpus, documentation governance and
  `git diff --check` passed.

The local CMake/Ninja aggregate build continues to stall after generation and
before it creates compiler children.  This record therefore distinguishes the
strict direct compilation of changed source from the complete already-built
unit regression run; it does not claim that the aggregate CMake build rebuilt
the changed sources.

## Scope and retained boundary

Production source paths add 167 and remove 104 lines (net +63); owner tests
add 76 and remove 5 lines.  The positive production delta is the explicit
acknowledged-fact reducer and no duplicate execution, native handle, generic
executor or product presentation-policy seam was added.  `console_control`
configuration remains the explicitly planned S8 batch; this S supplies its
shared action/fact control substrate only.
