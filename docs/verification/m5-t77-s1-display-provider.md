# M5 T77 S1 Verification

Date: 2026-08-02

QDCGA INT 10h display-mode notifications now reach the provider slot held by
their default-profile context. Composition binds that slot to the owning live
machine and generation counter. No global core display facade has a production
caller; the global frame mailbox remains explicitly scheduled for T80.

GCC 16.1.0 built `nxvm-0-5-0077` successfully. The following gates passed:

- `nxvm-vm-qdcga-boundary-smoke`: `M5:T40:S1:QDCGA-BOUNDARY:OK` after an
  INT 10h mode-set dispatch.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: `M5:T70:S2:DOS-PROMPT:OK`.
- `verify-facade-ownership`: `M5:T75:FACADE-OWNERSHIP:OK` with the display
  legacy facade limited to its implementation file.

Artifact: `build/output/nxvm_0_5_0077.exe`

SHA-256: `4AFECFBBC33CCD9D6EC5A299F56B13409E6CEBB8166A39FDCB709CE768A17275`
