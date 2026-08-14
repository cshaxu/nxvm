# T366 S9: Physical-Time Non-Admission

The selected Model 339 is 8 MHz, and selected uPD765 quantities are expressed
against an 8 MHz controller clock. However, the 80286 runtime cost classifier
still publishes `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` (one) for successful
prefixed instructions and for opcodes outside its local selected rows. Those
values are added to `elapsed_ticks` before all machine clock domains advance.

Intel-backed rows such as 80286 NOP therefore establish the meaning of their
own values, but not a global physical meaning for every elapsed tick. S9 makes
no conversion. The receiver is the 80286 complete successful-retirement timing
closure: it must eliminate or explicitly prohibit every successful unallocated
publisher before a selected-machine physical clock can be admitted.
