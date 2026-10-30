# T582 S26 — whole X86 Core LTO experiment

A scratch-only build compiled the existing X86 Core CPU and Machine source
files together with GCC LTO; all other linked product and shared libraries
remained the cached Release libraries. The normal source, CMake configuration,
media and published executables were not changed. This is wider than the
earlier three-CPU-file LTO experiment but is not whole-program LTO.

At the private-media 10,000,000-instruction startup checkpoint, ordered
baseline/candidate/candidate/baseline times were 22,295/20,893/20,506/22,293
ms on x64 and 25,602/21,606/24,880/26,539 ms on x86. Every run reported
79,964,294 ticks and PC `0003dbc5`. These samples suggest an opportunity but
do not prove complete CPU state equivalence, reproducibility across host
conditions, or behavior across CPU profiles and devices. GCC reported 12
serial LTO partitions, an additional build-cost concern.

A separate x64 private-media product-path Game probe linked the same Core
sources with LTO, and then ran a normal Release baseline with the same probe.
Both exited successfully. The probe's two-pixel condition fired 23 seconds
after Enter for LTO and 27 seconds for baseline; total run times were 258.70
and 255.93 seconds. The complete frames were visibly different: the baseline
still showed prior-window content across the top when its condition fired,
while the LTO frame showed the Game title. Thus the two-pixel condition does
not establish that both frames were fully rendered, and these times are not a
reliable end-to-end speed comparison. The source images were separately
inspected; the guest state and host scheduling were not matched. No native
Window/RDP input or complete interactive gameplay was qualified.

The LTO configuration is not adopted on this evidence. A meaningful follow-up
must first strengthen the task-only frame-completion criterion, then compare
equivalent guest work and verify the whole receiving and cache/build contract
before changing project configuration. Even the promising fixed-work samples
do not demonstrate the order-of-magnitude gain needed for PC110 usability.

SHA-256 (LTO log/frame, baseline log/frame):
`2A17E1B4595F494F16EACA8FF21C4A54D46CC4B80552BCA175B2CB8A6E5F60F4`,
`57597CA7238CA618B017A404F8156AEEEE0446330A1FBB1315635B661A5FD340`,
`356E01E9BB0BCCEC3C4FD1A08AB79A41F81740F8FBC1E74D2E92F0618E2D35C5`,
`CA44E03F12ECB49BBE68BDB5BD7489F2369C51A148060F10AC2D63B16CA0582F`.
