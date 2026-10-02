# M5 T540 S14 HDC Port Routes

All four HDC personalities now publish their complete port set through one
Core-owned typed route batch. ATA PIO, WD1003/ST-506 and Compaq WD use the
task-file map; XT Xebec uses its four-port map with the mask port write-only.
Compaq's extra 3F7 read remains an explicit wired-OR contribution alongside
the existing FDC read. Chip commands, media behavior, IRQ/DMA behavior and
guest timing are unchanged.

## Failure boundary and actual diff

- The same address list feeds HDC topology validation and route construction.
  The former availability probe, per-direction registration loop and
  board-held port checkpoint are deleted. Chip initialization precedes the
  atomic route batch. If any entry fails, Core rolls back that batch before
  HDC destruction; the board publishes no configured state.
- XT DMA channel binding occurs after route publication. If binding fails,
  Core removes only HDC-owned routes through a bounded owner operation; HDC
  state and topology are then cleared. FDC routes and an unrelated occupied
  port remain intact. This uses the existing Core owner-removal mechanism,
  not a second registration or rollback system.
- The actual production diff in `devices/{machine_board.c,port_interface.c,
  port_interface.h}` adds 65 and removes 108 tracked lines (net -43). The
  owner-local test adds 54 and removes 7 (net +47). Production plus test adds
  119 and removes 115 (net +4); the additional test cases account for the
  small combined increase. The registered static verifier is 33 new lines;
  its CMake registration adds 7. No Shared, MyNES, firmware, INI or media
  source changed.
- Actual-diff review finds one chip state owner and one publication path per
  personality. The route-removal operation is confined to stopped Core
  construction/teardown and removes by owner token. No generic controller
  framework, mirror state or timing fallback was added.

## Receiving proof

The `core_machine_configure_hdc` source sweep found no `executor_port`,
`core_machine_port_add_*`, availability probe or registration checkpoint.
The production search was:

`rg -n 'core_machine_configure_hdc|core_machine_port_add_|core_machine_port_registration_begin|core_machine_port_rollback_registration|core_machine_remove_port_routes|drive_address_port' src/app-nxvm/devices/{machine_board,port_interface}.c`

`verify-hdc-port-routes` guards the typed batch, Compaq wired-OR and
owner-scoped cleanup, and rejects the old raw HDC route path. Staged VADP
ports remain assigned to S15; Core-private port storage moves only with the
later neutral-Core cut. The full external four-profile integration gate
remains a T540 exit requirement.

## Verification and artifacts

- Both complete repository-only unit suites pass 467/467. Focused HDC,
  Compaq, XT DMA, Core authority and route-assembly tests pass 11/11 per
  width. The injection matrix covers each allocation position for all four
  personalities, late ATA port collision, occupied XT DMA channel, 3F7
  wired-OR, and failed-construction retry. The new static gate passes.
- Documentation governance and `git diff --check` pass. Four x64 and four
  x86 profile products were rebuilt as optimized Release executables.
  `objdump` confirms PE architecture and no `.debug` sections in all eight.
  SHA-256 identities:

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `2AC258B33F489626DB4F952E896129CB511A9E2DA64414556975C6ED6A7DE287` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `33F3FD2451FD834C13F33BACFBDC97EAA77984C0264C8FAC25829565221F21A3` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `971C2E2C254F804714AC8D77FA367F39FC9F5BD0427D3BCC393231BA665204BB` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `48AA7A4C74D7B24036FECE4A2CAD8AE2D2CACB57D32CAB0E5C93634EF1B38895` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `3E91EB4943D76080CC9E805C9FB93D76D20DB297A2170FD36C954B7816F8F10D` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `DDCF2E255D3025429CBCE54F160AC9829D23FE8C22B7C7FEE12BFC3C88458FF3` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `F41C55EE47BDECCDA8D33FBF99643A74F8EFD740EAB13A3C80049FACC727B127` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `67DC70C862A982F1DC4BF909FC21510BCFFD54D2AF0A144693DBF30514E8A99B` |

No timing-grade upgrade is claimed for this structural change.
