# T366 S10: 80286 Unallocated Publisher Inventory

The selected 80286 classifier has two direct successful-retirement one-tick
routes and one guarded lookup-fallback mechanism:

1. any nonzero prefix count;
2. the default opcode branch except `B0`--`BF`; and
3. a missing form in `core_machine_80286_source_timing_lookup`.

The audited source anchors are `if (prefixes != 0u)`,
`opcode >= 0xb0u && opcode <= 0xbfu`,
`core_machine_80286_source_timing_lookup`, and
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`.

The first two return `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` directly. The
third is available to both the ordinary and control/stack 80286 classifiers.
The S10 static check extracts every control/stack timing form and proves it is
present in the 80286 ledger, so no currently selected successful control/stack
row reaches that fallback. Its null-machine and non-80286 switch defaults are
not valid successful 80286 runtime paths.

Any reached fallback result is added to `elapsed_ticks` after successful
instruction refresh. These are CPU source-ledger rows, not device-time rows.
Their receiver is the complete 80286 successful-retirement source ledger; no
S10 timing value is allocated.
