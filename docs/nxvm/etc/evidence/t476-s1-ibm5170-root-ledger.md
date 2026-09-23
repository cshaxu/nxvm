# T476 S1 IBM 5170 Root Ledger

`M5:T476:S1:IBM5170-ROOT-LEDGER:OK`

`M5:T476:S1:IBM5170-PARITY-UNIVERSE:OK`

## Frozen Root

This ledger freezes the existing selected IBM 5170 Model-339 behavior as the
future immutable root `pc-at-5170`. It neither claims a complete physical
machine nor changes any controller/CPU implementation. IBM/accepted evidence
continues to determine L3 facts; explicit L2 boundaries remain unchanged.

| Root group | Current owner/value | Provenance and disposition | Resolver/parity receiver |
| --- | --- | --- | --- |
| Identity and revision | Descriptor identity `ibm-5170-model-339`, revision 1. | Existing selected VM profile identity. | S2 root declaration; current session/catalog names remain parity observations. |
| CPU/FPU | 80286 / no FPU. | Selected profile fact; Core CPU feature owner is unchanged. | S2 copied Core input; CPU observation parity. |
| Core timing and pacing | Macro-proportional 8 MHz axis; deterministic retirement; not verified physical. | Nominal source is an L2 pacing contract, not wall-clock L3. | S2 copied Core input; time observation/Standard-Turbo parity. |
| PIT and RTC | PIT `596591/4000000`, RTC `64/15625`, RTC 32768 Hz, zero phase. | IBM 6280099 source-backed conversions, Manual L3. | S2 Core input; clock/reset parity. |
| DMA and PIC rules | DMA `3/8`, rational-clock/service-phase rules; PIC L2 fallback. | Accepted controller ledger: DMA selected source rule; PIC has no elapsed-latency source. | S2 Core input; controller disposition parity. |
| KBC timing | Initial 4,000,000 and repeat 800,000 ticks; command response zero. | Selected Model-339 nominal behavior; no invented serial/response latency. | S2 Core input; KBC/reset parity. |
| RAM/A20/parity | 512 KiB, absent extended-memory window, planar parity at port 61h. | Selected board topology. | S2 topology/memory declaration; memory/parity observation parity. |
| ROM/reset/firmware | Existing reset mapping and IBM Rev.3 abstract firmware slot. | VM firmware policy; no IBM ROM bytes. | S2 VM policy declaration; reset-vector/firmware-slot parity. |
| Display | CGA VRAM/CRTC/status and no EGA leaves. | Selected digital CGA topology; remaining physical raster work stays transferred. | S2 port/topology declaration; CGA topology snapshot parity. |
| Port and memory decode | Shared AT leaves/routes, with HDC leaves disabled by descriptor flags. | Existing descriptor/core-plan route owner. | S2 typed windows/routes; exact port presence/absence parity. |
| PIC/DMA/FDC routes | CMOS IRQ8 and FDC IRQ6/DMA2; FDC 8272A-compatible configuration. | Selected board topology; controller state stays Core-owned. | S2 routes/device declaration; FDC topology parity. |
| Storage and media | FDD field-upgrade type; no HDC, HDD media or HDD boot. | Selected Model-339 policy; ATA is not IBM MFM/ST-506. | S2 policy/session allowance; reject-HDD parity. |
| CMOS and firmware services | 512 KiB base memory, floppy type `40h`, fixed disk fields zero, selected service list omits HDC. | Existing profile-owned firmware/CMOS policy. | S2 VM policy declaration; firmware/CMOS parity. |
| Dynamic product resources | BIOS object, media registry, display provider, debugger, transports and copied display mailbox. | Session/core/product lifetime owners. | Explicitly excluded from resolver data; bound only after resolution. |

## Existing Route And Parity Set

The only current Model-339 construction route is:

```text
session profile kind -> Model-339 descriptor -> CPU contract -> session copies
Core config/rules -> existing Core plan -> existing Core machine
```

`vm_session_profile_select`, CPU-contract selection, session plan assembly and
firmware/device setup are the direct legacy route that T476 must replace for
this root only. Core has no Model-339 name branch. Model-40 is a distinct
private direct route and default-at remains a separate descriptor selection;
neither is in T476's implementation scope.

The finite parity suite is:

- `vm-ibm-5170-model-339-composition-smoke`: CPU, RAM, A20/parity, no HDC,
  rejected HDD input and default profile separation.
- `vm-model-339-clock-contract-smoke`: copied L3/L2 controller rules, clock
  ratios, KBC timing, reset, Core deadline and Standard/Turbo observation.
- `vm-ibm-5170-model-339-cga-topology-smoke`: selected CGA ports, absent EGA
  leaves and copied text snapshot.
- `vm-ibm-5170-model-339-firmware-fdc-topology-smoke`: firmware slot, FDC
  ports and IRQ6/DMA2.
- Existing session catalog/profile smoke: visible name and 80286/no-FPU
  description.

## S2 Boundary

S2 may make the resolver's typed root declaration represent precisely the
groups above and construct the existing copied Core-plan input from it. It must
not manufacture a full generic hardware graph or make the resolver a second
Core plan builder. The current direct Model-339 route remains until a later
cutover S proves the frozen suite equivalent and removes that route.

The DeskPro and default-at candidates retain their distinct field deltas and
are not absorbed into this root task.
