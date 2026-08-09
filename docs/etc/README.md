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

The retired pre-S52 directory map is retained below only as migration evidence;
it does not index the current tree.

| Category | Owner | Purpose | Promotion or retirement condition |
| --- | --- | --- | --- |
| `architecture-notes/architecture-detail.md`, `architecture-notes/module-layout-detail.md`, `architecture-notes/machine-contract-details.md` | Architecture owner | Preserved M1--M5 design, layout, and interface detail. | Distill a durable macro decision into `design/`; retire or split into task history when M5 closes. |
| `architecture-notes/c-library-facade.md`, `architecture-notes/core-machine-*.md` | Architecture and coding owners | Detailed library, lifecycle, migration, and public-surface evidence. | Replace when the named boundary is superseded; retain only task evidence. |
| `architecture-notes/pc-at-profile.md` | VM profile owner | Detailed PC/AT profile research. | Promote only an approved profile decision to `design/`; otherwise retire with the research. |
| `requirements/cpu-verification.md`, `requirements/cpu-capability-matrix.md`, `requirements/hardware-device-verification.md`, `requirements/current-capability-baseline.md` | CPU and compatibility owners | Probe, differential-debug, capability, hardware, and detailed baseline evidence. | Move closed task evidence to `history/`; promote only an accepted product promise to `design/`. |
| `requirements/dos.md`, `requirements/profiles.md` | DOS and profile owners | Detailed future DOS/profile admission material. | Distill an approved product requirement into `design/UI.md` or `design/ARCHITECTURE.md`. |
| `requirements/nxvdm-runtime-detail.md` | NXVDM product owner | Exact CLI, containment, debugger, cancellation, and acceptance requirements. | Distill an interaction change into `design/UI.md`; retire superseded detail with its M8 task history. |
| `governance/source-policy.md`, `governance/asset-policy.md`, `governance/redistributability.md` | Source and release owners | Import, asset, license, firmware, and distribution procedure. | Fold any universal invariant into `rules/ARCHITECTURE.md`; retire superseded procedure. |
| `governance/evidence-policy.md`, `governance/differential-debug-policy.md`, `governance/invasive-integration.md` | Verification owner | Specialized evidence and research safeguards. | Fold a universal rule into `rules/EXECUTION.md`; otherwise retire with the experiment or decision. |
| `governance/m5-closure-checklist.md`, `governance/roadmap-detail.md` | M5 owner | Detailed M5 closure and pre-S48 roadmap material. | Archive into M5 closure history when M5 closes. |
| `baselines/`, `build/`, `fixtures/`, `templates/` | Execution owner | Reproducible baseline, toolchain, fixture, and evidence support. | Replace or retire when its referenced baseline or process is replaced. |
| `provenance/`, `references/`, `research/` | Source owner | Origin records and non-product research. | Never becomes runtime input without an approved task and source review. |
| `release/` | Release owner | Release templates and supporting procedure. | Retire or replace with an approved release process. |
| `legacy-history/` | Documentation owner | Read-only pre-Td-S47 summaries and superseded governance records. | Keep for Git-era retrieval; never cite as a current authority. |

Adding a new supporting document requires owner approval, an exact-file index
row or a declared indexed subtree, and a stated promotion, merge, or retirement
condition. The governance gate verifies that every supporting Markdown file is
covered by one of those index forms. `etc/` is not a shortcut for adding
another principal document.
