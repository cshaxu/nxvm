# Agent Instructions

Before changing a product, read its guide (`docs/nxvm/README.md` or
`docs/mynes/README.md`), its active packet, and every authority named by its S
brief. Read the shared rules under `docs/rules/` that the product guide names.

## Authority

- The authorities selected by the Task Reading Set are the project
  authorities. This file is an execution pointer and must not be used as a
  second architecture, policy, or status source.
- Sibling repositories are read-only unless an approved task explicitly
  authorizes an import into this repository.
- Read the selected product's source-and-research policy before importing,
  deriving, or researching source, firmware, media, ROMs, Microsoft components,
  or third-party code. Do not commit protected media, firmware, ROMs,
  Microsoft binaries, machine-local paths, or unreviewed third-party code.

## Execution

- `docs/<product>/states/CURRENT.md` holds that product's active packet.
  Follow `docs/rules/EXECUTION.md` for task allocation, evidence, artifacts,
  and closure.
- Apply the relevant architecture, coding, source, and documentation rules;
  record any owner-approved exception with the task evidence.
- Preserve unrelated changes and avoid destructive Git operations.
