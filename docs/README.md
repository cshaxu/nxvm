# Documentation Guide

Read current documentation in this order:

1. `architecture/overview.md` defines product shape and system terms.
2. `architecture/module-layout.md` defines source ownership, dependency rules,
   and root composition.
3. `architecture/contracts.md` is the sole current interface-design authority.
4. `requirements/` defines externally observable product behavior only.
5. `planning/roadmap.md`, `planning/status.md`, and
   `planning/execution-policy.md` define execution state and gates.

`governance/`, `source-policy.md`, `project-constitution.md`,
`redistributability.md`, and `invasive-integration.md` define stable project
policy. `verification/`, `provenance/`, `baselines/`, `build/`,
`compatibility/`, `evidence/`, and `fixtures/` record acceptance inputs or
evidence. `research/` and `references/` are non-default research inputs.

`history/` preserves earlier plans and evidence. It is non-authoritative and
must not be used to define a forward module boundary, interface, or milestone
scope. Historical task records remain in `planning/subtasks/` so their tracked
links remain stable; they do not override the current planning documents.
