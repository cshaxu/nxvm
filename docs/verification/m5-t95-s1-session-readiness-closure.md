# M5 T95 S1: Session-Readiness Closure

## Result

T87 through T95 are complete. Every implemented module has session-owned,
thread-local, or explicitly leased mutable state. `vdm/platform` and
`vdm/product` have no production implementation; that is an intentional
absence, not a placeholder session design. `vm/profile` and `vdm/profile`
retain only immutable descriptors and session-owned runtime context.

| Order | Module | Verdict | Primary evidence |
| --- | --- | --- | --- |
| 1 | `core/machine` | Pass | Executor closure and distinct CPU trace contexts. |
| 2 | `vm/machine` | Pass | One core executor authority, atomic control boundary, and two-session isolation. |
| 3 | `vdm/machine` | Pass for current scope | Two DOS-minimal sessions have isolated state. |
| 4 | `core/platform` | Pass | Opaque host-surface contexts and deterministic exclusive leases. |
| 5 | `vm/platform` | Pass | Context-owned renderers, transport/mailbox/mode tests, and explicit terminal/Console leases. |
| 6 | `vdm/platform` | Absent by design | M8/M9 must define it against the core platform contract. |
| 7 | `core/product` | Pass | Caller/invocation-owned debugger tooling plus thread-local wait/debug scopes. |
| 8 | `vm/product` | Pass | Caller-owned NXVM Console context and explicit debug target scope. |
| 9 | `vdm/product` | Absent by design | No speculative CLI or UI was introduced. |
| 10 | `vm/composition` | Pass | Composition owns machine, platform, debugger, and Console lifetimes. |
| 11 | `vdm/composition` | Pass for current scope | DOS-minimal wrapper and presentation are separately allocated and isolated. |

No proven-unused compatibility state remained to delete. The facade scan finds
no production caller of every listed legacy facade, the live-machine scan finds
no legacy storage definition, and the dependency allowlist is empty. The
unified-debug regression was updated to enter the composition-bound
thread-local target scope instead of assuming the removed process-global target.

## Verification

- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed, including
  dependency DAG, executor, facade, live-machine, and mutable-state gates.
- Direct runtime suite passed: trace context, core executor, host-surface
  lease, renderer contexts, platform execution/input/mailbox/mode, Console,
  core and VM debug targets, keyboard provider isolation, wait scope, VDM
  session/presentation/profile, and two VM sessions.
- Retained fixture gates passed:
  `nxvm-vm-dos-prompt-smoke D:\\fdd.img`,
  `nxvm-vm-debug-pause-boundary-smoke D:\\fdd.img`,
  `nxvm-vm-unified-debug-backend-smoke D:\\fdd.img`, and
  `nxvm-product-session-smoke D:\\fdd.img D:\\hdd.img`.
- `build/output/nxvm_0_5_0094.exe` SHA-256:
  `85E450BB5F71C369FDC699609A95096A6433C62AB312DF17C4A46BD3C01D605C`.
- `build/output/nxvm_0_5_0095.exe` SHA-256:
  `72528B9B30A4919B38D88CBC4E5CC544571D2A79142799C7C8098AF4D385EA87`.
  Its retained Console accepted `exit` and displayed the `0.5.0095` banner.

This closes session readiness only. It authorizes no VDM CLI, host integration,
or multi-session product UX; those remain later milestone work.
