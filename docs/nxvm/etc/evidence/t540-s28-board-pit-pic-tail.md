# M5 T540 S28 Board PIT/PIC Arbitration Tail

S28 preserves the one Core guest timeline and the original arbitration phase.
PIT clock-domain conversion still occurs before the D4 refresh and DMA
service, while PIT chip/output and PIC refresh remain after CPU prefetch.

## Owner and difference review

- The board returns a copied primary/auxiliary PIT tick pair. This carries
  only elapsed values, not either clock domain or controller pointer, across
  Core's refresh, DMA wait/grant and prefetch phase.
- The board tail then advances the primary PIT and optional auxiliary PIT,
  emits the PIT trace when primary ticks are nonzero, refreshes the master
  and slave PIC, and emits the PIC trace. The original order and trace
  conditions are unchanged. The board continues to own PIT output wiring;
  Core does not acquire IRQ-line state or a second timer.
- Both callbacks use the existing single board owner. The scheduler unit
  forwards them through its instrumented owner, while the new source gate
  prevents direct Core scheduler access to PIT clocks/chips or PIC refresh.
  CPU PIC INTA and HOLD locality remain explicitly assigned to S29.

The production/API diff adds 54 and removes 17 lines (net +37); the
scheduler test adds 21 lines. No Shared, MyNES, firmware, external media,
INI or profile source changed.

## Verification

- Focused scheduler test and all current specialized gates passed, including
  the new board PIT/PIC tail gate. Complete repository-only unit tests passed
  x64 **469/469** and x86 **469/469**.
- Four external profile boot checkpoints passed once per width (**8/8**).
  Eight optimized 0540 Release products were rebuilt in their unchanged
  profile directories. `objdump` confirms x64 `pei-x86-64`, x86 `pei-i386`
  and zero `.debug` sections in all eight. Owner INIs were unchanged.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `BF71014BD33CFD172D71863D27A1D57AD686BCB64A1A7051AE243EE7C4105FC4` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `D2D3440E9B9E202825267246B9AAB255B9DBF4CEDE5C55D7A7527EB7DFDAD4BB` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `1F150FC3C192FA2FACCD20DC49744A00717E0B997183FB4BA5239B5DCE579616` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `5F0AA165ABFE68836C5F6F0176C7F864B2F1DADC1BAE5BA2F52ACD3683510B71` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `F3B9FD3F45AE27997DAA04658EFB2659A139538C55471491805384B4F0272EB0` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `3CD5F76952BCC7F6E46C918041CDEADC453BD9E4C6959D2BCFDDD8FB3FA1D2C3` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `898C6AEA47F7C5A1C393337AF95EC13C3B015EF8B42EE15EC799CDA924D166B3` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `4E88618BF7AC237079DAC4B25672B02097E547C622173125B4F4B6685EFE7903` |

S28 closes only the PIT/PIC tail. S29 receives CPU PIC INTA and DMA-HOLD
locality; T540 remains open.
