# Documentation Rules

## Authority Boundaries

`docs/rules/` is the only shared documentation corpus and owns shared
governance only. Each product owns its own `docs/<product>/README.md`,
`design/`, `states/`, `proposals/`, `history/`, and `etc/` tree. Link to the
shared rules instead of duplicating an authority.

| Location | Owner | Purpose |
| --- | --- | --- |
| `docs/rules/` | Repository | Shared architecture, coding, documentation, and execution rules. |
| `docs/nxvm/` | NXVM | NXVM design, task state, proposals, history, evidence, and source policy. |
| `docs/mynes/` | MyNES | MyNES design, task state, proposals, history, evidence, and source policy. |
| `docs/myraidenii/` | MyRaidenII | Independent design, task state, queue, history, evidence and source policy; shared rules are linked, not copied. |
| `src/{lib,emulator,x86-debug,x86-core,x86-ibmpc,x86-nxvm}` and matching `test/` roots | Repository | Shared implementation and tests, subject to each package's documented scope. |
| `src/app-<product>` and matching `test/`, `tools/`, `assets/<product>` roots | Product | Product implementation, tests, tools, and versioned artifacts. |
| `src/app-myraidenii`, `test/app-myraidenii`, `assets/myraidenii` | MyRaidenII | Product-owned App roots; they are not shared component packages. |

There is one active packet per product, in
`docs/<product>/states/CURRENT.md`. A packet may declare NXVM, MyNES,
MyRaidenII and/or Shared change targets; for a cross-target S it lists every
affected consumer, boundary and verification. Its product-local location is task coordination,
not implicit authorization to edit another target. It does not create a root
task queue or authorize an undeclared target.

Historical records preserve their original paths and terminology where needed.
They are evidence, not current authority. Product `etc/README.md` files index
supporting records; supporting material cannot define current rules, task state,
or architecture.

## Required Structure

Each product documentation root contains exactly these principal directories:
`design/`, `states/`, `proposals/`, `history/`, and `etc/`; it does not carry a
private `rules/` copy. Product guides link the shared rules with `../rules/`.
The root MIT `LICENSE` is the sole license text. Product or shared provenance
and third-party notices belong in `THIRD_PARTY_NOTICES.md` and product `etc/`.

## Governance Check

Run the lightweight structural check for each affected product:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File tools/shared/Verify-DocumentationGovernance.ps1 `
  -RepositoryRoot . -Product nxvm
```

Replace `nxvm` with `mynes` or `myraidenii` for the selected product. This gate
verifies topology and essential reading-set links; task closure still requires the packet's actual
change review and product-specific verification.
