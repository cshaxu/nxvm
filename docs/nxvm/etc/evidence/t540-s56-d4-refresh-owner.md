# M5 T540 S56 Board-Owned D4 Refresh

## Scope and actual diff

The three D4 refresh electrical fields—pending HOLD request, pulse-active
edge and address—moved from flat `core_machine` into the existing board
attachment. PIT refresh output, board deadline, board request/completion,
reset and direct test/boot inspection use that sole state. Core's scheduler
still observes a copied address through `board_refresh_request_provider`,
performs its one HOLD/transaction operation, and invokes the existing
success-only completion provider. No new callback, mirror, DMA grant rule,
guest-time advancement or refresh phase was added. P1 is `71efa3bb5`.

The eight source/test files change **27 additions / 27 removals**. The
existing S26 ownership verifier initially expected the retired flat path;
it was updated in place (12 additions / 2 removals) to require board-owned
fields, reject flat Core storage and verify the new completion path. The
single verifier and both complete specialized gate sets then pass. Eight
optimized 0540 artifacts are updated. No Shared/MyNES, owner INI or external
asset changed.

## Verification

- Complete repository-only units: x64 **469/469**, x86 **469/469**, run
  sequentially without concurrent Release builds.
- Both-width `verify-current-specialized-gates`: pass after the existing
  refresh ownership verifier was updated. The full suites include D4 refresh
  HOLD, prefetch locality and time/deadline regressions.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `978DA4F8D0267205F5809FCBDBDA061EF4852F3E25C2AD2339BCCC0724794DA9`, `E4C7E187536653524CF5D154FC3133E049095B02811E1FC0837646CDB85C30EA`.
  - Default: `7D97E2B554218AD045AFE23AD4EAC23FD97E2EE8439CD16F952ED8F9B44E507C`, `DB52B89EC570FCA99FF4F26B355763C89D6361D84FCA40EC56830C1FF0302603`.
  - XT: `7A0AF74DB24902F9E3CAD1AAE145AD98A24AEF6B8E1F9A76BC276B6330B20D15`, `848E83DB610A91134BDB8E415E82CD1640110177410BAFAAEE91E3EEFDF4ADB0`.
  - IBM 5170: `6E310C2D5911E84B74689A9E08CA7DA2176A2A27B0414D2DAB5CAA3329A225AB`, `AAB1DF4CD62E7A063363BC37C2DBE14F7A2CF8CECF39D76D5E7EF92E3FBD52F6`.

S56 closes only D4 refresh electrical state. S57 receives XT speaker
gates/output; T540 remains open.
