# T384 S1: DeskPro Model 40 Profile And Capability-Gap Audit

`M5:T384:S1:DESKPRO-MODEL40-PROFILE-AUDIT:OK`

## Scope And Source Standard

This is an audit, not a runnable-profile decision and not an L3 decision.  It
freezes the system-board contract of the owner-selected original Compaq
DeskPro 386 Model 40 / DeskPro 386/16.  It does not silently turn a supported
option into a configured device.

The primary technical sources are Compaq's *DeskPro 386 Technical Reference
Guide*, Volumes I and II, September 1986, First Edition, inspected transiently
from [Volume I](https://archive.org/details/compaq-desk-pro-386-technical-reference-guide-vol-1-1986-09)
and [Volume II](https://archive.org/details/compaq-desk-pro-386-technical-reference-guide-vol-2-1986-09),
and Compaq's *D3PE (80386) Processor Board Circuit Descriptions*, dated 5
January 1987 and carrying Compaq 1986/1987 copyright, with its D4 schematics
and material list.  The latter original scan was inspected transiently from
the 1986 technical-information archive hosted by
[Minus Zero Degrees](https://minuszerodegrees.net/manuals/Compaq/Compaq%20DeskPro%20386-16%20-%20Technical%20Spec%201986.zip).
Neither archive nor any vendor byte is in this repository.  The primary corpus
identifies the Model-40 standard configuration, controller cards, 16 MHz 80386
design, one-megabyte standard memory, two 8237A-5 controllers, two 8254s, two
8259As, MC146818, 8042/8742 interface, ROM topology, I/O map, NMI/reset paths,
and D4 bus contract.

The later Compaq *DeskPro 386 Maintenance and Service Guide* is retained only
as a manufacturer-authored option/revision cross-check.  It explicitly lists
both keyboard types, several displays, several diskette drives and several
fixed disks as options; it therefore cannot select one of them for this
baseline.  PCjs is only a locating/cross-check source: its own DeskPro page
states that its present configurations use IBM display adapters and that Compaq
adapters remain to be added.  No PCjs, 86Box, MAME, clone, generic PC/AT, or
later DeskPro behavior is used to select hardware below.

## Frozen System-Board BOM

| Area | Primary-source disposition | Model-40 profile consequence |
| --- | --- | --- |
| Identity and CPU | D3PE specifies an 80386-16 at a full-time 16 MHz; its CPU clock is phase-related to a 32 MHz oscillator. | Select 80386DX 16 MHz.  Existing generic 80386 availability is not a DeskPro descriptor. |
| Main memory | The D4 description says one megabyte of standard 32-bit RAM, with the standard board in slot 1 and optional boards separately described.  It documents initial/page-hit/page-miss cycles and ROM replacement. | Select exactly 1 MiB standard memory board; do not inherit the generic PC/AT memory map. |
| System ROM | The board has four-ROM capability; standard decoding uses the high pair and the primary text documents 128 KiB at `FE0000h-FFFFFFh`, plus a `E0000h-EFFFFh` RAM replacement window. | Freeze geometry/mapping behavior, not a ROM image, revision, hash, or local path.  The BIOS image remains owner-managed external evidence. |
| PIC | Material list and I/O text specify two 8259A controllers, master/slave cascade on IRQ2, and the concrete IRQ0..IRQ15 map. | Select the dual-PIC topology and DeskPro IRQ map; no generic route table may stand in for it. |
| DMA | Material list and description specify two 8237A-5s, byte channels 0..3 cascaded through word controller channel 4, word channels 5..7, 4 MHz DCLK, a page register, and one DCLK wait on all DMA cycles. | Select this dual-DMA topology and arbitration; it is not merely PC/AT DMA compatibility. |
| PIT and speaker | The board specifies two 8254s: system/refresh/speaker at `40h-43h`, and failsafe/extra/speed-control at `48h-4Bh`; the compatible timing source is 1.19318 MHz. | Select both timer blocks, including the non-AT second block. |
| RTC/CMOS | Material list and circuit description specify MC146818 with battery-backed state and `70h/71h` access sharing the NMI mask. | Select MC146818 topology and shared NMI-index behavior. |
| Keyboard and A20 | The board specifies an 8042 or 8742 UPI, D4-SKEY PAL, `60h/64h`, keyboard IRQ1, reset and slowdown outputs, and PAL-intercepted A20 control.  Volume II lists both the 84-key and 101-key Enhanced Keyboard as available. | Select keyboard-controller behavior; no AUX device, AUX port, or IRQ12 is selected.  The physical keyboard model remains an explicit user configuration choice. |
| NMI and reset | The board documents masked NMI through `70h` bit 7, expansion-bus IOCHK latch/status at `61h` bits 3/6, a failsafe-timer NMI at `61h` bits 2/7, and reset arbitration across power, shutdown and 8042 program reset. | Select all three logical sources and their visible control/state paths. |
| Bus | D3PE describes seven compatible expansion slots, four with a second DeskPro-286-compatible connector, 15 IRQ lines, seven DMA request lines, and a bus re-synchronized to simulate 286 timing with an average below 8 MHz. | Select the D4 transaction/bus contract.  Electrical waveform, pin loading and physical READY remain out of scope. |
| FDC and fixed disk | Volume I names the Model 40 standard one 1.2 MB diskette drive and one 40 MB fixed disk.  It selects the Compaq Multipurpose Fixed Disk Controller Board: 765 FDC at `3F2h/3F4h/3F5h/3F7h`, DMA DRQ2/DACK2, plus the Compaq 40 MB fixed-disk subsystem through that board. | Select the Compaq controller and 40 MB subsystem.  Do not substitute the generic FDC, ATA/HDC, IBM MFM, a disk image geometry, or an optional second/tape device. |
| Display and monitor | Volume I makes either the Compaq Video Display Controller plus Dual-Mode Monitor or the Compaq Enhanced Color Graphics Board plus Color Monitor available; neither is stated as the Model-40 standard configuration. | Explicitly unselected and profile-blocking.  Do not substitute CGA, EGA, VGA, or an IBM adapter. |
| Numeric coprocessor | The board supports switch-selected 80287 or 80387 options, and the no-coprocessor setting is documented. | Select no x87 unless a later owner-approved configuration supplies one; no x87 work belongs in this baseline. |

## Current-Source Capability Reconciliation

| Selected requirement | Current disposition | Earliest repair owner | Timing receiver |
| --- | --- | --- | --- |
| 80386DX-16 architectural and mode behavior | Partial: `CORE_MACHINE_CPU_PROFILE_80386` exists, but no DeskPro CPU/clock/reset contract proves the source-defined 16 MHz behavior. | DeskPro 386 Model 40 80386 CPU closure. | DeskPro 386 board bus and device timing closure. |
| 1 MiB D4 memory, ROM overlay/replacement and high-address semantics | Missing as a DeskPro composition; the generic PC/AT descriptor cannot establish the D4 mapping or controls. | DeskPro selected-device functional closure. | DeskPro board bus and device timing closure. |
| Dual PIC, dual DMA, two PITs, MC146818, 8042/A20 PAL, NMI and reset | Shared devices exist in current product paths, but no current DeskPro descriptor binds their source-defined topology, extra timer, special NMI paths, A20 acceleration or reset arbitration. | DeskPro selected-device functional closure. | DeskPro board bus and device timing closure. |
| 8 MHz-compatible re-synchronized expansion bus, DMA arbitration and memory-cycle classes | Missing as a DeskPro machine contract.  The generic PC/AT profile does not prove D4 transaction phases or wait rules. | DeskPro board bus and device timing closure. | DeskPro 386 model-L3 closure audit. |
| Model-40 1.2 MB FDC and 40 MB storage subsystem | Missing: current generic FDC does not bind the Compaq Multipurpose Fixed Disk Controller, its custom LSI, 765/DMA2 contract, or Model-40 fixed-disk interface.  ATA/HDC is explicitly ineligible. | DeskPro selected-device functional closure. | DeskPro board bus and device timing closure. |
| Video/monitor | No source-selected adapter exists.  Current generic PC/AT EGA/CGA paths are ineligible substitutes. | Profile-evidence continuation below, then DeskPro selected-device functional closure. | DeskPro board bus and device timing closure. |
| Firmware corpus | No source-selected ROM revision/image identity is recorded.  Current product generic and IBM 5170 slots are ineligible substitutes. | Owner-managed external evidence, reconciled by the DeskPro CPU closure. | DeskPro 386 model-L3 closure audit. |

## Blocking Selections And Handoff

The requested machine name selects a system board and storage bundle, but not a
unique display, physical keyboard or firmware revision.  These three fields
are deliberately `source-unknown`, not "missing implementation":

| Field | Required evidence before a runnable DeskPro profile | Receiver |
| --- | --- | --- |
| Display/monitor | Owner-approved selection of one documented Compaq pair: Video Display Controller plus Dual-Mode Monitor, or Enhanced Color Graphics plus Color Monitor.  This is a configuration choice, not a fact that the Model-40 name determines. | T384 follow-up evidence S, before the selected-device closure may begin. |
| Physical keyboard | Owner-approved choice of the documented 84-key or 101-key Enhanced Keyboard. | T384 follow-up evidence S, before input acceptance may be claimed. |
| ROM identity | Owner-managed legal BIOS image provenance plus a source-supported revision/part identity; keep bytes and local paths outside the repository. | T384 follow-up evidence S, before the CPU closure can declare firmware execution. |

All other rows have a primary-source disposition and transfer cleanly to the
next queued owners.  This audit therefore permits the next CPU task to close
only its source-defined 80386 architectural contract.  It does **not**
authorize a runnable profile, selected-device function, board timing, or L3
acceptance until the three explicit configuration selections are made.  It
creates no generic-device waiver.
