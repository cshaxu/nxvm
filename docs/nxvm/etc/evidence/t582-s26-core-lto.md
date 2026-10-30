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

## Stronger frame-completion comparison

The task-only probe was subsequently tightened: the Game marker pixels must
be present, the top-right title area must contain only its expected background
colors, and two consecutive sampled complete frames must have identical pixel
hashes. This rejects the stale baseline frame shown above. One LTO run followed
by one ordinary Release run used separate copies of the same input media.
Both exited successfully and produced the identical final PPM SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
LTO reached the stable frame 29 one-second samples after Enter and finished
in 228.84 host seconds; the baseline took 33 samples and 286.51 seconds.
Visual inspection of the preceding comparison showed the intended Game
screen, while the hash and strengthened criterion establish the same final
frame in this pair. The probes inject Machine input, not native Window input.

This single ordered pair is encouraging but not a controlled complete-state
comparison: guest states and host load may vary, one-second sampling is coarse,
and the first run precedes the second. It does not qualify x86 Game or prove
interactive playability. Before adopting LTO, measure its effect on the many
Core-linked test and product links, incremental build and ccache behavior;
the observed serial LTO partitions may materially harm build efficiency.
The full functional and receiver contract must also pass. The configuration
remains unmodified and the PC110 usability goal remains open.

SHA-256 (stronger LTO log, stronger baseline log):
`B7AB4848A4DA80D6092DE8EA8A62A4A27DED7D998230789F10BE8F33B620DB54`,
`B98FB6D9C6BDA36AC8442EBA1CD9D23BE68EFDE5DE9F35A55BCEB229B2BB854F`.

SHA-256 (LTO log/frame, baseline log/frame):
`2A17E1B4595F494F16EACA8FF21C4A54D46CC4B80552BCA175B2CB8A6E5F60F4`,
`57597CA7238CA618B017A404F8156AEEEE0446330A1FBB1315635B661A5FD340`,
`356E01E9BB0BCCEC3C4FD1A08AB79A41F81740F8FBC1E74D2E92F0618E2D35C5`,
`CA44E03F12ECB49BBE68BDB5BD7489F2369C51A148060F10AC2D63B16CA0582F`.
