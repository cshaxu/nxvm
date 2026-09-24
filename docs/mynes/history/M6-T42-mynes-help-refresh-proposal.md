# M6 T42 MyNES Help Refresh

## Objective

Converge the cooked-monitor help text, its focused product assertions and the
versioned MyNES delivery pair after the owner cleaned the help presentation.

## Boundary

The only Shared change is a root `CMakePresets.json` target-name update for the
MyNES build routes. MyNES changes are limited to App command presentation, its
owned test, product build/version declaration, versioned artifacts and MyNES
task records. Shared component source, NXVM, ROM inputs, command semantics and
Common/Lib contracts are excluded.

## Acceptance

- `help` presents the owner-approved spelling, aligned command list and
  unambiguous running-game controls.
- The product test asserts the changed user-visible text; command behavior is
  otherwise unchanged.
- Release x64 and x86 builds publish
  `assets/mynes/mynes_0_0_0042_{x64,x86}.exe`.
- Complete MyNES test suites pass for both architectures, documentation
  governance passes, task evidence is recorded, and the worktree is clean
  after its one Shared configuration commit and one MyNES delivery commit.
