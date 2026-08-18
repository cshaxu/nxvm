# T404 S1: Current Public Device/Profile Capability Ledger

`M5:T404:S1:PUBLIC-DEVICE-LEDGER:OK`

## Frozen public universe

| Surface | Owner | Current disposition / next batch |
| --- | --- | --- |
| Startup catalog and selected session | `vm/product/session_catalog`, session factory | Public profiles: default PC/AT, IBM 5170 Model 339 and BYOB DeskPro Model 40. Verify catalog/snapshot/selection as Batch A. |
| Model-40 BYOB firmware and fixed media | Model-40 composition/profile | Existing functional binding; D4 and KBC repairs retained. Firmware lifecycle and physical media/timing transfer. |
| Model-339/default PC/AT descriptors | VM default profile | Existing named topology/device descriptors; profile/variant contract audit is Batch A. |
| CPU, memory, PIC, PIT, DMA, RTC/CMOS | Core machine | Existing functional proofs retained; physical board timing transfer. |
| KBC/keyboard/AUX | Core KBC | T403 functional owner/proof; physical timing transfer. |
| FDC/floppy and HDC/HDD media | Core controllers plus VM media profiles | Existing selected logical routes; removable/fixed lifecycle and public selection audit Batch B. |
| Display: CGA/EGA/CECG | Core VADP and VM profile | Existing selected digital paths; physical monitor/raster timing transfer. |
| YAML/CLI configuration and host copy boundary | VM session factory/product | Public input validation, declared-only profile selection and forbidden combinations are Batch A. |

## Completion predicate

Every row must receive one explicit profile contract and a direct functional
proof, an admitted repair batch, an unavailable/removed result, or a named
physical/reference transfer. Batch A freezes public catalog, profile, variant,
firmware-manifest and media-selection constraints before any broad profile
migration. Batch B reconciles retained controller/media/display/input function.
No batch may claim board timing or L3.
