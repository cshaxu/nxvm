# T591 S1 OmniBook original-ROM machine receivers

Production changes are limited to the MyOB300 App and its board smoke test.
The original 16 MiB 1.0 ABA System Card member was verified against SHA-256
`b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099`.
The boot probe used an ignored, disposable copy. The image remains external.

## Accepted routes

- The existing Core immutable ROM route borrows the card's 32 KiB range at
  offset `7E8000h` and exposes it at `E0000h`. Both real probes reached
  `E000:0003` at step 1,176,383. The fixture verifies mapping and write rejection.
- Board configuration enables the existing extended CMOS index path. The fixture
  verifies that the `F8h` address selects `78h`, separately from `38h`.
- The existing 8042 scheduler delays command replies by 256 guest source ticks
  for this App. This is an L2 model selected from the original POST's polling
  order. The fixture verifies delayed `55h` after `AAh`, plus keyboard and AUX
  IRQ delivery. Its halted CPU advances to the next deadline using the same
  Core operation as the Machine wait path.

## Next real-ROM diagnostic

Both production x86 and x64 builds passed the earlier option ROM, CMOS and 8042
checkpoints, and reached `F1390h` with `AX=1100h`. At the comparison at `F2B3Ah`,
the BIOS expected `10h` but received `3Eh` after writing `EEh` to PIT control
port `43h`. The [Intel 8254 data sheet, Figures 10 and 12](https://www.cs.umb.edu/cs341/Intel8254/I8254PIT.pdf)
defines bits D3:D1 as active-high counter selects. The shared PIT instead
skips counters whose select bit is set. `EEh` therefore selects none in the
current implementation.

An isolated probe built with only the select predicate inverted returned the
four expected comparison bytes (`10h`, `21h`, `10h`, `21h`) on both widths. It
reached `F1390h` with `AX=0000h` at step 1,176,831. The probe stopped at the
diagnostic boundary by design. This establishes the PIT failure and its owner;
it does not establish subsequent POST, storage, or interactive DOS behavior.
No shared source was changed in S1.

A second isolated run continued beyond the corrected PIT check. Both widths
reached guest PC `F28E0h` at step 1,214,482 and reported CPU shutdown
(`20000000h`). The instruction bytes in mapped guest memory decode to
`MOV ES,AX`. The saved state has `CR0.PE=1`, `CS=0040h` and `AX=0048h`.
The trace establishes the next failing operation. It does not yet distinguish
an invalid descriptor supplied by the board mapping from an error in Core's
segment-load or exception delivery logic. Advancing pending device deadlines
left the machine at the same PC, so the stop is not an ordinary IRQ wait.

| Verification record | SHA-256 |
| --- | --- |
| Production PIT diagnostic, both widths | `d8ba460446c72693f65010c81c5939d755ccb6947598a423b423eb707dd78295` |
| Isolated corrected PIT diagnostic, both widths | `7bb7d701e75a68fc94b9056647acd28f4715f961a3290f95494b3e4f7cdb3c2f` |
| Isolated protected-mode stop, both widths | `9a45a30d75a4c1e7683d839a3d11a815e7dafba77077d5d8f14a76f3e82e412e` |
| x64 unit aggregate before its 300-second cutoff | `5418520e787ecf3c47dc201474e22880e30498109de7d862a700f51c520677a8` |
| x64 exact two-test completion | `a79da51165dfa68ffd204818534a64fde2439b2c084fc06944876fa9091687fd` |
| x86 direct CTest run after host session interruption | `8dc12f3c4cd5bf39c43258bf7296118c1c11773b6f46c81b8b05fad6b78b1fe0` |

All 717 unit registrations passed per width on the stable cached Release
trees. The x64 aggregate recorded 715 passes before the outer watchdog
expired. CTest then ran just the two missing registrations (10.27 s).
The interrupted x86 aggregate was replaced by direct CTest (186.74 s).
The board target and focused regression had also passed on both widths.
There is no MyOB300 release target in this stage. The real-ROM checkpoint
is a temporary headless probe, with no desktop or interactive DOS run.
