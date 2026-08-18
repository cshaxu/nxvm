# T423 S1 CPU-To-Board Retirement Wait

`M5:T423:S1:CPU-BOARD-TRANSACTION:OK`

T423 adds one Core-owned completion/retirement wait state for an instruction
that accumulated chargeable external CPU-memory-cycle waits. The D4 profile's
existing original-source 2 KiB page result supplies those waits; while pending,
Core advances deterministic non-retiring time and does not refresh another CPU
instruction. Once drained, the existing retirement observation and physical
qualification gates publish the instruction through the same Core route.

The focused regression proves a budget slice advances one non-retiring wait
tick with zero completed instructions, preserves the pending state, and only a
later run publishes retirement. Reset clears the state. Model-40 selects the
existing D4 policy; 5170 retains its disabled policy.

This is not a CPU architectural rollback mechanism, asynchronous prefetch,
BWAIT signal model, ISA availability value, DCLK calibration, or Model-L3
acceptance. Those remain receiver-1 transfers.