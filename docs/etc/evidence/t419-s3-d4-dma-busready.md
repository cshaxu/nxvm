# T419 S3 D4 DMA BUSRDY Gate

M5:T419:S3:D4-DMA-BUSRDY:OK

## Evidence Tier

`original`. The approved Compaq DeskPro 386/16 D3PE processor description
states that every DMA cycle receives one DCLK wait and that a peripheral can
hold BUSRDY low to add waits. It separately describes CPU bus-cycle BWAIT and
refresh arbitration; neither is asserted by this increment.

## Implemented Contract

Core owns one discrete DMA BUSRDY input. Model-40 enables the gate, and reset
sets it ready. While the input is not ready, a pending DMA request neither
consumes the existing DCLK-equivalent wait quantum nor begins a DMA transaction.
When it returns ready, the existing Core DMA HOLD and transaction route resumes;
no VM-side executor or secondary CPU/DMA route exists.

The public configuration field is appended. The setter is valid only for a
machine whose composition enabled this capability. Its input is a deterministic
guest-time level, never host time.

## Verification

`core-machine-competition-s3-smoke` verifies a blocked pending byte DMA
transfer has no memory write and no consumed wait quantum across two time
advances; releasing BUSRDY creates the existing one-quantum wait and then
exactly one write. It also verifies cold reset restores ready and clears the
wait state. `vm-model40-dma-s17-smoke` verifies the Model-40 selection and
emits this marker.

## Remaining Transfer

This is not calibrated DCLK or CLK16 timing, a CPU BWAIT model, a physical
HAK/HAKDMA waveform, a peripheral BUSRDY binding, refresh/DMA priority, or
Model-40 L3 acceptance. Those remain the following D4 arbitration receiver
parts under the physical-cycle proposal.