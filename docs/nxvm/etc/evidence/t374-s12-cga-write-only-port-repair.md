# T374 S12: CGA Write-Only Port Repair

## Delivered Repair

VADP now registers the selected CGA `3D4h` CRTC index, `3D8h` Mode and `3D9h`
Color ports as write-only. Their former read callbacks are absent from plain
CGA initialization. `3D5h` CRTC data and `3DAh` status retain their existing
read routes.

Default EGA composition still registers the existing Mode and Color read
callbacks during EGA-port configuration. This preserves the unrelated EGA
consumer behavior without exposing those reads on the selected CGA-only
Model-339 profile. The repair does not emulate an open-bus value: an absent
handler remains the port framework's existing unsupported route.

## Sweep

The source/test/profile sweep used `rg` over tracked `src`, `tests` and CMake
for the three port constants, raw `3D4h`/`3D8h`/`3D9h` uses, VADP registration,
`core_machine_port_read`, and `core_machine_port_has_read`.

| Hit class | Disposition |
| --- | --- |
| CGA registration and callbacks | Removed CRTC-index/Mode/Color read registration; retained their writes plus CRTC-data/status reads. |
| EGA registration | Re-registers Mode/Color reads only after EGA configuration; retained EGA boundary regression proves behavior. |
| Generated firmware | Writes only; no invalid read consumer exists. |
| Core CGA graphics/text/640 tests | Removed invalid read assertions; graphics smoke now proves exact absent/present direction set while preserving snapshot behavior. |
| Model-339 topology test | Proves all three reads are absent and the selected write/status routes remain present. |
| Profile leaf table | It records available physical ports/ranges for composition, not handler direction. No production composition route consumes its VADP read flags to register handlers; this repair leaves that unrelated ledger shape unchanged. |

## Focused Proof

Fresh Git-Bash CMake/Ninja rebuild plus executable replay passed:

```text
M5:T193:S2:VADP-TEXT:OK
M5:T266:S3:VADP-TEXT-STATUS:OK
M5:T228:S1:CGA:PORT:OK
M5:T254:S2:CGA-640:PORT:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T314:S2:EGA-CRTC-BOUNDARY:OK
```

The last target is retained EGA regression evidence, not Model-339 scope.
No ROM, guest image, external code or local asset was opened or changed.

## Transfer

Full CGA 6845 geometry/state programming, composite output and all display
cadence/contention remain later T374 functional/timing work. This correction
does not claim full CGA, selected-device closure, Model-339 timing closure or
L3.
