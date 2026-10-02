# M5 T540 S26 Copied D4 Refresh Request

S26 moves the D4 refresh pending bit and address behind a board-owned copied
request. The Core remains the only owner of HOLD arbitration, the refresh
transaction, and CPU prefetch reservation. No controller timing, profile,
deadline formula, or evidence grade changes.

## Owner and difference review

- The board PIT output still asserts the one pending request. The board request
  callback returns its address as a value; the board completion callback clears
  pending and increments the address only after Core transaction commit.
- Core asks for the value before HOLD request/begin and rechecks the pending
  state for the existing prefetch condition. It retains the old HOLD request,
  acknowledge, begin, commit and release order, and does not access the board
  pending/address fields directly. A failed or unavailable transaction leaves
  the request pending for a later attempt.
- Both callbacks use the existing single board owner and are bound during
  machine construction. The scheduler fixture forwards them through its
  instrumented owner, just as it does for deadline and readiness callbacks.
  This repairs the fixture's owner substitution exposed by the new contract;
  there is no production compatibility path.
- The source gate checks that the scheduler does not own those board fields,
  that Core still owns the transaction, and that the board owns completion.
  The D4 unit checks the copied address, success clearing, and the prior
  failure/order/reset behavior. The scheduler unit checks the instrumented
  owner contract.

The production/API diff adds 40 and removes 7 lines (net +33). The two
focused test sources add 20 lines. No Shared, MyNES, firmware, external media,
INI or profile source changed.

## Verification

- Complete repository-only unit tests: x64 **469/469**, x86 **469/469**.
  The first x64 run exposed the scheduler fixture owner substitution; after
  correcting that fixture, the full suite was rerun successfully on both
  widths.
- `verify-current-specialized-gates` passed, including the new copied D4
  request gate. Four external profile boot checkpoints passed once per width
  (**8/8**).
- Eight optimized 0540 Release products were rebuilt in their unchanged
  profile directories. `objdump` confirms x64 `pei-x86-64`, x86 `pei-i386`,
  and zero `.debug` sections in all eight. Owner INIs and MyNES artifacts
  were not modified.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `C7C3F56BA9CC777441B9009D0DEA6BF86D678E13E96405CC12F0C9002B1BE5D9` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `22A608AE07C86D057761D92A03B5FE763F208932928941DA85A474A19DFA8E09` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `3B1814489C0B5E81F7576488494DBAA570A363CE05A12C5D13B964E4B67B8675` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `9CBB0DA6D30FE7605D3C5990649022E67E48860252B73E242DD70A00F7CFA26F` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `8AA54F8F1ACA150F52EEE9A57255E5B127A0DE68E61B3E3FAE9C7B50DCCCE1D8` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `614C5F57A7501D732CCFC51D5D50EE137936D759E5D54B47CD5C73361803D302` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `96A8B4ACC81FE746398C4A5DCF32E35B4D39BC50B04F206B24130D007283FD68` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `0405D01079FBCE9F278681A8442D004E1B091382AA7F373DFA77FF0255C12174` |

S26 closes only the refresh request/transaction boundary. S27 receives DMA
clock/request and wait/grant/prefetch ownership; T540 remains open.
