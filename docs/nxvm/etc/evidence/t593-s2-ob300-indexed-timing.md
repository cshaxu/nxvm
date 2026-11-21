# T593 S2: Indexed timing and exact fractional clock advancement

## Changes and equivalence

The four fixed CPU timing ledgers in
[cpu_timing_model.c](../../../../src/x86-core/cpu/cpu_timing_model.c) now use
one byte per enumerated form. The previous sparse representation repeated
the key in each row and linearly searched the profile's entries. A
mechanical comparison checked all 195 existing values, their per-profile
presence, and all 345 absent cells in the 135-form universe. Every present
entry is positive, so zero identifies an absent form. The bounds check,
missing status, unallocated tick, and successful form attribution preserve
the original contract. The separate REP ledger serves a different dynamic
contract and is unchanged. No model constants, formula selection, fetch
behavior, or preview semantics were changed.

The timing file diff is +218/-251, net -33. The x64 GCC object had 1,408 fewer
bytes in its text section and 1,024 fewer in read-only data. No persistent
table cache or mutable state was added.

[clock.c](../../../../src/x86-core/machine/clock.c) adds a +12/-0 fast path.
For a 32-bit elapsed count the product plus 32-bit phase fits in 64 bits.
Only if that entire sum fits in 32 bits does the code use native-width
quotient and remainder. Identity and wide conversions retain their existing
paths. The rational-clock smoke adds eight lines that first hit the exact
32-bit bound and then cross it solely due to the saved phase. Both the
returned ticks and the cumulative delivery are checked. Existing large
elapsed and odd-phase tests remain. There is no public interface change.

One unregistered historical script,
`cmake/nxvm/verify_80286_unallocated_publishers.cmake`, already failed on
the removed `if (prefixes != 0u)` anchor before the table conversion.
It was left unchanged. Current tests and the source manifest are the
qualification path.

## Serial original DOS comparison

The three variants were stock, indexed tables only, and indexed tables
plus exact fractional clock. One run per variant per width used the same
original System 1.0 ABA card, read-only DOS fixture and published worker
path described by [S1](t593-s1-ob300-execution-baseline.md). Only scratch
linking varied between runs. QPC samples inside individual helpers were
disabled to compare elapsed time with the same counter instrumentation.
x64 ran stock/indexed/combined, x86 indexed/combined/stock. All six runs
displayed the actual DOS prompt and completed DIR to a new prompt. The
readonly DOS fixture and external masters retained their input identities.

| Width | Variant | DOS prompt | DIR to next prompt | DIR retired | DIR guest ticks |
| --- | --- | ---: | ---: | ---: | ---: |
| x64 | Stock | 86.288 s | 5.767 s | 3,282,420 | 14,137,808 |
| x64 | Indexed | 76.764 s | 5.932 s | 3,286,380 | 14,161,189 |
| x64 | Combined | 77.637 s | 5.938 s | 3,280,482 | 14,126,374 |
| x86 | Stock | 96.716 s | 10.361 s | 3,274,837 | 14,093,031 |
| x86 | Indexed | 96.894 s | 7.997 s | 3,280,930 | 14,129,010 |
| x86 | Combined | 92.398 s | 7.965 s | 3,277,581 | 14,109,319 |

The x64 sample improved cold startup, and the x86 sample improved DIR.
The x64 DIR interval varied by about 0.17 s with no improvement.
The pairs are individual runs, with small variations in guest work and host
load. They show no reliably portable end-to-end gain of a specific size.
The isolated clock experiment below supplies independent arithmetic and
cost evidence. The original DOS usability gates are still unmet. No new
Window/RDP, power feedback or Windows throughput claim follows from this
comparison.

## Isolated clock comparison

The same existing differential probe ran 280,000 deterministic input cases
on each width. Candidate and stock agreed on return value, phase and
cumulative delivery. Each median is from five alternating-order pairs with
16 million calls using OB300 PIT and RTC ratios and matching final state.

| Width | Stock median | Candidate median |
| --- | ---: | ---: |
| x64 | 0.144297 s | 0.092876 s |
| x86 | 0.429558 s | 0.140653 s |

The small exact arithmetic optimization reduces the cost of the hot
operation even though it alone does not solve the guest latency.
Future work should sample the timing evaluator selection and repeated
branch-target preparation before deciding whether a dispatch index or a
decode cache pays for its complexity.

| Transcript | SHA-256 |
| --- | --- |
| Serial DOS comparison | `bfbc3e70691be6aa85209a39073b0d89f378f9e269d87e92cd63ba3817b40024` |
| Clock differential and cost | `87a37389f896d570d91af5f7ca5699b4048ffe71e348902c735649ada3e0f1ef` |

## Release and deployment

The cached Release product build published eight x64/x86 pairs. NXVM-family
banners advance from 0.5.0592 to 0.5.0593. MyPC110 keeps its established fixed
file names, and MyNES and MyRaidenII keep their versions. The previous
versioned PC binaries were retired after checking the replacements.
Every deployed PE architecture and SHA-256 was checked from the file bytes
and agreed with the publication receipts. Integration then relinked and
redeployed the default NXVM pair; the final hashes below match those tested
build outputs. Adjacent INI and owner media
were preserved. A private desktop started bit-identical copies of both
MyOB300 release EXEs, showed Ctrl+Alt+O in the startup menu and exited
normally. Each used a disposable configuration and CMOS and a readonly
C image. This verifies startup and configuration, not real RDP or suspend.

| Deployment under assets | PE machine | SHA-256 |
| --- | --- | --- |
| nxvm/nxvm_0_5_0593_x64.exe | 8664 | a68a23a9fc819066bc72b1b61d6b5dd42bc08c608eea5b317c995ad13c898abe |
| nxvm/nxvm_0_5_0593_x86.exe | 014C | 1dda2548ed2b7eb40048acecab8a85564e9d251c53e7f0c864bd7ce011263a9a |
| my5160/my5160_0_5_0593_x64.exe | 8664 | aaf230720686e80463f98725d50b4f63a9971ade05b74779e49a058b16a9c7ab |
| my5160/my5160_0_5_0593_x86.exe | 014C | 5a4359d91b4d6600a97b72b1e3d4b5dfb524120d05304edb9b8aea6e28870440 |
| my5170/my5170_0_5_0593_x64.exe | 8664 | 88472a7e11cd92fcba1014ac4533a09966e51ba65fae6be3cc4a7e53c6088232 |
| my5170/my5170_0_5_0593_x86.exe | 014C | b282b6359f88d0b48a0bb8b1bbed1df5632384930b536065018b7a38aae09a96 |
| mydeskpro386/mydeskpro386_0_5_0593_x64.exe | 8664 | a90c5163cb9f64664053088ea2769233116af4d22645a778dcd76c61e7189bf9 |
| mydeskpro386/mydeskpro386_0_5_0593_x86.exe | 014C | 5e9a4dfb899a8a81c0260923d82f5b637757cfeb74ea3a4ec3d9d96533cad443 |
| mypc110/mypc110_x64.exe | 8664 | 7385899b86e2acfac33ff3da367099fd0b15e0a7c2e94fa04b94561f3b8b86b4 |
| mypc110/mypc110_x86.exe | 014C | 009363933523f3a2776b36f389f1a6dc9e4b067223ec985c4f0d2cc5926a1022 |
| myob300/myob300_0_5_0593_x64.exe | 8664 | 474ace8b006589a2d723cc8110437b9899317158e4c50c56e7da9550a16ef461 |
| myob300/myob300_0_5_0593_x86.exe | 014C | 0ed6410e4f17f2c0c51c810b8a21348c1898ac2c52aaf86b28885bb4358e2260 |
| mynes/mynes_0_0_0044_x64.exe | 8664 | 6099952cc7ba83acb04ae104a1a3a698e123adc08c33cd4c50d2cc8127ead40f |
| mynes/mynes_0_0_0044_x86.exe | 014C | a11a8ded5881cff755c3da42994e5124d7a86834e1cc23d5558c8400301f7560 |
| myraidenii/myraidenii_0_0_0007_x64.exe | 8664 | 1629e3acf0924f9e28947fc45e3932fc4594a4573f4db82a1c2944af0e7385f9 |
| myraidenii/myraidenii_0_0_0007_x86.exe | 014C | 1637c47d66e3cd2bbb6d2ac76e8b24ec93dd39d40f5fdfd36526431c2ea7ca9c |

## Qualification

- Cached x64 and x86 Release builds passed for the 16 published receivers,
  followed by both unit-build targets and both registered integration suites.
- x64 units first passed 716/717. The added Markdown had one CRLF at EOF.
  The document was normalized and the exact registered LF case passed on
  rerun. x86 units passed 717/717 on the first run.
- Registered integrations passed 48/48 on each width with no timeout or
  selection changes. x64 took 271.44 s and x86 took 294.55 s.
- The original DOS comparison and native MyOB300 startup succeeded on both
  widths. No RDP interaction or release Windows boot run was added.
- Configuration, ROM, C media and unrelated pending documentation were
  preserved. Task-owned fixture images, binaries and copied configuration
  were removed after the probes exited. Bounded source and transcripts remain
  for review in the active task scratch.
