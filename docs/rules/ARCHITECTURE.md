# Architecture Rules

Use the public [Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).
The current ntvdm64 component, dependency, and contract design is
[design/ARCHITECTURE.md](../design/ARCHITECTURE.md) and
[design/CONTRACTS.md](../design/CONTRACTS.md).

- `core` remains product-neutral; `vm`, `mantle`, `dos`, and `vdm` own only
  their respective component behavior and compose only through approved roots.
- Every mutable state, namespace, registry, route, conflict decision, dispatch
  path, and public operation has one explicit authority. Legacy or adapter
  paths cannot form a second production truth source.
- Product-visible behavior changes require owner approval and before/after
  evidence. Source, host, firmware, and research boundaries are governed by
  [SOURCE.md](SOURCE.md).
