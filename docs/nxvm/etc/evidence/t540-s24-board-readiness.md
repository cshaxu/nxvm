# M5 T540 S24 Ordered Board Readiness

S24 preserves the one Core guest timeline and the original readiness order:
FDC service/trace, HDC service/trace, Core FPU advancement, then RTC
clock/chip advancement/trace. It does not change a controller timing formula,
event deadline, profile or evidence grade.

## Owner and difference review

- `machine_scheduler.c` retains the readiness guard, absolute `due_tick`
  and Core `x86_fpu_advance`. It calls a board media provider before FPU and
  a board RTC provider after FPU. It no longer directly advances FDC, HDC
  or RTC chips.
- `board_advance.c` contains the unchanged former FDC/HDC and RTC bodies.
  Media receives source ticks plus Core's absolute due tick; both board
  functions retain the zero-tick no-op. RTC uses the same frozen clock
  domain and emits the same trace, including when the RTC chip is absent.
- The two new callbacks share the already existing single `board_owner`.
  The source-tick provider type was renamed from peripheral-specific to a
  neutral board-tick name and reused for RTC. There is no second readiness
  route or board-side FPU owner.
- The direct scheduler unit checks zero-tick suppression, total media/RTC
  ticks and the absolute due tick. The S24 source gate checks FDC/HDC/RTC
  effects are board-only and Core's media-FPU-RTC call order. Existing RTC
  and Core DMA/RTC gates now look for the chip call in its actual board
  owner file; neither acceptance rule was deleted.

The production/API diff adds 47 and removes 20 lines (net +27), including
32 added lines in the existing board source. The direct scheduler unit adds
29 lines. No Shared, MyNES, firmware, external media, INI or profile source
was changed. Arbitration and CPU prefetch remain for S25.

## Verification

- Complete repository-only unit tests: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates` passed, including the new S24
  media-FPU-RTC order gate and the updated RTC owner checks.
- Four external profile boot checkpoints passed once per width (**8/8**).
  Eight optimized 0540 Release products were rebuilt in their unchanged
  profile directories. `objdump` confirms expected x64 `pei-x86-64` and x86
  `pei-i386` formats and zero `.debug` sections in all eight. Owner INIs
  were not modified.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `91327D69CD73E5591D9383AFF9583EF7BDC5A1A844A5C06462F57613BCDDEE52` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `B944CBA12B68072C36D506080C146E4331B167DEB1C273612A938813A6663E78` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `968D1E4647C1C55A264592645B4E0250589B77D404AF1821D9FB4416686D8572` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `CB1D4FE1B3A66112869176E4BF004DDEC2BA21E43E47A5D13B90F06505A741BA` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `C715E0066DC7AA6835BD1A858773DC6F912202732420FB76E8B63EBF94A2B067` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `7ABE00C162E2E740D0AB927DD9FA032D76DEB92C83ED11E9D9C892DE4819989A` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `907C40027473C0BA6E339B9261B9FED1E824569304E4BA6518475CD17DEE26A0` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `15FB054EBB74B5315F90FE7357263441032A087F76DEF3306A687EECD1B647A0` |

S24 closes only readiness effects. S25 receives the still mixed DMA/refresh
transaction, CPU prefetch and PIT/PIC arbitration phase. T540 remains open.
