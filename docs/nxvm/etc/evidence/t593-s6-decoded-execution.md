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

## Primary opcode classification candidate

Baseline `22540337f`. Classification of the first byte uses only the CPU
profile, primary opcode, and the existing first-stage ModR/M placeholder.
The scratch experiment generated both trait bits from the existing decoder
for all nine enum values and all 256 opcode bytes. The actual candidate
prepares the same 256 entries in the owning CPU context and refreshes them
on a profile change. The real fetch occurs before lookup. Group and ModR/M
checks, NEC emulation mode, exceptional entry, and instruction handlers
continue through their existing owners. The observational preview omits this
cache and keeps its own existing profile and byte checks.

Each width passes seven focused Core tests. The expanded legacy ALU fixture
also checks 286 to 386 to 286 rebinding in the same context: operand-size
prefix 66h is admitted and used on 386, then rejected with invalid opcode
on 286. Both widths pass the rebinding case.

Matched Release runs used a neutral AM386SXLV machine and three alternating
samples per condition. For each opcode mix, stock and candidate agree
in the one-million-instruction retirement count, guest ticks and final guest
value. The medians of the actual candidate are in seconds.

| Host | Workload | Stock | Candidate |
| --- | --- | ---: | ---: |
| x64 | ModR/M registers | 0.556248 | 0.472557 |
| x64 | ModR/M memory | 0.630936 | 0.587692 |
| x64 | Direct-address memory | 0.582800 | 0.542533 |
| x86 | ModR/M registers | 0.689177 | 0.686064 |
| x86 | ModR/M memory | 0.792358 | 0.743677 |
| x86 | Direct-address memory | 0.748831 | 0.720077 |

The two memory mixes show approximately 4 to 7 percent lower elapsed
medians. The two register mixes have visibly different dispersion, and the
x64 candidate has a high outlier. These short runs support retaining the
private classification cache for broader qualification. The candidate
does not yet have a new MyOB300 timing observation, complete receiving
qualification or any newly deployed executable.


The cached Release receiving unit builds passed 717/717 on both widths.
CTest elapsed time was 300.92 s on x64 and 129.92 s on x86. Transcript
SHA-256 is `15712bf0c03e0c9e9b43acde887b46a5a0009718f1ce2595598355a7df47d173`
for x64 and `bba2ee6ccc030b5475d474de5a8645adeb9444867a51a66fe89cb5209270a3cc`
for x86. These totals qualify the opcode-traits candidate before the separate
breakpoint guard below. Neither unit build invoked a publication target.

### Further instruction-path checks

The full architectural checkpoint cannot be dropped on the ordinary path.
The fault handler reads it for restart, and SMM and task paths can restore it
during the instruction. Retirement and I/O restart also borrow facts from it.
An instruction-class gate would need to prove that no synchronous or external
fault on that class reaches any of these uses.

The instruction-breakpoint comparator has a narrower opportunity.
`_debug_match_instruction_breakpoint()` currently walks all four comparators
even when the eight DR7 local/global enable bits are all zero. In that case
`enabled` must be zero, and its sole caller commits the computed match bits
to DR6 only when the return value is nonzero. An aggregate early check can
skip the walk while still reporting disabled matching comparators whenever
a separate enabled comparator causes the fault. The existing
`cpu_debug_state` matrix covers this distinction for all enable/match masks
and both local/global enable sources.

The exact one-line change was evaluated using the linked existing Debug
fixture on both host widths. Each architecture passed the comparator matrix
for the two supported CPU profiles, plus the existing RF, TF, segment-shadow
and exception checks. The source change now skips all four breakpoint
comparisons when neither local nor global breakpoints are enabled. The
architectural DR7 value, the condition for changing DR6 and the enabled path
remain intact. A cached Ninja rebuild of the combined source passed six
focused CPU tests on each width covering Debug, profile rebinding, V30 ALU,
operands, cache and protected IRET. Whole-machine performance for this
change was not measured.

### Single-byte prefetch copy experiment

A scratch replacement specialized the two byte copies in `_s_read_cs()` for
a one-byte request. Fetch validation, source selection, byte recording, and
the multi-byte path remained unchanged. Each timing used five alternating
pairs with the same one-million retirement count, final guest value, and
guest ticks. This experiment was compared against the opcode-traits candidate.

| Host | Workload | Unchanged, seconds | Specialized, seconds |
| --- | --- | ---: | ---: |
| x64 | Registers | 0.472961 | 0.467857 |
| x64 | ModR/M memory | 0.550661 | 0.516778 |
| x64 | Direct-address memory | 0.509520 | 0.499484 |
| x86 | Registers | 0.648879 | 0.655252 |
| x86 | ModR/M memory | 0.747778 | 0.770217 |
| x86 | Direct-address memory | 0.763738 | 0.746710 |

The additional branches improved the x64 ModR/M workload by 6.2 percent and
slowed the x86 version by 3.0 percent. Reject this implementation. The code
change stays in the scratch experiment.

### MyOB300 combined-source replay

One background run per host width relinked the existing DOS probe against the
combined private CPU changes. It reused the same read-only DOS fixture
(`34d5597e7b040ea7a8b09d84234eea118d7f2cd8934cc4fc6c5eceaadf90b67f`).
Each run verified DOS VER, DIR with a returned prompt, ten visible key echoes
and pause, and checked that the scratch disk hash did not change.

| Host | Cold through VER / budget | DIR to prompt / budget | Slowest key echo / budget | Pause / budget |
| --- | ---: | ---: | ---: | ---: |
| x64 | 76.576 / 60 s | 5.955 / 1 s | 35 / 250 ms | 17 / 1,000 ms |
| x86 | 99.894 / 90 s | 7.251 / 1 s | 51 / 250 ms | 18 / 1,000 ms |

Compared with the earlier S6 run, both observed DIR intervals fell and both
observed cold intervals rose. The separate runs retired different instruction
counts and guest ticks. This comparison does not isolate a causal change
in MyOB300 elapsed time. The two cold and two DIR targets remain unmet.
Restart, native redraw and RDP are outside this probe.

## Combined-source receiving qualification

Shared source and tests: `b1d81e009651d37611fdc3b3861914e1c75be934`.

A cached Release product build compiled and deployed every supported
app on both widths from that same source. GNU Ninja builds reused the
existing x64 and x86 configuration trees. No media or INI was part of
the publication. The receiving suites ran after artifact deployment.

| Host | Route | Passed | Transcript SHA-256 |
| --- | --- | ---: | --- |
| x64 | unit | 717/717 | `0525ad27565462df5946ed298dd87d2fbac28c9020cb0c4e346b671ed7e566fa` |
| x64 | integration | 48/48 | `de3eae170364fae9b6e1bebcd6de5f0ebc71c4c6bb575e5fef07de21114d142c` |
| x86 | unit | 717/717 | `1671cbb2eedb31bf13bedb459ba384531eb67361a6070d6766e4e8aa76226ce7` |
| x86 | integration | 48/48 | `454c93d5504eda231d4a476e9ff5dd809b597f502f94f3c73d69da2a5ebd0a26` |

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `e7df6c0158744a44f25e7eaaad9609101e053b35c1c13abed23ba72173205a1a` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `517196059027e3ba3673f9a4a0a24cdd9aee952696463ad108e1b42e626b07ab` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `02f15e2a94c3c7d62d81ec80152326defd1404855c9d46f09649fce7268fe73e` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `734a60f5ffdce61c7d002d4a9e41dae6cf054cd4fba4e21776c4c92b988ffe43` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `5b6a914d4cccb4201bfa6e1cc435bbd39dc2ea3a6ef64eb28833d023a0137421` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `c97a4c51b0c81f69c36ee583325613d0015cde175aa36b2b2c62e93695ae8c19` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `6e4126eb68d9e3e66795a52d840f257616f944a8e43309ba33864bed6de6486c` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `cc8c8160ddfed6f075afa30ee022c70de66f0521c8c5b22f987e9ab32216ced2` |
| `mypc110/mypc110_x64.exe` | `8664` | `ec85077063cc3ef6402c6723ae9676104f0d2298dd3c3fa0ddfc6bb541a4a0b0` |
| `mypc110/mypc110_x86.exe` | `014C` | `31031a2b448a07a46b41f8e4e817dc6586708f9ec9f3f3563860f3921eee919d` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `a510b120c53ee6d0f56daa96ea4b5574c8ba5bec6e44c7b7231d6bcf8994e5b5` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `6baae9d158ce7dfef66b4cd1ffb8e4648405e88802e06df271676e2b4ebcace3` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `5c6b79005b66183cd28b7b80dbf96212bac4031e7e2ba46fb913c52dfeee28ad` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `b3c57827b84291af306761a27da6e008506f71dbaefce5c34efa788a9b7c9830` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `6ca8f066df9befa3ece2d1e36286e4555793cae84b3a05af81a8c6909d4901db` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `ee513f07a47f65898ec6fee192051363075fe78a2cca6d21aad8999586635a3b` |
