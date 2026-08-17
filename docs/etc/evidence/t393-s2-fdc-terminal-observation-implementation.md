# T393 S2 FDC Terminal Observation Implementation

Core now accepts a generic copied FDC terminal-result provider through frozen
FDC topology. Only `PENDING_COMPLETE` publishes after final seven-byte result
formation and before IRQ. The record contains sequence, command, drive, result
bytes and success classification. Model 40 registers a private receiver that
copies the record; session reset invalidates that copy. No pointer, layout,
VM-specific Core type, C1, physical-time or firmware claim was added.

Focused Core and Model-40 smoke verify no preterminal record, successful result
copy, DMA-terminal success, media/geometry failure copy and reset invalidation.
