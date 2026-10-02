# M5 T540 S53 XT Keyboard Owner And Electrical Intake

## Actual owner correction

S51 moved the 8042 and XT PPI state but overlooked the separately allocated
`x86_xt_keyboard` chip pointer. Its only production consumers were board
construction, PPI line binding, native input, reset, advance/deadline and
destruction. S53 moves that pointer from flat `core_machine` to the existing
`core_machine_board_state`; the one chip lifetime and every call order remain
unchanged. The direct unit and boot-probe inspections follow that sole owner.
The P1 source/test change is 7 files, 11 additions and 11 removals. No new
state, API, keyboard behavior, compatibility branch or Shared/MyNES edit was
introduced. P1 is `e313e4660`.

## Measured remaining work

The initially assigned electrical/callback row is not one bounded edit. A
source inventory found roughly 210 flat-field accesses: planar parity 45,
D4 platform 55, D4 refresh 13, XT speaker/output 13, absent memory 4, board
provider callbacks 58 and board owner 22. The finite receivers are now S54
parity; S55 D4 platform/NMI; S56 D4 refresh/DMA; S57 XT speaker; S58 absent
memory; S59 callback install/revoke and firmware binding audit. S60 owns the
neutral private header, S61 the physical Shared Core move; subsequent board
receivers begin S62. Neither the electrical latches nor callback ABI are
claimed migrated in S53. The Core firmware operation guard and one frozen
plan remain distinct from board wiring; callback slots called by neutral
Core may have to stay on the neutral side after S59 audit.

## Verification

- x64 and x86 full repository-only units: **469/469** each.
- x64 and x86 `verify-current-specialized-gates`: pass.
- One external boot probe per fixed profile and width: Default reaches
  `dos-prompt`; XT, IBM 5170 and Model 40 reach `installer-running`; **8/8**.
- Eight optimized 0540 products rebuilt. Four are PE x64, four PE x86;
  none has a `.debug` section. SHA-256 by profile, x64 then x86:
  - Model 40: `C1E918A6D7D4D9AC5FF8B516A4A396F57742A2740A49CD0348F4D5905B2AF4A8`, `7DAE0EF90AC204DD6E2DDA98B363572A2760EC79DEFBA944CCEA18ABB6ACB945`.
  - Default: `E108327262F170F5F98920751CC7940C26C433FA9643541EC76EE2F6990ACEA2`, `4EA80706060A2C754B8946EF7A0B8B6142F67A4EB9DC66312DA82F2173F05750`.
  - XT: `AAC2A4A85E3D24F6FBEADD7D1A75EE76039F4204538D2BFB26CA9411085A341C`, `7830A357567456E5A808AE6D7F1D3F9A2C5D2E98B484A08D8870495225AEB31E`.
  - IBM 5170: `0FB3760D511F5E885D32215918C043A756E4C5F8CD40A19C38FEBB78B210C857`, `29A2200B7F11A867D57D477CD8DF4F9975E9F0E0C5641D1B5D19C879C66439CE`.

Owner INIs, external firmware/media, Shared and MyNES are untouched. T540
remains open for the explicitly mapped receivers.
