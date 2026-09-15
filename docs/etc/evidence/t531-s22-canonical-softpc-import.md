# T531 S22 Canonical SoftPC Import Evidence

## Frozen Source And Corpus Identity

S22 imports the clean committed SoftPC revision
`fa14c62bd34182bf9930988e25296bfb7d5be646`.  After the import, each of the
following NXVM trees is byte-identical to its SoftPC counterpart:

- `src/lib`
- `src/common`
- `test/lib`
- `test/common`

There is no NXVM patch inside those four trees.  In particular, the former
NXVM-only Common Debug provider and session reconciler are absent because the
canonical corpus folds their behavior into its current public boundaries.

## NXVM-Only Adaptation

- Root CMake includes all imported Shared test targets in the repository unit
  aggregate, so a normal NXVM unit invocation builds and executes the whole
  imported corpus.
- The integration YAML boundary verifier recognizes
  `test/integration/support/session_yaml.c` as the one provider which parses a
  YAML request and creates the machine.  The verifier still rejects direct
  machine construction in every integration consumer and still rejects media
  copying.
- NXVM's Debug and deferred-ownership gates now reference the canonical
  `command_runtime.h`, `command.c`, and current Session source set rather than
  retired canonical filenames.

## Verification

- Corpus equality: all four `git diff --no-index --quiet` comparisons return
  success.
- `ninja -C build run-unit-tests -j8`: **328/328 passed** in 17.54 seconds.
- Lib/Common manifests, corpus verification, dependency-DAG verification and
  the 67-current-specialized-gates suite pass.
- Stripped Release artifacts build for both x64 and x86 and are copied to both
  `build/output` and `assets/sessions`.

## External Integration Result

The complete external integration invocation did not pass. Before the
repository-wide 300-second deadline stopped it, these rows failed or timed out:

- `integration.vm-dos-mem-fault-smoke`
- `integration.vm-dos-keyboard-smoke`
- `integration.vm-model40-console-s20-smoke`
- `integration.vm-app-console-lifecycle-smoke`
- `integration.vm-profile-floppy-boot-matrix.compaq-deskpro-386-model-40-1200k.yaml`

S22 makes no pre-existing attribution for any row. T531 remains open for the
separate two-product execution gate and any justified NXVM adapter repair.
