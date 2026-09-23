# T532 S1 Single-Product Layout Ledger

This is S1 evidence for the active [T532 packet](../../states/CURRENT.md). It
records path ownership before any structural relocation; it is not a second
source-layout authority.

## Frozen Production Disposition

| Current tracked path | Disposition | Owner after relocation |
| --- | --- | --- |
| `src/core/machine/**` | Move intact | `src/core/core/**` generic machine implementation |
| `src/vm/machine/**` | Move intact | `src/core/machine/**` NXVM Core adapter, composition and media binding |
| `src/vm/profile/**` | Move intact | `src/core/profile/**` NXVM profiles and immutable board/firmware declarations |
| `src/vm/app/**` | Move intact | `src/app/**` configuration, CLI, recording and composition |
| `src/vm/main.c` | Move intact | `src/app/main.c` App executable entry |
| `src/vm/request_interface.h` | Move intact | `src/app/request_interface.h` App-only parsed session request |
| `src/vdm/machine/dos_minimal.*` | Delete | Future DOS/VDM is not admitted; no replacement |
| `src/vdm/composition/{input_event.h,presentation.*}` | Delete | Future DOS/VDM is not admitted; no replacement |

The existing source scan finds no other tracked `src/vm` root files and no
other VDM production subtree. `src/lib`, `src/common`, and `src/x86` are
outside this task and remain untouched.

## Frozen Test Disposition

| Current test owner | Disposition | Final owner |
| --- | --- | --- |
| `test/core/*.c`, `test/core/machine/**`, `test/core/support/**` and the Core-only root fixture | Move | `test/core/core/**`, including `support/` |
| `test/vm/machine/**` | Move | `test/core/machine/**` |
| `test/vm/support/common_machine_fixture.h` | Move | `test/core/machine/support/` |
| `test/vm/support/rom/**` | Move | `test/core/machine/support/rom/`; these build in-process test bytes and are not external assets |
| `test/vm/profile/**` | Move | `test/core/profile/**` |
| `test/vm/app/**` | Move | `test/app/**` |
| `test/vdm/**` | Delete | Tests only the retired VDM skeleton |
| `test/integration/**` | Retain in place | Independent external-ROM/media integration boundary |
| `test/{lib,common,x86,support}/**` | Retain in place | Shared or generic test owners, unaffected by NXVM path relocation |

## Consumer Sweep

The relocation batch must update every live source-path consumer in these
finite scopes:

- `CMakeLists.txt`: source lists, unit registrations, direct target
  compilation list and test source paths;
- `cmake/verify_*.cmake`: direct source reads, glob scopes and the retired
  VDM forwarding gate;
- production/test `#include` paths below the moved trees;
- `docs/design/ARCHITECTURE.md` and `docs/design/CODING.md` as the current
  component and source-map authorities; and
- active T532 proposal/packet only. Historical task records and indexed
  evidence retain their original paths unless a link itself needs repair.

The S1 command sweep was:

```powershell
rg -n --glob '!docs/history/**' --glob '!docs/etc/**' \
  'src/vdm|src/vm|test/vdm|test/vm|src/core/machine|test/core/machine' \
  CMakeLists.txt cmake tools docs src test
```

Every hit belongs to the move/delete rewrite or is an unaffected historical
record excluded by the command. The next admitted S must repeat this query
after relocation and add a static no-former-root check.
