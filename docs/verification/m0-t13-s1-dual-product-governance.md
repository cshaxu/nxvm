# M0 T13 S1 Dual-Product Governance Verification

## Evidence Reviewed

- The owner approved the direction where `nxvm.exe` and `ntvdm64.exe` are both
  first-class outputs over one shared core.
- M1 already preserves the GCC-built NXVM baseline and whole-machine boot
  evidence.
- M2 already required runtime-owned composition, explicit profiles, and no DOS
  or platform dependency from the machine core.

## Result

Governance now treats this repository as the canonical NXVM-successor project:
`nxvm.exe` keeps bootable whole-machine VM behavior, while `ntvdm64.exe`
implements the non-bootable DOS app runner later. The canonical roadmap,
architecture, planning index, source/contributor guidance, and status documents
use the same module names and milestone sequence.

This documentation-only subtask produced no runnable executable and therefore
no `build/output` artifact.
