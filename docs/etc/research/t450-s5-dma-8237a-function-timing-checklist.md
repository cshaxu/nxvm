# T450 S5 DMA 8237A Original Function And Timing Checklist

> S19 direct PDF check: scan with OCR text layer (not born-digital); rendered
> pages govern diagrams/tables. Chip facts are sufficient; AT wiring/service
> phase remains L2. No external emulator is primary.

## Scope And Source Key

This is the complete DMA source universe transferred unchanged to T450 S6.
`I8237` means Intel *8237A High Performance Programmable DMA Controller*,
order 231466-005 (Sep. 1993); page references are printed data-sheet pages.
`AT` means IBM *Personal Computer AT Technical Reference*, 1502243 (Mar.
1984), system-unit pages. The finite list describes the Intel chip and the
selected IBM AT board binding, not an electrical reconstruction: AC tables and
waveforms remain L4, and a board-visible transaction cadence must not be
invented from them.

## Register, Programming And Reset Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| DMA-R1 | I8237 pp. 4, 7, 10 | Each of four channels has 16-bit base/current address and word-count registers. CPU writes base and current together, reads current in low/high bytes through the first/last flip-flop. Address advances or retreats after a transfer; count decrements, and zero-to-FFFFh is terminal count. | Reset/master clear clears the byte selector; EOP can auto-reload current from base. | Programming occurs only while HLDA is inactive; registers select transfer extent, not elapsed time. | L3 chip state; board transaction scheduling is L2. |
| DMA-R2 | I8237 pp. 4, 7--10 | Command controls controller-wide enable, priority, timing, transfer and memory-to-memory selections; four mode registers select channel, transfer type, address direction, auto-init and service mode. | Reset/master clear clears command; valid mode programming is required before service. | CPU accesses A0--A3/IOR/IOW in Program Condition; bus electrical timings are L4. | L3. |
| DMA-R3 | I8237 pp. 4, 7--9 | Request register supplies software requests; mask bits suppress DREQ; status reports terminal-count and pending-request state; temporary register retains the last memory-to-memory byte. | Reset/master clear clears request, status and temporary state and sets all masks; status read clears its terminal-count bits. | Software request participates in priority; a mask blocks external DREQ, not the register definition. | L3. |
| DMA-R4 | I8237 pp. 4, 8--10 | Clear first/last flip-flop, master clear, clear mask, single-mask write and all-mask write are discrete programming commands. | Master clear has hardware-reset effect and returns to idle; clear mask enables all channels. | Commands are host transactions with no documented elapsed delay. | L3. |

## Service, Transfer And Arbitration Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| DMA-F1 | I8237 pp. 2--5 | An unmasked DREQ or eligible software request selects a channel; DACK acknowledges an accepted DREQ. Fixed priority is 0 highest through 3 lowest; rotating priority moves the last serviced channel lowest. | Reset masks every channel and restores DREQ/DACK active polarity defaults. | DREQ is sampled in idle; it remains asserted until DACK. Rotating priority guarantees recognition after at most three higher services on one chip. | L3 logical arbitration; selected DREQ input cadence is L2. |
| DMA-F2 | I8237 pp. 3--5, 11 | A valid request asserts HRQ; after HLDA the controller owns address, data and control buses and transfers. Programming and HLDA must be mutually exclusive. | End of service deasserts HRQ and waits for HLDA low before a new service. | Idle is SI; S0 waits for HLDA; S1--S4 perform a normal DMA transfer, with READY inserting SW between S2/S3 and S4. | L3 causal states; clock rate/CPU arbitration publication is L2. |
| DMA-F3 | I8237 pp. 2, 4--6 | Single mode performs one transfer then releases the bus; demand mode continues while DREQ remains active; block mode continues after accepted DREQ until TC or external EOP; cascade delegates a priority slot to another DMA controller. | TC or EOP ends the documented service; demand requires DREQ low before S4 to suppress a further transfer. Cascade suppresses conflicting outputs and ignores READY. | Single mode guarantees one CPU machine cycle between transfers only for the named legacy CPU protocols; no universal CPU-duration rule follows. | L3 mode semantics; selected Core interleave/timing is L2. |
| DMA-F4 | I8237 pp. 2, 5--6 | Write moves I/O to memory (IOR plus MEMW); read moves memory to I/O (MEMR plus IOW); verify advances state without memory/I/O controls. | EOP/TC follows the selected mode; verify ignores READY. | Data moves directly between I/O and memory during read/write, not through the 8237A. Signal pulse widths are L4. | L3 function; electrical waveform excluded. |
| DMA-F5 | I8237 pp. 2, 5--7 | EOP is external input or terminal-count output; TC/EOP clears request, updates mask unless auto-init, and may reload base values. | Auto-init restores current address/count after EOP and leaves its mask unchanged. | EOP is a causal end-of-process relation; pulse width and setup/hold are L4. | L3. |
| DMA-F6 | I8237 pp. 5--6 | Memory-to-memory uses channel 0 source and channel 1 destination, temporary register, block service and software DREQ0; either address can increment/decrement, and channel 0 can hold address. | Channel-1 TC produces EOP; external EOP can terminate service; both word counts need matching programming for dual auto-init. | One byte uses S11--S14 memory read then S21--S24 memory write; active AEN with no DACK identifies this operation. | L3 logical states; waveform/cycle rate L4/L2. |
| DMA-F7 | I8237 pp. 6--7 | Compressed timing removes S3 when system characteristics permit; S1 remains when upper address latch data changes. Address generation multiplexes A8--A15 through an external latch, uses ADSTB and AEN, and normally refreshes that latch only on carry/borrow. | The transfer/service end rules remain the selected mode's EOP/TC rules. | Normal transfer is state-based; compressed transfer is two clocks, while exact CLK parameters and latch propagation are electrical. | L3 state selection; L4 physical parameters; selected clock formula L2. |

## Selected IBM AT Binding And Timing Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Disposition |
| --- | --- | --- | --- | --- |
| DMA-T1 | AT pp. 1-8, 1-12--1-14 | IBM AT uses two 8237A-5 controllers: primary channels 0--3 are 8-bit, secondary channels 5--7 are 16-bit, and channel 4 cascades the primary to the processor. Channel 2 serves diskette; channel 1 is the refresh-request consumer. | This is selected board topology; reset follows each controller's normal reset/programming rule. | L3 topology. |
| DMA-T2 | AT pp. 1-12--1-14, 1-25--1-26 | The board supplies page registers for channels 0--3 and 5--7, refresh page 008Fh, 64 KiB blocks for 8-bit channels and 128 KiB blocks for 16-bit channels; 16-bit channels cannot use odd boundaries or cross their page boundaries. | Page registers are board address-extension state, not a second 8237A register bank. | L3 board function; page-register reset and ownership details require later selected-contract evidence. |
| DMA-T3 | AT pp. 1-8--1-9, 1-12 | The AT drives both DMA controllers at 3 MHz, making listed DMA data-transfer bus cycles five clocks (1.66 microseconds), excluding bus-control transfer. PIT channel 1 generates a 15-microsecond refresh request; controller channel 1 is its refresh consumer. | Refresh repeats through the PIT/request route; it is not a DMA-internal free-running timer. | L3 selected source formula and wiring; its publication into the Core timing/transaction owner is L2. |
| DMA-T4 | AT pp. 1-25--1-26; I8237 pp. 2--5 | AT exposes DRQ0--3 and DRQ5--7, reserves DRQ4 for the board cascade, and provides active-low DACK lines. A peripheral holds DRQ until DACK; cascade-capable adapters use a DMA cascade channel. | Device completion follows the selected channel's DACK/EOP lifecycle. | L3 wiring/polarity; actual DREQ production, grant, page and refresh transaction binding are L2 inputs for the later DMA candidate. |
| DMA-T5 | T433 S6/S7; T449 | Core has one transaction/arbitration/time owner. The DMA controller must eventually consume a selected request, grant, page and refresh contract rather than schedule itself or add a second publisher. | No implementation conclusion is made here. | L2 receiver: queued Core DMA 8237A phase contract. |

## Cross-Source Qualification

Each row keeps the admitted Intel/IBM source as its sole normative authority.
`Manual L3` means that source directly defines the retained logical or selected
board fact. `Other L3` is a dated, read-only corroboration from a named
emulator implementation; it cannot add a requirement or override the manual.
Every remaining Core assertion is `fallback to L2`, not inferred from an
emulator's host scheduler, compatibility workaround or non-8237 extension.

The corroborating observations were made on 2026-08-24 from
[MAME AM9517A](https://github.com/mamedev/mame/blob/master/src/devices/machine/am9517a.cpp),
[QEMU i8257](https://github.com/qemu/qemu/blob/master/hw/dma/i8257.c),
[86Box DMA](https://github.com/86Box/86Box/blob/master/src/dma.c),
[PCjs chipset](https://github.com/jeffpar/pcjs/blob/master/machines/pcx86/modules/v2/chipset.js),
and the retained Bochs compatibility source. MAME is the only observed
reference with an explicit per-state 8237-compatible engine. QEMU explicitly
rejects several command features; PCjs retains mask-command TODOs and a
machine-specific terminal-count workaround; 86Box includes later-machine DMA
extensions. Those limitations are evidence boundaries, not defects in the
manual ledger.

| ID | Manual L3 | Other L3 corroboration | Residual fallback to L2 |
| --- | --- | --- | --- |
| DMA-R1 | Base/current address and count, first/last access, increment/decrement and TC. | MAME, QEMU, 86Box and PCjs retain base/current pairs and byte selector. | No selected Core programming/service interleave. |
| DMA-R2 | Command/mode bit meanings and Program Condition. | MAME consumes the command and mode choices; QEMU/PCjs explicitly leave subsets unsupported. | No selected Core command-to-service phase. |
| DMA-R3 | Request, mask, status and temporary-register lifecycle. | MAME/QEMU model software request and read-to-clear TC; 86Box/PCjs corroborate register shape. | No peripheral DREQ production contract. |
| DMA-R4 | First/last clear, master clear and mask commands. | MAME/QEMU implement the command family; PCjs' missing mask handlers are a negative boundary. | No host transaction delay is inferred. |
| DMA-F1 | DREQ recognition, fixed/rotating priority and DACK relation. | MAME models polarity and DACK; Bochs models DACK and cascading request propagation. | Selected peripheral request cadence and visible acknowledge route. |
| DMA-F2 | SI/S0/S1--S4, READY wait and HRQ/HLDA service order. | MAME has explicit SI/S0/S1--S4/SW states. | Core transaction/arbitration publication and physical clock conversion. |
| DMA-F3 | Demand, single, block and cascade service semantics. | MAME models mode exits and cascade state; Bochs propagates the master/slave cascade request. | Selected CPU interleave duration and controller-to-controller topology beyond AT. |
| DMA-F4 | Read/write/verify data-control semantics. | MAME and QEMU preserve transfer-type separation. | Signal pulse widths and unselected device behavior. |
| DMA-F5 | TC/EOP, masking and auto-init consequences. | MAME and Bochs drive terminal/EOP and DACK release. | Physical EOP pin waveform. |
| DMA-F6 | Channel-0/1 memory-to-memory sequence and temporary byte. | MAME represents the separate S11--S14/S21--S24 state groups; 86Box corroborates M2M function only. | Exact board cycle conversion. |
| DMA-F7 | Compressed-state selection and address-latch relation. | MAME selects compressed state flow. | CLK/latch propagation and any unselected timing formula. |
| DMA-T1 | IBM AT dual-controller widths, cascade and channel bindings. | QEMU, Bochs and PCjs instantiate the two-controller AT pattern. | Non-AT topology. |
| DMA-T2 | IBM AT page mapping and 8-/16-bit boundary rules. | QEMU, 86Box and PCjs corroborate AT page/word-controller mapping. | Page-register reset details not selected by the board source. |
| DMA-T3 | IBM AT 3 MHz/five-clock transfer formula and PIT1 refresh route. | No external implementation elevates a board timing rule. | Core publication of this selected formula and refresh chain. |
| DMA-T4 | IBM AT DRQ/DACK polarity, channel reservation and peripheral hold rule. | MAME/Bochs model DACK; PCjs/Bochs corroborate AT cascade routing. | Selected DREQ producer, DACK consumer and page/refresh transaction contract. |
| DMA-T5 | None: this is an internal owner boundary, not an Intel/IBM chip fact. | None. | One Core-owned request/grant/page/refresh integration contract. |

## Completeness And S6 Transfer

The finite universe is `DMA-R1`--`DMA-R4`, `DMA-F1`--`DMA-F7`, and
`DMA-T1`--`DMA-T5`. It covers the 8237A's register/program/reset forms,
request-to-grant state sequence, service and transfer modes, EOP/TC and
auto-init lifecycle, memory-to-memory and compressed/address forms, plus the
selected AT's two-controller, channel, page, refresh and clock bindings. T450
S6 must retain every identifier, audit only present code/tests, and assign each
nonconforming or unallocated row once to the queued Core DMA 8237A phase
contract; it must not expand this source universe.
