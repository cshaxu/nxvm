# T460 S3 DMA Mode Phases

The Intel 8237A manual makes cascade a delegated priority slot, not a terminal
transfer, and specifies memory-to-memory as channel-0 memory read followed by
channel-1 memory write through the temporary register. `dma.c` now represents
only those two logical M2M phases in its existing data owner. Each phase uses
the existing T449 transaction owner; release/reset clears the phase, so a
failed write cannot retry as an orphaned destination write. No second queue,
page store, timer or device API exists.

The paired AT cascade remains the existing `advance_one` path. A programmed
cascade slot now releases without TC, masking, provider completion or transfer.
The channel smoke proves that result, M2M read-before-write, terminal/EOP,
auto-init and source/destination route failures. The five focused DMA,
transaction and FDC tests pass. Compressed electrical clocks/latch pulses stay
outside this logical Core increment and are retained for S4/closure review.

`M5:T460:S3:DMA-MODE-PHASES:OK`
