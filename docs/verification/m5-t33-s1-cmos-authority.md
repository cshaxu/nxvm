# M5 T33 S1 CMOS/RTC Authority

`vcmos` no longer owns global storage. `vm_composition_live_machine` owns one
embedded `t_cmos`; the transitional `vcmos` spelling dereferences that object.
The existing CMOS initialization, reset, refresh, finalization, and port
registration order remain unchanged.

Windows GCC, `M5:T33:S1:CMOS-AUTHORITY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t33.exe`.
SHA-256: `E7FEED574AC60AAD73030CDADCBFB801E74F9FF242089C0E8447FBB4CD1C1A2F`.
