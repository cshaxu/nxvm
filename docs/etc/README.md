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
| `evidence/t333-s1-interactive-input-inventory.md` | Verification owner | T333 S1 retained Console/debugger direct-input inventory and S2 boundary. Retire into the T333 history record when the task closes. |
| `evidence/t333-s2-interactive-input-repair.md` | Verification owner | T333 S2 owner-local interactive-input repair and static/build evidence. Retire into the T333 history record when the task closes. |
| `evidence/t333-s3-interactive-input-proof.md` | Verification owner | T333 S3 owner-separated EOF/allocation-failure smoke design and expected proof. Retire into the T333 history record when the task closes. |
| `evidence/t335-s1-xasm-contract-inventory.md` | Verification owner | T335 S1 complete xasm public-entry, caller, capacity, failure-publication, and S2 migration inventory. Retire into T335 history at closure. |
| `operations/` | Execution, source, and release owners | Toolchain, policy, templates, release procedure, and closure material. |
| `research/` | Source owner | Provenance, external references, and non-product research. |
| `history/` | Documentation owner | Read-only legacy and M5 migration/roadmap evidence. |

Adding a new supporting document requires owner approval, an exact-file index
row or a declared indexed subtree, and a stated promotion, merge, or retirement
condition. The governance gate verifies that every supporting Markdown file is
covered by one of those index forms. `etc/` is not a shortcut for adding
another principal document.
