# M5 Td S173: Shared Three-Scope Boundary

## Objective

Define the repository's only task scopes as `NXVM`, `MyNES`, and `Shared`.

## Decision

- An App-scoped task modifies only its own App's source, configuration,
  documentation, tests, tools, artifacts, and media declarations.
- `Shared` owns all six common source/test components: `lib`, `common`, and
  `x86`; it also owns shared configuration and shared governance documents.
- A Shared packet names every App consumer affected by a shared change and
  requires the applicable verification for each.

## Exit Evidence

`docs/rules/EXECUTION.md` contains only these three scopes and its isolation
rule leaves no App-to-App or App-to-Shared edit exception.
