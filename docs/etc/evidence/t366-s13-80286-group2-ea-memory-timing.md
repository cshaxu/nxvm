# T366 S13: 80286 Group-2 Effective-Address Memory Timing

## Source And Scope

Intel Appendix B, retained in the [T357 S6 80286 ledger](t357-s6-80286-instruction-timing-ledger.md),
assigns implicit-count-one Group-2 memory operations seven clocks and adds one
clock only for a 16-bit base-plus-index-plus-displacement effective address.
S13 reuses that already source-backed private helper for every valid memory
`D0`/`D1` encoding; direct memory remains seven clocks and the selected
base-plus-index-plus-displacement vector is eight.

## Sweep And Transfers

The sweep covers every valid `D0`/`D1` register or memory encoding, `D2`/`D3`,
`C0`/`C1`, the Group-2 executor, 80286 EA helper and fallback classifier.
Register and direct-memory values retain S11/S12 outcomes. `D2`/`D3` and
`C0`/`C1` retain count-dependent CPU-ledger receivers; `/6` remains undefined.
No device, bus or physical-clock claim is made.
