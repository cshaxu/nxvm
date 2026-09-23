# T408 S1 Original D4 CPU-Memory Timing Admission

`M5:T408:S1:D4-MEMORY-WAIT:TRANSFER`

The owner-managed copy of *Compaq DeskPro 386/16 Technical Spec*, D3PE
Processor Descriptions, pages 60--63 (January 1987) is primary material for
the selected hardware. It states that an initial or row-miss CPU memory read
reaches `MRDY*` after eight `CLK32` cycles/four processor states, inserting two
wait states over the CPU's two-state memory access. A read hit on the already
latched DRAM row reaches `MRDY*` after four `CLK32` cycles/two processor
states, inserting zero waits. It further states that an initial system-board
read waits for the board cycle to finish, and that writes have analogous but
separate signal timing.

This does not authorize a production scalar in the current source graph.
NXVM's Core CPU transaction is a checked logical physical access, not an
observable 80386 external bus cycle: instruction-byte fetch, page-table walk,
data operand access and split access share that path. The source does not
supply the address-to-DRAM-row decoder needed to determine `HIT*`, and the
current CPU has no prefetch/bus-cycle owner. Charging every logical access
would misclassify fetch and paging activity; choosing a guessed row/page key
would fabricate the board decoder.

T408 therefore accepts the source as an `original` evidence upgrade and
transfers implementation to the existing DeskPro physical-observable timing
receiver. Admission requires one Core-owned external CPU-memory-cycle boundary,
a source-backed D4 address-to-row/bank decoder, reset/cancellation treatment,
and a bounded trace that distinguishes fetch, page walk and data cycles. The
existing CPU/DMA transaction and HOLD/HLDA mechanisms remain functional
logical-owner evidence only. No firmware, media, source text, ROM byte,
machine-local path or physical/L3 claim enters the repository.