# M5 T539 S93 task-state CPU receivers

S93 transfers the CPU-only 16-bit task-switch, 32-bit task-switch decode and
32-bit task-state receivers plus their single CPU fixture to
`test/x86/devices/cpu`. The fixture remains the sole CPU-state construction
path for these receivers.

NXVM removes the duplicate App sources and targets. Its retained
`machine_task_switch16_pic_board_smoke` includes the Shared fixture but retains
the public PIC route. Cross-width task switching, paging, TSS I/O authorization
and other board construction remain named NXVM receivers.

Verification passed on x64 and x86:

- focused Shared plus PIC-board receiver set: 4/4 per width;
- T317 type vocabulary and T332 fixture lifecycle;
- CPU/PIC authority, Shared test manifest/corpus and documentation governance;
- detached repository-only unit suites: **450/450**, exit code 0, per width;
- `git diff --check`.

The delivery commits are Shared `ae3f5a46e` and NXVM `418ad2f63`. This changes
only tests, CMake ownership inventories and documentation; no production/API,
firmware, asset, INI or executable input changed, so no product binary rebuild
is required.
