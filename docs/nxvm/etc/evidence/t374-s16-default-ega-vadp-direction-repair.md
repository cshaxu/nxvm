# T374 S16: Default-EGA VADP Direction Repair

## Defect And Owner

T374 S15 replayed `vm-pcat-composition-s4-smoke` and found that the generic
default-PC/AT descriptor advertised CRTC index port `3D4h` as readable, while
the shared VADP correctly registers it write-only.  The resulting direction
loop failed even though Model-339 CGA and the configured-EGA CRTC regressions
passed.  This contradicts S12's retained default-EGA behavior claim.

VADP owns handler registration: `core_machine_vadp_register_cga_ports()`
registers `3D4h` only for writes; `core_machine_vadp_configure_ega_ports()`
adds only the configured-EGA Mode (`3D8h`) and Color (`3D9h`) read handlers.
The VM profile owns the advertised selectable port directions.  The repair
therefore changes only the default descriptor's `3D4h` leaf from readable to
write-only.  It adds no VADP handler and introduces no hardware read value.

## Complete Direction Sweep

| VADP leaf | Plain CGA / Model 339 | Configured default EGA | Descriptor disposition |
| --- | --- | --- | --- |
| `3D4h` CRTC index | Write-only | Write-only | Corrected default descriptor read flag to false. |
| `3D5h` CRTC data | Read/write | Read/write | Existing matching declaration retained. |
| `3D8h` Mode | Write-only | Read/write | Existing default-EGA declaration retained; Model-339 does not use the generic EGA descriptor. |
| `3D9h` Color | Write-only | Read/write | Existing default-EGA declaration retained; Model-339 does not use the generic EGA descriptor. |
| `3DAh` Status | Read-only | Read-only | Existing matching declaration retained. |
| EGA attribute, sequencer and graphics leaves | Not registered in Model 339 | Registered only after EGA configuration | Existing profile-specific declaration and Model-339 absence proof retained. |

The sweep covered all VADP descriptor rows in
`pc_at_profile.c`, CGA and EGA registration/configuration functions in
`vadp.c`, and every focused `core_machine_port_has_read/write` assertion in
the default-PC/AT, Model-339 and EGA/CGA VADP tests.  No other
descriptor/handler mismatch was found.

## Focused Proof

After rebuilding the affected targets, replay produced:

```text
M5:T353:S4:PCAT-COMPOSITION:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T314:S2:EGA-CRTC-BOUNDARY:OK
M5:T228:S1:CGA:PORT:OK
```

The first marker is the direct before/after regression proof.  The remaining
three retain the selected Model-339 write-only boundary and configured-EGA
Mode/Color plus EGA offset behavior.  No ROM, guest media, external source or
local asset was opened, copied, changed or committed.

## Transfer

This descriptor correction does not broaden EGA/VGA, complete digital CGA,
assign a display cadence or bus-contention value, or make any model-L3 claim.
The current-product display capability and all 5170 board/device timing remain
their queued receivers.
