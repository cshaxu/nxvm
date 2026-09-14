# M5 T531 S14 Raw-Console Monitor Ownership

## Defect and owner

When a Console-mode guest owned the host Console, each guest key also printed
`Console> `. Common Session was correct: after every KVM input it asked the
injected product callback whether the cooked monitor was Current, and supplied
`current=false` while the raw guest Console was bound. The sole NXVM callback
ignored that fact and armed another prompt anyway.

P1 `9e7b4eb2` changes only the NXVM product callback:

- `current=false` now returns with no prompt request;
- `current=true` retains the existing product prompt, including the Debug
  prompt selection.

No Lib/Common source, Console broker, guest input route, public ABI or YAML
was changed. The retained path is one host broker, one Common KVM-input event
route, and one product callback that converts the Common ownership fact into
product prompt text.

## Similar-issue sweep

The complete `src/` and `test/` search found one production
`note_monitor_current` provider and one production `arm_prompt` writer: the
corrected NXVM callback. Monitor output itself remains owned by Common Session
after a provider returns a copied result. No parallel callback or raw-Console
prompt route remains.

## Proof

| Check | Result |
| --- | --- |
| Focused `common.session_frame` | passed; raw VM Console state reaches callback as `current=false` and the result remains unarmed |
| Focused `unit.vm-product-console-smoke` | passed |
| Full repository-only x64 unit suite | 325/325 passed |
| Current specialized gates | 118/118 passed |
| x64 stripped Release artifact | built and copied to both required locations; SHA-256 `4C8DF834FAAA347933EC43B85974E5833CE99261D9D5C36C29E181D3A6A3CE2F` |
| x86 stripped Release artifact | built and copied to both required locations; SHA-256 `628D08300B2D6ADE234F5BC0C160F56B1DA71BE3F7462335A85B57A8A9A54AF5` |
| Documentation governance and `git diff --check` | passed |

## Disposition

S14 is accepted by P2. T531 remains open for its later package-level,
two-product convergence acceptance.
