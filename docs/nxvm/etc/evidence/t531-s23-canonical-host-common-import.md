# M5 T531 S23 canonical shared-corpus refresh

Frozen SoftPC source: `549ed44eb82e833e649ec3d2e3178c5bfeafdfab`.

The four canonical trees were mirrored from that committed source and verified
byte-for-byte after NXVM consumer adaptation:

- `src/lib`
- `src/common`
- `test/lib`
- `test/common`

NXVM's only adapter change is direct migration from the replaced Host names to
the imported `base` public contract; events retain their prior manual-reset
semantics.  No alias or compatibility target was retained.

Verification on 2026-09-15:

- Lib/Common source and test manifests: pass.
- Full repository-only unit suite: 332/332 pass.
- External YAML/asset integration suite: 42/42 pass, including 5160, 5170 and
  DeskPro Model 40 boot matrix rows.
- Stripped Release artifacts built and deployed for both architectures;
  recorded SHA-256 prefixes are x64 `DA393C4431FB7E5F0013E69B03FADC6E`
  and x86 `28B8BA0567AC92092D50077E9CC8891CF51E5541A563290C`.
- `git diff --check`: pass.
