# T437 S2 80386DX Result Producer

The 80386DX manifest runner retains the first real retirement observation for
each selected canonical metadata record. A captured record contains the timing
origin, ticks, formula-input bitmap, Core form identifier and unallocated
disposition required by the shared result contract.

The runner's final-result writer checks the full generated 1,410-key set before
any output-file operation. It refuses emission while even one canonical record
is absent, so S3--S7 cannot turn a partial recipe set into an apparently final
artifact. Repeated retirement of the same key does not overwrite the first
capture. S8 completes the sole non-scalar key, `I386-ESC`, using the separately
verified CPU/FPU handoff record required by Chapter 17; it does not invent a
CPU clock value for the coprocessor's clock domain.

S2 proves the producer with a genuine 80386DX `NOP` retirement. The remaining
1,410 records retain their S1 partition ownership and remain explicitly
incomplete; no key becomes conforming through this plumbing.

```text
M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=1:canonical=1410
M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS
```
