# M5 T416: DeskPro DMA HOLD Locality Bridge

T416 consumes the next bounded CPU/DMA ownership receiver. It uses original
D4 schematic topology and a labelled generic-AT policy: a completed DMA bus
handoff invalidates the Core CPU locality key. It cannot define an original D4
phase, page-retention or timing duration. P1 implements this boundary with
request/acknowledge/release/reset proof.
P2 independently closes T416. It confirms the existing Core transaction owner
invalidates locality only at acknowledged DMA HOLD, verifies request,
acknowledge, release, next-cycle miss and reset, and retains exact D4
page-retention and all DMA/refresh/BWAIT phase work as future receivers.
