# M5 T527 S9: Reusable Common Corpus Closure

## Scope and ownership result

The exact S9 corpus is revision `common-t527-s9-p1`, recorded by
`src/common/MANIFEST.sha256`.  `src/common/CMakeLists.txt` is its sole source
list and can be configured directly.  Root CMake consumes that same list via
`add_subdirectory(src/common)`; it no longer has a second Common source list.

The independent components and their intentional dependencies are:

- `common/machine`: public `lib/host` plus its own opaque driver ABI.
- `common/session`: public `lib/host` and `lib/ui-base` only.
- `common/ui`: public `lib/host`, `lib/ui-base`, `lib/ui-console`, and
  `lib/ui-window` only.
- `common/xasm32`: public `lib/types` only.
- `common/debug`: public `lib/types`/`lib/storage` plus Common xasm32 and the
  bounded Common-machine paused-debug API.

The corpus verifier checks every Common C/header and CMake file, validates the
manifest, and rejects Core, VM, old type facade, or native platform includes.
It passed both as a standalone build dependency and through the root build.

`test/common/common_adapter_conformance.c` instantiates two independent neutral
adapter configurations (distinct run IDs and opaque marker values).  Each
uses public Common machine, session, and UI APIs; it has no NXVM/Core/product
data or adapter.  It proves copied debug execution, ordered lifecycle delivery,
monitor-line delivery, and neutral UI construction.

## Integration-route correction

Full integration exposed four stale test routes.  The shared Console test fake
was restored as a standard-C host fake without the old project type facade.
Three integrations that directly started the deprecated control loop now use
the same public `vm_machine_start`, pause, step, resume, and stop path as the
product.  Their asynchronous step assertions wait for the requested pause
reason rather than a previously signaled pause event.  No Core or product
behavior changed; the parallel test routes were removed.

## Verification

- Standalone Common configure/build: all five Common static libraries built;
  manifest and Lib-only corpus verifier passed.
- Neutral Common conformance: `unit.common-adapter-conformance` passed.
- Repository-only unit: **299/299 passed** in 20.45 seconds.
- External YAML/asset integration: **42/42 passed**, 0 failed, 515.06 seconds.
- Specialized ownership/source/documentation gates passed, including the
  corrected subdirectory-aware T344/T345 source ledger: 239 rows, with two
  safely separable production targets.
- `git diff --check` passed before implementation P1 and again at governance
  closure.

## Artifact identity

Both Release artifacts are deployed byte-identically to `build/output` and
`assets/sessions`:

| Architecture | Bytes | SHA-256 |
| --- | ---: | --- |
| x64 | 1,210,986 | `980D6B786EC4C541E5C355917EC4F620D4FEAD6FD1B51AEA62E1BB72FBA156D7` |
| x86 | 1,362,568 | `2A0DF692BCCF9BE6A46CC476D09C01C55D6CFDDE9023FF7119C90129091F2940` |

No external asset was copied, modified, or committed.
