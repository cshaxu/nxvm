# T582 S26 — cross-file CPU compilation experiment

A scratch-only PC110 fixed-work probe compared three existing CPU translation units (`cpu_instructions.c`, `cpu_timing.c`, and `cpu_timing_model.c`) compiled together at `-O3`, with and without GCC link-time optimization. Other libraries remained the existing release binaries. Both variants used private copies of the same media and CMOS; no production source or build configuration changed. This is a partial link experiment, not a whole-project LTO qualification.

All eight runs retired 10,000,000 instructions, reached 79,964,294 guest ticks and PC `0003dbc5`. Elapsed milliseconds were:

| Width | Baseline, then LTO | LTO, then baseline |
| --- | --- | --- |
| x64 | 21,771 / 16,952 | 19,915 / 20,708 |
| x86 | 22,951 / 24,797 | 22,386 / 25,433 |

The first x64 pair suggests a large improvement, but its reversed pair is much smaller; x86 changes direction. Sequential runs are subject to host scheduling and frequency variation, and the checkpoint does not prove equality of the entire guest state or all other CPU profiles. The evidence does not justify changing the shared build or claiming real-time PC110 usability. The partial-LTO candidate has not been through functional or full product qualification. GCC also reported serial compilation of 11 LTO partitions in this scratch build, a build-time cost to consider in any future design.

Scratch log SHA-256, forward: `6E6E982220BB5784139073D6F832683DDB88FC4265FAAA893284B6EBA06F36C8`; reverse: `46AAFDB17A3C1E1024746CB0AC257967A04480B6824536A40B4B89F23B1168F5`.
