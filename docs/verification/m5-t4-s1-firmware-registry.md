# M5 T4 S1 Firmware Registry Verification

GCC built `nxvm-firmware-smoke` with project code under `-Werror`; it emitted
`M5:T4:S1:FIRMWARE:OK`. The smoke verifies ordered enumeration, duplicate id,
interrupt-vector conflict, and frozen-registry rejection. No ROM, host API, or
baseline firmware code changed.
