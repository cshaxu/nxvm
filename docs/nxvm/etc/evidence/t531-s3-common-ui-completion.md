# T531 S3 Common UI action and completion proof

## Result

Common UI now owns one mechanical action vocabulary for Window creation and
destruction, raw-Console creation and destruction, raw/monitor Console broker
binding, title, mouse-capturable state and mouse release.  Every successful
action returns copied surface facts; a failed native operation returns its
status and publishes no invented completion.

`presentation.c` is the only non-test Common consumer of Lib Window/Console
leaf APIs.  It owns the two optional native surfaces and the one broker-current
fact.  It owns no product display policy, lifecycle or machine state.

## Transition boundary

`common_ui_set_target()` and `common_ui_apply()` remain finite compatibility
translations for the current NXVM caller.  They contain no second native path:
each translates only into the action API.  T531 S4 is the explicit receiver for
the session reducer and removes this target-plan boundary.

## Tests and gates

- Owner-local fake-backed smoke covers initial facts; monitor binding; title and
  mouse no-ops without a Window; Window create/destroy and injected create
  failure; raw Console create, injected broker-claim failure, bind, monitor
  return and destroy; plus frame publication after those transitions.
- The new smoke was compiled with `-Wall -Wextra -Wpedantic -Werror`, linked
  against the production Lib archives, and passed.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 4 --output-on-failure`:
  299/299 passed in 21.74 seconds.
- Common manifest, Common Lib-only corpus, Lib manifest and documentation
  governance checks passed.

The CMake/Ninja aggregate build was observed to stall before creating compiler
children; direct strict compilation and the complete already-built unit suite
provide the recorded verification while that host build-process condition is
separate from this source change.

## Minimalism

The action and completion values are copied ABI data.  No platform handle,
product callback, second UI loop or Lib public ABI was introduced.  Production
source changes are 191 added and 42 removed lines across the UI owner and its
interface; the test fake adds native-leaf substitution only for deterministic
failure coverage.
