# M5 T540 S54 Board-Owned Planar Parity

## Scope and actual diff

Five planar-parity fields—configuration, Port-B value, configured, latched
and NMI-signaled—moved from flat `core_machine` into the existing board
attachment. All 44 production accesses retain their original condition,
value and ordering; the direct port-assembly rollback test follows the sole
board owner. Memory-fault delivery, Port-B read/write, speaker gate and NMI
delivery still use the same callbacks and phase order. No additional state,
API, mirror or compatibility branch was added. P1 is `8712ec968`.

The four source/test files change **51 additions / 51 removals**, including
the five-field move and two direct test expressions. Eight optimized 0540
artifacts are updated. No Shared/MyNES, profile INI or external asset changed.

## Verification

- Complete repository-only units: x64 **469/469** and x86 **469/469**.
  The first x86 run, made during concurrent release builds/boots, reported a
  timeout for unrelated `machine-t359-s3-timing-smoke` after it printed `OK`.
  The unchanged test passed alone in **0.18 s**; a complete x86 run without
  concurrent builds passed **469/469**. No timeout was raised or test removed.
- Both-width `verify-current-specialized-gates`: pass. The full suites include
  planar-parity/NMI, Model-40 D4/parity and port-assembly regressions.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `AE3CC7A74238DD4E6E6CFE4267FDD30E493F32AD611DB181D65A1197B7E0E05B`, `B1EFA6684474C9E7C3B277940B315F3504FF1FBEC9341C4CCD0F9DE2E14965CD`.
  - Default: `7492AB5F5867E570B1C91ED63F240853112940FCFDDD9F7AA03A228D36BB7945`, `75AA1B985FEF57B7D206A9FFD3B6261A160DDEFB2C1A5E40DA6B4506D479D9DD`.
  - XT: `769D204EEFF2FE82BBAAD2EEBD1FCBACB203FFB89C05807C277B6E9A38EFE694`, `F62DE19B242678AF25DC50E38349FDB1C046AA7FCA7310285B1D40ACF4478E3F`.
  - IBM 5170: `5E5D9A62E7A67304C2AAB357B98FD4176D525B31CBCD198ADC199867EF49CEE1`, `D3959489DA36AEFC00AA8CEACBFCC52694B807E79738CBE6D3DE98071AF4C01F`.

S54 closes only the parity state group. S55 receives D4 platform/Port-B/NMI
state; T540 remains open.
