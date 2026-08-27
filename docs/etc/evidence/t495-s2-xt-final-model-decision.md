# T495 S2 IBM 5160-268 Final Model Decision

`M5:T495:S2:XT-MODEL-READY:BOUNDED`

## Decision

The selected IBM PC/XT 5160-268 is **ready as an M5 functional model with
source-backed L3 relations and explicit L2 boundaries**. It is not a claim of
cycle-exact board reconstruction, electrical waveform simulation, or
host-wall-clock-equivalent execution. Those stronger claims remain unavailable
because the present Core axis is not a source-qualified 4.77 MHz physical axis.

| Ledger row | Decision and proof |
| --- | --- |
| M1 | Accepted. The XT resolver fixes 256 KiB, 8088, 8253, one PIC/DMA, XT PPI, CGA, 360 KiB FDC and Xebec; session requests cannot mutate the selection. `vm-xt-5160-268-profile-smoke` passes. |
| M2 | Accepted at its bounded CPU level. T486's single Core 8088 owner and four-byte queue remain selected; Group-3 range and WAIT service formula are explicit L2 CPU receivers, not XT estimates. |
| M3 | Accepted. The typed BYOB route validates synthetic owner-provided ROM manifests, installs only immutable maps and proves failure atomicity/reset-vector behavior. No firmware bytes, digest catalogue or AT firmware branch enters the XT route. |
| M4 | Accepted. The copied topology exposes primary-only board routes and omits RTC/CMOS, slave PIC and second DMA before and after Core reset. ISA/refresh/contention phase is explicitly L2. |
| M5 | Accepted. One 8259A logical order and single-controller topology are retained; elapsed-axis interrupt delivery has no falsely claimed physical placement. |
| M6 | Accepted. 8237A primary ports, refresh and the 8253 personality/IRQ0 route retain sole Core owners. DMA physical conversion and board phase remain L2. |
| M7 | Accepted. The XT 8255 owns keyboard and Port-B state; Core owns its parity/NMI consequence. Keyboard-device/clock inputs remain L1 rather than invented timing. |
| M8 | Accepted. The 8272A/360 KiB/DMA2/IRQ6 route has one Core FDC/media owner and excludes AT `3F7h`; zero XT conversion input truthfully selects the documented L2 no-delay fallback. |
| M9 | Accepted. CGA port and `B8000h` writes reach the sole VADP state and copied snapshot. Raster/board phase and external light-pen producer remain bounded. |
| M10 | Accepted. The Type-2 Xebec `320h`--`323h`/DMA3/IRQ5 stack, positive-count Read/Write and media persistence remain in the sole Core HDC/media owner. Adapter ROM execution, signal values and drive service are source-excluded; ATA is not an XT substitute. |
| M11 | Accepted. Core cold reset, controller reset and VM session failure rollback retain one owner chain; full current gate and `M5:T201:S3:HOST-CANCELLATION-BOUNDARY:OK` cover the cancellation boundary. No reset duration is invented. |
| M12 | Accepted as deterministic guest ordering only. Core retains guest progression/deadline ordering and VM does not inject host ticks. The absent physical XT timebase prohibits a wall-clock Standard claim. |

## Cross-profile and system proof

- Fresh Debug focused CTest selection: **13/13 passed**, covering XT profile,
  PPI, PIC, DMA, 8253, FDC topology, CGA, Xebec, default-at ATA, Model 339
  WD1003/ST-506 and Model-40 Compaq/WD isolation.
- Fresh serial current gate: **300/300 passed**, real time 254.46 seconds.
- Current specialized gates passed. They include the current-gate separation
  proof (`full=298`, `media=15`, `non-media=283`), public interface boundary,
  DMA/FDC authority, session/profile ownership and host-cancellation boundary.

No code, build, artifact, firmware, media or public-interface file changed in
this S: the tracked runtime/source delta is **+0/-0**. The inspected XT
construction flow remains singular:

`typed request -> immutable XT resolver/topology -> Core sole device state -> copied observation/presentation`.

The final model decision therefore closes T495's admitted audit scope. Its
explicit residual facts are already owned by their earliest receivers: CPU
range/WAIT selection, XT physical board axis and controller service/physical
media inputs. They are not a reason to create an XT profile-side scheduler,
controller or compatibility path.
