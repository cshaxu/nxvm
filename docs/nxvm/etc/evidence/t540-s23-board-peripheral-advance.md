# M5 T540 S23 Board Peripheral Advancement

Source intake found that the former all-phase move would place Core-owned
DMA/refresh transactions and CPU prefetch in a board routine, and would hide
the FPU step between FDC/HDC and RTC. The [handoff plan](../architecture/t540-s7-core-board-handoff.md)
therefore narrows S23 before code to the separable peripheral tail and assigns
readiness, arbitration, PIC/INTA, reset and physical relocation to linear
S24-S28. No S suffix, second scheduler or event queue was added.

## Actual owner/difference review

- `machine_scheduler.c` still publishes the sole `elapsed_ticks` value and
  calls arbitration, readiness, then the board peripheral provider. It no
  longer calls XT keyboard, KBC, PIC or VADP advance functions directly.
- `board_advance.c` owns precisely the former peripheral body: advance the
  KBC clock; choose XT keyboard or AT KBC; advance PIC; record the KBC trace;
  advance VADP clock and chip; record the VADP trace. The source-tick value,
  zero-tick return, clock conversions and effect/trace ordering are unchanged.
- One `board_owner` pointer now serves both the existing copied deadline
  provider and this peripheral provider; it is bound once at construction.
  There is no mirrored machine state or alternate advance loop. A focused
  scheduler probe checks zero-tick suppression and that all published ticks
  reach the board callback; the callback forwards to the real board effect.
- The S23 gate verifies the direct chip effects are absent from Core,
  present in the board file, bound at construction and called after readiness.
  Older T499 and rational-clock gates now read the moved source without
  relaxing their sole-timeline or clock-domain checks.

Production/API changes add 37 and remove 32 lines (net +5); the 27-line
board file replaces the removed scheduler body. The direct unit adds 33 and
removes three lines. No Shared, MyNES, profile,
firmware, external asset, INI or chip-timing formula changed.

## Verification

- Complete repository-only unit tests: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: **76/76** steps. The first gate run
  exposed an old T499 file-location assumption; after relocating its source
  check to Core scheduler plus board implementation, the full gate passed.
- One external boot checkpoint per fixed profile and width: **8/8 passed**;
  no group was repeated. Eight optimized 0540 Release EXEs were rebuilt in
  their existing product directories. `objdump` confirms x64 `pei-x86-64`,
  x86 `pei-i386`, and no `.debug` sections in any of them. Owner INIs were
  not modified.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `25E77F98C4ED583271DD39312E5D1E91CE6021CB354EB0A7976D48FBF9D9B206` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `8A1FD66A6DFDE429973EC7D9DF8BAAF069F135A83CDB034F78C98B9D18E2AEE4` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `E86CB4F762BF9B2A8B9B12534416C81B3E9E3920C256A9FFC24EA0C0C6CC22A0` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `9336E27265CFDC1F0B7B57A75228160A3B7C398D2990819FF05755862162D60E` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `BFBB33E669661B15E5EFF1D4837E1279279618EEA58CB1C287E5844413671781` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `556DBF773EF10D13928B88096580A23293501496FF52C31EC8201DEC26C51249` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `088F2B53C6D5C15C5E9270D157BEA66D3E331520BD1E6C1ED4A160C2387024F8` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `BD374EEE2FE8D66F7614C56754E5A9A92DA3022E6DBFD75657D5AC0E893A3567` |

S23 closes only the peripheral tail. The still mixed readiness and arbitration
effects are owned by S24/S25; PIC CPU delivery, plan/reset and neutral Core
move remain S26-S28. T540 stays open.
