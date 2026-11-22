# T593 S6 decoded execution experiments

The running baseline is `9d77899d1`, with the verified S5 RAM read
mapping. Local Bochs 2.6 is a design reference. No Bochs code or runtime
dependency is used. All short runs use project-authored guest instructions.

## Rejected experiments

- Sharing instruction fetch preparation while avoiding the execution
  checkpoint in branch preview passed eight direct contracts per width.
  In matched Release execution of the complete short loop, three of four
  combinations had slower medians and one improved by less than 1 percent.
  The candidate does not enter production.
- Combining the two ModR/M register selectors into one static table removed
  173 lines and passed twelve direct contracts per width. Both host widths
  had slower ModR/M memory-loop medians, while the pure register loop changed
  little. This table does not enter production.

## 16-bit effective-address consolidation

The 16-bit decoder now chooses the displacement encoding once and uses one
base and segment selection for MOD 00, 01 and 10. It still reads the guest
registers at execution time. The existing instruction-fetch helper owns the
displacement read and any fault. MOD 00, R/M 110 remains an absolute
displacement with DS. BP forms still use the effective SS path and offsets
still wrap to 16 bits. The 32-bit addressing, opcode handlers and timing
classification retain their existing owners.

The candidate relink passed twelve existing focused contracts on each host
width. The extended Core operand fixture exercised all 24 16-bit memory
encodings on 80386 and 80486SX, with different values in DS and SS, signed
8-bit displacements and wrapped offsets. Both stock and candidate passed
on x64 and x86. This is 48 architectural cases per host width.

Three alternating Release samples used one million instructions per
scenario. Stock and candidate agreed in retired count, tick count and final
guest value. Mode-specific medians below are in seconds.

| Host | Operand | Stock | Candidate |
| --- | --- | ---: | ---: |
| x64 | Register control | 0.589 | 0.504 |
| x64 | ModR/M absolute memory | 0.665 | 0.670 |
| x64 | ModR/M indexed memory | 0.688 | 0.703 |
| x64 | Direct-memory control | 0.648 | 0.679 |
| x86 | Register control | 0.725 | 0.735 |
| x86 | ModR/M absolute memory | 0.915 | 0.919 |
| x86 | ModR/M indexed memory | 0.913 | 0.898 |
| x86 | Direct-memory control | 0.920 | 0.883 |

The ModR/M memory measurements show no consistent material change. Controls
vary more than the target loops. The production change is selected for the
single owner and the 95-line reduction in the original candidate, without a generic throughput claim.
This experiment uses no customer ROM or disk and establishes no boot or
command-latency result.

## Final decode handoff

Register-only ModR/M instructions enter register selection without running
either effective-address decoder. This preserves the 80386 control-register
form that forces register operands and the 486 timing annotation for real
memory operands. In the execution loop, the same per-byte prefix decision
drives both dispatch and the loop boundary. The V30 prefix timing charge
uses that decision as well. Neither change adds fields to the CPU snapshot.

## Decoded sequence boundary

This batch establishes a smaller execution decode owner. It neither
retains decoded blocks nor amortizes retirement over a block. A later
in-S experiment must prove reuse of static decode information in real
execution while the existing fetch, fault and timing contracts remain in
charge. The bounded decoder consolidation has received the following checks.

## Receiving qualification

`2d4e7606e` is the source and test delivery. The focused dual-width tests,
source manifest and inward-dependency gates passed. Each host width then
passed 717 unit tests and 48 integrations without an aggregate retry.
The cached release build deployed all sixteen products and verified their
architecture, identity and hashes.

One replay per host width used the original System 1.0 ABA card and
the same read-only derived C fixture as S4. It checked DOS VER, DIR
CONFIG.SYS and prompt, all ten key echoes and pause at the Machine and
frame observation boundaries. Temporary guest media were cleaned up.
The replay completed and the latency observations are below.

| Host | Cold through VER, measured / target | DIR to prompt, measured / target | Slowest key echo, measured / target | Pause, measured / target |
| --- | ---: | ---: | ---: | ---: |
| x64 | 72.145 s / 60 s | 6.531 s / 1 s | 34 ms / 250 ms | 16 ms / 1,000 ms |
| x86 | 89.530 s / 90 s | 8.651 s / 1 s | 50 ms / 250 ms | 17 ms / 1,000 ms |

Only x86 met the cold target. Both widths missed the DIR target. Both met
echo and pause. The cold observations were shorter and the DIR observations
were longer than in S4. Separate runs across S5 and S6 changes do not
isolate an effect of the decoder change. Restart, native redraw and RDP
performance were outside this replay. The original targets remain open.

| Published artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `d0f25934e02dc49021ed1222b5eea11581aa0fcf1bde047bd01736d8c8831645` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `31f43d45d83069987f7c99c72fa017c34afd7a0d8ddfc1bbc2b6099fdeb570e4` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `fb343a389a8eb239bb6fc0700b9354a431f99c6a9ac71843a4337718bfd3374d` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `bc98b3f16f5fb55158d8e88f98ef3a47a11a046039e586616549dc663aa879f2` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `05d20b54de18541a3a7b798afc4f6f5554a3490754f40633e44a8e5fdb00f944` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `800fc7fc79d246a82f6e4dc3a00677ca7e9086b47cfd37dd38706271ee0db3f7` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `e851fe1ea92b30e3f7f59046fac2fd1e41c817b3652f741e77072fcac1e2afb1` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `af8be2297fc4b74f1c671c7e8669350f8a13826ed6ca7c1e43e0a44792f00ffe` |
| `mypc110/mypc110_x64.exe` | `8664` | `4cb60af66ab01b94e0082f621d50e26e6061eafe115bb575e69277302b4fdd09` |
| `mypc110/mypc110_x86.exe` | `014C` | `7fea2af6d067fdffb9c9679e73680cf4335184e7fb0b998f8a47ad6f06f4d35b` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `286582b323f3e2da1b010c785c98b896677d992c6e234e99390a35922128d61b` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `bffe5b72582c57abe38619504f9f2e48d8d8742164b30855215580300a249bcb` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `5f8342d2e119585e62070cdbfa4a858599ad29dfc1285627e2538d7bb040f0d3` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `06d72513c9b3fa522ba672f2d8b6c90d190c7833f64db144f1211bdd18b15a0d` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `cefd18679b9eb746d0190376ce69a55aad4ce69d34ceff6efa9e964c7682fa0d` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `4a261d042f8782d7b2c1e3fadeefcbba3457f215d5c163c68baab89b9e7ed7b1` |
