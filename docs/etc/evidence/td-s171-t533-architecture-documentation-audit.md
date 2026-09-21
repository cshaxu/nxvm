# M5 Td S171: T533 Architecture Documentation Audit

## Scope And Baseline

This standalone Td audits the live documentation that can direct future work:
`docs/design`, `docs/rules`, `docs/states`, `docs/proposals`,
`docs/etc/operations`, `docs/etc/architecture`, `docs/etc/research`, and the
supporting index. It uses the completed [T533 history](../../history/M5-T533-fixed-machine-products.md)
and [S5 convergence ledger](t533-s5-product-convergence-ledger.md) as the
current implementation baseline.

The uncommitted T534 S1 packet and draft inventory were withdrawn before any
CPU source, test, manual, asset or implementation work. T534 is not allocated;
the Queue remains unnumbered and unchanged.

## Required Current Facts

| Fact | Current authority/evidence |
| --- | --- |
| Retained runnable products | XT, IBM 5170 AT, DeskPro Model 40 and default PC/AT; PC110 remains later work. [Goal](../../design/GOAL.md), [Roadmap](../../design/ROADMAP.md). |
| Product construction | One CMake-selected Profile and one external BYOB asset root; no runtime board/CPU/firmware selector. [Architecture](../../design/ARCHITECTURE.md). |
| Runtime configuration | One adjacent NXVM.ini supplies only supported runtime memory/media/access/presentation. Firmware, CPU, topology and boot order are not INI choices. [UI](../../design/UI.md). |
| Asset and deployment route | Firmware/CMOS/font/media masters stay external; each product deploys only to `assets/binary/<profile>/` with its generated adjacent INI. [Source policy](../operations/policy/source-policy.md), [S5 ledger](t533-s5-product-convergence-ledger.md). |
| Source shape | `src/{lib,common,x86,app,core/{devices,machine,profiles}}`; old `src/vm`, `src/vdm`, `core/profile` and `core/core` names are not live roots. [Coding](../../design/CODING.md). |

## Findings And Disposition

| Finding | Disposition |
| --- | --- |
| Principal Architecture described YAML replacement as a future cutover. | Corrected: T533 completed the replacement; no compatibility loader remains. |
| Principal Coding described Devices/Profiles and INI as a target and named YAML as the runtime baseline. | Corrected: documents current roots and sole NXVM.ini product route; PC110 remains explicitly future. |
| Principal UI denied that fixed-build/INI was delivered. | Corrected: records T533 delivery without claiming PC110. |
| Toolchain conflated checked-in integration templates with deployed product INI. | Corrected: templates are inputs under `assets/sessions`; deployment produces the adjacent INI under the one product directory. |
| Source policy called YAML a current route. | Corrected: YAML is retired/historical; adjacent INI resolves runtime media only. |
| Five Queue proposals had no explicit post-T533 context and could be read as permission to revive a catalog/YAML path. | Corrected: each now starts from the fixed Profile, BYOB, INI and sole-deployment boundary. Queue order did not change. |
| `architecture-detail`, `module-layout-detail`, `pc-at-profile`, `core-machine-instance-design` and `machine-contract-details` contain old VM/VDM/root-type examples under a current-looking directory. | Corrected their headings/prologues and the `etc` index: they are retained historical rationale, never current path/product authority. Their detailed historical text remains intact. |
| M5 closure checklist still referred to withdrawn mantle work. | Corrected to the actual M6 PC110 receiver. |
| Fixed-machine research describes the pre-T533 source tree. | Corrected its prologue: observations are pre-T533 historical evidence unless its explicit scope override says otherwise. |

## Similar-Issue Sweep

The following live-context search was run after corrections:

```powershell
rg -n -i 'current session YAML|runtime baseline until cutover|does not claim fixed-build|src/vm/|src/vdm/|core/profile/|M6 mantle|build/output.*current|assets/sessions.*EXE|runtime profile selection' `
  docs/design docs/rules docs/states docs/proposals docs/etc/operations `
  docs/etc/architecture docs/etc/research --glob '*.md'
```

The remaining hits occur only in documents now explicitly labelled historical:
the pre-T533 research table, retired source-layout record and retired T208
profile record. `docs/history`, `docs/etc/history`, prior task evidence and
legacy material were excluded intentionally: their old terminology is a fact
of the closed work and the Documentation Rule forbids rewriting it merely to
match later architecture names.

## Result

The live documentation now has one post-T533 baseline. T534 has no active
packet or task identifier. Future Queue candidates start from retained fixed
products, Profile-owned construction, external lawful assets, NXVM.ini runtime
configuration and sole `assets/binary/<profile>/` deployment. This Td changes
no source, build, test, runtime configuration, asset or generated artifact.
