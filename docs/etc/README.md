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
| `evidence/t339-s7-80286-descriptor-transfer-audit.md` | Verification owner | T339 S7 audit of all S2--S6 80286 descriptor-transfer owners, publication/fault boundaries, retained T328 policy, and T341--T342 transfers. Retire into T339 history at task closure. |
| `evidence/t340-s1-80386dx-form-state-allocation.md` | Verification owner | T340 S1 finite 80386DX form/state allocation, mechanism-owned S sequence, and current Queue receiver correction. Retire into T340 history at task closure. |
| `evidence/t340-s2-prefix-width-fsgs-reconciliation.md` | Verification owner | T340 S2 actual prefix, operand/address-size, FS/GS, and LOCK mechanism reconciliation with exact owner-proof and T341 transfer dispositions. Retire into T340 history at task closure. |
| `evidence/t340-s3-nonprivileged-0f-reconciliation.md` | Verification owner | T340 S3 metadata, dispatch, proof, and transfer reconciliation for every assigned non-privileged 80386DX `0F` family. Retire into T340 history at task closure. |
| `evidence/t340-s4-80386dx-form-closure-audit.md` | Verification owner | T340 S4 original-request, source/evidence, transfer, and task-close audit for the 80386DX form package. Retire into T340 history at task closure. |
| `evidence/t341-s1-80386dx-system-state-allocation.md` | Verification owner | T341 S1 complete privileged-state owner, reader/writer, validation-to-commit, proof, and transfer allocation ledger. Retire into T341 history at task closure. |
| `evidence/t341-s2-control-debug-table-reconciliation.md` | Verification owner | T341 S2 CR/DR/TR/LDT/table state-owner, form/privilege/publication, retained-proof, and external-transfer reconciliation. Retire into T341 history at task closure. |
| `evidence/t341-s3-vm86-task-paging-composition.md` | Verification owner | T341 S3 VM86 entry/return, task/TSS, paging, frame, stack, fault, and commit-boundary composition graph. Retire into T341 history at task closure. |
| `evidence/t341-s4-ordinary-debug-vector1.md` | Verification owner | T341 S4 ordinary 80386 DR6/DR7 matching, RF/TF/task-debug, and vector-1 delivery graph. Retire into T341 history at task closure. |
| `evidence/t341-s5-80386dx-system-state-closure-audit.md` | Verification owner | T341 S5 allocation-to-owner closure audit and explicit 80386DX external transfers. Retire into T341 history at task closure. |
| `evidence/t342-s1-80386dx-profile-closure-audit.md` | Verification owner | T342 S1 source/evidence residual table for final 80386DX profile closure. Retire into T342 history at task closure. |
| `evidence/t343-s1-four-profile-cross-closure.md` | Verification owner | T343 S1 final 8086/80186/80286/80386DX profile and state disposition ledger. Retire into T343 history at task closure. |
| `operations/` | Execution, source, and release owners | Toolchain, policy, templates, release procedure, and closure material. |
| `research/` | Source owner | Provenance, external references, and non-product research. |
| `history/` | Documentation owner | Read-only legacy and M5 migration/roadmap evidence. |

Adding a new supporting document requires owner approval, an exact-file index
row or a declared indexed subtree, and a stated promotion, merge, or retirement
condition. The governance gate verifies that every supporting Markdown file is
covered by one of those index forms. `etc/` is not a shortcut for adding
another principal document.
