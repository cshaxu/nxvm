# M5 T3 S1 PC Device Verification

`nxvm-core-pc-devices-smoke` built with GCC `-Werror` and emitted
`M5:T3:S1:PC-DEVICES:OK`. It verifies device registration is single-owner, PIC
priority/acknowledge/EOI, deterministic PIT reload/tick/IRQ0 pulses, and DMA
channel registration/request/acknowledgement. The canonical PC/AT fixture smoke
still emitted `M5:T2:S2:PC-AT-BUILTIN:OK`; baseline source is unchanged.
