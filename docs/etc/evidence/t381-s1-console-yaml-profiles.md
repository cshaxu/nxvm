# T381 S1: Console YAML Session Profiles

`M5:T381:S1:CONSOLE-YAML:OK`

## Result

NXVM now starts with an empty session manager, retains its banner and help
prompt, then invokes the ordinary profile chooser.  YAML files are frozen at
startup, displayed by sorted file name, and opened as selected sessions.  No
profile YAML `name` is used.  Cancel/no-file leaves a true zero-session
Console; only global session commands, `HELP`, and `EXIT` operate there.

The strict catalog accepts the v1 mapping/scalar subset, isolates invalid
files, resolves relative media against the configuration directory, retains
absolute paths, rejects fixed Model-339 CPU/memory/HDD conflicts, and passes
generic CPU/FPU/memory values to VM composition.  Console configuration
commands have been retired; stopped-session floppy insert/eject remains the
sole mutable media operation.  HDD media remain creation-time only.

## Ownership and proof

- The approved core exception is limited to the generic session manager:
  no implicit entry, an empty count is valid, final close is valid, and IDs do
  not repeat.  No core machine source changed.
- VM product owns discovery, parsing, menu and command policy; composition
  owns applying frozen options.
- `vm-product-console-lifecycle-smoke` proves banner-to-YAML selection and
  selected Model-339 composition.
- `vm-product-session-catalog-smoke` proves accepted entry, invalid fixed
  profile rejection and invalid-file isolation.
- `core-product-session-manager-smoke` proves the empty lifecycle.

## Verification and artifact

The fresh GCC build ran `run-current-smokes` and
`verify-current-specialized-gates` successfully, including 250 current tests.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0381.exe` | `514A5FFED31B4919A1E215AD4E6587F910846216148F7520C5334EEA8500FD80` |

No ROM, guest media, external parser, or machine-local asset was committed.
