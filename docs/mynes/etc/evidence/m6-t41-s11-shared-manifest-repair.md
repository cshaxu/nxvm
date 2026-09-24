# M6 T41 S11: Shared Manifest Integrity Repair

## Result

All six Shared corpus manifests now self-verify:

- `src/lib/MANIFEST.sha256`
- `src/common/MANIFEST.sha256`
- `src/x86/MANIFEST.sha256`
- `test/lib/MANIFEST.sha256`
- `test/common/MANIFEST.sha256`
- `test/x86/MANIFEST.sha256`

The validators stop at the first mismatch. An exhaustive read-only comparison
found 28 stale SHA-256 entries across five manifests; `src/x86` was already
correct. This delivery replaces those hashes only: no source, test behavior,
ABI, target, or product path changed.

## Verification

- All six manifest validators pass.
- Shared dependency verification passes.
- Shared labelled unit suite: 56/56 passed.
- MyNES documentation governance passes.
- Actual diff review confirms only 28 manifest hashes plus this S11 status/evidence.
