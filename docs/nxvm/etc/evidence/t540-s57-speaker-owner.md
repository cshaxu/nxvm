# M5 T540 S57 Board-Owned Speaker State

## Scope and actual diff

The four XT PPI speaker fields—configuration, timer gate, data enable and
observed output—moved from flat `core_machine` into its existing sole board
attachment. The PPI/PIT/Port-B signal calculation, reset, output refresh and
copied observation use that state. No chip waveform, host-audio path, second
state owner, callback, timing rule or public API was added. P1 is `1825a1859`.

The three production files change **17 additions / 17 removals**. Eight
optimized 0540 products were rebuilt. No Shared/MyNES, owner INI or external
asset changed.

## Verification

- Complete repository-only units: x64 **469/469**, x86 **469/469**, run
  sequentially without concurrent Release builds.
- Both-width `verify-current-specialized-gates`: pass. Existing speaker/PPI,
  PIT/Port-B and observation regressions remain within the complete unit run.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `9678EBA39E9838FBD3A476003B2DE583D376E580C22193EFBA8C0CD8A027B858`, `0417309593C11F7A3E8BDCEC367EC86508F908E2432822DF0E06E007E87AC79F`.
  - Default: `E34A1DB125196C56DA31484DB4FD2A75CD25C15107846D47E5F26376B5F4A991`, `2727980F3DD3585645F6E18E54AB5D0902EFE5EDC4F7A2279910DE2CDB908AA4`.
  - XT: `240B1EA4A9D2D032C8C299E82D09D77FD28E1FF8C45DBA447B151594F4788F16`, `46854C46C68EB18F132828C88103F9C02B7FD272B7358538370416C93379FD90`.
  - IBM 5170: `DB6D27BBB95B444C5110C2502BE06DB2639BBAC8AE2393B44D87A3285C7D0AE1`, `68B63F360E632C4301564812319275EC51FDB71B06816CC5BDE4CCF180958703`.

S57 closes only speaker electrical state. S58 receives absent-memory windows;
T540 remains open.
