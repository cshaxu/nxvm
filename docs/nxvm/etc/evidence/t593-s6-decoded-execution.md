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

## 32-bit effective-address prototype

The existing decoder repeats 32-bit register and segment selection for the
three displacement encodings. A scratch replacement chose SIB and base once,
read displacement through the existing fetch path, and used the same handler
and 486 timing annotation. It removed a net 255 lines. The existing operand
contract passed on both hosts, covering the 80386 and 80486SX, and all timed
workloads agreed on one million retired instructions, guest ticks and final
result. The x86 medians rose by 1.4 percent for plain 32-bit addressing,
4.0 percent for SIB, 4.2 percent for the 16-bit ModR/M memory control and
11.5 percent for the register control. The x64 results were mixed. Reject this
version despite the source reduction. Transcript SHA-256 is
`ae798b1d7d665b485852934ced71c7868a720924412441a78aea3fbe4256514b`.

## GCC link-time optimization feasibility

The comparison used the combined released source at `71edc3b0c`. The
scratch tool collected 22 real Release translation units and their flags
from the existing Ninja graphs. The candidate compiled only CPU, FPU and
Machine with GCC LTO. It excluded changes to the board, App and guest
model. Three alternating baseline/candidate pairs per cell retired one
million instructions with matching guest ticks and final register or
memory values. Medians below are elapsed seconds.

| Host | Workload | Standard | LTO |
| --- | --- | ---: | ---: |
| x64 | ModR/M registers | 0.443270 | 0.379184 |
| x64 | ModR/M memory | 0.548893 | 0.477730 |
| x64 | Direct-address memory | 0.545605 | 0.523403 |
| x86 | ModR/M registers | 0.638012 | 0.538555 |
| x86 | ModR/M memory | 0.755912 | 0.658155 |
| x86 | Direct-address memory | 0.726937 | 0.594562 |

The same objects passed the existing 486 timing, Debug, fault/event and
Machine branch-preview contracts on both host widths. The combined test
transcript SHA-256 is
`4771accedbfbea402875a61ad17d6ec4ccb46ba7a49a03bf8cb4970cba77bae9`.

The paired MyOB300 DOS replay used the same read-only scratch disk and
System Card as the earlier replay. It wrapped only Product entry and the
Machine run boundary for both builds so the inner CPU and Machine calls
could be optimized. It ran stock then candidate on x64 and candidate then
stock on x86. Each run verified DOS VER, DIR, the returned prompt, ten key
echoes and pause. All media hashes matched after use.

| Host | Cold through VER, standard / LTO | DIR, standard / LTO | Slowest key echo, standard / LTO | Pause, standard / LTO |
| --- | ---: | ---: | ---: | ---: |
| x64 | 86.464 / 69.159 s | 7.164 / 5.482 s | 33 / 34 ms | 17 / 18 ms |
| x86 | 88.808 / 81.757 s | 7.264 / 7.283 s | 40 / 35 ms | 17 / 18 ms |

The measured workload at the DOS boundaries retired slightly different
work. These single-pair results favor LTO for startup on both host widths
and DIR on x64. The x86 DIR observation is unchanged within the resolution
of this comparison. The x64 cold and both DIR gates remain open. RDP, native
redraw, restart and other App integrations were outside this experiment.
The replay transcript SHA-256 is
`4bac97f1fbc2101bd6d3ec531763fbbe6da1f6cb48f6f89b5eb8a967c58413ad`.

The actual release still uses its standard GCC configuration. Production
qualification would need the regular static library and link layout, the
complete cached product matrix and its tests. The individual-object replay
does not validate a new shipping configuration.

### Static-library linkage

Three scratch archives with the normal CPU/FPU/Machine split linked and
ran on both hosts, including when the caller added no LTO flag. A second
four-way rotation compared the same ModR/M memory workload. All sixteen
runs per width agreed on 1,000,000 retired instructions, 4,500,000 guest
ticks and final value 53,392. Medians are elapsed seconds.

| Host | Standard | Archives, automatic | Archives, explicit `-flto` | Archives, one partition |
| --- | ---: | ---: | ---: | ---: |
| x64 | 0.534880 | 0.487736 | 0.489180 | 0.475577 |
| x86 | 0.725341 | 0.628791 | 0.591226 | 0.604974 |

In an earlier three-way rotation the x64 archive-auto median was 0.526356 s
against 0.526233 s standard, so code layout and host variability matter.
x86 archive-auto improved in both rotations. The smaller integration
surface is to try the normal archive path with GCC's native IPO, requiring
the full qualification build before changing the release. Neither these
timings nor the DOS object-link result justify a repository-wide linker
flag or a forced partition policy.
The four-way transcript SHA-256 is
`bdbdf3708d7bce10522ed1dce54615eed6fb2e96d5c917883d04af50e9eb1c28`.

## Native IPO and reusable-link qualification probes

Baseline `5f5b1b1bd`. A temporary CMake include enabled Release IPO for
exactly the CPU, FPU and Machine production targets. The two existing Ninja
trees generated IPO commands for all 22 units, built their real MyOB300
target, and passed the four existing 486 timing, Debug, fault/event and
branch-preview tests on each width. The temporary setting was removed and
the standard Core archives were rebuilt. The deployed 16 EXEs kept their
recorded hashes throughout.

The following single-pair DOS comparison used the three archives generated
by CMake and the normal link command. It retained the same read-only DOS
fixture, board, firmware and guest I/O. It verified DOS VER, directory
listing with returned prompt, ten key echoes and pause. Stock and candidate
retired slightly different amounts of work at those externally observed
boundaries.

| Host | Cold through VER, stock / IPO | DIR, stock / IPO | Slowest echo, stock / IPO | Pause, stock / IPO |
| --- | ---: | ---: | ---: | ---: |
| x64 | 77.752 / 63.779 s | 6.515 / 5.514 s | 36 / 35 ms | 17 / 17 ms |
| x86 | 84.733 / 82.866 s | 8.843 / 7.650 s | 35 / 34 ms | 17 / 19 ms |

The x64 cold and both one-second DIR gates remain open. Native redraw and
RDP were outside this replay. The temporary media copies were removed.
The real IPO target also showed a build cost: the four x86 test links took
about 23 to 24 seconds each, and the real MyOB300 link took about 28 seconds.

A second scratch experiment used GCC relocatable LTO to compile the 22
units to a native object once per host width. The result contained native
text and no deferred LTO sections. The CPU fault/event contract passed on
both widths. All three executable variants agreed on 1,000,000 retirements,
4,500,000 guest ticks and final value 53,392. Link values below are one
measurement per executable. Runtime values are medians of three rotations.

| Host | Measure | Stock | IPO at every link | IPO once |
| --- | --- | ---: | ---: | ---: |
| x64 | Final link, seconds | 1.829 | 27.459 | 1.749 |
| x64 | Memory loop, seconds | 0.674374 | 0.604749 | 0.619409 |
| x64 | Probe executable bytes | 637128 | 600450 | 682676 |
| x86 | Final link, seconds | 0.946 | 25.005 | 0.952 |
| x86 | Memory loop, seconds | 0.703658 | 0.605372 | 0.611465 |
| x86 | Probe executable bytes | 696657 | 641991 | 737528 |

The native object cost 30.227 s once on x64 and 29.927 s once on x86.
The unified object is a feasibility probe. One large object draws all
three components into a consumer and increases the probe executable size,
so it is unsuitable as the shared package boundary without further work.

| Transcript | SHA-256 |
| --- | --- |
| Native CMake qualification | `f8ba619bfa41f870aba6a398440d122470afa3d6349c915eb024670cee0d3e3a` |
| DOS comparison | `3b7f9c33d7948f1cc63f424c63096b9ce7f01473e4f378b493853d53a4d13ab3` |
| Relocatable link comparison | `16c56de53f43d8f298c7bee5d5a8df28acd5b1468240eb012f610ddca89df2b5` |

### Independent native archives

The next prototype performed the same relocatable step independently for
each existing CPU, FPU and Machine archive. The release link retained the
same three-library order. The input was the same 22 IPO translation units.
The output archives contained native text and no deferred IPO sections.
All measured variants agreed on retirements, guest ticks and final values.

| Host | Loop | Stock | Unified native object | Independent native archives |
| --- | --- | ---: | ---: | ---: |
| x64 | ModR/M memory | 0.519282 | 0.480460 | 0.513057 |
| x64 | Registers | 0.443126 | 0.399388 | 0.419035 |
| x86 | ModR/M memory | 0.731282 | 0.640149 | 0.696838 |
| x86 | Registers | 0.598604 | 0.512012 | 0.569978 |

The final link took 1.474 s on x64 and 1.073 s on x86, each measured once.
The independent archives retained about five percent in the register
loops and less in memory, with a noisy x64 memory result. They preserve
component ownership but give up some cross-library inlining. Neither
reusable variant has passed the full receiving suite or an actual-guest
latency replay. There is no shipping configuration change in this batch.
The three-way run transcript SHA-256 is
`73e8786efe8308ccf7f87939b2bc34d8aee9b30224bd52db4724728d09523a8e`.

## Root GCC Release IPO delivery

Build source revision: `f1d57e5bd9508e0d1f5dbdd834df29fa6acbb4f9`.

The repository root selects native CMake IPO for the three X86 Core
production libraries when the Windows GCC toolchain supports it.
The standalone source package retains its own build defaults.
Both width configurations verified all 22 Core compilation commands.
The normal cached Release build rebuilt and published all sixteen
products. The native integration link may refresh the default NXVM
timestamp, so hashes below identify the final qualified files.

| Host | Suite | Passed | Transcript SHA-256 |
| --- | --- | ---: | --- |
| x64 | unit | 717/717 | `ea3824130c9dc5a513cab32fff461aee0bd95f3aa971f997fecb4168effb3b01` |
| x64 | integration | 48/48 | `adb5b73e0c3a64c52709493ca1502dc3eabfcf3894f9d13472a43fc8ae514f22` |
| x86 | unit | 717/717 | `b5c4f146432cf8325de78e2946db3977ca3f2584fd3ea9a46ae9e589d588ec3b` |
| x86 | integration | 48/48 | `30192f1a2b366d10377bc764cc66465d6fa1960987fd273e55d60eb6bf6364a5` |

The reduced-instrumentation MyOB300 replay used the existing read-only
DOS and System Card fixture. Each width reached VER, verified DIR and
return to prompt, then verified ten echoes and pause. These are separate
single runs of the final build configuration. The paired stock comparisons
above provide context. Native redraw and RDP were not measured here.

| Host | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | Transcript SHA-256 |
| --- | ---: | ---: | ---: | ---: | --- |
| x64 | 56.300 | 4.936 | 34 | 18 | `8d92bdeff897eec7a706ad210590c7c7752b40e55d1ef8ac2c921c5ede8f185e` |
| x86 | 81.699 | 8.695 | 49 | 17 | `df5c4ce0e972795c2fdcaf4f5bd7f219839ca40d42552783fa734c13dd76fc5a` |

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `d018e5a55b6d0eda305d35190bbf50936c45062d73eda20c9e3c4386136036ab` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `97eb623e7598510acbc3bc7f1130b3cc7d25974aa64d0d5580b6e9803d3b2cbd` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `9299b2cbf4c520471a204b12cc515d1b8eefc54ce843c0d98df17f2dac8e0948` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `0c47bc173883f7fc83f61642a9b6257053320c24225a711bdbca5d8882669c22` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `ec037328b4986eddb38427e6d6b96845b70efdb435467a64ec5596e82b603008` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `29498196f7ceb5edd26ca144b39fe17207bae507abf8c4879ff0ae505b2decf0` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `ef7b5023310e945174ba4df39de727d6f40c9d07fb5b70494948a27717c2914d` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `b51821e5e49a8b49d15710ecb8ff631d0a4f94a6b54f9c02f5321ccc620d0ae0` |
| `mypc110/mypc110_x64.exe` | `8664` | `f826c38e5d164fc65fb1528eb3aa58861f790f7dcbc46419274a425e23fc22c7` |
| `mypc110/mypc110_x86.exe` | `014C` | `f69ad3aaeae8d1e4c28b607dc235e7ef2b33bd308db8d0f65a5ac62c77b94ff7` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `d8eeb360fe8323b82936b0b3b8d63bf6001a8c4407f2d64c75db256f20e70b39` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `40f7b399d45898a9cb4be78f3ff7c6bdafbcd56a740e671590dfd6dda1a5717c` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `66da8e1fcd63f093136dfa41e78243aaeee265e6075e7998b0f44538dc94ee96` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `b46d715a167850876cdb899c2fc60fd28fe6d481f6c7588e2b4b2c1b8441cb50` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `61c8b96ca335aca2d29efd632261f7ec094acfe2e6046638aa6decc033fd8e2e` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `dbee124eb6c2b27ccdedd5e2d52322f97737d0298e2e865b56aac0e2d8a38574` |

This first full IPO test build took x64 22.1 min and x86 22.3 min
for the unit targets. The isolated per-owner and caching probes remain
follow-up build-cost work.

## Post-release runtime and link cost probes

The baseline was `9246e0937`, the published Windows GCC Release IPO build.
All experiments used the same sources from that release. Changes were
confined to temporary overlays and no product executable was replaced.

The fractional clock prototype updated only saved phase and returned zero
when the scaled elapsed value was below the denominator. This is exact and
passed the existing rational-clock contract on both hosts. The four
two-million-advance scenarios produced identical board checksums with and
without the candidate. Comparisons included a single-tick step and one to
eight ticks, each with the auxiliary PIT installed or absent. x86 medians
moved between a 2.8 percent improvement and a 0.6 percent slowdown. x64
medians moved between a 3.2 percent improvement and a 6.4 percent
slowdown. No demonstrated general win justifies adding the branch.

The PIC prototype packed four source counters at a time into a level mask.
It preserved overlapping IRQ sources and did not add a second copy of
state. It covered all 256 byte values per lane and mixed high-bit cases.
Both architectures passed the existing lifecycle fixture, including its
same-owner allocation failure injection. The two board workload checksums
matched throughout.

| Host | Board | Stock median, seconds | Packed PIC median, seconds |
| --- | --- | ---: | ---: |
| x64 | Cascaded, no auxiliary PIT | 0.605067 | 0.622041 |
| x64 | Single with auxiliary PIT | 0.863398 | 0.849575 |
| x86 | Cascaded, no auxiliary PIT | 0.961710 | 0.922624 |
| x86 | Single with auxiliary PIT | 1.263322 | 1.202997 |

This extra source is not selected given the mixed x64 behavior. The
standalone function instruction count fell from 31/32 to 22 on x64/x86,
but this does not predict an application-level latency reduction.

The 22-minute unit-target relink is an independent build cost. Installed
GCC 16.1 wrote an incremental LTO cache and passed three sampled existing
Core fixtures on each host through stock, initial-cache and repeat links.
The cache held 96 x64 files (8.77 MiB) and 84 x86 files (8.08 MiB) for the
three test consumers. A modified test was then recompiled with a different
success marker. Both the warm and cold relinks executed the new object and
printed that marker.

| Host | Standard link range | Repeat with cache | Edited test, warm / cold |
| --- | ---: | ---: | ---: |
| x64 | 29.10–31.12 s | 0.86–0.92 s | 0.862 / 24.670 s |
| x86 | 25.85–27.02 s | 0.90–0.92 s | 0.984 / 26.460 s |

This toolchain fails a link if the cache directory does not exist. It
also prints that its incremental cache has no file-locking support. A
temporary CMake fixture confirmed that two concurrently built executable
targets independently populated pre-created target-specific cache
directories on both widths. A shared writable directory is excluded
from the repository candidate. These probes establish reuse of an
unchanged Core archive after a test object edit. Reuse after a Core edit
and the benefit on the full Ninja graph still need verification.

| Transcript | SHA-256 |
| --- | --- |
| Rational-clock comparison | `ff7fa514b0b87952af38d3501ad0d42ea15abd128ae86c790a1a33eab48346df` |
| PIC board comparison | `5735f5bfb97d07ae6e9ffd9bcff1b8d5a40726603bd268a31443518fb7d88336` |
| Incremental links | `164a7db239d66226186136d0e55b75e4a27897483dbce5e1a5a290cd37a07b5f` |
| Modified consumer relink | `ed16224adf01b5e729033e5ea40f8a9dcc6146930293f7e86a56daed712b133b` |
| Independent CMake caches | `f8eb2729e16ca84ef74e4103fbd3d7b6c6886bfe422aab362a499c13c78b453e` |
