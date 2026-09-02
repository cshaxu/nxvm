# T511 S3 DMA Phase-Consumer Audit

`M5:T511:S3:DMA-PHASE-CONSUMER:NO-HIT`

## Sources

The primary source is Intel *8237A High Performance Programmable DMA
Controller*, order 231466-005 (September 1993), owner-managed copy
`../../../../nxvm-assets/manuals/dma/intel-8237a-dma-controller-sep1993.pdf`.
It defines DREQ as held through its corresponding DACK, one transfer's
S1--S4 service, terminal-count/EOP termination, and the distinct Demand,
Single and Block transfer rules.  In particular, Block mode permits a held
DREQ through the block, whereas Single mode releases the bus after one
transfer and Demand stops when DREQ is withdrawn.

Bochs's DMA model independently retains separate DRQ, DACK and TC state and
withdraws the cascade acknowledgement at transfer end.  It corroborates the
line distinction only; it is Other-L2 reference material and was not copied.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| FDC byte request | The FDC publishes a DMA2 request; its byte provider withdraws that request, schedules `next_dma_byte_tick`, then only the FDC deadline republishes it. | Fresh observation required and present for every FDC byte. |
| 8237A arbitration and DACK | `core_machine_dma_advance_transaction` owns request selection, logical DACK, S1--S4 and one provider transfer. | The provider cannot reuse a past grant. |
| Single/Demand terminal path | `dma_complete_transfer` clears the serviced request state, invokes the binding's terminal callback and applies TC/mask/autoinitialize together. | Fresh request or owner re-publication is required and present. |
| XT refresh | PIT channel 1 is the sole board signal source for DMA0; it publishes the next refresh request independently. | One pulse is one request; it is not data streaming. |
| Xebec XT block transfer | The Xebec owner retains DMA3 while its documented block service remains active, then releases it only for response, error or TC. | A held request is the correct Block-mode contract, not a stale observation. |
| CPU/firmware/VM | No firmware or VM code consumes a DMA-ready latch directly; Core routes provider callbacks, memory transaction, TC and subsequent owner state. | No bypass or second consumer. |

## Executed Proof And Conclusion

`core-machine-dma-channel-smoke` covers DREQ, DACK, Single/Demand/Block,
EOP, TC and cascade ownership.  `core-machine-fdc-smoke` proves two DMA bytes
with the first DRQ withdrawn before the scheduled second DRQ becomes visible,
including the no-duration logical fallback.  `core-machine-xebec-wiring-smoke`
proves two contiguous 512-byte sectors through DMA3, TC/error termination and
response publication.  `core-machine-dma-rtc-authority-smoke` covers the
PIT1-to-DMA0 board route.  All four passed in the repository Debug gate.

No ATA-style stale-ready hit exists.  Re-requesting Xebec per byte would
contradict its Block-mode service; retaining FDC's existing re-publication is
the minimal correct structure.  No production code changes are warranted.
