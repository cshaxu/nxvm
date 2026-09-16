# M5 T531 S24 canonical queue/storage corpus refresh

Frozen SoftPC source: `ea7e9823a7e333318ecc4b9645c466f33add51ee`.

The four canonical trees were atomically mirrored from that committed source
and verified byte-for-byte after NXVM consumer adaptation:

- `src/lib`
- `src/common`
- `test/lib`
- `test/common`

NXVM changed only the root shared-test aggregate: it now depends on the
canonical `storage_position`, `session_queue_failure`, and
`input_queue_ownership` executables before invoking CTest. This restores the
single test-production path; it does not copy or fork a shared test.

Verification on 2026-09-16:

- Four-tree identity against the frozen SoftPC source: pass.
- Lib/Common source and test manifests, Lib component dependencies, and Common
  platform/component boundary gates: pass.
- Full repository-only unit suite: 335/335 pass.
- External YAML/asset integration suite: 42/42 pass. The aggregate's fixed
  300-second budget recorded the first 40 passing rows; the remaining IBM 5170
  360K and 1.2M rows were then directly run and both passed.
- Stripped Release artifacts built and deployed for both architectures:
  x64 SHA-256
  `3E71F4024D2C27685E9E56BA4735951790B45F129C37681200EAD775BA909B14`;
  x86 SHA-256
  `8BFBA59E03C13989DE49D2F404547F36E51DA76D04785741492B007C970E5D44`.
- Documentation governance and `git diff --check`: pass.
