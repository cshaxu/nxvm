# T387 S6: Task-Closure Transfer Audit

`M5:T387:S6:TASK-CLOSURE-TRANSFER-AUDIT:OK`

## Independent Scope Matrix

| T387 requirement | Current source/evidence result | Closure disposition |
| --- | --- | --- |
| Model-40 board topology, memory/ROM, ports and event ownership | T384 S1/S2 select the board; T386 S15--S29 bind the functional Model-40 composition; T387 S1 inventories every board timing consumer. | Functional/topology evidence is retained. Physical availability and waits transfer to the next DeskPro physical-device receiver. |
| CPU-to-PIT and CPU-to-DCLK calibration | T387 S2 and S5, corroborated by T366 S9 and T368 S7, prove that the shared retirement axis includes successful one-tick sentinels. | Transfer to four-profile CPU physical-timebase closure; no ratio is admitted. |
| Port `61h`, refresh, NMI and second-PIT facts | T386 S25 supplies the shared port-B counter-1/counter-2 mechanics and D4 NMI ownership; T387 S5 reconciles D3PE's composite bit map. | Functional bit ownership is retained. Physical refresh phase, failsafe and bus-event timing transfer to the next DeskPro receiver. |
| DMA, PIC, PIT, RTC, 8042 and reset ordering | T387 S1 maps the shared deterministic callback order and each selected topology owner. | Deterministic order is retained, but DCLK/waits/settle periods are not physical contracts; transfer after timebase closure. |
| FDC/DMA2/IRQ6 and Compaq HDC/IRQ14 service | T386 S24/S26 establish the selected logical RAW-IMG routes; T387 S1 records their unconverted board/device timing boundary. | Transfer to the next DeskPro receiver; HDC physical-media semantics retain their existing TODO. |
| Compaq EGA/firmware/monitor behavior | T386 S6--S13/S27/S28 establish the selected digital personality and shared VADP state; its named CECG TODO retains physical/firmware/raster/ISA breadth. | Transfer to the next DeskPro receiver and existing CECG/physical-media TODO boundaries. |
| Independent secondary bridge | T387 S3 defines the exact observation contract; S4 rejects available local 86Box/PCjs material and records no local MAME source. | No generic-AT or clone scalar; a later primary-constrained observation may be admitted by the receiving candidate. |

## Closure Recommendation

T387 completed the required board/device source inventory, current-owner sweep,
primary port-`61h` reconciliation and non-substituting reference qualification.
It does not complete physical board timing: doing so requires the shared
physical-timebase prerequisite, then the device/firmware availability work
listed above. The Queue orders those two receivers before the Model-40 L3
audit. This audit recommends closing T387 through explicit, owner-visible transfers; it does not
accept a Model-40 clock, device phase, physical-media behavior or L3 result.

S5 P1 ran the 281-target current gate, specialized gates and documentation
governance on the unchanged runnable graph. This closure audit re-ran
`git diff --check` and documentation governance and inspected the actual S5
P1/P2 changes, the current Queue and all cited evidence. No new source,
firmware, media, artifact or runtime behavior enters T387.