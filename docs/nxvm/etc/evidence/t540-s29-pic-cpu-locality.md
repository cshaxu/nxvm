# M5 T540 S29 PIC Signals and CPU Locality

S29 makes the PIC chip's pending and acknowledge results board-owned copied
values. Core remains the only owner of the CPU interrupt-acknowledge
transaction, vector consumption and CPU prefetch locality. There is no second
PIC route or change to interrupt timing.

## Owner and difference review

- The CPU bus provider asks the board whether an interrupt is pending.
  On acknowledge, Core still begins the one CPU INTA transaction; the board
  performs the original PIC IRR-to-ISR acknowledge and returns one vector;
  Core writes that vector into, then commits, its transaction.
- The D4 PIT refresh-pulse edge still occurs in board wiring. Rather than
  directly mutating CPU execution locality there, it notifies a named Core
  CPU-bus event; Core performs the same invalidation at that exact call site.
  DMA HOLD acknowledge invalidation remains in Core's transaction phase
  observer and its ordering is unchanged.
- Both PIC callbacks share the existing board owner. The scheduler fixture
  forwards them through its instrumented owner. The former CPU/PIC gate now
  checks Core's copied-signal contract and the board's original PIC scan and
  acknowledge calls; it did not discard its CPU dependency negatives. The
  negative fixture now copies the board source needed by that gate, and all
  72 baseline/negative controls still run.

The production/API diff adds 36 and removes 5 lines (net +31). Focused
scheduler and negative-test fixtures were updated. No Shared, MyNES,
firmware, external media, INI or profile source changed.

## Verification

- Focused scheduler, PIC phase, CPU PIC lifecycle, and prefetch locality
  tests passed. Complete repository-only unit tests passed x64 **469/469**
  and x86 **469/469** on the final source. The historical CPU/PIC negative
  test and `verify-current-specialized-gates`, including the new S29 boundary
  gate, passed.
- Four external profile boot checkpoints passed once per width (**8/8**).
  Eight optimized 0540 Release products were rebuilt in their unchanged
  profile directories. `objdump` confirms x64 `pei-x86-64`, x86 `pei-i386`
  and zero `.debug` sections in all eight. Owner INIs were unchanged.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `1C98722A307D3A058CEDFB30C4B18709ECADD7DD25C61DEF7589D1DBDAB33CA3` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `C5074A2977E9CAF1E6C535C426ED5FFDFACADB2B1E864FC1C3E515FD9768B5E7` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `1CB6C62D3B38C738D4779B56AC64046318CC7C082A2991B214CA45189B23CD97` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `1058138D3BBAE29ECB14E609969D213E9BA8466AEC72300143E2CD24500A8EDB` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `BABD14D7ED5A21E1F10F71700778FD63B2BF3C841779079CB1827CAD4D3982CC` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `565C247EDAFAAB1ED65081FCB6F74FF6D5F1CEBC84B7C9ECAA1E6C6C58A5D25D` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `8E49A919527421B71D6A2B730C2CF80B369FB1FCB6188C4E76AA804665807361` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `E7E57DEF3B87C9B380787914B4B8EBD435C693CA626A356F2D1185AA859FD84C` |

S29 closes CPU PIC INTA and DMA-HOLD locality; S30 receives mixed plan/reset
ownership. T540 remains open.
