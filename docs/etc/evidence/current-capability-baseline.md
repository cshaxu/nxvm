# Current Capability Baseline Detail

> Supporting T300-era capability inventory. [STATUS.md](../../states/CURRENT.md) is
> the sole current technical-baseline authority; this record provides detail
> for planning and must be refreshed or retired when its evidence changes.

| Area | Current bounded capability | Open boundary |
| --- | --- | --- |
| NXVM product | One session/composition path, retained Console/debugger, FDD/HDD boot regressions, GCC artifact and CTest gates | Preserve this path while every device evolves; do not quietly start VDM behavior. |
| CPU | T343 closes the accepted 8086, 80186, 80286, and 80386DX form/state program, including its named delivery, task, paging, and debug boundaries. | x87/80287/80387 numerical execution, VME/PVI, persistent cache/TLB behavior, timing, devices, and Windows/product compatibility are outside that CPU closure. |
| Interrupts and time | PIC source lifecycle; deterministic core elapsed ticks; PIT/IRQ0 -> ROM -> BDA -> `INT 1Ah` evidence; independently ratio-converted DMA/PIT/VADP/KBC/RTC/provider advances. | T346 S1 classifies this as deterministic L2, not L3: no common due-event queue, equal-time arbitration, or transaction/bus visibility contract yet exists. |
| Keyboard | KBC, IRQ1/IRQ12, ROM `INT 09h`/`INT 16h`, set-1 break/E0/E1, typeahead, selection/query, translation observation, LED, command-state typematic, ACK/RESEND, bounded PS/2 AUX packets, and one DOS guest-driver corpus | Set-2/3 conversion, wheel/advanced AUX protocol, broad guest mouse API compatibility, and native POSIX runtime validation remain deferred. |
| Display | CGA text plus bounded digital `320x200x4`, ROM-selectable `EGA-320x200x16-direct`, and `EGA-640x350x16-direct` mode 10h; copied text/indexed frames; `console`/`window`/`auto` selection | Remaining digital CGA modes/CRTC behavior, composite video, broader EGA/VGA, and VBE. |
| Storage | Bounded ATA PIO and FDD boot paths through declared ROM/device owners; ATA `nIEN` IRQ14 behavior; neutral media/controller mechanisms in core with VM backing/topology policy; core-owned 8237 DMA through frozen FDC binding | Full FDC state machine, controller-ready/DRQ/IRQ ordering, generic bus wait states, broad DMA behavior, extended IDE, and error/timing compatibility remain open under T346. |
| VDM | Isolated non-runnable scaffold over the shared core | Owned DOS design, CLI, host-drive policy, and product implementation remain deferred. |
