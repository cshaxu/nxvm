# M5 T540 S12 RTC/CMOS Port Routes

The RTC/CMOS index-write and data-read/write endpoints now publish as one
Core-owned typed port-route batch. The board still owns the selected register,
NMI-mask wiring, IRQ source and frozen CMOS defaults; the MC146818 chip remains
the only live register/timing owner. No guest-visible register or timing rule
changed.

The old board precheck, two separate range publications and external port
checkpoint are removed. The chip is constructed before publication; if the
Core route batch rejects a collision or allocation, the uncommitted chip is
destroyed and its pointer cleared. Core rolls back the batch without disturbing
any earlier route. Only after both routes publish does the board bind IRQ and
load defaults. The existing read/write provider functions remain the sole
guest I/O behavior path, now referenced directly by typed route descriptions.

## Scope and actual-diff review

- Production: `src/app-nxvm/devices/machine_board.c`, 14 added and 35 removed
  tracked lines (net -21). One owner-local port-assembly test adds 31 and
  removes 1 (net +30). The two RTC source verifiers add 16 and remove 2
  (net +14). Source/test total: 45 added, 36 removed (net +9); the positive
  result comes from a dedicated collision regression, not a second path.
- The port-assembly fixture now checks an existing data-port read/write route
  survives RTC construction collision with no partial index route or RTC chip.
  Its existing two injected allocation failures still verify zero partial
  routes, zero configuration, chip cleanup and a successful retry.
- The static RTC boundary rejects any future raw `executor_port`, external
  checkpoint, range-provider or rollback path inside RTC construction. The
  Core DMA/RTC authority gate follows the retained typed RTC provider rather
  than a deleted wrapper constant.
- The exact sweep
  `rg -n 't_port \*|core_machine_port_add_|core_machine_port_registration_begin|core_machine_port_rollback_registration|executor_port' src/app-nxvm/devices -g '*.c' -g '*.h'`
  finds no RTC-construction hit after this change. Port-B/D4 remains the
  prospective S13 receiver, HDC S14, VADP S15. The Core-private port table,
  CPU bus and debug/firmware access belong to the later neutral-Core cut.
  The S7 handoff records this source-intake split; the former broad S12 row
  is not silently claimed complete.

## Verification and artifacts

- Both-width unit builds compile. Focused RTC/CMOS, NMI and port-assembly
  selection passes 11/11 per width. The registered RTC deterministic-time
  and Core DMA/RTC ownership verifiers pass on both widths.
- Complete repository-only unit suites pass 467/467 per width at `-j 4`.
  The NXVM documentation gate and `git diff --check` also pass.
- All four optimized Release pairs were rebuilt in `assets/nxvm/<profile>/`.
  Every x64 EXE is `pei-x86-64`, every x86 EXE is `pei-i386`, and none has
  a `.debug` section. Adjacent INIs and MyNES artifacts remain unchanged.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `779213914CD292102B11E5C99CC05AEB84D7B29BE5F2B007B88516A4980D20E6` | `9347B7DF82B21317E0273FD6C066220D2EB0DD1A8CD22F9CB05F4EDD17BCFE0F` |
| AT 5170 | `9F7E2F8D63C9CFCFA5492EBA1C1C4A980A6DB5BAED5AAAF7F6BE834031D53A89` | `86C50527915782D40B5069CE0FF89018062E7831C53CCF75606139924BD82B99` |
| Model 40 | `55EB4B37538259A3401F1CC01A8D11CD15B13329EADFB1D66A024411ED1BACC6` | `4FDAA0B47F4C1F03B0C865E585B243CA47DFC9590C6B2CCD201CF64F6C21E5D7` |
| Default PC/AT | `5ECC6A173555F911C5E5E9387E2F2DC7B73B701D59CE1089EDF0783E6D7BF11E` | `16C35F85F2FB2A5966530BFEF3C203D0D4BB7B7FA23BB33422948576A9E2F485` |

No timing grade changes. The full external four-profile integration gate
remains due at T540 closure.
