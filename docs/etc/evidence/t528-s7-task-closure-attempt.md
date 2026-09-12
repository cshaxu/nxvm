# M5 T528 S7 Task-Closure Attempt

Baseline: `05408699`.  This is a task-level verification attempt, not a
runtime repair.  S6 remains the authority for the final Common-protocol versus
NXVM-bounded-runner owner disposition.

## Required Artifact Identity

`CMakeLists.txt` now declares only `vm-0-5-0528` as the current artifact
target.  Its x64 optimized, stripped Release build succeeded and copied the
same executable to all required destinations:

| Architecture | SHA-256 | Verified locations |
| --- | --- | --- |
| x64 | `6994CEA26F926B8D892351D4F84AF4D94716B7902494F9CE97820F8139F0C6F2` | `build/mingw-gcc-x64-release/output/nxvm_0_5_0528_x64.exe`, `build/output/nxvm_0_5_0528_x64.exe`, `assets/sessions/nxvm_0_5_0528_x64.exe` |

The required x86 artifact was **not** produced.  This is an environment
failure before NXVM source compilation: the configured MSYS2 i686 GCC frontend
`D:\programs\msys64\mingw32\lib\gcc\i686-w64-mingw32\16.2.0\cc1.exe`
exits with Windows status `0xC000007B`, including for a trivial preprocessing
probe.  `gcc --version` and `as --version` succeed, but no C source can pass
the frontend.  The task may not claim dual-architecture artifact closure until
that toolchain is repaired or replaced and the x86 artifact is rebuilt.

## Gates Run

- Full repository-only unit aggregate: passed (299 registered unit cases).
- `verify-current-specialized-gates`: passed.
- Documentation governance: passed.
- x64 current artifact optimized/stripped architecture verifier: passed.

## External Integration Result

The complete serial external integration suite completed with **39/42 passing**.
The following pre-existing boot-contract rows fail and block T528 closure:

| YAML case | Result | Terminal observation |
| --- | --- | --- |
| `compaq-deskpro-386-model-40-1200k.yaml` | failed at 180.07 s | FDC remains in execute phase for READ TRACK (`cmd=E6`, `CHRN=18/1/3`, `EOT=15`); no DOS terminal. |
| `ibm-5170-model-339-360k.yaml` | timeout | IBM 5170 boot did not reach the terminal within its registered budget. |
| `ibm-5170-model-339-1200k.yaml` | timeout | BIOS remains at `448 KB OK`; KBC remains disabled and no video ROM is mapped (`C0000=FFFF`). |

All other 39 integration rows passed, including every default-PC/AT CPU/media
matrix row, IBM 5160, DOS, video, ATA, Windows checkpoint, Common Debug, and
the non-boot Model-40 integration probes.

## Disposition

These failures concern model-specific firmware/board/FDC/KBC/video-ROM boot
paths.  They are not caused by, and cannot be repaired by, the T528 owner
cleanup without violating its explicit non-goals.  T528 S7 remains active and
T528 remains open.  A revised, separately approved boot-repair packet is
required before attempting task closure again.
