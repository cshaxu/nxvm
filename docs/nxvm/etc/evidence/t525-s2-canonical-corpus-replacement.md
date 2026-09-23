# T525 S2 Canonical Corpus Replacement

## Result

NXVM `src/lib` is the exact canonical SoftPC corpus frozen by S1:

- source: owner-controlled SoftPC `src/lib` at `140af7ab9b988ef5613c1702036f387b282603c3`;
- frozen manifest SHA-256: `665CA0E5AD9A9C9F3B5CB105E3082E3FDDECD80A553A75BD94236FC7A8DDEC37`;
- 70 tracked paths compare byte-for-byte equal with `git diff --no-index`;
- NXVM's copied `MANIFEST.sha256` verifier passes.

The old unified `src/lib/ux` corpus is gone.  Its replacement is the canonical
`base`, `host`, `storage`, `ux-base`, `ux-window`, and `ux-console` corpus.
No NXVM compatibility wrapper or copied SoftPC application/product source was
added.  The two static CMake ownership checks now name the canonical split
source paths and native target owner.

## Verification

The standalone canonical Windows configuration was freshly generated in
an isolated `t525-s2-lib` build tree and built these roots successfully:

`base-console`, `ux-base`, `ux-window`, `ux-console`, `host-sync`, and
`storage-medium`.

Its `lib-verify-manifest` target also passed.  The first command attempted to
build an INTERFACE aggregate (`lib-neutral-corpus`), which is intentionally not
a Ninja build target; the concrete roots above are the actual build proof.

## Transfer

S3 owns the single NXVM product migration: it must replace deleted
`ux_presenter` callers and old UX-only test declarations with the canonical
Window/Console/broker contracts.  That work must not alter this corpus.
