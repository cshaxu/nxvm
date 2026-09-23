# T404 S2: Public Catalog And Session Contract Reconciliation

`M5:T404:S2:CATALOG-CONTRACT:OK`

## Batch A disposition

| Public route | Owner | Disposition |
| --- | --- | --- |
| YAML discovery and schema | `vm/product/session_catalog` | Accepted after repair: only known profiles, display/boot values, profile-local fields and Model-40 BYOB manifest shape enter the catalog. |
| Default PC/AT CPU/FPU variants | catalog plus session factory | Repaired: the catalog now accepts only the same CPU names (`8086`, `80186`, `80286`, `80386`) and FPU value (`none`) that the session factory can start; empty fields retain defaults. |
| IBM 5170 Model 339 | default-profile descriptor/session factory | Accepted: generic CPU/FPU/memory/HDD overrides are rejected; its fixed descriptor supplies the selected configuration. |
| DeskPro Model 40 | Model-40 composition/session factory | Accepted: local BYOB manifest is required and copied before session publication; CPU/FPU/memory overrides are absent, and the fixed HDD path is startup-only. No firmware bytes or paths enter the repository. |
| Removable FDD lifecycle | session/console adapter | Retained for Batch B: the existing stopped-session insert route remains public and requires its complete controller/media lifecycle reconciliation. |
| Controller, display and physical timing | Core and VM profiles | Retained for Batch B/later L3: no register, IRQ/DRQ, media or physical timing conclusion is made here. |

## Defect and proof

The catalog previously accepted arbitrary `default-pc-at` `cpu` and `fpu`
strings, while the session factory accepted only its finite implementation set.
A catalog entry could therefore be presented as selectable and fail only after
selection. `vm_product_session_catalog_default_cpu_is_valid` and
`vm_product_session_catalog_default_fpu_is_valid` reject that mismatch at
catalog discovery. The focused smoke adds invalid `80486` and `8087` entries,
retains valid default and BYOB entries, and reports eight rejected entries.

Similar-issue sweep: all catalog profiles and direct factory options were
inspected. Model 339 already rejects generic overrides; Model 40 already
requires the copied BYOB manifest and only admits fixed HDD media during
startup. The only shared mismatch was the default-profile YAML CPU/FPU fields.

## Verification

- Direct focused smoke: `build/t400-current-gate/vm-product-session-catalog-smoke.exe` - pass (`M5:T381:S1:SESSION-CATALOG:OK`).
- Full current gate and documentation governance are required before S2 acceptance.