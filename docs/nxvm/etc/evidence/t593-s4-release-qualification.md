# T593 S4 release and usability qualification

[Timing dispatch proof](t593-s4-cpu-timing-dispatch.md) records the exact
opcode predicates and the two 30-case development checks. The production
change is +47/-12 lines plus a replaced manifest hash. No memory routing,
timing formula, guest clock, public API, or unselected profile was changed.

Both cached Release builds and all sixteen product deployments passed.
Both widths passed 717/717 registered unit tests and all 48 integration
cases. The x86 one-shot aggregate hit its 300-second deadline after 35
passes, while the PC110 cold-boot handoff was running. Only that
unfinished test and the remaining 12 were run in the continuation,
with unchanged per-test timeouts. This proves 48 case outcomes across
the two runs. The original aggregate invocation remained a timeout.
The final default NXVM hashes match the builds used
after integration re-publication. Each other EXE matches its publication
receipt. Native private-desktop startup showed the expected Ctrl+Alt+O
menu and clean exit in both published MyOB300 EXEs.

## Original MyOB300 DOS qualification

One new DOS run per width used the original System 1.0 ABA card and the
same derived read-only C fixture as S3. The production Release archives
were linked with the observation driver. Each run checked the original
DOS identity with VER, the CONFIG.SYS entry and following ready prompt,
ten successive key echoes in published frames, and pause completion.
The temporary guest media copies were cleaned after process exit.
Timing is measured at Machine admission and published-frame observation,
without a native keyboard or direct/RDP paint timing claim.

| Width | Initial DOS | Cold through VER | Cold target | DIR to prompt, target 1 s | Pause, target 1 s | Slowest echo, target 250 ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| x64 | 74.125 s | 74.514 s | 60.000 s (miss) | 5.337 s (miss) | 14 ms (pass) | 33 ms (pass) |
| x86 | 90.510 s | 90.893 s | 90.000 s (miss) | 6.771 s (miss) | 15 ms (pass) | 47 ms (pass) |

Each key value below is milliseconds. The frame consumer polls in 8 ms
intervals. These are response checks, not emulator instruction timing.

| Width | A | B | C | D | E | F | G | H | I | J |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| x64 | 15 | 32 | 33 | 32 | 32 | 31 | 32 | 32 | 33 | 32 |
| x86 | 30 | 47 | 31 | 16 | 47 | 16 | 32 | 47 | 47 | 47 |

The DOS workload adds VER before DIR and echo/pause afterward, so the
timestamps are evidence for the acceptance limits and not matched
benchmarks of the S3 and S4 implementations. This pass does not retime
cold restart through reset/resume or stop/start, exercise original
Windows performance, or qualify direct/RDP native input and redraw.
Keep the original target unchanged until the owner decides the remaining
scope based on these measurements.

## Final artifacts

| Deployment under assets | PE machine | SHA-256 |
| --- | --- | --- |
| nxvm/nxvm_0_5_0593_x64.exe | 8664 | `1383c0a11a6713ace5379b0b01f34d5095cbb1d69c682c71b4dbcad5da8ef307` |
| nxvm/nxvm_0_5_0593_x86.exe | 014C | `fae0b9b1da581b59ab0c36093886db72a800dbb43073ae3839d85fabd09f3e62` |
| my5160/my5160_0_5_0593_x64.exe | 8664 | `8ec8eb675156cf5fff5b592371564f8e11fc06e1a8f1719ea9f0e4bfd28a94b5` |
| my5160/my5160_0_5_0593_x86.exe | 014C | `fac2cea67d9e8e46e1a98dc0fb834f79d15bfadbaf0cd4806e137bcd38ada3c7` |
| my5170/my5170_0_5_0593_x64.exe | 8664 | `084a4bc5992aab2c61bd973b3ab1bd47e1b61945640d0d23b995d0cd5860fa05` |
| my5170/my5170_0_5_0593_x86.exe | 014C | `2a458a55e873df225910a9c07618dcc142dd8fdd018c4800cf6774c255000966` |
| mydeskpro386/mydeskpro386_0_5_0593_x64.exe | 8664 | `516993aac3b90f56470507cd6d5b4a73fee0c188cb3b05a4b6ed328f92570eb7` |
| mydeskpro386/mydeskpro386_0_5_0593_x86.exe | 014C | `4ce91c852030f6e73b265eea79fb8d96697ada3232a7c7d086fd9aafc2877978` |
| mypc110/mypc110_x64.exe | 8664 | `9ba07aea842cf39ddcf21175c430aa8b6a3f651c0de77183d919fd0acc561f92` |
| mypc110/mypc110_x86.exe | 014C | `1feac785e08582d710005b00ad91e22cc55bbc5c8ff4229873c8b4a4ed6a64ce` |
| myob300/myob300_0_5_0593_x64.exe | 8664 | `5f8b1329eb93ac94d416aca1548a652b644d19f663fe3ea5b6775ca67a1c63b4` |
| myob300/myob300_0_5_0593_x86.exe | 014C | `b02b75973f22a023e2aa69a83651a9c625ee18b8361c42173a38fcd4810b0eb3` |
| mynes/mynes_0_0_0044_x64.exe | 8664 | `cc9b7f4444c133f174a175456e8fde112b36c90d5e2c4e2b25ca3f1c75d19a02` |
| mynes/mynes_0_0_0044_x86.exe | 014C | `7be5c480e1ad56a12bc25dd4066bdf01748ca9e5ee8dd735809c8c2d67ddc67f` |
| myraidenii/myraidenii_0_0_0007_x64.exe | 8664 | `d3318c75c38211b077783ac20e4bf31ea78d4d2f792843e946380f969d62ae92` |
| myraidenii/myraidenii_0_0_0007_x86.exe | 014C | `13fb34deabe0741357a6ab0d13fb27c46f3f42f369e4bba65f468b28ab6a9e7b` |

## Transcript identities

| Run | SHA-256 |
| --- | --- |
| x64 unit 717 287.67 | `e9517bd40eecd651f4f2867a3a1d1469a4cbcb4b446ce1c65cd5ce9349b5f87b` |
| x64 integration 48 234.86 | `77742ff61d64b140686fbbf39991f3a83ac4e26c5d5b8a8a95e29ee882cc4068` |
| x64 F1 36218 DOS 74125 VER 74514 DIR 5337 PAUSE 14 KEYS 15,32,33,32,32,31,32,32,33,32 | `fec62b77f91c84dcfacf6767600afdd37cc4812d70c1a7d3b0de8476b035bbe1` |
| x64 native startup pass | `88141733f33150d644dadac7dd696ade99d55438bc0ae04ac2ef4b4d4381e672` |
| x86 unit 717 113.51 | `b12417059ce99971cc76948151be014cb1812b05bf02a4add33dbbd518d0b010` |
| x86 integration 48 35 before the 300 s aggregate cutoff | `b4ca0ea495306d6b8febd6ba8c10a7da4fe247122b11157e06c875ed1436cdee` |
| x86 integration remaining 13 81.70 | `eac7f56516f12f39d9de28bb0524e31c5597b15ea743e1ba9fa9dffc84b2b11e` |
| x86 F1 45463 DOS 90510 VER 90893 DIR 6771 PAUSE 15 KEYS 30,47,31,16,47,16,32,47,47,47 | `ea64a9c7e71449b11b1b7dbd01f470a20cbb08beac969485e98289f0b1e5efd4` |
| x86 native startup pass | `88141733f33150d644dadac7dd696ade99d55438bc0ae04ac2ef4b4d4381e672` |
