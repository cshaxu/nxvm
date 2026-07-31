# M5 T6 S2 Presentation And Debugger Adapter Verification

GCC built and ran `nxvm-product-presentation-debug-smoke` under `-Werror`; it
emitted `M5:T6:S2:NXVM-PRESENTATION-DEBUG:OK`. The smoke rejects input and text
publication outside a boundary, preserves FIFO input at the boundary, copies a
text snapshot, and rejects debugger CPU access outside its boundary before
allowing paused-state CPU inspection and finite stepping through `core/debug`.

The retained bootable NXVM Console task artifact is
`build/output/nxvm-m5_t6.exe`, SHA-256
`043e3c54fadc992e7e73b70bcd5bd9c6ad5ccd8ae18f19552dd291ac7f05e67e`.
It emitted banner `0.4.015d.m5t6`; piped `exit` returned zero.
