# M5 T494 S4 Xebec Read Block-Count Repair

`M5:T494:S4:XEBEC-READ-COUNT:OK`

## Scope and retained owner

IBM 6139790's `08h` Read Data DCB has a positive block-count byte. Before this
repair, `core_machine_xebec_start_transfer()` read the first sector but did not
initialize the already-owned `sectors_remaining` field; Read therefore
completed after one 512-byte DMA3 transfer while Write used the count.

`src/core/machine/hdc.c` now initializes that existing field for Read and, at
each completed sector, uses the existing CHS advance and media-read helpers.
The terminal path remains one DMA release plus one response. A next-sector or
media-read failure follows the existing Write error shape: release DMA and
return source-shaped error/sense data. No interface, state owner, media cache,
ATA route, profile state or service-delay model was added.

## Focused proof

`tests/machine/core_machine_xebec_wiring_smoke.c` now supplies two adjacent
512-byte sectors with distinct contents, programs DMA3 for 1024 bytes, and
issues `08h` with block count two. It proves both byte ranges reached guest
RAM, then retains the existing single-sector terminal-count error, DMA release,
response, Write, reset and no-ATA-alias checks.

| Check | Result |
| --- | --- |
| Direct rebuild of changed HDC object, archive member, smoke object and smoke executable | Pass; WinLibs GCC with the configured Debug flags. |
| `build/mingw-gcc-x64/core-machine-xebec-wiring-smoke.exe` | Pass: `XEBEC-STACK`, `XEBEC-NO-ATA-ALIAS`, `XEBEC-DMA-MEDIA`, and `XEBEC-DMA-RAM` all report `OK`. |
| `ctest --test-dir build/mingw-gcc-x64 -R 'current\\.core-machine-xebec-wiring-smoke' --output-on-failure` | Pass: 1/1. |
| Similar-issue sweep: `rg -n "XEBEC|xebec|0x032[0-3]|320h|321h|322h|323h" src tests CMakeLists.txt cmake docs/proposals/m5-xt-xebec-fixed-disk-phase.md docs/etc/evidence/t494-s3-xebec-current-code-gap-list-2.md` | The sole Read/Write progression pair is in `hdc.c`; all other hits are the one Core topology, immutable profile ingress, focused tests, and documentation. No second production transfer path exists. |

The normal Ninja target command was attempted twice, once through CMake and
once directly with one job. In this existing build tree it started with no
output or CPU progress and was stopped after observation; the source generator
it first schedules completes directly. This is recorded as build-tree hygiene
follow-up, not a passing gate or a source defect. The direct rebuild above
compiles and runs the exact changed target. A clean/full gate remains required
before T494 closure.

## List-2 disposition

R1 is closed. X7--X10's undocumented hardware-status, jumper-readback and
mask-bit values remain the accepted explicit L1 boundary; physical-media timing
and ECC/MFM mechanics remain outside this logical controller route.
