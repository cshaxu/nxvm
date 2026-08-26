# M5 T479 PC Fixed-Disk Controller Standardization

T479 replaces the current universal-ATA assumption with one standards-led,
Core-owned PC fixed-disk controller boundary. It consumes the queued
[admission proposal](../proposals/m5-at-fixed-disk-controller-standardization.md)
and preserves the existing ATA PIO path only as one explicit personality.

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: 27-row source/cross-model and current-gap ledgers retain one current HDC owner, source-qualified Compaq/ATA behavior, IBM/ESDI boundaries and S2's finite caller sweep. |
| S2 | Accepted: owner-expanded 5160/5170/DeskPro/default-at reconciliation identifies the IBM XT adapter, a distinct 5170 fixed-disk configuration and WD1007A-WAH ESDI research target; one Core HDC owner and immutable discriminated replacement seam are proven. |
| S3 | Accepted: deleted the duplicate default-PC/AT ATA payload and field copy; one explicit Core personality now reaches the sole HDC owner, zero initialization is rejected, and Default-AT/Model-40 regressions plus the 78-gate run pass. |
| S4 | Accepted: primary/manual and bounded-emulator ledgers establish the IBM 5170-339 one-30-MB Type-3 fixed-disk/diskette-adapter contract, correct the false no-disk premise, and bound one Core/profile S5 implementation plus explicit physical-media residuals. |
| S5 | P1 implemented: the sole Core HDC owner now has an IBM WD1003/ST-506 personality, exact 8 MHz step-selector representation and `3F6h` high-head state; Model 339 selects its one Type-3 disk while ATA and Compaq regressions pass. Pending coordinator review and gate/artifact closure. |
