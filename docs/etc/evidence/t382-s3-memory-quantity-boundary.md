# T382 S3: YAML Memory Quantity Boundary

`M5:T382:S3:MEMORY-QUANTITY:OK`

`core_product_utils_parse_memory_kib()` is the sole decimal KiB-to-byte
conversion owner used by the YAML catalog and `--memory-kib` session-option
construction. It rejects empty, signed, non-decimal, zero, trailing and
overflow input before any session configuration is built.

The catalog smoke accepts one KiB exactly and rejects negative, trailing,
overflow, empty and zero values. The production sweep finds only the YAML
field, the session option, firmware CMOS metadata, and non-parser display
uses; no second text conversion remains.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `23D3212C59D1DD1BCE2760FF248AD104C045E7BBDD063533AEF59F373DE87B72` |
