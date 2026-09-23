# M2 T9 S5 Boundary Evidence

S5 adds direct product-owned evidence for two previously aggregate-only
acceptance areas. The following executables passed after the changes on both
Windows x64 and x86 build trees.

| Ledger row | Direct receiver | Cases proved |
| --- | --- | --- |
| ROM format and mapping | `test/core/cartridge_contract_smoke.c` | 16 KiB PRG mirror, 32 KiB PRG bank selection, CHR 0/1 acceptance, bad magic, zero PRG, unsupported CHR/mapper bits/padding, and exact length minus/plus one. |
| APP grammar | `test/app/command_smoke.c` | empty insert, quoted empty path, unquoted extra token, quoted space-bearing path, non-ASCII input, extra eject argument, numeric overflow and session's overlong-line callback. |
| APP production media states | `test/integration/app_media_commands_smoke.c` | stopped insertion/reset request, rejected replacement, running eject rejection and paused eject/stop request through App, Common and Core. |

`rom insert` now accepts one ASCII path. A double-quoted path groups spaces;
backslashes are passed literally to the Win32 storage provider. This is App
syntax only and does not alter Lib/Common interfaces.

The direct fixtures leave two S5 acceptance classes open: fault-injected
storage allocation/read/close combinations, and a concise MyNes-owned mapping
from the existing Common construction/sink/wait/join fault fixtures to HOST.
S6 owns those outcomes. This record is not M2 closure evidence.
