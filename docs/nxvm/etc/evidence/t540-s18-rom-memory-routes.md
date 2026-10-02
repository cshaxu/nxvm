# M5 T540 S18 ROM Memory Routes

Immutable ROM images, ordinary aliases and pre-A20 reset aliases now publish
through the same Core-owned memory route operation used by other board memory
owners. ROM retains one copied image owner; aliases borrow its bytes. Firmware
configuration still selects the bytes and encloses its derived alias in one
rollback boundary. No ROM bytes, asset policy or new device framework entered
Shared x86.

## Ownership and failure proof

- Before S18, ROM image and alias constructors registered providers directly,
  while rollback manually searched and swapped entries in Core's provider
  array. Both constructors now submit a typed one-route Core batch. The image
  uses the standard priority, ordinary aliases use overlay priority, and the
  reset alias uses the existing pre-A20 priority. Rollback removes only that
  mapping's owner through Core; it no longer edits the provider table.
- The firmware entry points in `machine_firmware.c` still call the same ROM
  constructors. Core's batch admits its bounded active firmware-configure
  phase, without exposing the provider table or relaxing the ordinary
  non-firmware construction check. A failed bind rolls back only the candidate
  ROM owners and retains previously installed unrelated memory routes.
- The new transaction test injects failure at the first ROM provider, at the
  reset alias after an image was installed, at an ordinary alias after its
  source image was installed, and at an invalid alias source. Each verifies
  provider and mapping counts, an unrelated prior ROM byte and a successful
  subsequent retry. The retry checks the 80386 high reset byte through the
  pre-A20 read path. Existing reset-ROM and firmware tests cover the 80286
  and 80386 reset priority and ordinary alias bytes.
- The old generic direct memory-registration wrapper had only test callers;
  they now use the same batch and the wrapper was deleted. D4 and VADP now
  state their route mode explicitly. The remaining non-ROM absent-memory
  fallback and KBC/DMA memory cycles belong to S19.

Tracked production/API changes add 67 and remove 55 lines (net **+12**).
Tracked existing-test changes add 13 and remove 8 lines; the new ROM
transaction test is 138 lines. The ROM static gate is 45 lines, CMake target
registration adds 11 lines, and the pre-existing T344 test inventory changes
eight lines in and six out to classify the added test. No Shared, MyNES,
INI, external asset, media or firmware-byte content changed.

## Verification

- Complete repository-only unit suites on final production/test source:
  x64 **469/469** and x86 **469/469**. An initial concurrent x64 run had a
  no-output failure in the CPU bus negative fixture; the isolated fixture
  passed, and a clean complete x64 run without concurrent suite passed
  469/469. The clean complete run is the acceptance evidence.
- Focused ROM/firmware/reset/D4/EGA receivers passed. The complete
  `verify-current-specialized-gates` target passed **72/72** build steps,
  including the new `verify-rom-memory-routes` gate. Documentation governance
  and `git diff --check` passed.
- External floppy-boot checkpoint, once per profile and width: 5160 x64
  **1/1** (20.93 s), x86 **1/1** (24.87 s); 5170 x64 **1/1** (32.12 s), x86
  **1/1** (46.38 s); Model 40 x64 **1/1** (54.83 s), x86 **1/1** (69.51 s);
  default PC/AT x64 **1/1** (2.60 s), x86 **1/1** (2.75 s). The complete
  external integration suite remains a T540-level gate.
- All four profile x64/x86 0540 Release products were rebuilt from final
  source. `objdump` reports `pei-x86-64` or `pei-i386` as expected and zero
  `.debug` sections in every EXE. The Release-artifact check passed in each
  build. Adjacent `NXVM.ini` files were not changed.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `E9C9DC0E56093CA2C3D8C5E81F18A42197F670ACC30B8285D7EAD0C6F56852CE` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `8078086989DF87FD4D59EC9DB1DBCBAFD7367D39957B9F9771CB02AD56611D00` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `8BA1D2626C0D7237D5686243E924A233A5609A3F455D7B12DAC0E3F255C92B53` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `9B42B0547E67DA42F5084FCB62A13E24B25914795F8E05C9C455B6F48D18656A` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `7C969D4A769A2BBDC8B70723E927EC4B455614FEBF0C603E5F8CBAB8BC690D66` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `9CCB8C4525156F4035CCD678040B8AA3CCD4B4FF15F567720856F652DF9363CE` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `82633CBF58BE1778E6CCCDCAEA477F9894C718A4C15716953021557090FADC4C` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `636B7B08E0BA6F39D3C0331952E76BEE9985A11276C4B2442EE32055550C1028` |

This is a routing ownership and rollback change, not a claim that ROM or any
chip gained a higher functional or timing evidence grade.
