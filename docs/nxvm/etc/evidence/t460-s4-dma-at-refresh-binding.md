# T460 S4 DMA AT Refresh Binding

IBM AT's PIT counter 1 refresh request now reaches only the Core-owned DMA1
binding. `core_machine_configure_dma` creates that private binding beside the
existing FDC binding, and the PIT output uses only its opaque request token.
The board never mutates DMA state directly. DMA1 is consequently reserved from
FDC topology selection, and no second refresh scheduler or page owner is added.

The focused authority regression proves invalid FDC=DMA1 rejection and the
machine-owned DMA1 binding/token; binding-token and authority smokes pass.
The five-clock/3 MHz numerical conversion remains deliberately unclaimed: the
manual supplies a board frequency but the selected Core clock-plan does not yet
provide that calibrated relation. It remains an explicit L2 timing boundary,
not a guessed delay.

`M5:T460:S4:DMA-AT-REFRESH-BINDING:OK`
