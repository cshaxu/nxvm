# M5 T540 S19 A20 And Absent-Memory Routes

KBC output-port D1, board port 92h and the paused debugger now reach the
same Core-owned A20 state. The board no longer borrows `t_ram` for either
signal. An unpopulated board aperture is registered through the same typed
Core memory-route operation as other providers, retaining fallback priority.
No KBC command, A20 electrical rule, firmware byte, profile, INI or media
policy changed.

## Caller and failure disposition

- The KBC bind formerly retained `t_ram *` and wrote `flagA20` from its
  output-port callback. It now receives one bounded A20 signal callback;
  `machine.c` binds that callback to the Core machine. The KBC can change
  A20 while the guest runs without bypassing Core ownership.
- Port 92h formerly read and wrote `executor_memory.data.flagA20` directly.
  It now observes and signals the same Core state. The debugger's paused-only
  setter delegates to that signal after its existing lifecycle check; it did
  not become a second running-state control path.
- `core_machine_configure_absent_memory` formerly registered its provider
  directly on RAM. It now submits a typed fallback route through Core's
  memory batch. The provider retains lower priority than a decoded ROM or
  video window. A failed registration clears the candidate absent slot and
  preserves the provider count; retry succeeds.
- The running-state test checks that paused-debug A20 writes still reject a
  running machine, KBC D1/03 sets A20, port 92h reads it, and port 92h clears
  that same state. The fallback test injects first-provider allocation
  failure, then checks retry, ROM-over-fallback overlap and adjacent open-bus
  `0xff`. Existing KBC reset and AUX tests retain their command behavior.
- A source sweep found no remaining board/KBC direct `flagA20` access or
  board-level fallback registration. Core's own RAM access remains its
  private implementation. DMA still takes a raw RAM/transaction argument;
  its distinct HOLD/bus-cycle contract is assigned to S20, not disguised as
  another A20 caller.

Tracked production/API changes add 68 and remove 27 lines (net **+41**).
Tracked existing-test changes add 68 and remove 14 lines. The CMake target
registration adds seven lines; the new static gate is 43 lines. No Shared,
MyNES, adjacent `NXVM.ini`, external asset, media or firmware-byte content
changed.

## Verification

- Final-source complete repository-only unit suites: x64 **469/469**, x86
  **469/469**. Focused KBC, instance and reset-ROM/fallback tests also passed.
- `verify-current-specialized-gates` passed **74/74** build steps, including
  the new `verify-a20-fallback-routes` gate. Documentation governance and
  `git diff --check` passed.
- One external floppy-boot checkpoint per profile and width: 5160 x64
  **1/1** (21.11 s), x86 **1/1** (27.56 s); 5170 x64 **1/1** (34.90 s), x86
  **1/1** (41.67 s); Model 40 x64 **1/1** (61.61 s), x86 **1/1** (79.19 s);
  default PC/AT x64 **1/1** (2.46 s), x86 **1/1** (2.80 s). The complete
  external integration suite remains a T540-level gate.
- All four profile x64/x86 0540 Release products were rebuilt from final
  source. `objdump` reports `pei-x86-64` or `pei-i386` as expected and zero
  `.debug` sections in each EXE. The Release-artifact check passed in each
  build. One x86 default-at link attempt collided with the still-running
  Model 40 test executable; it succeeded after that process exited, without
  code changes.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `5F7E71D7225AEBAB1B66E9445AABC198CAD3AA4F96EC7EF80E8F0A6C3ED9D39A` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `622D789C3DB1039133E368F8D9B0EC87599E054548A82FF0E42B34B3369D535A` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `AFED083D08FFFAC53C70C32322E1527DF24E1E05B7DDFB48076773EE5CCF3974` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `9C5432AA88515F23163B9711EDF7C81BF4FD117B627CFF46724E83C9DA35D9EC` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `901E53F4AB96CDBA9A3A46BE4643AF7D741E45FC0ED933F6C53FCFE7C12C8A26` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `AFD2B8B587A84F6315267331CF77FC9F532BC1585F5E6B501ADA69A483B44987` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `CF72D5B8747D4FF4A1FC9C2D5DBBAB9B1F95D12A63C7032E6ABC12D4A6C41C42` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `DDDB3043DDC8D958A0054EB97B4E09EF7C9FE9D085E7C4CF7CA4487C4231069B` |

This S closes routing ownership, not an increase in any controller's
functional or timing evidence grade.
