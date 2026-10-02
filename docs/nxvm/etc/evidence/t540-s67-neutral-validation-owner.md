# M5 T540 S67 Neutral Validation Owner

## Scope and actual source allocation

NXVM only, from clean S66 acceptance `469485cf6`. Shared six components,
MyNES, owner INIs and external asset masters remain unchanged.

Five existing private definitions move from board `machine_plan.c` into
existing neutral `machine.c`, immediately before their construction consumer:
retirement-time contract, timing capability, external-cycle timing,
external-access wait windows and transaction contract. Each complete definition
matches its original text after newline normalization. Signatures, declarations,
enum values, validation results and callers are unchanged. No new file, wrapper,
input copy, validation framework or second implementation is introduced.

The caller sweep finds four neutral Core calls (two constructor validations and
two capability validations), one board-plan capability call and two internal
transaction-validation calls. All five definitions exist once, in Core;
their private declarations remain in `machine.h`. Board controller, clock-plan,
topology and frozen-plan validation remain with the board.

The existing controller-authority gate now requires these definitions in Core
and rejects their return to board plan. Production changes are relocation only:
65 added / 63 removed lines, including spacing; the prevention gate adds ten
lines. The three tracked production/gate paths add 75/remove 63 lines, net +12:
two spacing lines separate the relocated functions and ten lines enforce their
owner. There is no new runtime logic. This removes a measured board-plan
implementation dependency, not the remaining lifecycle handoff or the whole
Core/board coupling.

## Verification and receiving boundaries

Complete repository-only units pass **469/469** on x64 and **469/469** on x86
(278.04 and 76.06 seconds). Both-width specialized targets, the exact 37-edge
dependency inventory, documentation governance and diff checks pass.
The first x64 specialized-target build attempted to relink a unit executable
while that executable was running and failed with an output-file lock. It is
not counted as a gate pass: the receiving rerun after the unit suite finished
passes, without any source workaround. Do not relink a live test executable.

Eight one-shot external rows pass, exit zero: default x64/x86 reach
`dos-prompt`; XT, AT and Model 40 x64/x86 reach `installer-running`.
Each row has exactly one qualifying run, with the rebuilt matching probe and
unchanged deployed INI. Media remain external overlays. These checkpoints do
not replace complete T integration or prove indefinite absence of intermittent
failures. All eight builds finish successfully; no MyNES build is performed.

All eight optimized 0540 EXEs have the expected `pei-x86-64` / `pei-i386`
architecture and zero `.debug` sections. Their SHA-256 identities are:

- `nxvm_model40_0_5_0540_x64.exe`: `CAD8765FE633D8D6353CC27A2FE8E4F10998768E13150BAB9DC238ED47BA7FB8`.
- `nxvm_model40_0_5_0540_x86.exe`: `8F4999C232DF800DDA227F1D8EA0B3932A03EA879D647460BF8DE69D7022CDA6`.
- `nxvm_default_0_5_0540_x64.exe`: `7CC2ACD49A9C54ECC221AAEDE6995ED10C847D09D66DBB407FADAF4462C94FC4`.
- `nxvm_default_0_5_0540_x86.exe`: `20B5F4F9055173B7852583A8FC3CBDE1E7A8BD2E31E9C8E4AFD33DB6B56B7A68`.
- `nxvm_xt_0_5_0540_x64.exe`: `659508E2C22F6E9B8918BDB13F74548DE4FD4E9E0559BF398109D2280053B875`.
- `nxvm_xt_0_5_0540_x86.exe`: `42D90C81CABE58946DC11D61FB4DCA651EC46AD8F64724328EA6D7F7368CE2C9`.
- `nxvm_at_0_5_0540_x64.exe`: `8EBA0FE729EDA8FDD9A52EB2F09C3A62C035B2A0BF24F4BB0F4A6D80BCB69AB3`.
- `nxvm_at_0_5_0540_x86.exe`: `2E1D4B2FFCCB173733D3F97F8B608949FCD9B7A0A0AE139BFFD72C2A337FAB25`.

S68 retains the four direct reset-device, reset-clock, NMI-refresh and
finalization calls. S69 must prove independent neutral compilation; S70 owns
physical Shared relocation. T540 and the IBM-PC board extraction remain open.

## Actual P1 acceptance

Coordinator review accepts pushed P1 `5db0fc37b`: fourteen NXVM paths,
clean `git show --check`, five exact original definitions at their sole neutral
owner, and unchanged caller/declaration inventory. Source/gate delta and all
eight artifact identities match this record. HEAD equals `origin/master` and
the worktree is clean at review; no Shared/MyNES or INI path appears in P1.
S67 closes only this five-validator ledger batch; S68-S70 remain required.
