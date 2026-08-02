# M5 T38 S1 BIOS/POST Authority

The retained default-profile `vbios` now directly aliases the one `t_bios`
embedded in `vm_composition_live_machine`. Its POST and interrupt registration
tables, build addresses, and boot-disk state retain default-profile ownership;
the existing POST/reset sequence is unchanged.

Windows GCC, `M5:T38:S1:BIOS-AUTHORITY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t38.exe`.
SHA-256: `51BAD059FAA12305E66512D8BABB5EF7A8F3EDC375158C4F96083357078CD4C9`.
