# T366 S10: 80286 Unallocated Publisher Inventory

The selected 80286 classifier has three successful-retirement one-tick routes:

1. any nonzero prefix count;
2. the default opcode branch except `B0`--`BF`; and
3. a missing form in `core_machine_80286_source_timing_lookup`.

The audited source anchors are `if (prefixes != 0u)`,
`opcode >= 0xb0u && opcode <= 0xbfu`,
`core_machine_80286_source_timing_lookup`, and
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`.

All three return `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`, and the run loop adds
the result to `elapsed_ticks` after successful instruction refresh. They are
not device-time rows. Their receiver is the complete 80286 successful-retirement
source ledger; no S10 timing value is allocated.
