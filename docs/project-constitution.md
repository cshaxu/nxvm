# Project Constitution

1. This repository is the canonical successor to NXVM.
2. `nxvm.exe` and `ntvdm64.exe` are first-class products over one shared core.
3. NXVM whole-machine boot ability remains supported, not legacy-only.
4. The independent DOS runtime is the default ntvdm64 compatibility path.
5. Non-invasive execution is the primary ntvdm64 product path.
6. The default release must not depend on Microsoft NTVDM binaries.
7. Real-program compatibility has priority over speculative private-ABI work.
8. Invasive Windows integration remains isolated until explicitly approved.
9. Microsoft NTVDM component support is a research option, not a committed
   backend.
10. Microsoft component implementation may begin only after host integration
   research and a formal Go decision.
11. Existing working NXVM code is reused unless replacement has clear ROI.
12. Module boundaries remain visible in the source tree.
13. Compatibility is validated through tests and real programs.
14. Research code must not become an implicit runtime dependency.
