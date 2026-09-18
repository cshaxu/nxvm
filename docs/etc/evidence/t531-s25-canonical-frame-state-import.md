# M5 T531 S25 canonical frame/state corpus refresh

Frozen SoftPC source: `17c5b1195e2e7591622fda2d27da4ed79bbff456`.

The four canonical trees were mirrored from that clean committed source and
verified byte-for-byte after NXVM consumer adaptation:

- `src/lib`
- `src/common`
- `test/lib`
- `test/common`

NXVM's only adapter change is adding the canonical `kvm_frame_copy` executable
to the existing root shared-test aggregate. It does not copy a test, retain a
former full-frame path, or create a snapshot shim. The imported optional Common
state-transfer driver hooks remain unbound by NXVM and therefore explicitly
unavailable to product commands until a separately admitted Core-backed owner
exists.

Verification on 2026-09-17:

- Four-tree identity against the frozen SoftPC source: pass.
- Lib/Common source and test manifests, Lib component dependencies, and Common
  platform/component boundary gates: pass.
- Full repository-only unit suite: 336/336 pass. An initial first-build run
  had one 30-second `common.common_machine` timeout; its isolated replay passed
  in 0.15 seconds and the unchanged formal aggregate replay passed in full.
- External YAML/asset integration suite: 42/42 pass. The fixed 300-second
  aggregate recorded its first 39 passing rows; the remaining 5160 360K and
  5170 360K/1.2M rows were then directly run and all passed.
- Stripped Release artifacts built and deployed for both architectures:
  x64 SHA-256
  `83E296530A8B8FCA5BD6486CAE9B8269359159BDC264A4BD38437B98B83AF789`;
  x86 SHA-256
  `363DA9EE2C3785441204E6412F55A49A9CCF9D9583D0D723C6156357CB0D7B1F`.
- Documentation governance and `git diff --check`: pass.
