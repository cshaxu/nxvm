# M5 T528 S7 Task-Closure Attempt

Baseline: `05408699`.  This is a task-level verification attempt, not a
runtime repair.  S6 remains the authority for the final Common-protocol versus
NXVM-bounded-runner owner disposition.

## Required Artifact Identity

`CMakeLists.txt` now declares only `vm-0-5-0528` as the current artifact
target.  Both optimized, stripped Release builds succeeded and copied their
same-architecture executables to all required destinations:

| Architecture | SHA-256 | Verified locations |
| --- | --- | --- |
| x86 | `72B371DB6DC9A2837F0392924530DDE8A0F7C414B478C31A6408F37C64623E4E` | `build/mingw-clang-x86-release/output/nxvm_0_5_0528_x86.exe`, `build/output/nxvm_0_5_0528_x86.exe`, `assets/sessions/nxvm_0_5_0528_x86.exe` |
| x64 | `A0DAFA99892AF426515102621AB32D4A509F3552CC0007D9BB6E2BF674AB007B` | `build/mingw-gcc-x64-release/output/nxvm_0_5_0528_x64.exe`, `build/output/nxvm_0_5_0528_x64.exe`, `assets/sessions/nxvm_0_5_0528_x64.exe` |

The configured MSYS2 i686 GCC frontend remains unusable:
`D:\programs\msys64\mingw32\lib\gcc\i686-w64-mingw32\16.2.0\cc1.exe`
exits with Windows status `0xC000007B`, including for a trivial preprocessing
probe.  The installed Clang 22 toolchain successfully targets
`i686-w64-windows-gnu` against the existing MSYS2 sysroot instead.  Its PE
verifier reports x86, its `SizeOfStackReserve` is the historical 2 MiB, and
`objdump` finds no `.debug`/`.zdebug` compiler-debug sections.
The checked-in `mingw-clang-x86-release` configure preset and the required
`current-gcc-x86` build-preset identity reconstruct that route using the
caller-provided `NXVM_I686_CLANG` and `NXVM_I686_SYSROOT` environment values.

The first Clang artifact exposed a real portability defect: the product Console
put multiple complete copied `ui_frame`/`common_session_plan` payloads on one
32-bit call stack.  The existing 2 MiB stack was insufficient.  Those
single-console transient values now belong to the one heap-allocated Console
context; no Common contract, mailbox, API or behavior changed.  Focused Console
and lifecycle integration tests pass, and a controlled x86 debugger launch
reaches its interactive input wait without the prior `_alloca` fault.

The x64 build was rebuilt after the prior output-file holder exited; its PE
architecture verifier and no-debug-section scan pass.

## Gates Run

- Full repository-only unit aggregate: passed (299 registered unit cases),
  including the Console stack-owner change.
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
T528 remains open. A revised, separately approved boot-repair packet is
required before attempting task closure again.
