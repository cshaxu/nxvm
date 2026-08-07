# Project Status

## Current Work

**Idle. M5 T263 is closed. The next planned implementation is T264, the
core/VM PC/AT ownership closure. Further CPU instruction-family admissions
require a separate failing corpus and approved packet; they are not the default
queue.**

No subtask is active. Before T264 source work begins, create one complete
packet with the original request, owner, non-goals, applicable rules, focused
evidence, stop conditions, and artifact decision.

## Current Technical Baseline

- **T263 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0263`; static/ownership
  checks and 99/99 CTest cases passed. Artifact `nxvm_0_5_0263.exe` SHA-256:
  `CDECA028180652317C2EDC9C872B8BDB0F49A5465C506A6921BC2A00DB0CAF02`.
- **Core boundary:** T243--T246 retain checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact evidence |
| --- | --- |
| T257 | Bounded 80286 GDT/CPL0 protected-mode baseline; `0.5.0257`, 91/91 CTest. |
| T258 | Bounded 80386 CPL0 paging; `0.5.0258`, 92/92 CTest. |
| T259 | Bounded 16-bit protected privilege and `#GP` IDT delivery; `0.5.0259`, 93/93 CTest. |
| T260 | 80386 TSS I/O-map behavior through the real CPL3 corpus; `0.5.0260`, 94/94 CTest. |
| T261 | Bounded 16-bit-TSS far-JMP task switching; `0.5.0261`, 95/95 CTest. |
| T262 | Exact-8087 finite `m32real` baseline; `0.5.0262`, 97/97 CTest. |
| T263 | 80286 protected-mode `ARPL r/m16,r16`; `0.5.0263`, 99/99 CTest. |

Detailed contracts, commands, and artifact provenance are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S29:** aligned the architecture overview with the task-identity
  artifact rule.
- **M5 Td S30:** retired completed T257 and T260--T263 planning records,
  compacted status, and committed the current T264--T278 forward-governance
  updates. This documentation task changes no runtime behavior or active-task
  scope.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
