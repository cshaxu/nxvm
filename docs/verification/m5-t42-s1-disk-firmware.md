# M5 T42 S1 Disk Firmware Boundary

Default-profile `qddisk` has no mutable service object. It registers its two
INT 13 handlers in the one bound QDX dispatch table and uses the existing core
block interface, whose provider already reaches the live HDD media object.

Windows GCC, `M5:T42:S1:DISK-FIRMWARE:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t42.exe`.
SHA-256: `5FBD6574048DB29DBEDCDB084BFEC299EF3707BB8D5EDD84DD3B73101D62AA66`.
