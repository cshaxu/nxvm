# M5 T539 S92 protected CPU receivers

S92 transfers the CPU-only protected data-access, protected far-transfer and
outer-return test receivers plus their CPU-local fixtures to the Shared
`test/x86/devices/cpu` corpus. Their one source of CPU state construction is
now the Shared fixture pair.

NXVM removes the duplicate sources and targets. Its named PIC/board receivers
remain product-owned: `machine_outer_iret_pic_board_smoke`,
`machine_protected_far_pic_board_smoke` and
`machine_protected_data_pic_board_smoke`. They consume the same Shared
fixtures, while retaining public interrupt routing and board construction.

Verification passed on x64 and x86:

- focused Shared and board receiver sets: 6/6 per width;
- T317 type vocabulary and T332 CPU fixture lifecycle;
- CPU/PIC authority, Shared test manifest/corpus and documentation governance;
- detached repository-only unit suites: **447/447**, exit code 0, per width;
- `git diff --check`.

The delivery commits are Shared `8fd54caba` and NXVM `b84c7bd44`. This changes
only tests, CMake ownership inventories and documentation; no production/API,
firmware, asset, INI or executable input changed, so no product binary rebuild
is required.
