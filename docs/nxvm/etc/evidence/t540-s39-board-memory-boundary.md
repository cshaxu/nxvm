# M5 T540 S39 Board Reset RAM And Memory Resize Boundary

## Actual source diff

The one default-memory-size resolution is now a small private calculation
used by both neutral allocation and board assembly. The existing 286/386
firmware-less high reset mapping to F0000h RAM backing moved from neutral
creation to the board-create phase, still before port registration, public
machine publication and any firmware ROM overlay. It retains the same
at-least-1-MiB condition, CPU-profile selection, physical alias, backing
offset, length and `INVALID_ARGUMENT` failure status. An error still destroys
the sole candidate; there is no second memory copy or reset route.

The existing public `core_machine_reconfigure_memory` is now the board-side
entry for the planar-parity veto. It calls one private Core operation for the
unchanged stopped/frozen/mutable checks, mapping bounds, allocation and cold
reset. The board does not retain or mirror RAM bytes. Both prior veto and
Core validation still return `INVALID_STATE` in their original cases.

Three NXVM files changed: **33 lines added, 18 removed, net +15**. The new
private entry is a necessary owner seam, not a second implementation. No
Shared/MyNES, owner INI, firmware input, profile RAM value or timing formula
changed. D4 shutdown and input dispatch remain S40 work.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
- Specialized NXVM gate target passed, including RAM reconfiguration and
  direct strict-compilation checks.
- Existing firmware-less reset, 286/386 alias, memory resize and planar
  parity regressions are included in the complete unit runs.
- External boot checkpoint passed once for Default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `9A49CC45EFF01E7022BBE10F42E2006B5F193CD03AA0C5646C0E2AF438677304`, `E7C5148520B82191583861168F91705857C121B678538DAB755472DE87B38C4C`.
  - Default: `5D662E1C353CD5E9EF46EC7E87174794B193A9BFE757149BFADD67C866E94333`, `35D93DAAC076F598B73C89B8764C7C77EB366A51E1EEE10FE13FC0D8ACB905D0`.
  - XT: `DC01595325FB0AE89A4B775C1C237A70AC5999B4D162C6272B07CB828AA841F7`, `2A5708B5D6135EE6C1863CD89DBDFC964547449BD4F08B83A7D8B25A254A7FB1`.
  - 5170: `B169EBFE0228AD7D4FC7457C61EFC1C0445EB8CEFB57B2CF208D5C5AE7292488`, `162976CA21911397F4E730D556076B678861C70597FB2C464902D074FC34B548`.

T540 remains open for S40-S43 and later IBM-PC board extraction.
