# M5 T540 S17 D4 Memory Routes

Model 40 D4 now publishes its two replacement windows, parity allocation and
write observer through one Core-owned memory transaction. Before this change,
`d4_memory.configured` became true before the first registration, and a later
route, parity or observer failure could leave a partially configured machine.
The D4 state is now committed and reset only after the complete Core batch
succeeds. This changes no D4 protocol, timing formula or profile input.

## Ownership and failure proof

- The two replacement routes, parity fault callback and observer share the
  `core_machine` owner. Core retains the sole provider/observer tables and
  parity allocation; D4 retains only its board-specific mutable state. No
  second memory registry, chip state or generic device framework was added.
- The existing Core route batch now distinguishes ordinary and replacement
  ranges and optionally allocates parity. It records provider/observer counts,
  releases candidate parity and restores both counts on failure. Owner-scoped
  removal releases parity only when that owner holds it.
- The new transaction test injects failure at the first route, at the second
  route after the first append, at parity acquisition and at observer append.
  Each case proves unchanged counts and parity state, then removes only its
  unrelated filler and retries successfully. It also checks replacement flags,
  unified ownership, D4 read/reset state and final owner removal.
- The piecemeal D4 memory registration, observer and parity wrappers were
  deleted. VADP uses the same bounded batch with an explicit ordinary-route
  flag; its EGA candidate transaction test remains green. ROM/reset aliases
  remain the separate S18 receiver, and KBC/DMA memory cycles remain S19.

Tracked production/API changes add 61 and remove 66 lines (net **-5**).
The existing EGA test changes five lines each way; the new D4 transaction
test is 127 lines. The new S17 static gate is 30 lines, and the CMake target
registration adds 11 lines. No Shared, MyNES, external asset, firmware, media
or INI content changed.

## Verification

- Complete repository-only unit suites on final source: x64 **468/468** and
  x86 **468/468**. The focused D4/Model-40 receivers passed **6/6** on each
  width before the final owner-conversion helper cleanup; the final complete
  unit suites include them.
- Final Release Model 40 floppy-boot integration: x64 **1/1** (64.79 s) and
  x86 **1/1** (82.15 s). The four-machine external integration matrix is
  still the T540-level gate, not a claim made by this S.
- The complete `verify-current-specialized-gates` target passed **72/72**
  build steps, including `verify-d4-memory-routes` and the updated CMOS/RTC
  boundary. Documentation governance and `git diff --check` passed.
- All four profile x64/x86 0540 Release products were rebuilt from the final
  source. `objdump` confirms the expected `pei-x86-64` or `pei-i386` format
  and no `.debug` section in each file. The build's Release-artifact check
  passed for every profile/width.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `1A85DFAF8278E0C8248CA66D85EB9391D67F8D9887EE1E81D0D9F7BB923B71A9` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `10D98DDB3F7007B5931C0472AC078B6C827D134FDF4E2EF3B56D3FD3445B6F34` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `F9C63A398642978F2DB296DCCB0A14C00EF2EE714B8ED9BDB6FB6EFC41CE7119` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `7AF10907A44A22322D2BC8DD495B1416A7138D8077D73B857D572933E75E2D40` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `EADD716166371FC70F1AC75EF94E4C95428A3A4B91A46F7A0AA0B6B35321CDB7` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `A4F9A0B5BF5A1CA42D50AFCE256AAFE9875789EA4CFD5CB848B1FC7BEBCEBC89` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `3C9D8599F7B3A6761FACC0A641735E5B66E3C07737264DCAF73206F2356AFCFD` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `8DFF735C1C867A91108D7B17B473D3E7288C940B0319C91CFB7B0305C624B020` |

This extraction is an ownership and failure-atomicity change, not an upgrade
of any chip's functional or timing evidence grade.
