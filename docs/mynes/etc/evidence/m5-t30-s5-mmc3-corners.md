# M5 T30 S5 MMC3 Capacity-Corner Verification

S4 identified that T30's finite MMC3 descriptor range had only direct upper
corner evidence. This corrective S extends the synthetic descriptor contract
fixture with the remaining rectangle corners:

| PRG banks | CHR banks | Expected result | Result |
| --- | --- | --- | --- |
| 2 | 0 | Header/profile admission, then explicit deferred execution | `LIB_STATUS_UNSUPPORTED` |
| 2 | 32 | Header/profile admission, then explicit deferred execution | `LIB_STATUS_UNSUPPORTED` |
| 32 | 0 | Header/profile admission, then explicit deferred execution | `LIB_STATUS_UNSUPPORTED` |
| 32 | 32 | Header/profile admission, then explicit deferred execution | `LIB_STATUS_UNSUPPORTED` |

The existing M4-size MMC3 fixture remains the positive runtime receiver. These
new rows do not claim extended Mapper 4 execution and therefore leave T33's
hardware scope intact.

Fresh builds and the focused `mynes.core.cartridge-contract-smoke` CTest passed
on x64 and x86. `Verify-DocumentationGovernance.ps1 -Scope All` and
`git diff --check` also passed. The product executable pair is unchanged: this
S modifies test-only source and evidence.
