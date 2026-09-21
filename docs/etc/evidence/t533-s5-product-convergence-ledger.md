# T533 S5 Product-Convergence Ledger

This ledger freezes the T533 closure universe. A row is accepted only with the
named configuration, executable, INI and test evidence. A skipped or
unavailable owner-provided asset is a disposition, not a pass.

## Fixed Product Builds

| Product profile | CPU | Media topology | Required release artifacts | Current disposition |
| --- | --- | --- | --- | --- |
| `ibm-5160-model-268-360k` | 8088 | 360 KiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Release x64 reached `installer-running`; x86 replay pending |
| `ibm-5170-model-339-1200k` | 80286 | 1.2 MiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Release x64 reached `installer-running`; x86 replay pending |
| `compaq-deskpro-386-model-40-1200k` | 80386 | 1.2 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Release x64 boot reached `installer-running` within 60 seconds; dual-architecture replay pending |
| `default-pc-at-80386-1440k-hdd` | 80386 | 1.44 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Release x64 reached `dos-prompt`; x86 replay pending |

Every artifact row must validate the same profile manifest selected at CMake
configure time. The checked-in root INI remains the canonical template. The
two deployed copies adjust only the relative `nxvm-assets` prefix for their
one-level-deeper product directory; media remains relative to the adjacent
INI, not to a CMake build directory.

## Construction And Test Universe

| Surface | Required disposition |
| --- | --- |
| Production construction | One CMake-selected profile binding, one App INI parser, no YAML/catalog fallback. |
| Integration construction | Each registered scenario uses the matching profile INI and external BYOB assets directly through readonly/overlay storage; no copied-media route. |
| Repository-only tests | Complete unit suite passes without INI, firmware, CMOS, font or media files. |
| Product integration | Each profile's registered boot row reaches its declared terminal in the optimized product build; Debug keeps repository-only unit coverage but is not a production-speed qualification runner. |
| Release deployment | Eight optimized, stripped x64/x86 artifacts exist with their matching INI companions in the required product-output locations, without overwriting another product's configuration. |
| Cleanup | No user-selectable CPU/profile/firmware route, product YAML loader, catalog, or duplicate construction/reset/media path remains. |

## Known First Failure

The Model 40 Debug boot row reaches the timeout while a valid FDC `E6` transfer
is still progressing. Replays with the pre-S4 YAML probe and with the current
INI both demonstrate that this is not an INI/configuration regression. The
optimized Release x64 product reaches `installer-running` within 60 seconds.
Accordingly, Debug-timeout output is diagnostic-only; S5 validates product
bootability with the optimized build, then must replay every required product
and architecture before it can close.

## Completed S5 Evidence

- The fixed-product deployment no longer writes a shared `NXVM.ini`.
  Each Release product deploys only to
  `build/output/<profile>/` and `assets/sessions/<profile>/`; both receive its
  matching executable and an adjacent INI whose external-asset relative prefix
  is correct from that directory.
- The complete repository-only suite was rebuilt and run from
  `build/t533-s5-default-release-x64`: **336/336 passed** (212.71 seconds).
  It does not consume an INI, ROM, CMOS, font or guest-media file.
