# M2 T9 S6 Host And Storage Evidence

S6 converts the remaining reliability rows into direct receivers. All listed
executables passed on the current Windows x64 and x86 build trees.

| Ledger row | Direct receiver | Proven outcome |
| --- | --- | --- |
| ROM allocation/read/close boundary | `test/core/media_failure_contract_smoke.c` | The Core storage boundary reports injected `NO_MEMORY`, I/O and capacity failures without replacing the accepted cartridge; unsupported writable media is rejected. The injected I/O result is the public storage abstraction's read-or-close failure result. |
| HOST construction and producer/sink retention | `test/common/composition_smoke.c`, `test/common/common_machine_smoke.c` | Invalid construction options reject with a null output; window/console/broker teardown failures retain their owning UI and event context; state/frame sink failure enters terminal host error without a dangling callback. |
| HOST native wait and join failure | `test/common/machine_wait_smoke.c` | Fault, invalid and cancelled native waits are terminal and bounded; reset/stop races are covered; a failed worker join retains the worker and all referenced machine fields until a later successful shutdown. |
| Terminal cooked-console path | `test/common/session_monitor_smoke.c` | A failed monitor wait is terminal, creates no second reader or request, and preserves single-reader/prompt ownership. App `quit` is covered by the retained production command result tests. |

These Common tests are unchanged adopted source. They are valid HOST receivers
because the MyNes App/Core composition invokes the same public Common machine,
UI and session contracts; S6 reruns their exact executables rather than treating
an aggregate suite as proof.
