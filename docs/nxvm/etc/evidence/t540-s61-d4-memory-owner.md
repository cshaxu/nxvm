# M5 T540 S61 Board-Owned D4 Memory State

## Scope and actual diff

The D4 control, diagnostics, RAM-setup and parity-fault-mask value moved
from flat `core_machine` into the sole board attachment. `d4_memory.c`
continues to publish the same two atomic replacement routes and parity/
write observers through Core. Their owner remains the machine because the
parity callback invokes the existing bounded IOCHK service; each callback
now reads or updates only `machine->board->d4_memory`. This is one board
state and one Core checked-memory route, not a mirror or alternate path.
P1 is `ab3c8f437`.

Six source/test/gate files change **32 additions / 26 removals**. The
standalone transaction test now supplies its own zeroed board attachment;
its routing and rollback assertions remain unchanged. The integration
diagnostic probe reads the board value, without changing its checkpoint.
The existing D4 route verifier initially expected the retired flat path;
it was updated in place to require board storage and reject the old access.
No Shared/MyNES, owner INI or external asset changed.

## Verification

- D4/Model 40 focused units: x64 **23/23**, x86 **23/23**.
- Complete repository-only units: x64 **469/469**, x86 **469/469**.
- Both-width `verify-current-specialized-gates`: pass after the existing D4
  owner assertion was corrected. No gate was skipped.
- One external boot per fixed profile and width: Default `dos-prompt`; XT,
  IBM 5170 and Model 40 `installer-running`; **8/8** terminals pass.
- Eight optimized 0540 products have the expected four PE x64/four PE x86
  formats and zero `.debug` sections. SHA-256 by profile, x64 then x86:
  - Model 40: `9F744C8554EC72F855CF65C9940D7ABC1F3244D6526E8DBBFAADA17A12E75B9C`, `054A3C097C7745F3964800319A9E0B0C1668383C9B8D97E31B420BBF96697032`.
  - Default: `557AF2E055B9A22F6BFC3C5B57758D41B40B5322A4FFDE0435C2F263D8A72FBD`, `FD5F6A674F12966D16C99CBA9E8842C28E10E9E464AF75FBDCA21C6E4B58D787`.
  - XT: `CE1D0C4E8E00EFA1A2E54962DDF7E1F1EDAABC69D4C7546F56103E8234D59B78`, `8BA683261FB1026DFB2E0FE303C22F348E0EB1869A6126B799B314B93CD30498`.
  - IBM 5170: `A195EB51733913C6798C783BFAE618FF6E112BFDF79CD250BF64CFEA83D90BCB`, `716A7A00E8D3B660436895C281BFAC0CA8CE84A357FE471604993B28AA792807`.

S61 closes only the D4 mutable value move. S62 receives frozen-plan
board topology/type separation; T540 remains open.
