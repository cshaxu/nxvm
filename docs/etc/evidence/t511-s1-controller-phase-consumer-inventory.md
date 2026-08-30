# T511 S1 Controller Phase-Consumer Inventory

`M5:T511:S1:PHASE-CONSUMER-INVENTORY:OK`

## Defect Class And Scope

The 0508 reproducer was a default-PC/AT firmware consumer that observed ATA
DRQ once, transferred a multi-sector request, and did not observe the HDC's
subsequent BSY/deadline/DRQ transition. The sole HDC repair is recorded in the
T510 evidence; S1 does not reopen it.

The static sweep covers every tracked current production controller/device
owner, Core scheduler/wiring, VM firmware and composition consumer, plus the
matching repository-only tests. It uses these reproducible queries:

```powershell
rg -n "next_due_tick|advance_elapsed|elapsed_ticks|next_service_tick|next_deadline" src/core/machine
rg -n "CORE_MACHINE_.*PHASE|phase ==|phase !=|phase =" src/core/machine/fdc.c src/core/machine/hdc.c src/core/machine/kbc.c src/core/machine/xt_keyboard.c src/core/machine/rtc.c src/core/machine/pit.c src/core/machine/vadp.c src/core/machine/dma.c
rg -n "core_machine_.*(install|connect|bind|register)" src/core/machine/machine_board.c
rg -n "\\b(in|out)\\b" src/vm/profile --glob '*firmware*.h' --glob '*firmware*.c'
```

## Owner/Consumer Matrix

| Owner | Repeated transition | Consumers inspected | S disposition |
| --- | --- | --- | --- |
| PIC | request/assert/acknowledge and cascade publication | CPU interrupt acceptance; PIT/RTC/FDC/HDC/KBC/XT sources | S2: one PIC owner and every source/ack consumer. |
| DMA | DREQ/HRQ grant, S1--S24 and terminal count | FDC channel 2, XT refresh channel 0, Xebec channel 3, RAM transaction | S3: DMA owns each service phase; providers must not consume a grant twice or presume a following grant. |
| PIT | count/output edge | PIC IRQ0, refresh DREQ0, speaker/board output | S4: no firmware bulk consumer; prove every wired edge is independently published. |
| RTC/CMOS | periodic/update/alarm deadline and IRQ8 | PIC, CMOS ports and firmware register reads | S5: acknowledgement/read-side effects and repeated IRQ publication. |
| KBC 8042 | command response, serial byte and typematic byte | port 60h/64h firmware/guest, PIC IRQ1, A20/reset receiver | S6: each queued byte/response must be independently observed. |
| XT keyboard/PPI | BAT/serial edge, PPI latch/IRQ1 and NMI request | PPI ports, PIC and firmware/guest reads | S7: prove first and subsequent serial edges are distinct. |
| FDC 8272A | command, DMA/NDMA byte, pending completion and result phases | DMA2, PIC IRQ6, MSR/data firmware path, media | S8: DMA and NDMA must consume every byte/result phase, not the first only. |
| HDC ATA/WD1003/Compaq/Xebec | command, PIO sector, DMA and response phases | firmware PIO, DMA3, IRQ14/IRQ5, media | S9: retain T510 two-sector PIO proof and audit every other personality consumer. |
| VADP CGA/EGA | raster/attribute display enable and copied frame publication | guest ports/VRAM, copied display snapshot, VM presentation | S10: a snapshot cannot turn one raster event into a continuing ready state. |
| D4 board route | request/hold/cancel transition | DMA/memory transaction | S11: bounded board route; it remains separate from generic DMA state. |

## Exclusions With Reasons

CPU, FPU, RAM, ROM, port dispatch, media bytes/persistence, firmware interrupt
services and VM presentation are not independent asynchronous-device owners in
the current source graph. They appear only as listed consumers. No controller,
firmware, profile, test or build path was otherwise excluded. Third-party
emulator code is not read in S1 because this stage establishes the current
source inventory; each later controller S reads its primary manual and then
uses permitted external implementations only as corroboration.

## Initial Result

The only already proven production instance is ATA PIO and is repaired in
`5182e2c6`. FDC is not presumed equivalent: its standard data path is DMA
provider consumption rather than firmware PIO, and its distinct NDMA path is
reserved for S8. Xebec likewise uses its DMA provider. Every remaining row is
an audit obligation, not an asserted defect or a reason to add generic polling.
