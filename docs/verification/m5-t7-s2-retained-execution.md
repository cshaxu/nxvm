# M5 T7 S2 Retained Execution Verification

The retained bridge was exercised through the existing NXVM Console with the
M1 local fixtures and instruction recording enabled. Each run used a twelve
second external wall-clock budget; timeout is the intended bounded stop and no
guest media was modified.

| Boot target | Local trace result | Final observed execution |
| --- | --- | --- |
| FDD | ignored `build/m5-fdd.trace`, 664,407,892 bytes | active DOS/BIOS instruction stream at timeout |
| HDD | ignored `build/m5-hdd.trace`, 266,861,745 bytes | DOS code at `0250:01AA` invoking BIOS `INT 16h` at timeout |

Both fixture identities were previously revalidated in
`m5-t5-s2-pc-at-block-composition.md`. The external timeout leaves no NXVM
process running. This confirms continued bounded execution through the retained
bridge; it does not yet prove that the final product Console entry routes all
configuration through the product session.
