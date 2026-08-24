# 8086 Decoder-Ledger Guard Reconciliation

The current 8086 decoder-ledger verifier read the pre-T447 monolithic
`machine.c` and required an obsolete adjacent-case layout. T447 moved decoder
registration to `cpu_instructions.c` and source-timing selection to
`cpu_timing_model.c`, so the guard failed even though the 1,053-key runtime
runner and result verifier succeeded.

The repair inspects the two current owners and their durable semantic markers:
`D7 -> XLAT` registration, `0F -> INS_0F` registration, the 8086-only POP-CS
route and timing case, and the XLAT source timing form. It preserves every
canonical key, source rule and generated result.
