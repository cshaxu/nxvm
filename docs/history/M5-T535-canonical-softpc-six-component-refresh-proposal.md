# Canonical SoftPC Six-Component Refresh

## Goal

Refresh NXVM's shared `lib`, `common`, and `x86` source-and-test corpora from
one committed SoftPC revision, then adapt only NXVM-owned callers. The six
trees must again be byte-for-byte identical to the pinned SoftPC source:

```text
src/lib      src/common      src/x86
test/lib     test/common     test/x86
```

## Frozen Upstream And Scope

The source is project-owned MIT material at SoftPC commit
`1c5a47146dd4fd87b09423b7a7b960becb50cd67`. Its dirty media files are neither
read nor imported. The source policy permits this owner-authorized source
import; no firmware, guest media, binary, App, MVDM, or third-party source is
in scope.

The S1 inventory finds an exact x86 corpus and these upstream changes:

- Lib adds a neutral, bounded PCM `audio` stream component, platform leaves,
  type declarations, manifest/build registration and three owner-local tests.
- Common corrects a paused-state rule: a missing Window may be created only
  while the machine is running, with a corresponding state-matrix test.
- Manifests and source-only build descriptions change only to account for the
  preceding owner changes.

## Plan

1. **S1 — freeze and audit the six-tree import.** Record the exact upstream
   commit, provenance, file/hash inventory, build-DAG consequences and every
   NXVM adaptation required outside the six trees. Do not copy source yet.
2. **S2 — exact corpus replacement.** Replace all six trees from the frozen
   committed revision using one canonical copy operation. Preserve notices,
   manifests and independent source-only build/test entry points. Do not make
   NXVM-specific edits in a shared tree.
3. **S3 — NXVM product adaptation.** Update only NXVM App/Core build and
   consumer wiring needed by the imported public contracts. Audio remains
   unconnected unless NXVM has one explicit product owner and a later approved
   use; importing its neutral corpus does not invent a PC-speaker route.
4. **S4 — convergence and release proof.** Verify exact six-tree equality,
   independent Lib/Common/x86 suites, NXVM's full unit and integration gates,
   and current optimized x64/x86 artifacts for every runnable fixed product.

## Invariants And Non-goals

- SoftPC is the canonical source of the six trees; NXVM-specific edits belong
  outside them. A later shared change is made upstream first, then imported.
- `lib` remains platform/C-runtime capability only; `common` remains neutral
  coordination; `x86` remains the optional x86 frontend. None gains profile,
  firmware, ROM, guest-media, App or Core ownership.
- This task neither enables audio in NXVM nor changes device timing, guest
  sound, firmware/media assets, machine profiles or user-visible policy.
- No external repository becomes a build/runtime dependency. The committed
  corpus is copied into NXVM and independently buildable here.

## Completion Standard

The six NXVM trees match the frozen SoftPC commit exactly by relative path and
normalized content hash; all manifests verify; no consumer uses retired local
contracts; applicable independent and full NXVM tests pass; and all runnable
products have verified stripped Release x64/x86 T535 artifacts. Any upstream
contract that cannot be adapted without changing NXVM product behavior is
recorded and returned for owner direction rather than patched in the shared
corpus.
