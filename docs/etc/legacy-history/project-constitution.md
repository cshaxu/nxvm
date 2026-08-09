# Project Constitution

> Superseded by [design/GOAL.md](../../design/GOAL.md),
> [design/ARCHITECTURE.md](../../design/ARCHITECTURE.md), and the current local
> rules. Retained as historical rationale only.

1. This repository is the canonical successor to NXVM.
2. `nxvm.exe` and `nxvdm.exe` are first-class executable products.
3. `core` is their shared foundation; `mantle` is the future shared VDM
   composition component; `dos` is the independent owned DOS backend.
4. NXVM whole-machine boot ability remains supported, not legacy-only.
5. The independent DOS runtime is the default NXVDM compatibility path.
6. Non-invasive execution is the primary NXVDM product path.
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
