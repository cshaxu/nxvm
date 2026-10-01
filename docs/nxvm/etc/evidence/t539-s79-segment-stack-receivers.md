# T539 S79 Segment-Stack Receiver Evidence

S79 moves five CPU-only test sources—FS/GS stack, legacy segment stack,
LES/LDS, S41 LES/LDS, and LSS/LFS/LGS—to `test/x86/devices/cpu/`. The Shared
CPU fixture remains the only fixture implementation. NXVM retains public Core
board tests for descriptor construction, real memory/fault state, and PIC/IRQ.

Focused successor tests pass on x64 and x86. Complete repository-only units
pass 427/427 on both widths. CPU/PIC authority, Shared manifest/corpus,
documentation governance, and diff checks pass. This is test/CMake-only work:
no runtime input changed, so S76's 0539 artifacts remain current.

- `2cbc600f9`: Shared receiver corpus.
- `594fd1d4c`: NXVM duplicate-path retirement.
