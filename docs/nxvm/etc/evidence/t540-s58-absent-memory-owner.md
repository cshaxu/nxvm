# M5 T540 S58 Board-Owned Absent-Memory Windows

## Scope and actual diff

The four-slot configured absent-memory window array moved from flat
`core_machine` into its existing board attachment. The sole configuration
path still selects the first free slot, publishes one typed fallback memory
route with that stable slot as owner, and clears the slot if publication
fails. Read returns the configured open-bus value, write is ignored, and
query retains its original validation. The copied topology plan, route
priority, ROM alias and Core memory arbitration did not change. P1 is
`930d35bec`.

Four source/test files change **6 additions / 5 removals**. The affected
existing unit test observes rollback through the board owner; no test was
removed. Eight optimized 0540 products were rebuilt. No Shared/MyNES,
owner INI or external asset changed.

## Owner and lifetime audit

The board attachment is allocated before configuration and remains fixed
throughout the runnable machine lifetime. The route owner pointer addresses
one slot within that allocation; no copied window or pointer relocation was
added. On destroy, Core stops guest execution before the board is freed.
The later memory finalizer releases its provider table without invoking
provider callbacks or dereferencing their owners.

## Verification

- Complete repository-only units: x64 **469/469**, x86 **469/469**.
- Both-width `verify-current-specialized-gates`: pass. Existing absent-window
  rollback, ROM alias and memory-route tests remain in the complete unit run.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `D788CEE4F97CFA47F3DE2075501695FAD6A66E1E787347A5EE03ECFCEB32010A`, `3E81B0B688107E9FDAA526A03E81E52069EB63C8C248A819E61150D0DA8BCB5D`.
  - Default: `941887F73223E6CC1D0AC8A8FBA09971491FD0250E54B785BFE53E596F309233`, `4CF4F1F16DAEB75B11D1FFF60153E31B16347CC89CD1015C1A548A6C2CEF70FC`.
  - XT: `98749EB64E1F8FDDD9FB0E6105FEFF02733728873652D134A1FF414239FAF079`, `96AEFE58411363ED60ACFE06C65254CDD5BBE715B0AAD8BEC4B7C372D2A23A2F`.
  - IBM 5170: `CC2D2AC57F81D0A4F0C47987538B341A543CCA6758A65FFE4E76C2EC6CCB5EB5`, `05589E6F6D0A7D9A1AC630404C09339E212C3230BAB7AF9599722A73482D8704`.

S58 closes only absent-memory window ownership. S59 receives callback and
firmware binding; T540 remains open.
