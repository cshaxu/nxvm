# M5 T70 S1: VM Machine Provider Authority

T70 removed the selected-object layer for VM-only CMOS, HDD, FDD, FDC, and
debug state. Each device is composition-owned and receives only direct or
non-owning links to the same live object graph. No guest storage is copied.

## Verification

- GCC/CMake/Ninja built `nxvm-0-5-0070` and the focused authority, full-PC,
  pause-boundary, unified-debugger, and profile smoke targets.
- FDD, FDC, and debug authority smokes passed.
- Full-authority, pause-boundary, and unified-debugger fixture smokes passed
  with `D:\\fdd.img`.
- The retained Console accepted `device fdd insert D:\\fdd.img`, then `start`,
  and remained alive for five seconds.
- Source scan found no declaration, macro, or call of `vcmos`, `vfdd`,
  `vfdc`, `vhdd`, `vdebug`, or their current/bind/unbind APIs.

## Artifact

- Developer artifact: `build/output/nxvm_0_5_0070.exe`.
- Identity: `Neko's x86 Virtual Machine [0.5.0070]`.
- SHA-256: `3955295905B1308B22860429EBFD615C355AEC4E98590CB06B16FF35D6B8B2DB`.

