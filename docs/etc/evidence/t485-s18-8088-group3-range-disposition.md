# T485 S18 8088 Group-3 Range Disposition

`M5:T485:S18:8088-GROUP3-RANGE-DISPOSITION:ACCEPTED`

Visual review of Intel Table 2-21's `MUL`, `IMUL`, `DIV`, and `IDIV` rows
confirms that every 8088 Group-3 arithmetic form is specified as a clock range,
not as an operand-to-clock formula.  The table's four-clocks-per-word-transfer
rule does not select a point inside those ranges.

| Form family | Primary source fact | Current owner/result | 8088 disposition |
| --- | --- | --- | --- |
| `F6/F7 /4` MUL | register/memory byte/word ranges | primary shape recognises the form; its exact evaluator returns no result | source-unallocated; no Manual-L3 receiver |
| `F6/F7 /5` IMUL | register/memory byte/word ranges | same | source-unallocated; no Manual-L3 receiver |
| `F6/F7 /6` DIV | register/memory byte/word ranges | same | source-unallocated; no Manual-L3 receiver |
| `F6/F7 /7` IDIV | register/memory byte/word ranges | same | source-unallocated; no Manual-L3 receiver |

`core_machine_l2_dynamic_arithmetic_model_cost()` is explicitly an
external-model-derived L2 path for 8086/80186 only; the exhaustive 8088
selector does not call it.  T361's cross-reference records why 86Box, MAME,
PCjs, Bochs and QEMU cannot turn the Intel ranges into an 8088 exact source
formula.  No current implementation, timing claim or public contract changes.

The earliest lawful future receiver is a source-qualified 8088
operand-to-clock formula or an approved, reproducible hardware-observation
contract that supplies it.  Until then, selecting an endpoint, midpoint, or
external emulator calibration is prohibited.
