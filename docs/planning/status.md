# Project Status

## Current Work

**Idle. M5 T263 S6 is closed. The next planned implementation is T264, the
core/VM PC/AT ownership closure. Further CPU instruction-family admissions
require a separate failing corpus and approved packet; they are not the default
queue.**

No subtask is active. Before T264 source work begins, create one complete
packet with the original request, owner, non-goals, applicable rules, focused
evidence, stop conditions, and artifact decision.

## T263 S6 Packet

### Original Request And Scope

Correct the protected 16-bit IDT error code so synchronous CPU exceptions do
not set the external-event (`EXT`) bit merely because `is_exception` is true.
The sole core CPU executor retains original-fault diagnostics when an exception
delivery attempt itself fails; this subtask does not add double-fault handling
or a second diagnostic channel. It extracts one IDT error-code helper used by
all synchronous gate-validation failures and adds a valid-but-not-present
vector-13 gate corpus expecting `#NP(0x006a)`.

No hardware IRQ delivery, 32-bit gate, generic exception-delivery, firmware,
platform, Console, debugger, or boot behavior may change. Stop if correct
coverage requires changing original-fault retention, double-fault behavior, or
an additional executor. Rebuild the same T263 artifact only after focused and
current GCC/CTest gates pass.

### S6 Closure

`_ser_idt_error_code()` now encodes every admitted IDT validation failure as a
synchronous CPU event with `EXT=0`; `is_exception` no longer alters the code.
The owned corpus uses a valid but not-present vector-13 gate and proves
`#NP(0x006a)`. An internal `#GP` whose gate cannot be entered still retains
the original terminal fault by existing contract, so the test locks the shared
helper rather than falsely reporting an undelivered nested `#NP`. Focused
smoke and current GCC gates passed 99/99 CTest. The clean-rebuilt
`nxvm_0_5_0263.exe` SHA-256 is
`5DA40651469CA7FFA4A2C38A060F458D4528274EBBDFBFAC74AA58DAEA766B82`.

## T263 S5 Packet

### Original Request

Correct the protected IDT path so a valid but not-present IDT gate or target
code descriptor reports `#NP` with its architected IDT/selector error code,
while malformed types remain `#GP`. Make the T259 privilege-stack transition
prevalidate the TSS result, target SS cache, target entry, descriptor writes,
and every frame write before it changes CPL, SS, SP, CS, FLAGS, or guest stack
contents. Add negative corpus coverage for both failure classes and for a
failed outer-stack frame leaving the pre-transition privilege state intact.

### Scope, Rules, And Stop Conditions

The sole owner is the existing `core_machine` CPU executor. The patch may
touch its 16-bit protected interrupt-gate helpers, the owned core corpus, and
the T263 evidence. It must not add an executor, firmware/platform shortcut,
or second CPU state. It must not admit 32-bit gates, task gates, or IDT
delivery beyond the retained `#GP` and requested `#NP` routes, or alter NXVM
Console, debugger, or boot behavior.

The preflight may build copied candidate CS/SS caches and test the same core
memory route that commit will use. It must not mutate CPU state or target stack
before all checks succeed. Stop and split if the correction requires 32-bit
frames, general exception delivery, task switching, or a new memory path.

### Evidence And Artifact Decision

The focused `core-machine-protected-privilege-smoke` must prove a not-present
gate, a not-present target code segment, and a deliberately too-small TSS
stack. The latter must retain user CPL/CS/SS/SP. Then run current GCC/CTest
gates, rebuild
`nxvm_0_5_0263.exe`, record its new SHA-256, and close S5 before T264 starts.

### S5 Closure

The 16-bit path now distinguishes malformed gates/target code (`#GP`) from
valid not-present gates/target code (`#NP`) with the IDT or selector error
code. The retained bounded protected exception handoff now delivers `#NP` as
well as `#GP`, still in the sole core executor. A privilege transition first
prepares candidate CS/SS caches, validates descriptor writes, target fetch,
and each frame word through core memory; only then does it commit CPL, SS/SP,
the frame, and CS. The owned corpus covers both `#NP` paths and a short TSS
stack retaining the original CPL3 CS/SS/SP. Focused smoke and current GCC
gates passed 99/99 CTest. The clean-rebuilt artifact
`nxvm_0_5_0263.exe` SHA-256 is
`D274A5C40D2045E4B88D0ABC3FDCD1C88C520864A12588E553B9FDB57EB68F9D`.

## T263 S4 Packet

### Original Request

Correct the architecture contract so it matches the retained real-mode IVT and
bounded protected-mode 16-bit IDT execution paths. Move the stale
`TODO(Low, T263)` descriptor-vocabulary note out of `cpu.h` into the durable
TODO ledger with no false task identity.

### Scope, Evidence, And Stop Conditions

This is documentation and source-comment governance only: no CPU, firmware,
platform, build, test, artifact, or user-visible behavior may change. The
contract must say that both software-interrupt routes remain inside the one
core executor, while hardware IRQ delivery remains a separate PIC path. The
ledger entry must state that a future vocabulary-only admission preserves
layout and debugger behavior.

The similar-issue sweep searches tracked `docs`, `src`, and `TODO.md` for the
obsolete “always guest-IVT” claim and `TODO(... T263)` vocabulary marker.
Expected closure is one corrected contract, no stale marker, one ledger owner,
`git diff --check`, and no artifact because no runnable path changes. Stop if
the correction reveals a second interrupt executor, firmware shortcut, or a
need to change runtime semantics; those require a separately approved task.

### S4 Closure

`contracts.md` now states the real-mode IVT route and the bounded protected
16-bit IDT interrupt-gate route without conflating either with hardware IRQ
delivery. Source inspection confirms the latter remains in the core executor's
`_ser_int_protected_16()` path; no provider, firmware portal, or second
executor was found or changed. The stale `cpu.h` marker is deleted and the
descriptor-vocabulary cleanup is now a durable `TODO(Low)` entry with its
layout/debugger preservation condition. The required repository searches found
no remaining obsolete contract claim or inline vocabulary marker. `git diff
--check` passes. No executable or artifact was produced because this subtask
does not change a runnable path.

## Current Technical Baseline

- **T263 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0263`; static/ownership
  checks and 99/99 CTest cases passed. Artifact `nxvm_0_5_0263.exe` SHA-256:
  `5DA40651469CA7FFA4A2C38A060F458D4528274EBBDFBFAC74AA58DAEA766B82`.
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
