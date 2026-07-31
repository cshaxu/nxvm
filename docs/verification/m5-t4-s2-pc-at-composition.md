# M5 T4 S2 PC/AT Composition Verification

`nxvm-pc-at-firmware-smoke` emitted `M5:T4:S2:PC-AT-FIRMWARE:OK`. The package
registers ROM, POST, and INT 10h/13h/16h/1Ah identities in deterministic order
and states reset plan `F000:FFF0`. It intentionally contains no ROM bytes, BDA
construction, POST execution, or interrupt handler; those remain T4 work.
