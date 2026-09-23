# T374 S11: Model-339 CGA Functional Audit

## Decision

IBM's [PC Technical Reference](https://bitsavers.org/pdf/ibm/pc/pc/1502234_PC_Technical_Reference_Apr83.pdf)
defines CGA I/O at `3D4h`, `3D5h`, `3D8h`, `3D9h` and `3DAh`. It describes
the 6845 index at `3D4h` as write-only, Color Select at `3D9h` as six-bit
output-only, and status as input-only. The selected 5170 uses this retained
digital-CGA surface; composite and timing are excluded.

| Surface | Current disposition |
| --- | --- |
| `B8000h`--`BBFFFh` 16 KiB VRAM, text snapshot, 320x200x4 and 640x200x2 digital snapshot | Present VADP-owned routes with focused coverage. |
| CRTC `3D4h`/`3D5h` | Partial: only cursor, start and offset indices are accepted although the IBM 6845 has a full accessible register set. Full geometry/state work is later T374 work. |
| Mode `3D8h` and Color `3D9h` writes | Present and consumed by selected snapshot routes. |
| `3D4h`, `3D8h`, `3D9h` reads | Incorrectly registered by VADP and return stored state although the selected contract makes them write/output-only. **Select S12 repair.** |
| `3DAh` read | Present deterministic route; its exact durations transfer to 5170 timing. |
| Model-339 CGA-only composition and generated firmware consumer | Present; EGA ports are rejected and generated firmware initializes text/cursor state. Neither proves full CGA/INT 10h breadth. |

## Selected Repair

`core_machine_vadp_register_cga_ports()` must stop registering reads for CRTC
index, Mode and Color. S12 must add a focused port/topology regression and
sweep every VADP port registration plus reads and `has_read` checks for those
ports. It retains `3D5h` and `3DAh` only where the selected contract permits.
Full 6845 geometry, composite, EGA/VGA and physical cadence remain separate.

## Focused Baseline

Git-Bash CMake/Ninja passed unchanged:

```text
M5:T193:S2:VADP-TEXT:OK
M5:T266:S3:VADP-TEXT-STATUS:OK
M5:T228:S1:CGA:PORT:OK
M5:T254:S2:CGA-640:PORT:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
```

No ROM, guest image, external code or local asset was opened or changed. This
does not claim full CGA, Model-339 functional closure, timing closure or L3.
