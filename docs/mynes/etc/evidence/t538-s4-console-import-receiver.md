# T538 S4: MyNES Shared Console Import Receiver

NXVM-hosted M5 T538 S4 admits MyNES receiving binaries/evidence only. MyNES T43
remains closed. No MyNES source, configuration, snapshot or media is changed.
Shared source is imported byte-for-byte from SoftPC
b79769c19b9b69d2c751fad942f4bc3a1490e7c2; no sibling runtime/build dependency.

Both 0043 products are rebuilt using Release -O3 -DNDEBUG and strip-all.
Objdump confirms pei-x86-64 / pei-i386 respectively, without debug sections.
x64 and x86 full suites each pass 132/132 (127 non-desktop plus five serial
desktop cases). Both products retain revision 0043.

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_0_0043_x64.exe | 7BF3830C99041784FF34F95CE1783D576F2FE92874AD235F4ED4D938EFFDA022 |
| mynes_0_0_0043_x86.exe | 1E93E4F29A8D2DA1D993B0E8C228E763741DD296E415E1F93B7B780B5116BA6A |

All six shared source/test roots are exactly equal to the pinned source;
test/register.cmake also matches. Six manifest checks and component boundaries
pass. This receiver uses the same broker path, not a MyNES presentation fork.
The import fixes frame extent and failure-tail retention; it makes no claim
about unrelated Console rollback debt or future MyNES work.
