# M5 T540 S20 Core-Owned DMA Memory Cycle

The DMA board adapter no longer receives `t_ram` or a transaction-state
pointer. Its byte/word cycle submits a physical address, width, local DMA
channel, direction and copied value to one Core operation. Core owns the
physical route query, transaction begin, memory transfer, commit and cancel.
The board still owns the device provider, latch, terminal signal and the
primary-only/paired arbitration choice. No Shared chip, profile, INI,
firmware, media or timing value changed.

## Caller and failure disposition

- The scheduler's four DMA advance calls now pass their Core machine owner;
  its existing HOLD request/acknowledge/release decision is unchanged.
- The DMA adapter's former direct query, begin, read/write and commit/cancel
  calls are deleted. Memory-only transfer still uses the chip-provided byte;
  device-to-memory invokes the existing read provider before Core memory;
  memory-to-device invokes the existing write provider after Core memory.
  VERIFY keeps its existing device-only path.
- Core rejects an invalid/preflight route before the device effect or
  transaction begin. A successful cycle records begin, device-before,
  memory, device-after, commit in that order. A failed memory provider
  records cancel, does not invoke device-after and leaves no active owner.
  Focused transaction and DMA channel tests cover this order, byte/word,
  primary-only/paired, request and terminal behavior. FDC and XT HDC
  wiring tests retain their original device paths.
- The old FDC boundary gate now expects the DMA adapter to call the Core
  cycle, with physical memory access present only in Core. Its negative
  fixture includes that Core file so an unmodified source tree passes before
  each injected violation. The new DMA gate rejects reintroduced raw RAM,
  transaction-state or memory/transaction calls in the board adapter.
- A source sweep of `dma_bus.c/h` finds no `t_ram`, transaction-state type,
  `executor_memory`, direct physical query/read/write or transaction
  begin/commit/cancel. Core-private memory and HOLD state remain where they
  belong. Board deadline/PIC exchange is assigned to S21; mixed plan/reset
  lifetime to S22.

Tracked production/API changes add 103 and remove 56 lines (net **+47**).
Tracked existing-test changes add 226 and remove 137 lines; most are the
mechanical standalone-fixture change from separate RAM/transaction values to
one Core machine. CMake registration and the repaired existing gate add 13
and remove three lines; the new static gate is 30 lines. There is no parallel
legacy DMA memory path.

## Verification

- Final-source complete repository-only unit suites: x64 **469/469**, x86
  **469/469**. An initial x64 run had one negative-fixture failure because
  that fixture omitted the new Core source; after correcting the fixture,
  both the focused negative control and a fresh complete x64 run passed.
- `verify-current-specialized-gates` completed **76/76** build steps;
  `verify-dma-core-cycle-boundary` independently reported
  `M5:T540:S20:DMA-CORE-CYCLE:OK`. Documentation governance and diff hygiene
  are checked at closure.
- One external floppy-boot checkpoint per profile and width: 5160 x64
  **1/1** (20.79 s), x86 **1/1** (29.03 s); 5170 x64 **1/1** (38.24 s), x86
  **1/1** (53.47 s); Model 40 x64 **1/1** (65.07 s), x86 **1/1** (80.03 s);
  default PC/AT x64 **1/1** (3.56 s), x86 **1/1** (2.50 s). The complete
  external integration suite remains a T540-level gate.
- All four profile x64/x86 0540 Release products were rebuilt from final
  source. `objdump` reports `pei-x86-64` or `pei-i386` as expected and zero
  `.debug` sections in each EXE. The build's Release-artifact check passed.
  The x86 unit-tree all-target build incidentally relinked the tracked MyNES
  x86 executable; it was restored from HEAD before acceptance and has no
  retained diff. Subsequent Release builds targeted NXVM only.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `9BB59171375250C271B2A7986463863CFE697B57BE8E7E3630A1C45603BEBA0D` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `44F9F67EEBF6C0B2ACFE33C38C34133A113D5B7F13DB4FC0B5833E0459B1908E` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `4DB2DFFDAB9370F0AC2011F1D91C529B0640D8CC95615BA66BFFF7DDF5FA29D1` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `30436E0F3CAE9188D58798BAA99BA6699F0EE04F4F00454DE600567BB5A20D9C` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `4406E7B115D4AA8B2DC7D3EED059DD293AA079C1931C9844C8D0540BD679163E` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `6C7004C05186DAAB556F11DD4526DC1952F6B6EE27FF5308564BC15ABC973990` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `4EEA37091482D297AE0FBE52C8E6D0FFE453DB5287427F98D8F3F38F6EACCC6E` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `CF8BE8A90B2C4F6E6E2C4CD9E6E914DD9385B80CB72D14042A935D0CA295EDDD` |

S20 closes the DMA memory/transaction ownership boundary, not any
controller's functional or timing evidence grade. S21 receives board
deadline/advance and PIC acknowledge separation from Core's CPU timeline.
