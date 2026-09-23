# T487 S2 IBM 5160 Board Function And Timing List 1

`M5:T487:S2:IBM-5160-BOARD-LIST-1:ACCEPTANCE-CANDIDATE`

This is the complete **board** surface of T487, not a second 8088 or
controller ledger. Every row is an IBM source fact from the accepted S1
corpus. `Manual-L3` describes the authority of the named function or timing
relation; it is not a claim that current NXVM code already implements it.

| Key | Complete board rule | Primary source | Source level | Explicit boundary |
| --- | --- | --- | --- | --- |
| XT-B01 | OSC is 14.31818 MHz, 70 ns and 50% duty. | 1-16 | Manual-L3 formula | No host pacing or generic CPU-MHz conversion. |
| XT-B02 | CLK is OSC / 3: 4.77 MHz, 210 ns and 33% duty. | 1-16 | Manual-L3 formula | No instruction-retirement cost; that belongs to T486 and the Board transaction mapping. |
| XT-B03 | RESET DRV is active-high and synchronized to the CLK falling edge. | 1-16 | Manual-L3 logical timing | Controller-local reset states stay in their unit T. |
| XT-B04 | Under the documented not-ready condition, processor memory read/write is four 210-ns clocks (840 ns/byte). | 1-14 | Manual-L3 conditional cycle rule | Exact transaction classification and wait insertion are board-transaction work, never a guessed per-instruction surcharge. |
| XT-B05 | Processor I/O read/write is five clocks (1.05 us/byte). | 1-14 | Manual-L3 cycle rule | Device command/service phases belong to the addressed controller T. |
| XT-B06 | DMA transfers are five clocks (1.05 us/byte). | 1-14 | Manual-L3 cycle rule | DMA arbitration/state belongs to the 8237A T. |
| XT-B07 | Refresh is once per 72 clocks, occupies four clocks, and is approximately 15 us/7% bandwidth. | 1-14 | Manual-L3 formula | PIT/DMA refresh request production and transfer state belong to PIT/DMA T units. |
| XT-B08 | I/O CH RDY is the channel ready input used to accommodate slow I/O or memory devices. | 1-14; 1-16 | Manual-L3 signal relation | The manual does not make every card's ready behavior a 5160 board constant. |
| XT-B09 | The channel exposes the documented demultiplexed 8-bit data bus, 20 address lines, control, DMA, interrupt, refresh and check signals. | 1-14 through 1-16 | Manual-L3 topology | Card/device personalities and their pin-level response timing remain distinct units. |
| XT-B10 | I/O Channel Check reports an error as 8088 NMI. | 1-14; 1-16 | Manual-L3 causal route | PPI/PIC delivery order and latency belong to their respective unit ledgers. |
| XT-B11 | The NMI mask register is at `0A0h`; power-on masks NMI, `80h` enables and `00h` disables it. | 1-8 | Manual-L3 register/board route | This task neither recreates PPI/PIC state nor adds a second NMI path. |
| XT-B12 | System-board R/W memory is 128--256 KiB and the documented map partitions board, expansion, reserved and adapter windows. | 1-11; 1-12 | Manual-L3 construction/map fact | Exact selected RAM size and immutable mapping are profile/Core construction decisions, not a second memory owner. |
| XT-B13 | Base BIOS/BASIC occupies the documented 64 KiB `F0000h`--`FFFFFh` map region. | 1-12 | Manual-L3 map fact | No ROM bytes, discovery, default image or access-delay value follows from the map. |
| XT-B14 | The board diagram fixes the 8284A/8288/8088 and expansion-bus interconnect context, while 8259A, 8237A, 8254 and 8255A remain named component boundaries. | 1-6; 1-7 | Manual-L3 topology | Each named chip is an independent Queue unit with its own complete List 1/List 2. |

## Completeness And Exclusions

The S1 source topics are all represented: oscillator/CLK, normal and five-clock
cycles, refresh, reset, RAM/ROM, I/O channel, NMI mask/check and component
interconnect. A rule is not omitted merely because an existing device already
uses a related signal. Conversely, controller commands, CPU opcode timing,
firmware contents, selected card behavior, rotational/mechanical delays and
host pacing have no board-only implementation disposition here.

S3 must map every row above to one current Core transaction/time owner, its
sole production entry and its consumer paths. It cannot add a code row, repair
a symptom, or enlarge this frozen surface.
