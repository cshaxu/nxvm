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
| `evidence/t335-s2-xasm-migration.md` | Verification owner | T335 S2 bounded facade/caller migration and artifact verification evidence. Retire into T335 history at closure. |
| `evidence/t335-s3-xasm-contract-proof.md` | Verification owner | T335 S3 deterministic capacity and failure-atomicity proof. Retire into T335 history at closure. |
| `evidence/t337-s1-real-ud-delivery.md` | Verification owner | T337 S1 real-mode `#UD` vector-6 policy, producer proof, and mechanically bounded current-gate owner inventory. Retire into T337 history at task closure. |
| `evidence/t337-s2-shared-delivery-reconciliation.md` | Verification owner | T337 S2 cross-owner reconciliation of shared synchronous, software, external, TF, and task-debug delivery paths. Retire into T337 history at task closure. |
| `evidence/t338-s1-8086-80186-form-allocation.md` | Verification owner | T338 S1 exhaustive 8086/80186 primary-form, legacy-LOCK, and later-S allocation ledger. Retire into T338 history at task closure. |
| `evidence/t338-s2-legacy-alu-profile-matrix.md` | Verification owner | T338 S2 low-profile ALU/FLAGS/condition form, rejection, and legacy-LOCK crosswalk. Retire into T338 history at task closure. |
| `evidence/t338-s3-legacy-data-control-profile-matrix.md` | Verification owner | T338 S3 8086/80186 data, string, stack, real-control/interrupt, ordinary-I/O, and legacy-LOCK form/owner crosswalk. Retire into T338 history at task closure. |
| `evidence/t338-s4-80186-extension-profile-matrix.md` | Verification owner | T338 S4 80186-only primary extension, 8086 rejection, and legacy-LOCK crosswalk. Retire into T338 history at task closure. |
| `evidence/t338-s5-profile-close-audit.md` | Verification owner | T338 S5 complete 8086/80186 allocation, transfer, and legacy-LOCK closure audit. Retire into T338 history at task closure. |
| `evidence/t339-s1-80286-descriptor-transfer-allocation.md` | Verification owner | T339 S1 80286 descriptor/table, selector, gate, 16-bit frame/return, and TSS16 allocation ledger. Retire into T339 history at task closure. |
| `evidence/t339-s2-descriptor-system-word-matrix.md` | Verification owner | T339 S2 80286 descriptor-table and system-word form, atomicity, privilege-boundary, and configuration-gate evidence. Retire into T339 history at task closure. |
| `evidence/t339-s3-selector-cache-matrix.md` | Verification owner | T339 S3 80286 selector-query, table-selection, cache-materialization, and exact-transfer evidence. Retire into T339 history at task closure. |
| `evidence/t339-s4-protected-entry-gate-matrix.md` | Verification owner | T339 S4 80286 direct/gate/external entry, TSS16 outer-stack, frame, origin-order, and exact-transfer evidence. Retire into T339 history at task closure. |
| `evidence/t339-s5-protected-return-matrix.md` | Verification owner | T339 S5 80286 IRET/RETF frame-consumption, cache, atomicity, restored-IF, and exact-transfer evidence. Retire into T339 history at task closure. |
| `evidence/t339-s6-tss16-transition-matrix.md` | Verification owner | T339 S6 80286 TSS16 direct/task-gate/nested-return, validation, cache, fault-boundary, and exact-transfer evidence. Retire into T339 history at task closure. |
| `evidence/t339-s7-80286-descriptor-transfer-audit.md` | Verification owner | T339 S7 audit of all S2--S6 80286 descriptor-transfer owners, publication/fault boundaries, and T340--T342 transfers. Retire into T339 history at task closure. |
| `operations/` | Execution, source, and release owners | Toolchain, policy, templates, release procedure, and closure material. |
| `research/` | Source owner | Provenance, external references, and non-product research. |
| `history/` | Documentation owner | Read-only legacy and M5 migration/roadmap evidence. |

Adding a new supporting document requires owner approval, an exact-file index
row or a declared indexed subtree, and a stated promotion, merge, or retirement
condition. The governance gate verifies that every supporting Markdown file is
covered by one of those index forms. `etc/` is not a shortcut for adding
another principal document.
