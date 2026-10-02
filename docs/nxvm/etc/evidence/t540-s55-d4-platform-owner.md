# M5 T540 S55 Board-Owned D4 Platform

## Scope and actual diff

The D4 platform configuration, Port-B, configured flag, IOCHK/failsafe
latches and NMI-signaled flag moved from flat `core_machine` into the existing
board attachment. All 54 production accesses retain their original
condition, value and order; the direct port-assembly rollback test follows
the board owner. Failsafe PIT output, shutdown/reset, speaker gate, Port-B
read/write, NMI assertion and latch clearing still use their one existing
signal path. The D4 refresh pending/pulse/address latches are deliberately
unchanged and assigned to S56. P1 is `8ca51989a`.

The four source/test files change **64 additions / 62 removals**, including
the six-field move and two direct test expressions. Eight optimized 0540
artifacts are updated. No Shared/MyNES, profile INI or external asset changed.

## Verification

- Complete repository-only units: x64 **469/469**, x86 **469/469**, run
  sequentially without concurrent Release builds.
- Both-width `verify-current-specialized-gates`: pass. The full suites include
  Model-40 D4/parity and Port-B transaction regressions.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `2545B53BD3D13AF16C66730A8C37BBCDAF2DBE0FDA17C6B251655F8D516C6873`, `C948392D33445BB0A72672FA21FE3E8461F338FB18A08713C69E2EB4652BD10E`.
  - Default: `2936E8185739612C5F0D00D70DC7A45921A77A460C2C1094F90F1CA9CC30FAFA`, `87355BBE21C0CBDB954FB7A6D56AEE6FDBA27E4C7135F505456559FA70F6358E`.
  - XT: `47151BBA5101140B0AABC751D831F603B91920A0C1BCEF0C324EEA8BE6F2EB34`, `B10E27A9851386970F25E5E9798A713D74CE1A33099E5E4C8AE7314B59CAEA9C`.
  - IBM 5170: `E403E1570FFFC9CBB1C36552DDBF1DBBD059E3B75C87017F3D22D2A2B697939E`, `C85C6180F4E6BC8AD41BCB24A87FB6DD1F36A4C62F98DA056AE217ADDE0FA7E8`.

S55 closes only the D4 platform Port-B/NMI state group. S56 receives D4
refresh request state and DMA wiring; T540 remains open.
