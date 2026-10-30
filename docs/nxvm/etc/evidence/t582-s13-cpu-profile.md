# T582 S13 — CPU profile-guided optimization experiment

Five existing X86 Core CPU translation units were compiled with GCC 16.1.0 Release flags. A task-owned executable trained them with the existing PC110 fixed-work probe and private CMOS/media, then recompiled the same objects with profile use and no missing profiles. This is a temporary toolchain experiment: profile data and objects remain under the task scratch directory, not product source, build presets, public APIs or releases. The first attempt used a profile directory containing a Windows drive path and produced no data; the corrected attempt used object-local profile files and produced five. Training and every comparison reached 10,000,000 instructions, 79,964,294 guest ticks and PC 0003dbc5.

| Width | Order | Baseline ms | Profile-use ms |
| --- | --- | ---: | ---: |
| x64 | baseline, profile | 20,780 | 17,790 |
| x64 | profile, baseline | 22,248 | 18,644 |
| x86 | baseline, profile | 26,205 | 22,891 |
| x86 | profile, baseline | 28,051 | 23,563 |

These fixed-work observations suggest roughly 13–16% lower time in this early workload, not a full behavior-equivalence proof. The profile was trained on only PC110 startup; it has not been qualified for other Intel profiles, NEC V30, compiler versions or unit/integration suites. Training and repeated compilation have nontrivial build and reproducibility costs. No `.gcda`, guest media or host paths are committed.

A sequential x64 product-path comparison used the same CPU objects, private CMOS/media and 25 ten-second samples. Baseline and profile variants exited zero in 252.61 and 252.47 host seconds. Their last quantum reported 672,816,871 and 745,539,570 guest ticks; RTC reached second 20 and 22. Baseline first recorded graphics at sample 21 and profile at sample 18. Last frame hashes differed, so this is not proof of identical guest state, interactive performance, or full timing correctness. Both paths remain far slower than guest real time.

GCC's PGO output split cold code substantially: in this particular x64 executable, the hot `x86_core_cpu_timing_select` size was 0x5d0 bytes with a separate cold part versus 0x2cc0 bytes without PGO. This is an observation, not proof that this function alone causes the gain. An untrained `-ftracer` candidate measured baseline/candidate 22,361/22,276 ms x64 and 26,387/28,290 ms x86. A separate prototype prevented inlining the existing V30/80186/80286 selectors using a compiler-specific temporary annotation; it measured 21,266/22,380 ms x64 and 27,615/27,843 ms x86. Single ordered pairs do not establish robust regressions, but neither shows a useful gain, so neither is admitted or proposed as production code.

No production code, compiler configuration or artifact changed. Further work must test representative late workloads, both architectures and other CPU profiles, and establish a reproducible profile or simpler portable equivalent without importing protected media or creating another execution path. User-facing desktop/RDP usability remains unverified.

Evidence SHA-256: 9B4C9FADB2AF8CA9F6AD73C0061EA1978F963A269CEA8F9A4F33A59086CD6B70, 3A2A1345842F3B7ED9F91B87557690254F5141191F1FD94A8EEECEDA9E4C30E8, 545DEA1012DD04AC8BD0E26DAA0EB3C9C200C52A2CB582C9BAE63E0CDDC5C970, 3EDDE68E61DA9D06D3EEF8A3E2BCB4009B9568EF56498FFADC7981C64B47DE0C, 0296A59CA5A71354CE0B33DB2913DF9B1E9D6BAEAFA08BAC2E5E6ED08A93B32D, 2143D51001F167074C9670C79F7ED0E05BF774F9D93092554CE1D9649F105CD1, 0B012EC04EDCA548D5948F8D7236F849091B71533DAE3F10419AD512ADF561B3, A3D366FC3C0DCB580B71F3AABFC53F37BA3D2D233466145A0C67DC81A88299C8.
