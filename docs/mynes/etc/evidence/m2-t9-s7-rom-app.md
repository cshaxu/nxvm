# M2 T9 S7 ROM And App Evidence

S7 completes the previously unenumerated strict-ROM and App state receivers.
The listed executables passed on Windows x64 and x86.

| Ledger scope | Direct receiver | Covered members |
| --- | --- | --- |
| Strict iNES header | `test/core/cartridge_contract_smoke.c` | Header truncation lengths 0 through 15; magic; all 256 PRG/CHR/flag byte values with the legal alternatives accepted; all nonzero values for every reserved byte; exact one-byte short/long images; 16 KiB mirror, 32 KiB banks and both mirroring flags. |
| App management grammar/states | `test/app/command_smoke.c` | startup reset, completed reset/pause/stop/error notifications, help/status/run/pause/stop/reset/quit availability, host-error status/quit-only policy, ROM subcommand/path grammar, numeric overflow and overlong-line rejection. |
| App paused debug commands | `test/integration/app_debug_commands_smoke.c` | copied observation, memory, poke, counts, disassembly, step, break/delete/list and soft reset through production Common/Core. |
| App media states | `test/integration/app_media_commands_smoke.c` | stopped insert, failed replacement, running eject rejection and paused eject stop request through production Common/Core. |

The receiver split is intentional: unit coverage proves syntax and management
transitions without a machine, while the integration fixtures prove the two
paths that cross the App/Common/Core boundary.
