# T373 S4: Baseline Capability Ledger Closure Audit

## Decision

**Ready to close T373 as an audit/allocation task; no machine is L3-ready.**
The task's purpose was to freeze a source-labelled three-machine capability
ledger and allocate all resulting work. S1--S3 satisfy that purpose. Their
result is expressly negative on implementation completeness: no selected
baseline capability is complete, DeskPro and XT have no composition, and the
generic product device paths retain later closure responsibility.

## Requirement-To-Evidence Matrix

| T373 required clause | Durable evidence and finding | Residual receiver |
| --- | --- | --- |
| Freeze all three selected configurations, including board/revision, CPU/clock, memory/ROM, video, input, storage, interrupt/DMA/timer/NMI/reset and probe boundaries. | [S2 BOM/source ledger](t373-s2-baseline-bom-source-ledger.md) records every documented identity/configuration fact and explicitly marks missing board/device facts unknown. [S3 classification](t373-s3-baseline-capability-classification.md) maps those unknowns and all selected shared board services to machine profile/functional/timing work. | 5170 functional then timing/final; DeskPro profile audit; 8088 profile then XT profile audit. |
| Retain the Model-339 Type-3/8 MHz/Rev.3/512 KB/CGA/no-fixed-disk configuration and aftermarket-only 1.44 MB field upgrade. | S2 states each fact and rejects factory-configuration inference; S3 marks FDC/field upgrade partial and MFM/ATA explicitly absent. | 5170 functional closure, then 5170 timing and final audit; separately selected MFM remains TODO only. |
| Do not infer unspecified DeskPro/XT facts from clone or generic PC/AT behavior. | S2 leaves display, controller/route and ROM-slot facts source-unknown; S3 classifies the DeskPro/XT compositions empty and names their profile audits before implementation. | DeskPro Model 40 and XT 5160-268 profile/capability audits. |
| Reconcile every selected component and currently exposed product capability with one of complete/partial/empty/test-only/unsupported (or source-unknown/unselected), and name state/IRQ-DMA/reset/timing/consumer gaps plus earliest owner. | [S1 inventory](t373-s1-current-support-inventory.md) covers actual production owners, routes, firmware and absent families. S3 reconciles each machine and retained EGA/ATA/AUX capability, specifies concrete state/reset/route gaps, and identifies exact Queue/TODO receivers. No row is classified complete or test-only by implication. | 5170 functional/timing/final, DeskPro profile/CPU/functional/timing/final, 8088/XT profile/functional/timing/final, and current-product device closure. |
| Separate shared semantic repair from profile-specific binding/timing. | S3 labels 5170 shared/logical mechanisms partial, while DeskPro/XT machine composition is empty. It expressly prevents generic EGA/ATA/AUX paths becoming baseline devices. | The earliest profile/functional receiver owns binding; later board/device candidate owns timing. |
| Keep out-of-baseline product support only with a later current-product disposition; otherwise do not pretend it is supported. | S3 assigns generic EGA, ATA/HDC and AUX to the current-product closure; serial/parallel/game/PPI/speaker/joystick are unsupported and remain bounded TODO admissions. | Current-product device-capability L3 closure; unimplemented families have no M5 completion claim. |
| Do not import ROM/media or use reference emulators as authority. | S1--S3 record the external-only BYOB boundary. No ROM, media, download mechanism, machine-local path, or emulator outcome entered T373 evidence or Git. | Later profile-local asset admission only if separately selected. |

## Receiver And Dependency Audit

1. The Queue's next candidate, **IBM PC/AT 5170 selected-device functional
   closure**, is the earliest implementation receiver for every selected
   Model-339 partial row. It is therefore eligible after T373 closes.
2. Its board/device phase-timing and final-L3 candidates remain ordered after
   functionality; neither may absorb missing functional state machines.
3. DeskPro Model 40 first needs its profile/capability audit. The CPU,
   selected-device, timing and final-audit candidates follow it in Queue order.
4. The distinct 8088 profile candidate precedes the XT profile/capability,
   functional, timing and final-audit sequence.
5. Generic EGA/ATA/AUX remain product-level debt until the current-product
   device-capability closure either completes or removes their exposed support.
   Historical MFM and unsupported peripheral families remain TODO and do not
   block Model-339's no-HDC configuration.

The Queue has one and only one receiver chain for each finding. No new Queue
candidate, reordering, TODO entry, source fact, timing value, or profile
selection is required to close this audit task.

## Closure Result

T373 may close after coordinator acceptance. Its proposal must be retained in
task history and removed from the active Queue. The replacement Current row
must state that this is a planning/ownership closure only: **5170, DeskPro and
XT L3 are all not ready**, and M5's next active implementation candidate is
the 5170 selected-device functional closure.
