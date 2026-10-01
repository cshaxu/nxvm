# T539 S80 Segment/Data Receiver Evidence

S80 moves three CPU-only test sources—SREG MOV, segment selector, and
MOFFS—to `test/x86/devices/cpu/`, with the Shared CPU fixture remaining the
sole fixture implementation. NXVM retains public Core memory, fault, and IRQ
board tests.

Focused Shared receivers pass on x64 and x86. Complete repository-only unit
suites pass 427/427 on both widths. CPU/PIC authority, Shared manifest/corpus,
documentation governance, and diff checks pass. This is test/CMake-only work;
S76's 0539 artifacts remain current.

- `7ae3cc12c`: Shared receiver corpus.
- `a5f7a9ae4`: NXVM duplicate-path retirement.
