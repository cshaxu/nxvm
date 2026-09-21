# T533 S5 Product-Convergence Ledger

This ledger freezes the T533 closure universe. A row is accepted only with the
named configuration, executable, INI and test evidence. A skipped or
unavailable owner-provided asset is a disposition, not a pass.

## Fixed Product Builds

| Product profile | CPU | Media topology | Required release artifacts | Current disposition |
| --- | --- | --- | --- | --- |
| `ibm-5160-model-268-360k` | 8088 | 360 KiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Pending S5 verification |
| `ibm-5170-model-339-1200k` | 80286 | 1.2 MiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Pending S5 verification |
| `compaq-deskpro-386-model-40-1200k` | 80386 | 1.2 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Boot terminal currently fails: FDC `E6` remains active at 180 seconds |
| `default-pc-at-80386-1440k-hdd` | 80386 | 1.44 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Pending S5 verification |

Every artifact row must validate the same profile manifest selected at CMake
configure time. The checked-in INI template is copied unchanged beside its
matching executable; its relative media paths therefore remain relative to the
INI, not to the CMake build directory.

## Construction And Test Universe

| Surface | Required disposition |
| --- | --- |
| Production construction | One CMake-selected profile binding, one App INI parser, no YAML/catalog fallback. |
| Integration construction | Each registered scenario uses the matching profile INI and external BYOB assets directly through readonly/overlay storage; no copied-media route. |
| Repository-only tests | Complete unit suite passes without INI, firmware, CMOS, font or media files. |
| Product integration | Each profile's registered boot row reaches its declared terminal, or the shared owner defect is repaired and the full affected product set is replayed. |
| Release deployment | Eight optimized, stripped x64/x86 artifacts exist with their matching INI companions in the required product-output locations, without overwriting another product's configuration. |
| Cleanup | No user-selectable CPU/profile/firmware route, product YAML loader, catalog, or duplicate construction/reset/media path remains. |

## Known First Failure

The Model 40 real boot row is the first unresolved product qualification
member. It is an active S5 repair target, not a retired test or a successful
result. The 5170, XT and default-PC/AT terminal observations recorded in the
S4 evidence remain regression rows when correcting its shared FDC/DMA/time
path.
