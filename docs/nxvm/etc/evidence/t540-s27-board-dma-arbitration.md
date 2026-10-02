# M5 T540 S27 Board DMA Arbitration Boundary

S27 leaves the one guest timeline and the original arbitration order in
place. Board code now advances the DMA clock, reports a copied pending-request
fact and advances the DMA chip; Core remains the only owner of cycle wait,
bus-ready gate, HOLD/grant, bounded memory-cycle transaction and CPU prefetch.

## Owner and difference review

- Core first advances the board DMA clock, then the still-Core-owned PIT
  clocks. It captures D4 refresh pending before service and completes the
  Core refresh transaction before evaluating DMA wait and grants.
- The 286/386 HOLD request/acknowledge/release conditions and wait-quanta
  loop are unchanged. The board request callback uses the existing DMA-chip
  signal predicate; the board advance callback invokes the one existing DMA
  transaction/chip implementation. Its guest memory access still goes through
  the previously bounded Core memory-cycle operation, not a board RAM pointer.
- Core checks the updated board DMA request after service before granting CPU
  prefetch reservation. DMA trace remains before the as-yet Core-owned
  PIT/PIC tail, which is explicitly S28 work.
- All three callbacks share the existing board owner. The scheduler fixture
  forwards through its instrumented owner, and a new source gate rejects
  direct DMA chip/clock/request access in the Core scheduler while requiring
  Core HOLD and prefetch operations to remain.

The production/API diff adds 56 and removes 21 lines (net +35); the
scheduler test adds 25 lines. No Shared, MyNES, firmware, external media,
INI or profile source changed. There is no second DMA executor.

## Verification

- Focused scheduler, D4 refresh, DMA channel/RTC authority, transaction and
  CPU prefetch locality tests passed.
- Complete repository-only unit tests: x64 **469/469**, x86 **469/469**.
  `verify-current-specialized-gates` passed, including the S27 board DMA
  arbitration boundary gate.
- Four external profile boot checkpoints passed once per width (**8/8**).
  Eight optimized 0540 Release products were rebuilt in their existing
  profile directories. `objdump` confirms x64 `pei-x86-64`, x86 `pei-i386`
  and zero `.debug` sections in all eight. Owner INIs were unchanged.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `069ED417742D9AB85E2978369A4A62104F624A09BEA11DF07CF02FF1C080DABE` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `AF2ADA3FC79366F81A9049FAACC829A30B6D051BAF3E2DC537C34FA9D9643C52` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `2673202D636427951553C60305C3528095C656195C0271A71EF882E948141F0C` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `0E7F53CE19BFF0F5F1B9B072BF98652E80EEE7FB209265B5C25DDB8F10F2F493` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `8CF4AA2934EDB4C173CCC2852EF4A02AD2ECA5A5FAE688A6D3EDDF49766512FA` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `76782F377D7D377F7D75387CC95BD8A5E51E1A49A1AE8EE05D834233F2D0D4CE` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `22A33D554D3ADC8A4D945575523810B3BEE5D0D49339DFAEBC002D68016383DA` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `3921C624F5DA9ECAC16A22711DE557219D31CBAED54A1712DFA5AF48F968B79C` |

S27 closes only DMA clock/request/chip effects around Core arbitration. S28
receives the PIT/PIC post-prefetch board tail; T540 remains open.
