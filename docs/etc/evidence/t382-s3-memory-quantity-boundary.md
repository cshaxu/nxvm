# T382 S3: YAML Memory Quantity Boundary

`M5:T382:S3:MEMORY-QUANTITY:OK`

`core_product_utils_parse_memory_kib()` is the sole decimal KiB-to-byte
conversion owner used by the YAML catalog and `--memory-kib` session-option
construction. It rejects empty, signed, non-decimal, zero, trailing and
overflow input before any session configuration is built.

The catalog smoke accepts one KiB and the exact 64-bit KiB upper boundary,
then rejects negative, trailing, overflow, empty and zero values. The
production sweep finds only the YAML field, the session option, firmware CMOS
metadata, and non-parser display uses; no second text conversion remains.

Focused catalog and Console input-failure smokes pass. The full current-gate
aggregate was live and completed, but retains two pre-existing VM construction
failures (`current.vm-multi-window-session-smoke` and
`current.vm-session-manager-smoke`); neither reaches the new conversion owner.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `09D7F0E8DED60DC0A52FF3BD45BDEE1155850013C15F8346E8AC0BEAFEAC6AEA` |
