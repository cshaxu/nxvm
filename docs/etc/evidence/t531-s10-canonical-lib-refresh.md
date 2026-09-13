# T531 S10 Canonical Lib Refresh

## Source And Exactness

- Imported exactly SoftPC commit
  `2ea35ce87bb538a6cc520be1353641a99a94d427`, `src/lib/`, revision
  `shared-t55-s20-p2`.
- Both trees contain 93 tracked Lib files.  A whole-directory comparison after
  import returns no difference.  SoftPC's only dirty path is
  `assets/binary/softpc.ini`, outside `src/lib/`; it was not read or copied.
- The imported corpus remains project-owned MIT material.  It includes no
  application, MVDM, test, firmware, media or binary source.

## NXVM Integration

The current NXVM Common/VM consumers compiled directly against the imported
public contracts.  No consumer source adaptation, Lib fork, compatibility
wrapper or second route was required.  Lib manifest and component-dependency
checks pass; public interfaces retain no product or native SDK vocabulary.

## Verification

- x64 and x86 optimized stripped builds pass and report the expected PE
  architecture.
- Complete repository-only unit suite: 299/299 passed, `ctest -L unit -j 8`.
- Rebuilt `0531` artifacts exist in both required locations:
  `nxvm_0_5_0531_x64.exe` SHA-256
  `E7B4ECD6631B69D449EA4306F1399A06607D89131A8921C2DC720E5BDF3A80C2`;
  `nxvm_0_5_0531_x86.exe` SHA-256
  `E051AF351059577FC034C648802BF3A834BC5A0AD65DBAE386452BA3716A0CE0`.

T531 remains open.  This refresh does not claim the future two-product
CCPU-affine Debug adapter or whole-task execution proof.
