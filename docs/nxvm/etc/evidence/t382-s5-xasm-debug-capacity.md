# T382 S5: xasm/Debug Capacity And Failure Contract

`M5:T382:S5:XASM-DEBUG-CAPACITY:OK`

The shared `core_product_utils_copy_text()` and
`core_product_utils_append_text()` owner rejects an invalid destination or a
non-fitting source without modifying the destination. The xasm contract smoke
proves exact fit, overflow rejection, and unchanged output on failure.

| Path | Capacity / failure owner | Result |
| --- | --- | --- |
| Public assemble/disassemble facade | Existing 255-byte statement and 15-byte code contract | Retained and tested. |
| Paragraph assembler | Existing per-line capacity and checked allocation | Retained and tested. |
| `aasm32` private instruction/label text | Fixed 256-byte fields, bounded line materialization | Audited; no unsafe copy/append remains. |
| `dasm32` private fields and final statement | Checked copy/append; trace error clears temporary output | Migrated. |
| Debug address, file, command, and display strings | Checked copy/append; display failure becomes deterministic `<ERROR>` | Migrated. |

The production sweep finds no `STD_STRCPY` or `STD_STRCAT` in the S5 owner
domain. Focused xasm contract, xasm smoke, and debug-input-failure smoke pass.
The current aggregate remains live but has the separately transferred
zero-session manager smoke contracts for S6.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `74FC086992B64CFFD22ACE64307A4CA6DD0A7BB8D96A10A3ED448F32627424B7` |
