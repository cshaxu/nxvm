# T524 S8: Portable Closure Audit

## Current Corpus

The tracked `src/lib` corpus has 61 non-manifest files and the manifest has
the same 61 entries: no missing or extra path exists.  `verify_manifest.cmake`
checks the complete selected tree.  The standalone CMake entry selects every
root and now builds/runs `library-neutral-consumer` through public headers;
the workflow runs configure, manifest verification, build, and CTest on both
Windows and Ubuntu.

The full-tree exact-word sweep for `softpc`, `mvdm`, `ccpu`, `cvid`, `bios`,
`rom`, `guest`, `controller`, `profile`, and standalone `vm` is empty.  The
common public contracts contain no native SDK type.  Native Windows types are
confined to `ux/win32` headers that form the explicitly selected native
presenter implementation; no common `ux` header exposes them.

## Root And Consumer Ledger

| Root | Direct dependency boundary | Two neutral public consumers |
| --- | --- | --- |
| `ux` | `base` plus files in the same root | component smokes; `library-neutral-consumer` |
| `host` | `base` plus files in the same root | `host-sync-smoke`; `library-neutral-consumer` |
| `session` | `base` plus files in the same root | `session-state-smoke`; `library-neutral-consumer` |
| `storage` | `base` plus files in the same root | storage smokes; `library-neutral-consumer` |
| `observability` | `base` plus files in the same root | outcome smoke; `library-neutral-consumer` |

No root includes another root.  The product bindings are one presentation
binding (`vm/platform`), one lifecycle binding (`vm/composition/session` plus
its platform adapters), one copied start-result binding, and storage byte/file
uses owned by their existing FDD, HDD, catalog, BYOB and debug owners.  The
S6 candidate-swap sweep confirms FDD/HDD have one shared replacement route;
this audit found no second library implementation or product-side duplicate
of a retained library capability.

## Verification And Open Gate

- Standalone Windows MinGW configure/build: pass; manifest plus neutral
  consumer CTest: 2/2 pass.
- Strict Linux C11 syntax of every selected common/Linux library source plus
  the neutral consumer: pass.  There is no local Linux runtime: WSL is not
  installed.
- The first upstream matrix for `724027ed` is red. Windows selected the
  runner's MSVC compiler, which rejects the C11 atomic contract before the
  library can build; Ubuntu stopped at the text-string manifest verifier.
  S8 corrects both owner-local publish defects: Windows now selects the MSYS2
  UCRT64 GCC C11 toolchain, and the verifier requires LF-only source then
  hashes its raw bytes. The revised matrix must pass before this evidence can
  become S8 acceptance proof.
- Full repository unit: 311/311 pass in 22.36 seconds.
- Full integration was started.  Its first row,
  `compaq-deskpro-386-model-40-1200k.yaml`, failed after 180.09 seconds before
  the expected terminal with FDC command `E6`, phase `2`, and 400 bytes
  remaining.  The run was stopped after this decisive red gate rather than
  consume the remaining serial matrix under a known failing T-level gate.

## Closure Disposition

The library corpus and local unit/standalone evidence are ready for S8
acceptance, but T524 is **not closable**.  The product integration row must be
repaired by its FDC/Model-40 owner and the complete integration gate rerun;
the Linux runtime CI matrix must then report its standalone CTest result.  No
library compatibility path or product-specific workaround is admitted to make
either gate appear green.

The owner approved a committed S8 P1 checkpoint for the completed local library
batch.  That approval preserves this red gate and does not constitute S8 or
T524 acceptance.
