# M1 T2: Shared Refresh Import

T2 refreshes the prior T1 S2 snapshot without adding a product capability. The
owner admitted it on 2026-09-20 with root `test/` as the only test location.

## Frozen Coverage And Result

SoftPC Git commit `1fe946a242df9356f84a0b4e5961ee4266930810` is the sole
source. The finite coverage universe is 180 files: 91 under `src/lib`, 23
under `src/common`, 46 under `test/lib`, and 20 under `test/common`. The
[per-unit ledger](../etc/provenance/softpc-shared-units.csv) records each blob,
destination and SHA-256 disposition; every unit is unchanged and matched.

The refresh retains no `src/test/` directory. MyNes-owned root composition now
adds `test/lib` and `test/common`, while its standalone verifier locates test
roots outside `src`. All Lib/Common files themselves are byte-identical to the
frozen source. The current upstream snapshot removes Common's inherited x86
debug/xasm implementation; no local replacement, optional component or product
debug policy was introduced.

## Verification

Windows MinGW GCC 16.1.0 configured strict C11 Release builds on x64 and x86.
Both aggregate suites passed 62/62. On each architecture, standalone `src/lib`
and `src/common` built successfully, standalone `test/lib` passed 44/44, and
standalone `test/common` passed 18/18. The exact summary is retained in
[T2 test evidence](../etc/evidence/m1-t2-tests.csv). The dependency-layout
self-test is the only intended longer probe: it passed in 35.38 seconds on x64
and 34.56 seconds on x86, under a root-composition 180-second limit; regular
test cases retain their upstream 30-second properties.

Manifest, Common corpus and dependency-negative checks are included in the
aggregate and independent suites. Documentation governance and whitespace checks
also pass. No ROM, firmware, guest media, source from a mutable sibling
worktree, or sibling write occurred.

## Coordinator Acceptance

The coordinator reviewed implementation commit `f2f5a97`: all four source/test
roots match the frozen ledger; root-only CMake and verifier changes are limited
to the owner-required `test/` layout and bounded self-test budget; manifests,
dependency checks and complete suites passed. T2 S1 and T2 close with no
remaining in-scope work. The queued neutral-contract candidate remains separate.
