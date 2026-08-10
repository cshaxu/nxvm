# Supporting Documentation Index

The fixed documentation topology retains this owner-approved area for supporting detail
that does not fit the principal `rules/` and `design/` authorities. A supporting
document may explain evidence, implementation detail, research, or a retired
decision, but it cannot redefine current architecture, source layout, product
UX, roadmap, rules, status, queue, or debt. Conflicts are resolved by the
principal documents named in [docs/README.md](../README.md).

| Current category | Owner | Purpose |
| --- | --- | --- |
| `architecture/` | Architecture owner | Current detailed machine, profile, layout, and contract rationale. |
| `requirements/` | Product and profile owners | Future DOS, NXVDM, and profile admissions. |
| `evidence/` | Verification owner | Capability, verification, baseline, and fixture evidence. |
| `operations/` | Execution, source, and release owners | Toolchain, policy, templates, release procedure, and closure material. |
| `research/` | Source owner | Provenance, external references, and non-product research. |
| `history/` | Documentation owner | Read-only legacy and M5 migration/roadmap evidence. |

Adding a new supporting document requires owner approval, an exact-file index
row or a declared indexed subtree, and a stated promotion, merge, or retirement
condition. The governance gate verifies that every supporting Markdown file is
covered by one of those index forms. `etc/` is not a shortcut for adding
another principal document.
