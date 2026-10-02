# M5 T540 S13 AT Port-B Routes

The AT planar-parity and Model-40 D4 Port-B personalities each publish their
single 61h read/write route through the Core-owned typed route batch. They
remain mutually exclusive. Board latches, speaker gate, refresh source,
failsafe and NMI wiring stay with the existing NXVM board attachment; PIT
chips remain the only counter owners. The guest byte behavior and timing are
unchanged.

## Failure boundary and simplicity

- Planar parity now allocates its RAM parity store before publishing the route.
  A failed allocation publishes nothing. If route publication fails, the one
  RAM owner releases the prepared parity store and clears its metadata. Only
  after publication succeeds does the board publish its configuration and
  install the existing speaker/PIT callbacks. This also removes the former
  failure path that could leave PIT callbacks changed after parity allocation
  failed even though the port route was rolled back.
- D4 has no separate parity allocation. It publishes one route before changing
  its board latches or callbacks. Both personalities preserve the former
  invalid-argument outcome for an occupied 61h port and the invalid-state
  outcome for repeated configuration of the same personality.
- The old availability probes, range-provider wrappers and board-held port
  registration checkpoints are deleted. The RAM parity release operation is
  used by both failed preparation and final destruction, rather than creating
  a second cleanup path. No new device framework or state mirror exists.

## Scope and receiving proof

- Production changes are confined to `devices/{machine_board.c,memory.c,memory.h}`:
  38 added, 35 removed tracked lines (net +3). The owner-local test adds 44
  and removes 0 lines (net +44); source/test together adds 82 and removes 35
  (net +47). The net increase is the one rollback primitive plus the four
  failure-and-retry matrix cases, not another production route. The new
  static verifier and its CMake registration add 55 lines outside that count.
  No Shared, MyNES, profile, firmware, INI or media input changed.
  The owner-local port-assembly fixture injects both route-allocation failures
  for both personalities and verifies no partial route, parity allocation,
  configured flag or Port-B latch, followed by successful retry. Existing
  D4/planar tests cover 61h exclusivity, parity/NMI, refresh and failsafe.
- The new registered `verify-board-port-b-boundary` gate checks both functions
  use typed Core publication without direct port-table access or external
  checkpoints, and that parity preparation precedes route publication with
  one release path. It is included in the specialized verifier inventory.
- The production sweep used
  `rg -n 't_port \*|core_machine_port_add_|core_machine_port_registration_begin|core_machine_port_rollback_registration|executor_port' src/app-nxvm/devices -g '*.c' -g '*.h'`.
  It has no hit within
  the two Port-B constructors. HDC personalities remain S14, staged VADP
  routes S15, and generic Core-private port storage belongs to the later
  neutral-Core move.

## Verification and artifacts

- Both-width unit trees compile. Complete unit suites pass 467/467 on x64 and
  467/467 on x86. Focused Port-B, parity, D4, refresh and port-assembly
  selections pass 8/8 per width. The registered Port-B static gate passes on
  both widths. Documentation governance and `git diff --check` pass.
- All four profile-specific x64 and x86 0540 executables were rebuilt as
  optimized Release products. `objdump` confirms the eight PE architectures
  and absence of `.debug` sections. SHA-256 identities:

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `F9900D133E7DC85E6EC3887EF2D03C63E3F91223EA6D53210A37DBA5500B27F7` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `AE4681010D0D80D7D5BBB4E1107423E02308DD4B70E264432229356C80AC3047` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `3E3919BA76174736AE190FA5A4F3047650909F9FC6E638F416C5B80E82E0B1AB` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `A6196239D89451FAC459335443EEF8CCDB3D71C36659AA117B1F8C4A2B601E99` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `95F1B0E9635E0F6E8CD356E91590C899C8884711277FF350BC55771ED802ABDE` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `20BA2235F8BB1AAAB22366D8C1B9FCE578E3D46F7256364FEEF720F609A4D2B0` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `34EB5933E09B4AE8D617ED91618FF3BCACC8C618478DB37C99F55456FD5280CB` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `6ECAB721B3B88205A227E487D133FE39D238C67C32B7EC7AE22153F405F85852` |

No L3 or other timing-grade upgrade is claimed. The external four-profile
integration gate remains due at T540 closure.
