# T373 S3: Baseline Capability Classification And Ownership Ledger

## Classification Boundary

This is a capability and responsibility ledger, not an implementation or L3
decision. `Complete` would mean that the selected functional contract is fully
implemented and evidenced; no row below meets that standard merely because a
source file or smoke exists. `Partial` means a bounded product mechanism
exists but has an identified functional, profile-binding, reset/state-machine,
or evidence gap. `Empty` means no selected machine composition or mechanism
exists. `Unsupported` means NXVM does not expose that device family. A
`source-unknown/unselected` device is neither supported nor absent hardware: a
later primary-source profile audit must select it before it can acquire an
implementation or timing obligation.

The source configuration comes only from the T373 S2 BOM ledger. The current
implementation status comes only from T373 S1's repository inventory. Named
references are Queue receivers, not a claim that the receiver has begun.

## Baseline Machine Capability Ledger

| Baseline and required capability | Current classification and concrete gap | Earliest functional receiver | Later timing/audit receiver |
| --- | --- | --- | --- |
| **5170 Model 339 -- 80286, 8 MHz** | Partial. Shared 80286 contract and accepted successful-retirement ledger exist, but the selected machine still lacks source-backed wait/bus availability integration and no full selected-profile functional proof exists. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md), consuming the prior 80286 transfer ledger without reopening a settled row. | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then the historical [5170 final audit](../../history/M5-T377-5170-final-l3-audit-proposal.md), whose repair/re-audit successors remain open. |
| **5170 -- 512 KB planar memory, Rev.3 abstract ROM slot, reset** | Partial. Checked memory, immutable mapping and logical reset path exist, but the selected board/ROM-slot/reset-visible configuration has not been proved as a complete Model-339 contract. External IBM ROM remains optional owner asset, not a Git/runtime dependency. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md). | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then final audit. |
| **5170 -- PIC, PIT, DMA, RTC/CMOS, parity/NMI routes** | Partial. Logical owners and selected ordering exist; T365/T366--T372 explicitly retain unselected NMI source/latch, full device state, and physical-board conversion gaps. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md). | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then final audit. |
| **5170 -- 101-key AT keyboard and 8042, IRQ1** | Partial. A bounded AT keyboard/8042 path exists, but selected-device command, buffer, reset and complete IRQ-visible behavior are not proven. AUX/mouse is not part of this baseline. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md). | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then final audit. |
| **5170 -- CGA digital display** | Partial. Model-339 selects CGA and the VADP path exists, but the exposed CGA register/mode/state/reset behavior and contention/cadence are not complete evidence. EGA is expressly not selected. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md). | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then final audit. |
| **5170 -- FDC, IRQ6/DMA2 and aftermarket 1.44 MB field upgrade** | Partial. Bounded FDC, BIOS and DOS boot paths exist, but controller/drive state, error/reset and selected field-upgrade behavior are not complete; the drive remains aftermarket, not a factory 339 fact. | [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md). | [5170 board/device phase-timing closure](../../history/M5-T375-5170-board-phase-timing-closure-proposal.md), then final audit. |
| **5170 -- fixed disk / ATA / MFM** | Explicitly absent from the locked baseline. Current ATA/HDC is a generic product path and cannot stand in for IBM MFM/ST-506. | No 5170 functional receiver. Historical MFM needs the bounded [TODO](../../states/TODO.md#hardware-and-compatibility-debt) admission if separately selected. | None for Model 339; no 5170 L3 audit may require it. |
| **DeskPro Model 40 -- 80386 at 16 MHz, 1 MB, 128 KB ROM, keyboard, 1.2 MB FDD, 40 MB fixed disk** | Empty as a machine composition: the shared 80386 contract and generic PC/AT mechanisms exist, but no DeskPro descriptor, board binding, ROM mapping, keyboard/controller, FDC, fixed-disk controller/interface, reset or IRQ/DMA topology exists. The listed product facts do not select undocumented controller details. | [DeskPro Model 40 profile/capability audit](../../history/M5-T384-deskpro-model40-profile-capability-audit-proposal.md) first chooses source-backed board/device facts; then [DeskPro 80386 CPU closure](../../history/M5-T385-deskpro-386-model40-cpu-closure-proposal.md) and [DeskPro selected-device functional closure](../../history/M5-T386-deskpro-386-model40-device-functional-closure-proposal.md). | [DeskPro board/bus/device timing closure](../../history/M5-T387-deskpro-386-board-bus-device-timing-closure-proposal.md), then [DeskPro final audit](../../history/M5-T398-deskpro-386-model-l3-audit-proposal.md). |
| **DeskPro Model 40 -- display adapter** | Source-unknown/unselected. The locked model name and product facts do not select a display adapter; generic EGA and Model-339 CGA are not substitutes. | [DeskPro Model 40 profile/capability audit](../../history/M5-T384-deskpro-model40-profile-capability-audit-proposal.md) must select a primary-documented adapter or explicitly narrow the corpus. | Only after selection: DeskPro functional closure, then board/bus/device timing closure and final audit. |
| **DeskPro Model 40 -- PIC/PIT/DMA/RTC/NMI** | Empty as a DeskPro-specific board binding. Shared logical owners are not evidence of Compaq chip/revision/route/reset behavior. | [DeskPro Model 40 profile/capability audit](../../history/M5-T384-deskpro-model40-profile-capability-audit-proposal.md), then DeskPro selected-device functional closure. | [DeskPro board/bus/device timing closure](../../history/M5-T387-deskpro-386-board-bus-device-timing-closure-proposal.md), then final audit. |
| **PC/XT 5160-268 -- 8088, 256 KB, enhanced keyboard, one half-height FDD, no fixed disk** | Historical pre-T484 classification. | [8088 CPU-profile admission/timing closure](../../proposals/m5-8088-cpu-profile.md), then the [closed XT 5160-268 profile/capability audit](../../history/M5-T483-xt-5160-268-profile-capability-audit-proposal.md) before the [closed XT selected-device functional closure](../../history/M5-T484-xt-5160-268-device-functional-closure-proposal.md). | [XT board/device phase-timing closure](../../proposals/m5-8088-5150-xt-l3-baseline.md), then [XT final audit](../../proposals/m5-xt-5160-268-l3-audit.md). |
| **PC/XT 5160-268 -- display adapter** | Source-unknown/unselected. A system-unit description does not choose MDA, CGA, or another adapter. | The [closed XT 5160-268 profile/capability audit](../../history/M5-T483-xt-5160-268-profile-capability-audit-proposal.md) selects documented display/corpus facts. | Only after selection: XT functional closure, then XT timing closure and final audit. |
| **PC/XT 5160-268 -- PIC/PIT/DMA/RTC/NMI/reset** | Empty as an XT-specific board binding; shared PC/AT logical owners are not XT implementation evidence. | The [closed XT 5160-268 profile/capability audit](../../history/M5-T483-xt-5160-268-profile-capability-audit-proposal.md), then XT selected-device functional closure. | [XT board/device phase-timing closure](../../proposals/m5-8088-5150-xt-l3-baseline.md), then final audit. |

## Retained Current-Product Capability Ledger

These capabilities are outside the frozen 5170 baseline and cannot be used as
evidence for DeskPro or XT hardware until those profile audits make a separate
documented selection.

| Public/current product capability | Current classification and concrete gap | Earliest functional receiver | Later timing/audit receiver |
| --- | --- | --- | --- |
| Generic PC/AT EGA/VADP paths | Partial. Bounded EGA paths exist, but retained exposed register/mode/CRTC/reset breadth is not complete; no baseline inherits them. | [Current-product device-capability L3 closure](../../proposals/m5-current-product-device-l3-closure.md), or remove unsupported exposed paths at that task. | Same task's L3 evidence, then [pre-Windows audit](../../proposals/m5-pre-windows-l3-admission-audit.md). |
| Generic PC/AT ATA/HDC, IRQ14 and BIOS INT 13h | Partial. ATA PIO/HDD paths exist, but full error/recovery, reset and timing contract remain unproved; it is neither Model-339 MFM nor a DeskPro controller. | [Current-product device-capability L3 closure](../../proposals/m5-current-product-device-l3-closure.md), or remove it. | Same task, then pre-Windows audit. |
| AUX/mouse, IRQ12 and generic KBC extension | Partial. Bounded guest path exists; advanced controller protocol and its retained public contract are incomplete. | [Current-product device-capability L3 closure](../../proposals/m5-current-product-device-l3-closure.md), with advanced protocol explicitly bounded by [TODO](../../states/TODO.md#hardware-and-compatibility-debt) if still exposed. | Same task, then pre-Windows audit. |
| Serial, parallel, game port, speaker/PPI and joystick | Unsupported. No production controller owner or public capability was found; tests and historic hardware descriptions do not make them product support. | No M5 receiver unless a future corpus separately admits one; bounded entries remain in [TODO](../../states/TODO.md#hardware-and-compatibility-debt). | None; the current-product closure must not claim them. |
| Vendor ROMs and guest media | Unsupported as product dependencies. External owner assets may be used only through a future profile-local BYOB admission; no current descriptor consumes a vendor ROM. | Per-machine profile admission, if selected; no generic device receiver. | Profile-local validation only; never a substitute for timing proof. |

## Result And Transfer

No selected baseline capability is classified complete. Model 339 has a
partial shared/logical foundation and therefore proceeds to its already queued
functional closure, then timing and final audit. DeskPro Model 40 and PC/XT
5160-268 are machine-composition gaps, not merely timing gaps: their profile
audits must first resolve every source-unknown device selection. Current
EGA/ATA/AUX paths retain independent product-level closure responsibility.

This ledger satisfies T373's required machine-to-device/product-capability
classification. A final T373 closure audit must verify its S1--S3 evidence,
Queue/TODO transfers and absence of an implied L3 claim before the candidate
is removed and the 5170 functional candidate is admitted.
