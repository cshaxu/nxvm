# Supporting Documentation Index

M5 Td S47 approves this area for retained detail that does not fit the fixed
`rules/` and `design/` authorities. A supporting document may explain evidence,
implementation detail, research, or a retired decision, but it cannot redefine
current architecture, source layout, product UX, roadmap, rules, status, queue,
or debt. Conflicts are resolved by the principal documents named in
[docs/README.md](../README.md).

| Material | Owner | Purpose | Merge or retirement condition |
| --- | --- | --- | --- |
| `architecture-notes/architecture-detail.md`, `module-layout-detail.md`, `machine-contract-details.md` | Architecture owner | Preserved M1--M5 design, layout, and interface detail. | Distill a durable macro decision into `design/`; retire or split into task history when M5 closes. |
| `architecture-notes/c-library-facade.md`, `core-machine-*.md` | Architecture and coding owners | Detailed library, lifecycle, migration, and public-surface evidence. | Replace when the named boundary is superseded; retain only task evidence. |
| `architecture-notes/pc-at-profile.md` | VM profile owner | Detailed PC/AT profile research. | Promote only an approved profile decision to `design/`; otherwise retire with the research. |
| `requirements/cpu-verification.md`, `cpu-capability-matrix.md`, `hardware-device-verification.md` | CPU and compatibility owners | Probe, differential-debug, capability, and hardware evidence. | Move closed task evidence to `history/`; promote only an accepted product promise to `design/`. |
| `requirements/dos.md`, `profiles.md` | DOS and profile owners | Detailed future DOS/profile admission material. | Distill an approved product requirement into `design/UI.md` or `design/ARCHITECTURE.md`. |
| `governance/source-policy.md`, `asset-policy.md`, `redistributability.md` | Source and release owners | Import, asset, license, firmware, and distribution procedure. | Fold any universal invariant into `rules/ARCHITECTURE.md`; retire superseded procedure. |
| `governance/evidence-policy.md`, `differential-debug-policy.md`, `invasive-integration.md` | Verification owner | Specialized evidence and research safeguards. | Fold a universal rule into `rules/EXECUTION.md`; otherwise retire with the experiment or decision. |
| `governance/m5-closure-checklist.md` | M5 owner | Detailed M5 closure evidence checklist. | Archive into M5 closure history when M5 closes. |
| `baselines/`, `build/`, `fixtures/`, `templates/` | Execution owner | Reproducible baseline, toolchain, fixture, and evidence support. | Replace or retire when its referenced baseline or process is replaced. |
| `provenance/`, `references/`, `research/` | Source owner | Origin records and non-product research. | Never becomes runtime input without an approved task and source review. |
| `release/` | Release owner | Release templates and supporting procedure. | Retire or replace with an approved release process. |
| `legacy-history/` | Documentation owner | Read-only pre-Td-S47 summaries and superseded governance records. | Keep for Git-era retrieval; never cite as a current authority. |

Adding a new supporting document requires owner approval, an index row or an
update to an existing precise row, and a stated promotion, merge, or retirement
condition. `etc/` is not a shortcut for adding another principal document.
