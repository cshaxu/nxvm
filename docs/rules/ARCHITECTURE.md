# Architecture Rules

Use the public [Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).
The concrete ntvdm64 component and composition map is
[design/ARCHITECTURE.md](../design/ARCHITECTURE.md).

## Non-Negotiable Invariants

- Dependencies flow only toward more neutral, declared capabilities. No
  dependency cycle, reverse dependency, or peer-to-peer integration may hide
  behind an aggregate target, adapter, callback, global, or test helper.
- Every mutable state, namespace, registry, route, conflict decision, dispatch
  path, and public operation has one explicit owner and one production path.
  A legacy or adapter path cannot become a second truth source.
- A public cross-module interface never exposes mutable internal layout or a
  raw CPU, RAM, port, device, executor, or session pointer. It uses an opaque
  handle, copied snapshot, or bounded typed operation with explicit lifetime
  and failure semantics.
- Only an approved composition root integrates independently owned
  capabilities. Platform adapters report through their contract and do not
  mutate guest state or select implicit current state.
- Product-visible behavior changes require owner approval and before/after
  evidence.

## Source And Research Admission

- Imported or derived source requires provenance, preserved copyright notices,
  authorization, focused verification, and separate review for an independent
  third-party notice. Research, proprietary binaries, and third-party firmware
  do not become a product dependency or release input without an owner-approved
  decision.

Detailed source, license, firmware, and research handling is supporting
material in [etc/governance/source-policy.md](../etc/governance/source-policy.md).
