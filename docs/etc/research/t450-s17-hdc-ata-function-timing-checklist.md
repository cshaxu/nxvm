# T450 S17 ATA/HDC Original Function And Timing Checklist

## Scope, Source Form And Sufficiency

`ATA` denotes X3T13 *AT Attachment-3 Interface*, 2008D Rev. 7b (Jan. 1997).
The admitted PDF is a born-digital text document with searchable tables and
figures; S19 must directly verify this classification and every cited page
against the rendered source.  ATA-3 defines a host/device interface, not a
selected PC controller, cable/bridge, mechanical disk, or repository backing
media grammar.

The standard is sufficient for the ATA task-file, protocol, command, reset,
interrupt and electrical-timing rows below.  It is insufficient for the
currently selected HDC personality because none is admitted: current Compaq
WD/ST-506-oriented behavior and generic PC/AT port compatibility cannot be
silently relabelled ATA.  It also does not choose physical geometry, a disk
image format, persistence policy or board service phase.  Those are blocked
inputs.  No mature-emulator reference is selected; S19 may name one only as a
labelled reference-derived follow-up for an exact gap.

## Task-File And Command Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| ATA-R1 | ATA-3 clauses 5.1--5.2, pp. 19--27 | Command Block contains Data, Error/Features, Sector Count/Number, Cylinder Low/High, Device/Head and Status/Command; Control Block contains Alternate Status/Device Control. | Command-block values are indeterminate while BSY/DRQ conditions the standard identifies; command write begins processing. | Command block selects device with CS0-/CS1- and address lines. | Primary sufficient for ATA task-file register semantics: L3. |
| ATA-R2 | ATA-3 clause 5.2, pp. 21--27 | Status reports BSY, DRDY, DF, DSC, DRQ, CORR, IDX and ERR; Error/Features and Device/Head fields carry command result/addressing state. | Host Status read clears INTRQ; Alternate Status does not. | During DMA completion BSY/DRQ clear within 400 ns of INTRQ; data PIO completion has its documented ordering. | Primary sufficient: L3. |
| ATA-R3 | ATA-3 clauses 5.2.6, 8.3--8.5, pp. 24--25,109--116 | Data register transfers 16-bit PIO words in command-defined data-in/data-out phases; DRQ governs data-block service. | A command error terminates the transfer through Status/Error rather than an independent data path. | PIO protocol diagrams specify host/device handshake and required wait states. | Primary sufficient for protocol; selected host I/O timing conversion is L2. |
| ATA-R4 | ATA-3 clause 7, pp. 40--105 | Defined command set includes diagnostics, identify, set features/multiple mode, read/write/verify/seek/recalibrate, buffer, DMA, power, security and other listed commands, each with prerequisites and result rules. | Unsupported/non-executable commands return documented abort/error state; selected device capabilities constrain optional commands. | Command tables and clauses define BSY/DRDY/DRQ/INTRQ phase relations. | Primary sufficient for command forms; a selected device's supported subset is L2. |
| ATA-R5 | ATA-3 clauses 7.7--7.8, pp. 48--64 | IDENTIFY DEVICE / IDENTIFY DEVICE DMA return 256 words of configuration, geometry/capability, transfer-mode and command-set information. | IDENTIFY follows normal command/result/error lifecycle. | Identify is a PIO/DMA data command with its specified DRQ/interrupt protocol. | Primary sufficient; exact identify words require a selected device personality: blocked. |

## Data, Reset, Signal And Timing Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| ATA-F1 | ATA-3 clauses 4.1--4.2, pp. 11--18 | Interface signals include DIOR-/DIOW-, CS0-/CS1-, DA0--DA2, DD0--DD15, DMARQ/DMACK-, INTRQ, RESET-, DASP-, PDIAG- and IORDY. | Unselected/non-DMA devices tri-state specified signals; RESET- controls device reset. | Tables define host/device direction, pull requirements and electrical limits. | Primary sufficient for interface signals; selected bridge/cable implementation is L2. |
| ATA-F2 | ATA-3 clauses 4.2.8--4.2.13, pp. 15--17; clause 8.6 pp.117--121 | DMA command transfers use device DMARQ and host DMACK- handshake; INTRQ signals command completion subject to nIEN and status-read clearing. | During DMA CS0-/CS1- shall not be asserted; reset/nIEN affect interrupt visibility. | Standard supplies DMA handshake and timing diagrams, not ISA DMA channel assignment. | ATA logical relation L3; selected board DMA/arbitration is blocked. |
| ATA-F3 | ATA-3 clauses 8.1--8.2, pp.106--108 | Power-on/hardware reset and SRST software reset initialize attached devices, diagnostic state and Status/Error conditions. | SRST assertion/release controls reset; reset cancels current command state. | Figures give BSY/DRDY timing for hardware/power and software reset. | Primary sufficient for ATA reset timing; selected host reset pulse/service is L2. |
| ATA-F4 | ATA-3 clauses 8.3--8.5, pp.109--116 | PIO data-in/out protocols use DRQ, DIOR-/DIOW-, IORDY and command/data block counts; host advances only under defined conditions. | Error/abort ends command state; reset terminates transfer. | Register transfer and PIO cycle diagrams give setup/hold/recovery values. | Primary formula/value source: L3; Core tick conversion and board waits are L2. |
| ATA-F5 | ATA-3 clause 9, pp.122--137 | Register, PIO, DMA, reset and signal timing tables/figures define nanosecond minima/maxima and mode-dependent transfer requirements. | Electrical values apply only to a compliant selected interface/cable/device. | Exact timing depends on selected PIO/multiword-DMA modes and host/device constraints. | Primary timing facts L3; no repository controller/cable/device values selected: blocked. |

## Controller, Board And Backing-Media Boundary Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- |
| ATA-T1 | ATA-3 clauses 4--5 | ATA defines host/device registers and signals but not a PC ISA adapter's decode, IRQ, DMA channel or compatibility personality. | Host/device reset and interrupt behavior are interface facts, not a board profile selection. | Blocked: selected ATA controller/personality source is absent. Receiver: queued HDC/ATA phase contract. |
| ATA-T2 | ATA-3 clauses 5,7--8 | ATA permits device selection, diagnostics and device-specific IDENTIFY capability; it does not supply a concrete disk model or device word image. | Master/slave presence and diagnostic outcomes are selected-device facts. | Blocked: selected ATA device and master/slave policy absent. Receiver: queued HDC/ATA phase contract. |
| ATA-T3 | ATA-3 clauses 7--9 | Logical sector commands and IDENTIFY geometry fields do not define raw-image grammar, write persistence, failure injection or media lifecycle. | Device command failure must reach one Error/Status path. | Blocked: selected backing-media contract absent. Receiver: queued HDC/ATA phase contract. |
| ATA-T4 | ATA-3 clauses 4,8--9; T449 | Core transaction/scheduler/PIC/media owners must consume any chosen ATA request, PIO/DMA, IRQ and reset terms through their existing single paths. | An ATA interface must not create a second scheduler, media authority or IRQ delivery path. | Unallocated L2 board phase, not a code defect. Receiver: queued HDC/ATA phase contract. |
| ATA-T5 | T450 S1; current HDC boundary | The current HDC may retain its existing selected/non-ATA personalities until a source-backed ATA personality is admitted; no compatibility label implies ATA conformance. | Existing reset/finalization remain owner-local. | Explicitly blocked ATA implementation claim; receiver is queued HDC/ATA phase contract. |

## Completeness And S18 Transfer

The finite universe is `ATA-R1`--`ATA-R5`, `ATA-F1`--`ATA-F5` and
`ATA-T1`--`ATA-T5`.  It covers task-file/protocol/command/identify, PIO/DMA,
signal/reset/timing and the controller/device/media/board exclusions.  S18
must retain every identifier and record current owner/test disposition only;
it must transfer each nonconforming or blocked item once without declaring the
existing HDC an ATA controller.
