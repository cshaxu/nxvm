# M5 T40 S1 CGA Text-Service Boundary

Default-profile `qdcga` owns no mutable service object. Its text, mode, cursor,
and snapshot baseline data belong to the existing RAM and video-adapter
authorities; its mode-size table is now private immutable profile data. The
snapshot provider continues to read those same live objects directly.

Windows GCC, `M5:T40:S1:QDCGA-BOUNDARY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t40.exe`.
SHA-256: `FFD7FA2B39CF13BB4FB4F48CBCB6F8E55E156421E5A1BFFA7609B35ADD75DFE3`.
