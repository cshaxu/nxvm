# T531 S11 Canonical KVM Lib Refresh

## Source And Exactness

- Imported exactly SoftPC commit
  `987d82e5e87559a00b910a21d2c715d4b844dcae`, `src/lib/`, revision
  `shared-t55-s21-p2`.
- Both trees contain 94 tracked Lib files. A whole-directory byte comparison
  returns no difference after import.
- SoftPC's sole dirty worktree path is `assets/binary/softpc.ini`, outside
  `src/lib/`; it was neither read nor copied. The source is project-owned MIT
  material with no independent third-party notice.

## One KVM Consumer Path

- The canonical source replaces `ui-base`, `ui-window` and `ui-console` with
  `kvm-base`, `kvm-window` and `kvm-console`. NXVM deletes all 43 retired Lib
  files left by archive replacement and retains no alias or forwarding root.
- Common, VM, CMake and unit fakes now directly include/link the canonical KVM
  contracts. The renamed KVM contract smoke is part of the existing unit
  aggregate dependency set, so CTest cannot observe an unbuilt target.
- The KVM naming gate finds no retired component spelling in current NXVM
  CMake/source/test/design surfaces. Common remains `common/ui`: that is its
  own product-capability component, not a duplicate Lib presenter corpus.

## Verification

- Canonical directory comparison, Lib manifest, component-DAG, KVM-name and
  Common manifest gates pass.
- x64 and x86 optimized stripped Release builds pass and report their expected
  PE architecture.
- Complete repository-only unit aggregate: 299/299 passed with its configured
  four-way concurrency; real time 19.28 seconds.
- Rebuilt `0531` artifacts exist identically in `build/output` and
  `assets/sessions`: x64 SHA-256
  `F355062DC2ECB6843E1638CA5F1CBEC49ABBDE234644A2D661F69AE8F723EF2C`;
  x86 SHA-256
  `4A4AB1FA44D03C4CC4EEA0B47BD1F1BB38AC62334DD01E2AEA55E0D842316DA2`.

T531 remains open. This exact Lib refresh does not claim the outstanding
two-product CCPU-affine Debug adapter or whole-task execution proof.
