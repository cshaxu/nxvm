# 8086 Decoder-Ledger Guard Reconciliation

The current 8086 decoder-ledger verifier reads the pre-T447 monolithic
`machine.c` and requires an obsolete adjacent-case layout. T447 moved decoder
registration to `cpu_instructions.c` and source-timing selection to
`cpu_timing_model.c`, so the guard fails even though the 1,053-key runtime
runner and result verifier succeed.

Repair the guard to inspect the two current owners and their durable semantic
markers: `D7 -> XLAT` registration, `0F -> INS_0F` registration, the 8086-only
POP CS timing case, and the XLAT source timing form. Preserve every canonical
key, source rule and generated result unless focused evidence proves an actual
CPU/timing mismatch. The task must rerun runner, result verifier, decoder guard
and the complete current gate.
