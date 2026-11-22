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
slowdown. The owner selected the branch because the zero-tick path avoids division.

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

The owner selected the packed calculation. The standalone function
instruction count fell from 31/32 to 22 on x64/x86,
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
unchanged Core archive after a test object edit. A subsequent test rebuilt the
clock object itself and verified its new behavior through the existing
rational-clock contract. The warm and cold edited-archive links took
18.951/23.013 s on x64 and 23.190/29.022 s on x86. Reverting to the known
object linked in 0.838/1.041 s. This is partial reuse after a shared edit.
The benefit on the full Ninja graph still needs verification.

| Transcript | SHA-256 |
| --- | --- |
| Rational-clock comparison | `ff7fa514b0b87952af38d3501ad0d42ea15abd128ae86c790a1a33eab48346df` |
| PIC board comparison | `5735f5bfb97d07ae6e9ffd9bcff1b8d5a40726603bd268a31443518fb7d88336` |
| Incremental links | `164a7db239d66226186136d0e55b75e4a27897483dbce5e1a5a290cd37a07b5f` |
| Modified consumer relink | `ed16224adf01b5e729033e5ea40f8a9dcc6146930293f7e86a56daed712b133b` |
| Independent CMake caches | `f8eb2729e16ca84ef74e4103fbd3d7b6c6886bfe422aab362a499c13c78b453e` |
| Modified Core archive relink | `76f22c0c7cba5c149d5083b4b9faf0f3d301184e5540c9233f72648912facfe2` |

## Clock and PIC algorithm delivery

Build source revision: `508261dff94ffded29e80b769d369b7b86af585b`.

The Core rational clock skips division when no ticks accrue. The
NXVM PIC packs validated pending counts four at a time. Equal-work
results above distinguish the reduced operations from elapsed noise.
Both widths verified all 22 Core IPO compilation commands.
The normal cached Release build rebuilt and published all sixteen
products. The native integration link may refresh the default NXVM
timestamp, so hashes below identify the final qualified files.

| Host | Suite | Passed | Transcript SHA-256 |
| --- | --- | ---: | --- |
| x64 | unit | 717/717 | `968c407f2c930ccbc322560a7c1b48f455234594e18ab0d6f3b9a35defa1f964` |
| x64 | integration | 48/48 | `dc414fe1e71a03f98770158776b8d1988a02e5b95e419655dd36c5f4f2185e6a` |
| x86 | unit | 717/717 | `f4dbb06d61d0d41943f2152f19197efc53ea08b0544fb361bf86aec8862a79b6` |
| x86 | integration | 48/48 | `d3ad42db1b43d59e73092cb0a98521aedaf04470cb9796fc9bb6ad07120d7b8b` |

The reduced-instrumentation MyOB300 replay used the existing read-only
DOS and System Card fixture. Each width reached VER, verified DIR and
return to prompt, then verified ten echoes and pause. These are separate
single runs of the final build configuration. The paired stock comparisons
above provide context. Native redraw and RDP were not measured here.

| Host | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | Transcript SHA-256 |
| --- | ---: | ---: | ---: | ---: | --- |
| x64 | 63.154 | 5.795 | 48 | 16 | `71cbd1dbf678f9a7b418cc1a50921d9cbacee41eaf633021ff224a976a63e9bc` |
| x86 | 85.546 | 7.679 | 32 | 16 | `add346ac04a7c6dd4e752bf89f18d43a8b0bdbe468d61f65c1784290ae435a44` |

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `87078b487ef39fb43de1c875e2b92d5e155e26231082ca72d56b0dc538be99eb` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `7e59b18def68e8cb69fe814ac6b852e675920369f27b60bc1309b1330bf462e2` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `529bb7fbc6ba3e4c1db877caf921edede79863c1f8dc86492766e8946e999b69` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `8b73a8f91883ff0fe3820a32093e90a0efb53313e3e4a8a9f10b656c1084474b` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `ab398f43eb684b118ae194130571e7d383f9775f0c29f705b9e6a5caa8e0ec89` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `d78aeb6727844b98dd306f733ee12aaace3fa489ee53a570537c08b25f695b2c` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `c4f9686e93f9717a9b64f51cdb8939f1bb5f82f1354e7113127da5bf8dcfe40f` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `7c02e561fcfe134c48d21d2e898e30db758d62c61bf1f2615dd11002db33bc32` |
| `mypc110/mypc110_x64.exe` | `8664` | `13731a9fbdbb6a1363d84c7e5eadf3154fff32d4cf1a62702fc3bc7dbe8de752` |
| `mypc110/mypc110_x86.exe` | `014C` | `5763295e2ec6b12d82275c7734ab821e7a39d8798e283c03ed0875ae60808502` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `5f16dbbb0e48f082b771ca88ad5f685f1f6a88830890c93c019b35fa8c7b8c7b` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `30624bdf43d5e7031651e4d4fd06c5f45f7e16cebe5091f2afcfe7be695b8967` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `e6175d9fa02749e435c2caf3e2965632e7b05828b540e3a16309df1cfef76b4f` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `c94a65179b1ebfa68bd0976f118e6e3f3d087def4388f1374655c7f83a432ddc` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `069c900ee988ae0b734c417c3f28e7ff23e77839963fb52decf3da11b626871c` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `e420e570e9dde373ec25fe1c13e20f746968fb507a42bdb084336346aac7f199` |

## PIT quiet intervals and board clock work

For long quiet intervals, the PIT advances all three counter values
to the next boundary in constant work. The original per-counter order
still handles every boundary tick, including changes triggered by an
OUT callback. Mode 3 uses the same decrement rule for a single tick
or multiple ticks. No persistent state or public interface was added.

In each width, the differential probe covered 72,000 mixed operations
across 8253 and 8254 personalities, with and without cross-channel Gate
feedback. Counter state and ordered OUT transitions agreed at each
operation and each observed callback. In a one-million-input-tick
long-divider probe, the original three million per-counter tick calls
fell to 93. A load of three still handled about two million per-counter
ticks, and one-tick calls retained their existing work. The four existing
PIT fixtures passed on both widths. The terminal-count waveform fixture
also reads the latched count after a full free-running wrap.

The optional PIT clock is advanced only on boards with that PIT
installed. Its relative order with the primary PIT stays the same.
The PIT minimum search exits at one device tick, the smallest positive
boundary. Board deadlines outside the PIT still go through the existing
queries. Long quiet intervals do not establish a MyOB300 latency gain.

| Transcript | SHA-256 |
| --- | --- |
| Differential PIT operation and callback proof | `baf13e9ef82db4d4afc0878bdf4a5584094a87a30ea42a5e53f26ed1b2c6b783` |
| Existing PIT fixtures and tick-count proof | `7525c3af8821b765d9d9d9ddb0e14f5d55b0fa088048ce917b608e437dc0e213` |

## PIT and board advance delivery

Build source revision: `aea89116e26937aa2723109e482eafb0685f5afb`.

PIT bulk advancement uses existing event boundaries and the original
edge processing. Board advancement skips absent auxiliary PIT clocks
and ends the PIT minimum search at the lowest possible tick.
Both widths verified all 22 Core IPO compilation commands.
The normal cached Release build rebuilt and published all sixteen
products. The native integration link may refresh the default NXVM
timestamp, so hashes below identify the final qualified files.

| Host | Suite | Passed | Transcript SHA-256 |
| --- | --- | ---: | --- |
| x64 | unit | 717/717 | `e015cf5fb2d84ec976975fd9fe30bb0c73629755e584dd73d5d4c6aa1860258b` |
| x64 | integration | 48/48 | `42615b131601ce4801529971f5b3297c7a065e1f770e0def9bea0b324440c05a` |
| x86 | unit | 717/717 | `bbf1f94f446ab284350d4881afaababadc9ff1c93890b239016c85fa0f2525e4` |
| x86 | integration | 48/48 | `a290cc89d497a0cd2cd624626e2248e72d1d9b23fc0eab3b216e11e75c7ce78b` |

The reduced-instrumentation MyOB300 replay used the existing read-only
DOS and System Card fixture. Each width reached VER, verified DIR and
return to prompt, then verified ten echoes and pause. These are separate
single runs of the final build configuration. The paired stock comparisons
above provide context. Native redraw and RDP were not measured here.

| Host | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | Transcript SHA-256 |
| --- | ---: | ---: | ---: | ---: | --- |
| x64 | 67.678 | 6.745 | 32 | 16 | `00f28127d7c1a3fc5913f753fcc898e682fcb25e9bdcdcf6ccb39e240202c7fb` |
| x86 | 88.465 | 7.645 | 32 | 16 | `58c93d8e87efc1aa659134440b4e852972f7495ce51e0543fdd4ad9ea6faf47d` |

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `0b13a33cfb8d357506b10dd36e5814074c80198fa9223942c4c818729d72c76d` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `78fc98694d790cbf757059605b0cbd7f9291d440ec08aac170bb54e7e7775d74` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `e026a8c6c8fba5350c9fb10edff7a4670bb68d915b7c2f3efd20fc2d4425afae` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `1c3d0a503f4100d92f61bd3d0a58afd9e92d05a9f1b4f096efeec6c70d3fd323` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `bfac32e765cfe203530a7b493fa5fc7cd2a34449bb13dced7335c00da6b818a9` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `cf77dc28111b1bdbeececcb76c28c81666d4682d50f9ffc58d4140bf7f4909a1` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `eb97a1a8830d30767a4b2336a98c60188c66d16f8eff882517f6372907c849ea` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `3772d6ba25015015638dca28094eceea7aeb44e3e81b296cc25984851d6ce2b6` |
| `mypc110/mypc110_x64.exe` | `8664` | `d6321c1a1f3ab69f062f7a009c1b27fce6734be6affbd0559637fc8c3aa78aa5` |
| `mypc110/mypc110_x86.exe` | `014C` | `cb97e4e559e413f95d12760ac3f58cff507f9128a79ebfe0d9d5e3653628f723` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `2d119617bce5a96df1a84af8da1655d5517d5b6c2e6a2ff361d308c3d086856a` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `c8541addd6ebddc28152288fb4b7f960b02a050a57d14f72b5dd58592e02e347` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `464cb6d80d292268453e1d0c76642830b95c8f5496db75983d5fff64b7f5edb0` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `0395b2b920d87973a7e88d4d52053fe8a639842350a874c9589bc06b9316b0a3` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `edf490d4e77e47d08e1c3836a3f95d9d59aa13307f6f5130cf8022cc2f670b07` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `0bae1cc4f3f73f03b5979b58428866d13e5f09ce911868e1b191f084d4061d05` |

## CPU access records, clock conversion and IRQ line aggregation

The private CPU record retains all 512 address ranges for hardware breakpoints.
The public debugger consumes values for the first 32 accesses. Recording one
range per access and one value per observable access reduces the arrays from
12,288 to 4,352 bytes on both architectures. One helper records completed reads
and writes. It preserves watchpoint and fault ordering. An 80186 ENTER fixture
matched all 510 ranges and all 32 visible values on both widths. A 386/486
late stack write still triggered DR0 in both the baseline and candidate.

The separate unpaged translation early return was excluded. Across three
alternating x64 register-loop samples even its fastest run exceeded the
slowest stock run. Its memory-loop results were mixed. The measurements
below describe that isolated experiment only.

The inverse rational-clock conversion uses
positive integer ceiling division without computing a remainder. Each width
matched the original status and result in 8,019 boundary cases.

The board stores resolved IRQ levels and counts additional sources for each
line. Each refresh supplies the existing mask directly. On each architecture
17,202 stock/candidate observations matched the levels, IRR, IMR, ISR and
selected vector, including reset, saturation and mixed shared sources.

Single and cascaded DMA request arbitration now reads the primary chip only
when its signals decide the answer. Both widths matched all 51,200 signal and
topology combinations, reducing reads from 76,800 to 51,360 across that
inventory. Both existing DMA channel and binding tests passed per width.
The primary ALU ModR/M formats share one encoding rule with the same result
across all 256 opcode bytes on both widths. Neither change adds state.

All eleven combined CPU, Machine and board contracts passed on each width.
Their isolated build and the operation counts do not establish end-user latency.

| Host | Million-instruction case | Stock median, s | Unpaged candidate median, s |
| --- | --- | ---: | ---: |
| x64 | register | 0.456066 | 0.574392 |
| x64 | modrm-memory | 0.677713 | 0.637470 |
| x64 | indexed | 0.602558 | 0.632791 |
| x64 | control | 0.566959 | 0.550389 |
| x86 | register | 0.650683 | 0.586295 |
| x86 | modrm-memory | 0.753415 | 0.736954 |
| x86 | indexed | 0.696706 | 0.749987 |
| x86 | control | 0.760001 | 0.749826 |

## CPU access record and IRQ aggregation delivery

Build source revision: `b9aaae7defa66dbef85cfd82193257b9353a8747`.

The receiving set uses the compact CPU access record,
clock ceiling division, primary ALU decode, resolved IRQ inputs and DMA request arbitration.
Both widths verified all 22 Core IPO compilation commands.
The normal cached Release build rebuilt and published all sixteen
products. The native integration link may refresh the default NXVM
timestamp, so hashes below identify the final qualified files.

| Host | Suite | Passed | Transcript SHA-256 |
| --- | --- | ---: | --- |
| x64 | unit | 717/717 | `dbb8d6ce998f8f77f247b91a37cc1eb33c75739b31093f5c1be19e730fbf1f7e` |
| x64 | integration | 48/48 | `f652defc1ea63b43f6823723093c6ccf34cabcada31e12ac1ac7c33b9b6c4fb7` |
| x86 | unit | 717/717 | `f1206133c59dc7bc693d814bc0f732d5d8dede0ca9922569d2e6d852df0fccda` |
| x86 | integration | 48/48 | `3bde67300946cdac2b261b1d408fbb16e455e97e35b07569381b41035204d4a1` |

The reduced-instrumentation MyOB300 replay used the existing read-only
DOS and System Card fixture. Each width reached VER, verified DIR and
return to prompt, then verified ten echoes and pause. These are separate
single runs of the final build configuration. The paired stock comparisons
above provide context. Native redraw and RDP were not measured here.

| Host | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | Transcript SHA-256 |
| --- | ---: | ---: | ---: | ---: | --- |
| x64 | 55.376 | 4.592 | 32 | 16 | `2344925a10d15a84d5596cd0e6de13f13692ca10672a3af31685d9247c9ef9d2` |
| x86 | 74.776 | 6.677 | 32 | 16 | `042bde126b466fa13716f0d30f14ff4e3cbe02ba8f3657f9180ee19098df10e3` |

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `f4da8fc34e563ff97c592cdb65ce70da8fdf2d01aeec4408a396746bb9362807` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `64d0a20c1f8f4c3c2d1cf7a0814f7807f5cb761cd52c2bbd37de8b76861065fb` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `1fb712a10f105c9a0fc4ddd708f41438760bef77454a735e7ea0e9116db63df7` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `60ae66fde05e7178b689bf2e9e05a4e1c4b0cd0a2af8ea8e9277e62c9c5dedbb` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `2ff8d6c9a636208cd0a5597eb4cda5bbd64790731676832e0754122daa843b1e` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `d9c98f152ccb5203beca56ceb663136f607f53194b361386ea1a0551d535b5dc` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `dac3912b0a0452c84c93524a9c77d5a2d2e93d541b39d18d74517d5924e1dbf9` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `7051a06b6ef0d87c167ea903e944505bbf14f3217c6c2d1a7f73238b6fdf7e24` |
| `mypc110/mypc110_x64.exe` | `8664` | `27f17e08ba8c4c407870e157642bdb5d44e29088b5693979313f21a802f06e8c` |
| `mypc110/mypc110_x86.exe` | `014C` | `6e360390ed05a4b7c6a98f466dc6033473a9992e121043da6db1cb687f20f64d` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `8c148eda1eff30977ade1bd8b99a4a1ee6675cad12c398556da6e858b6a9dc54` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `502ca0fe9d08fe6e6218be760f4181f45b14c928c7151725ce9de70d9eac7ca5` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `84b4de8889d40e4e9dc5099a0f69f0332a8b50ee5840d7df7bd85117ab72b07f` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `f79b1168dddded2b84ac38f4cbf23101c9990302a372f93dd620ce538b62519e` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `97af1295301185b8393cfa2a9d0710da076d5ff43c9272f4d17d497062fa7a06` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `b141dc2e838587db356d4423bb402aa947397005f483f6f3ffa5c474b1b6890a` |

## Held candidate: retired prefixes, resident memory pages and board idle work

Retirement timing consumes the prefix count already recorded by the
executor. The count is bounded by the captured bytes for Intel timing.
The NEC continuation retains its full decoded count. The existing prefix,
time, fault and repeat fixtures passed for both widths.

Ordinary RAM stores can reuse a resident read-page mapping. A store on a
different page still uses the existing route resolver and leaves the read
mapping intact. The store still invalidates CPU memory, updates parity and
notifies observers in the established order. All 256 byte values and
injected parity mismatches matched on both widths. Firmware routes and
address decode keep their existing precedence.

The shutdown policy is queried only when the CPU enters Shutdown. Both
widths observed no policy calls during normal HALT and one during actual
Shutdown, with the existing reset result. Board deadline observation skips
unconfigured storage controllers and unnecessary positive conversions once
the minimum is one tick. PIT and RTC receive calls only for nonzero
device time. The PIC countdowns now use the width of their programmed
delay and preserve saturation for a wider elapsed interval.

The differential storage fixture matched 3,528 configurations per width.
At the OmniBook clock ratios, 100,000 short board advances made 32,849
PIT and 902 RTC calls. The same synthetic run delivered the same ticks
and clock phases at every step. The PIC fixture matched 400,000 timer
results per width. These are operation counts, separate from guest latency.


The cached Release build produced all sixteen receivers. Both host widths
passed 717 unit and 48 integration tests. The artifact identity list for
the held candidate had SHA-256 `5a40a9fc895669e5809c14c53a18e8aa560b245823f0416ebc50672383239504`.
No source or executable from this candidate was published.

The first candidate replay was followed by a baseline/candidate check.
The first baseline of each width started in an empty launch directory.
The initial candidate comparison was discarded because it inherited the
baseline's generated CMOS file. Only the candidate leg was repeated,
in newly created directories. All rows below began with no CMOS sidecar
and handled F1. They used the identical read-only DOS and System Card
fixture. The short checks do not measure native redraw or RDP.

| Host | Run | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | Transcript SHA-256 |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| x64 | first candidate | 62.068 | 5.648 | 33 | 16 | `787fa7f1f726341afd1decec488839613eeb19fa5da5310097925dc6d8fed124` |
| x64 | baseline | 60.434 | 4.339 | 33 | 16 | `01ffa054e9b211bb8fe41f9277176f2334e840eaee46e3f9d2cdbb9d7e1a8009` |
| x64 | candidate | 60.313 | 4.809 | 33 | 16 | `3903da9acdb3e4dcce0c8ff0f012c1c53dc94e94dac06325bfc94f3af7ff39fa` |
| x86 | first candidate | 69.182 | 5.604 | 33 | 16 | `7280df9cd98bab442bcfb337cafe718dc9063b8768200c6de5705a1f523f4606` |
| x86 | baseline | 65.006 | 5.684 | 33 | 16 | `35dadf75b8c981172601f0483b57393fe5bb73f5b5094d2d674d10a908bacec2` |
| x86 | candidate | 68.684 | 5.306 | 35 | 16 | `61bec90a623521cdc5515920293504fd508da8542ba41f9b153360cb45cc4c21` |

The fresh x64 check reproduced higher DIR latency with essentially the
same cold time. The x86 check improved DIR with slower cold time. These
are paired wall runs, not a controlled estimate of each source change.
The candidate is held while the shared costs are separated. The tracked
receivers were restored byte-for-byte to the previously qualified set.


A separate x64 diagnostic sampled the executing host worker during DIR.
Of 272 usable instruction-pointer samples, 54 were in physical memory
route resolution, 11 in routed reads, and 10 in plain-span checks.
This is flat statistical attribution with a sampler, independent of
the ordinary latency runs. It points to routing work worth isolating;
it does not prove the cause of the difference between the two builds.
The sampling transcript SHA-256 is `5aacce5940d5928734634a656342a21e78b767d04ef199f935699fe17b624ef4`.

## Held candidate: ROM route resolution and arithmetic timing

This batch isolated decoded-device resolution, skipped the raw scan below
the lowest possible raw mapping, and reused the existing results of
bit-scan and 8086 division instructions in timing selection. It also folded
odd parity without multiplication. Three production files changed by
+87/-112 lines. The existing CPU bit-scan fixture gained seven lines.
The two manifests changed four entries. Both widths matched 300,000
synthetic route cases each, including status and callback sequence,
and passed seven focused contracts.

All sixteen cached Release receivers were rebuilt and verified. Each
host width passed 717 unit tests and 48 integrations with zero skipped.
The receiving hashes are recorded in the qualification inventory, SHA-256
`05f1f77ceff1386d38bc8e932006a3a088ac8a398bab37c8ba9b54150cfb7ed6`. All four DOS replay
legs used the same read-only derived disk (SHA-256
`34d5597e7b040ea7a8b09d84234eea118d7f2cd8934cc4fc6c5eceaadf90b67f`),
the same System Card, identical instrumentation source, and fresh
launch directories without CMOS sidecars. All acknowledged the guest's
F1 check. x64 ran baseline then candidate. x86 ran candidate then baseline.

| Host | Run | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | DIR retirements | DIR guest ticks | Transcript SHA-256 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| x64 | baseline | 53.012 | 4.270 | 32 | 16 | 3,289,088 | 14,177,065 | `dbebb1249248ee3354933c77938d952921c7f1db87e64e84d3f79bcf57fe7abd` |
| x64 | candidate | 52.952 | 4.704 | 34 | 16 | 3,297,280 | 14,224,714 | `eee6014f8775564a6a7b80255aaf8cac152d7e2e44731800929c887d225411f3` |
| x86 | baseline | 67.015 | 6.050 | 47 | 16 | 3,297,280 | 14,225,104 | `8da7d833108dbc712398073fc134ea57b956f50aeb550b2cb1a75f7a8e7049d3` |
| x86 | candidate | 64.677 | 5.085 | 33 | 17 | 3,289,088 | 14,176,965 | `e9e57f84a46ac470aec372c4c849b8dbec182c55a554e023981a4f3e45cb86c3` |

Both candidates passed the cold, key echo and pause budgets. x86 DIR
improved by 0.965 s. x64 DIR regressed by 0.434 s despite only 0.25
percent more retired instructions. Neither width met the one-second
DIR budget. This paired check does not identify which source edit
caused the change. The batch is held. The six unpublished source/test
paths and sixteen receivers were restored to the previous published
bytes. Native MyOB300 Window redraw, restart and RDP were outside
these DOS runs.

The previously published x64/x86 run-end samples put 692/804 and
705/800 DIR endpoints on the guest page at ED000h. The sampled leading
opcode matched the mapped System Card ROM at every one of those
endpoints. In the local ROM inspection, 675 and 672 endpoints landed
in packed FAT entry reads, sector selection or sector-cache lookup.
These counts sample the end of each Core batch, not every instruction.
They support tests that combine ROM fetch with RAM data accesses. They
do not support bypassing active guest filesystem work as idle time.

## Selected candidate: SMI admission, compact restore state and memory routing

The existing SMM handler now runs only when the serialized CPU owner sees
a pending SMI. This retains the same two admission points and all internal
profile and state checks. Dual-width disassembly confirms that the ordinary
instruction path skips the call. The private CPU restore record shrank
from 340 to 308 bytes by removing eight storage slots with no architectural
reader or writer. The published CPU state layout and SMM image format are
unchanged. Seven existing CPU contracts passed per width for that layout.

Core retains the results of its two most recent page classification misses in the same
space as the previous one-page result and its padding. Entries contain
geometry decisions only. The existing A20 test now warms both slots
and reads both after the change. Deliberately clearing only one slot
caused the test to fail on both widths. A candidate that separated
instruction and data slots slowed the x86 FAT12 ROM workload and was held.

A selected device access skips byte-by-byte route queries if the
registered geometry cannot contain a higher partial claim. The earlier
route priority and per-provider fallback remain in charge. Differential
execution on each host width matched 184,087 observable records while
reducing side-effect-free queries from 45,541 to 39,877 in the fixture.
The particularly favorable disjoint-ROM probe is distinct from the
overlapping System Card geometry used by MyOB300.

The batch changed five production files by +56/-31 lines and one existing
test by +9/-2. Two manifests changed six entries. It uses the same
interpreter, bus transactions, guest clock and timing grades.

### MyOB300 paired DOS replay

Both variants use the same read-only derived DOS disk and external
System Card. All launch directories were fresh without a CMOS sidecar and
acknowledged F1. The isolated Core archives used the normal cached Release
configuration. The replay uses identical instrumentation source for both
variants. The baseline harness is the preserved previously published
receiver control. x64 ran baseline first. x86 ran candidate first.

| Host | Run | Cold through VER, s | DIR, s | Slowest key echo, ms | Pause, ms | DIR retirements | DIR guest ticks | Transcript SHA-256 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| x64 | baseline | 49.789 | 4.276 | 32 | 16 | 3,293,184 | 14,202,436 | `600f4758e4b6d2dfb24ad9a5441df2a97d90afe6477c96f0933ae35e2be41c28` |
| x64 | candidate | 44.412 | 4.123 | 32 | 15 | 3,293,184 | 14,200,984 | `0a4c964a2c427fc724810d7911d068cfe399f401bfb8e420eb8b4328d4cba780` |
| x86 | baseline | 62.340 | 5.310 | 33 | 16 | 3,289,088 | 14,178,218 | `4114e56cbdc37032a289d29356f9e818e072d0854a53178bc6f12244b1569c34` |
| x86 | candidate | 58.855 | 5.206 | 35 | 16 | 3,293,184 | 14,202,403 | `cb48df042186454f18592cb1def67e5485af44010b436f7fcd5498f590710907` |

Both candidate boots, all ten key echoes per width and both pauses were
within their declared budgets. The x64 and x86 DIR times improved by
0.153 s and 0.104 s respectively. Both still exceed the one-second
target. x64 matched the baseline DIR retirement count. The x86 count
varied by about 0.12 percent at the observed prompt boundary.
These paired observations describe the combined candidate on this host.
Restart, native MyOB300 Window redraw and RDP were outside this replay.

At the candidate's observed DIR workload of 3,293,184 retirements,
the one-second target requires about 3.29 million retirements per second.
This replay achieved about 0.80 million on x64 and 0.63 million on x86.
The gap includes all work between command submission and the returning
guest prompt. It cannot be estimated from isolated instruction-loop
throughput alone.

### Disposition of local System Card experiments

The banked-window experiment advances an observational query by one
16 KiB page once its selection is known. It retains bytewise reads and
the registered window priority. The dual-width differential fixture
matched 42,000 cases per width, including disabled windows, attribute
space and page crossings. Its lookup count fell from 1,052,273 to
51,048 per width. This is an isolated operation-count result. The
candidate stayed outside the Shared receiver batch.

A second experiment indexed enabled windows by page. It removed the
linear scans in its fixture, but the paired real replay measured x64
cold 53.533 to 61.311 seconds and x86 cold 68.091 to 71.724 seconds.
DIR improved on both widths. This trade-off was held. The same-size
register packing experiment matched 60,000 transfer/register cases
per width, then increased isolated median mixed cost by approximately
8 percent on both widths and slowed register writes. It was also held.

| Local transcript | SHA-256 |
| --- | --- |
| Window-span differential | `d17c581d2ae953433b23c59bab2b1e280468a5d3c71044202e5d3e424f7202da` |
| Page-index differential | `dd8015e7e9a98b3ee6b2c30b79413aef503f24c3fc8d2b23f47a832081c5337d` |
| Packed-register differential | `dd464a133c9fb89ee366dc85c7a51a9013b2612b24f2698c84aee54dc21fba3a` |
| Packed-register measurements | `d40976c34f6f5496db10d8f428f961fb46fdd6b2a09b1d77dc76dd3def06e62a` |

### Full receiving qualification

The qualified receiver set was built from Shared source `5a801f3329d364a3e73509fc4c833a9d5400589b`.

All sixteen releases were rebuilt together in the existing cached GCC
Ninja Release trees. Core CPU, FPU and Machine compilation used IPO on
both widths. Each architecture passed all 717 unit and 48 integration
tests, with zero skips. The final inventory records each deployed
executable after checking PE width and product version. Its SHA-256 is
`761d374080ca6ecd6944270e7f7c77d44eb3f04f425b519febba6e35833be8e6`.
The frozen Core input inventory used by the receiving build has SHA-256
`7599c361b61bfe962b28a8601728f5e7bd114253feefb5d28c1a735918b0ec5f`.

The integration build refreshed the default NXVM entry on x64 and x86.
Each refreshed deployment matched the build output used by the INI
identity test. The remaining executables matched initial publication.

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `472d196999259af54b70fdab68bc3bd9eef53261987a241d4467459f679e622e` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `82e1f1671c2c7d12cb44a1a2b364d65abebc35f5db3aeffb7882faf2fc01879f` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `7b816a4a1318a03386fe768a1b8540d8332c2246f369f666bdb3ca869b2561df` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `5d1e944781def36f3b163dd8aa8317a99aa33f3244a7b9e158218da2ae89e359` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `f27f3fe16fc5078e5236f34560454cf239add0d07c3beee0445575478b5148ca` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `094ebb86a560e4ac7055a43f66b746b4220b5f64fa07c6ceb555c183d66960cf` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `ceb675854a2abd6e8f5854c8446f9817b3054f02678fe25bdcae23a0a1b96339` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `0b7731f8d982a63398f0fdfbbc66287189a4ca710c069ec3a4c74718152f5dbe` |
| `mypc110/mypc110_x64.exe` | `8664` | `2b9373d5e0d040c14a1bf9b5aa04789ad7766bafc7abbb28a8fd69f378a810f8` |
| `mypc110/mypc110_x86.exe` | `014C` | `857b0f1158db687f05071ee6fcc6de27331cbacfb6170087a675163c05a3c3c2` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `56cef7b709380f315d260ff58e47a36dfe40059e35b615d90065bfb77212928f` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `15f5cd04d0d8b176d45f7435f73742f1162180c0b4583dff3a0def266bc43248` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `da105170a3fb412beec3d3852287d60fe5cfc192b7be2dda8db069e6cfd274a9` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `53a51d1afd2499aba49c1056017925d6d259d09e38194b6e0cf84427d013f318` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `055e1645a409d3cf7e13c3445c30eb9a4b5ff9f8f2881a9b57e3fb300b1db209` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `e77b8ca19359035e6e5c5408a5cb66b1208674228d66bdf8b33643430c532360` |

| Gate | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| unit | `f52098a53fc6e3bc78368364fe35ef929e9278383a3293dcc0d7131db93a949f` | `2dd575670c0d3256d5ca3af742971460d8cdcd5f22e1a9b2b03dbd19749d909f` |
| integration | `8c652f69b216786ccc1425e23c1c2e1db439061572cf1a19a46f86bc8a0aa2fc` | `4d37cbc6daf5022dba61d249aa84374c12722e9a178b196d1abf330d1908e26a` |

The 48 registered integrations on each width include five desktop-labeled
tests. MyOB300 native input-to-paint latency is a separate observation.

### MyOB300 DOS input and native redraw

The instrumented MyOB300 used the qualified Release link inputs with a
paint observer in the existing Win32 presenter. It ran on a private,
unswitched Windows desktop with the same read-only DOS disk and System
Card. Each run issued the monitor's `start`, acknowledged F1 through
the emulated Window, then sent ten make/break pairs to that Window.
For each letter the observer found the expected guest echo in the
region of a completed, successful `WM_PAINT`. All ten letters painted,
and both processes exited normally after the Window closed and the
monitor received `exit`.

| Host | Cold start to painted DOS prompt | Slowest key to completed paint | Transcript SHA-256 |
| --- | ---: | ---: | --- |
| x64 | 48.734 s | 16 ms | `c4d06fff503d05bfe314a7f6d7ed6118f3d06e80349ca41db4c8a2bbbca80e86` |
| x86 | 58.141 s | 31 ms | `580f642441cbbc37899a13600de9c5cb2a66bd9ccadddedbe71d4e6688044db2` |

Both native runs were within the 250 ms key echo budget at the host
tick counter's resolution. This measures posted native input messages
through GDI painting, without physical input or display scanout timing.
RDP, Windows guest workloads and MyOB300 restart performance were
outside this probe. The earlier paired DOS runs still miss the
one-second DIR gate.

### MyOB300 production Machine restart

The follow-up linked the same qualified Release inputs for both widths and
used the same read-only DOS fixture. It drove public Machine operations on
one machine per width. Reset reached PAUSED in a new generation before
resume. Stop reached STOPPED before the subsequent start. Each transition
required the guest to display the DOS fixture marker and C prompt in the
next generation. Only the fresh launch requested F1.

| Host | Cold start | Reset then resume | Stop then start | Transcript SHA-256 |
| --- | ---: | ---: | ---: | --- |
| x64 | 45.191 s | 45.626 s | 47.216 s | `cb3e086710a9ee15fb2b8dbe7418ef12cb59c9a1cda06de8bfb7c22e06c27da` |
| x86 | 57.944 s | 54.899 s | 56.045 s | `556069ed973490ea8bd4dff3aad511aeda2d23dfee770e911b5d76d19a840e96` |

Timing starts after start or resume is submitted. State checks and
generation checks establish the reset and stop boundaries separately.
These are Machine-to-DOS results. They leave RDP, physical keyboard and
guest Windows behavior outside the measurement.

### Performance gate decision

The current paired DIR results still require about 76 percent lower wall
latency on x64 and 81 percent on x86 to reach one second at the same guest
work. The earlier sampling run attributed 38.6 percent of its samples to
physical memory, card and bus functions and 23.2 percent to board, clock
and device functions. Even setting both sampled groups to zero in that
run's arithmetic leaves about 1.68 seconds. This arithmetic illustrates
why another isolated routing or device optimization is insufficient
under that profile. It does not bound what a different execution design
can achieve.

The reported improvement and receiving qualification stand. The proposal
requires an explicit scope or budget decision before a larger execution
redesign or Windows qualification. Preserve the one-second gate pending
that decision.

### Published x64 follow-up sample

One diagnostic run reused the published Shared source `5a801f332`,
qualified x64 Release link inputs and the existing read-only DOS replay.
During DIR it obtained 264 host instruction-pointer samples from the
Machine worker, with zero unavailable or previously suspended samples.
The instrumented interval took 4.279 s for 3,293,184 retired instructions
and 14,200,876 guest ticks. This is profiling evidence separate from the
unprofiled acceptance measurement.

| Sampled code | Samples | Share |
| --- | ---: | ---: |
| Physical routing, ROM, card and CPU memory-bus wrappers | 99 | 37.5% |
| Board, clock and devices | 68 | 25.8% |
| Remaining symbols and unclassified addresses | 97 | 36.7% |

The route resolver alone accounted for 52 samples. Its geometry scans
repeat on device pages, where the plain-RAM page shortcut cannot apply.
A bounded routing experiment could filter these scans by page, while
running the same provider queries in priority order on every access.
The experiment would need to cover raw reset aliases, decoded address
lines, provider range updates and removal, and accesses spanning pages.
It would change only the list of geometrically eligible providers.

Even subtracting every sample in the first two rows from the instrumented
interval leaves 1.572 s in this diagnostic arithmetic. An isolated
routing experiment can establish incremental value. Reaching the
one-second target calls for evaluating throughput across CPU retirement,
memory routing and device scheduling together. The source and shape of
such a redesign remain subject to the active proposal's scope decision.

The complete diagnostic transcript SHA-256 is
`1bd8a98ef95caccef4d3e25b3d758d6981ef5c0b47067b7817f18d9e83114ea6`.

### Page-filter prototype after continuation approval

The owner confirmed continued performance work on 2026-11-22. The
prototype filtered raw and decoded provider scans by page without
retaining a device's query result. The existing resolver still checked
the exact range and performed queries in the same order. Cross-page
accesses used the original unfiltered scan. Scratch state was cleared
for each geometry change in the differential fixture.

Both widths matched all 1,011,074 observable records and 315,355
provider queries. Including page fills, visits to provider geometry
fell from 5,067,779 to 2,113,405. Without trace output the randomized
fixture's median fell from 0.857 to 0.841 s on x64 and 0.852 to
0.833 s on x86. The same result does not imply equivalent gains when
executing guest code through the Core.

The equal-work FAT12 instruction fixture executed 1,549,105
instructions in 6,668,907 guest ticks, with result 15,552, in every run.
The following medians use three measured runs per case after a warmup.

| Host | Instruction mapping | Published Core | Page filter |
| --- | --- | ---: | ---: |
| x64 | RAM | 0.742 s | 0.760 s |
| x64 | ROM with overlapping route | 1.153 s | 1.328 s |
| x64 | ROM with disjoint route | 1.005 s | 0.992 s |
| x86 | RAM | 0.983 s | 0.961 s |
| x86 | ROM with overlapping route | 1.506 s | 1.674 s |
| x86 | ROM with disjoint route | 1.223 s | 1.267 s |

The overlap path regressed by about 15 percent on x64 and 11 percent on
x86. The prototype is held. The experiment changed neither production
sources nor deployed executables, and did not repeat the full DOS boot.

| Diagnostic transcript | SHA-256 |
| --- | --- |
| Route differential | `b07c633d8cc94476301ff2b9f5763c4e3a0a139f09c905209dbf0c0645968b0a` |
| Route timing | `06f982d931a1d6e83e49f395be506c76e362a326f82ba09a2a526256bcbae99c` |
| FAT12 Core execution | `00c536304bd91f8a4febec65cb0bf4bcd2508de5392407c73e14a00a3ebd047e` |

### Why the page filter regressed

The randomized differential counted provider visits without assigning
a cost to each visit. The overlapping-ROM Core fixture registers two
providers. The raw scan discards both through their mode flags. The
decoded scan usually has to check both. The x64 measured route counts
for the same 1,549,105 retired instructions were:

| Phase | Direct provider visits | Filtered provider visits plus cache-fill scans | Added filter lookups |
| --- | ---: | ---: | ---: |
| Raw address | 37,366,094 | 952,004 | 18,683,047 |
| Decoded address | 37,366,094 | 37,842,096 | 18,683,047 |

The filter discarded cheap flag checks in the raw stage and added
nearly the same number of lookups as there were discarded visits in
both stages combined. The decoded stage made more provider visits
after including fills. It still ran each selected provider's exact
range check. The generated x64 resolver calls the filter twice, and
uses the returned indices to visit the providers. Counting fewer
provider visits did not capture those operations or their cost.

One controlled comparison enabled the stages independently. Timing
runs did not include the counters. Each run kept the retired instruction
count, guest ticks and checksum fixed. Four measured medians followed
one warmup per case.

| Host | Overlapping ROM | Direct | Raw filter only | Decoded filter only | Both |
| --- | --- | ---: | ---: | ---: | ---: |
| x64 | Seconds | 1.152 | 1.230 | 1.308 | 1.315 |
| x86 | Seconds | 1.520 | 1.492 | 1.594 | 1.647 |

The decoded filter alone reproduced the regression on both widths.
The raw filter's effect depended on host width. Differences between
the four independently compiled variants also include generated-code
layout. The data does not attribute the exact cost of each instruction
or prove cache-miss or branch-misprediction counts.

A separate candidate fused the provider scans without storing a page
cache. It matched the complete routing trace on both widths, but its
overlapping-ROM median went from 1.144 to 1.192 s on x64 and from 1.486
to 1.458 s on x86. Its x86 disjoint-ROM median also increased from
1.220 to 1.269 s. Fusing the scans still performs both address
containment checks and moves decoded work ahead of raw selection.
The candidate stays out of the release.

For further route optimizations, count page lookup and fill work in the
same workload as the discarded checks. Keep the overlapping-ROM,
disjoint-ROM and ordinary RAM cases as controls, and separate stages
before measuring the combined candidate. The earlier held batches
remain separate observations.

| Diagnostic transcript | SHA-256 |
| --- | --- |
| Filtering stage attribution | `82c8094e5ba8bd749821a92c802b16c4ad428f9b2ac74f221552b0d68208ff20` |
| Single provider-table pass | `c06e3affac6de6a687657f0d98654984c22870149dfa592388f8492b531d76d1` |
