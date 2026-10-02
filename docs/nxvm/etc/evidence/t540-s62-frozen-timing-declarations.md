# M5 T540 S62 Frozen Timing Declarations

## Boundary and diff

The validated construction plan remains the sole input to
`core_machine_create_from_plan`. The live Core no longer retains a full copy
of that plan, including board topology, media registry and configuration.
Instead it retains only capability-indexed timing declarations; the sole
board attachment retains controller timing rules. The existing plan validator
proves the declaration count, unique capability identities and dispositions
before the success-only copy. The board constructor records whether the DMA
ratio was explicitly supplied *before* clock-domain normalization, retaining
the previous L1/deadline qualification semantics without keeping the whole
plan alive. No second plan or application route was added.

P1 `25a350639` changes nine source/test files and eight optimized profile
executables: **31 additions, 49 removals** overall. The dead linear
`core_machine_plan_declaration_find` lookup is removed. Three tests/diagnostic
consumers now read the sole board-owned controller timing value. No Shared,
MyNES, owner INI or external asset input changed.

## Verification

- Focused plan/clock-contract units: x64 **8/8**, x86 **8/8**.
- Complete repository-only units: x64 **469/469**, x86 **469/469**.
- Both-width `verify-current-specialized-gates`: pass (82 targets).
- One external boot per profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** pass.
- Eight optimized 0540 products: four PE x64, four PE x86, with no `.debug`
  sections. SHA-256 by profile, x64 then x86:
  - Model 40: `09753956B3AE81235BC727A71DC40FD1665DE2A1503087685C091A061AA9B7B9`, `3948A8A1C2EABFEB2DE72AAB7CDB0CE9B872E4B69073AD14BF188E888CD88A06`.
  - Default: `D3891D80047EF9806B85FFD88DF8DB6EFB2E5038A0B51C940FCC1DA9482187FD`, `5A428694035A76D8E7699F3375E40780AD1427F46181B7CA671A559F71D8FADF`.
  - XT: `71BB67EC43195F61DC0EC639413A566D659245BF130E89BD75AEB9F537718399`, `221E407DD90DFE5EA65BB6F98448FA3238BB0E3E93BB42EAF9841752D0B4E9B3`.
  - IBM 5170: `30DCBAD2E5A01D74FF736FDA188ADEC852EA8652E8CBEE7CBAB5D2EB2045D76C`, `C67FE665D711D2C4B66EC26EE1566EB000134E6DA361D53FE088745E47FFEF69`.

The plan's public type is not yet neutralized; S63/S64 receive the private
and public header/interface cuts. Physical Shared relocation remains S65.
T540 remains open.
