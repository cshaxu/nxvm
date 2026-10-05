# Residual AT Composition Extraction

Owner-approved T542 corrective S20, baseline 398fa5a1a. Retain all four PC
products and their current 0542 identity. No Lib/Common, MyNES, firmware/media
master, INI, chip algorithm, timing qualification or independent-App split.

## Complete Batch

| Member | Disposition and proof |
| --- | --- |
| Default/5170 private endpoint and route searches | Move neutral lookup to board-at; preserve enabled-device filtering explicitly and migrate callers/tests; DeskPro consumes the same endpoint grammar. |
| Standard AT display-port, RTC and DMA assembly | Share bounded materialization from existing copied configuration values; all three AT models use it. Memory decode, video personality, RTC defaults and timing remain explicit model inputs. |
| FDC endpoint/IRQ/DMA projection | Share projection into existing FDC config; preserve installed drives, media IDs, READY/diagnostic values and Model40 terminal observation. HDC already uses the existing Core configure API and copied personality; do not add a forwarding wrapper. |
| Repeated immutable ROM/alias registration | One bounded explicit-region registration mechanism; App owns sizes/addresses/aliases, preparation and provider lifetime. Preserve Model40 odd/even organization and skipped video alias prefix. |
| PC version and entry/binding | One public version header in ibmpc/product. Retain the tiny App composition main and fixed immutable binding because they contain no duplicate runtime mechanism; no new registry or generic entry framework. |
| Residual descriptor and CPU/topology policy | Keep genuine model values/constraints local; do not force Model40 through the default descriptor or duplicate Core config through another schema. |

One S consumes the entire batch. Shared tests cover alternative endpoint inputs,
missing routes, no partial publication, ROM mapping/alias errors and copied
configuration preservation. Existing product tests retain all model-specific
assertions. Full units on both widths, all eight selected product builds and
original once-only integration contexts prove the changed delivery. Manifests,
corpus/dependency checks and actual-change review are required. Each commit has
one target; artifact/source identity and code-size accounting accompany closure.

PC110 may later consume matching AT mechanisms; MyArcade may use neutral chips
and Core but is not automatically an IBM PC board consumer. Neither is an
implementation target or a new dependency here.
