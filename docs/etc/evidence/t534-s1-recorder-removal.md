# T534 S1 Recorder Removal Evidence

## Scope

The removed App feature exposed `RECORD start <file>` and `RECORD stop`, but
the production session never bound its Core Debug observation callback. Its
only observation consumer was the dedicated smoke; the command could therefore
claim success without producing instruction records.

## Removal

- Removed the App recorder source, public header and console command.
- Removed the recorder-only copied Debug observation, callback binding and
  propagation from the Machine adapter.
- Removed the recorder smoke, its CMake target, fixed-write fixture and
  specialized verifier.
- Retained Core Debug's internal instruction observation and execution-plan
  behavior; those are used by the hardware debugger and are not recorder I/O.

## Static Proof

The recorder-specific source/build/test identifiers have no live match outside
the active task packet:

```text
vm_app_recorder
vm_machine_debug_observer
vm_machine_bind_debug_observer
vm-debugger-recording-lifecycle
```

`git diff --check` passes. The changed production translation units
`app/command.c`, `core/machine/debug.c`, `core/machine/lifecycle.c` and
`core/machine/debug_adapter.c` compile with the generated project flags.

## Verification

The initial sandboxed Ninja invocation could not make progress. Re-running the
same fresh build outside that constrained session rebuilt the complete target
set. The final repository-only unit run is recorded in
`build/mingw-gcc-x64/t534-s1-fresh-unit.stdout.log`:

```text
100% tests passed, 0 tests failed out of 334
Total Test time (real) = 20.96 sec
```

The earlier incomplete-tree run (311 passed, 23 not-run executables) is not
used as closure proof.
