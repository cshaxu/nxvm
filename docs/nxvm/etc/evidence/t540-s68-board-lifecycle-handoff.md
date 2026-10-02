# M5 T540 S68 Board Lifecycle Handoff

## Actual source boundary

NXVM only, from accepted S67 `6b118fb07`. Shared six components, MyNES,
owner INIs and external asset masters remain unchanged.

Four private phase callbacks replace the four named board calls in neutral
`machine.c`: device reset, clock reset, NMI refresh and device finalization.
Their existing board implementations are unchanged. Cold reset invokes the
device phase after CPU/FPU/port/memory reset and the clock phase after timeline
reset; NMI refresh remains after CPU unmask; the sole destructor invokes board
finalization after firmware invalidation and before neutral resource release.
Processor-only reset and firmware failure behavior are unchanged.

Board composition binds all four callbacks immediately after its allocation
succeeds, before fallible clock initialization. Thus a later construction
failure still uses the sole Core destructor to release the partial board.
Neutral failures before board allocation have no phase binding to invoke.
Callbacks receive the actual machine, not the replaceable `board_owner`.
There is no new public API, phase enum, dispatch framework or second state.
The concrete board-state include is removed from neutral `machine.c`.

The existing controller-authority test now verifies the four production
bindings, original reset order, masked/unmasked NMI delivery, real machine
argument despite replaced owner context, finalization and invalid-clock
construction failure. The existing prevention gate rejects concrete board
includes/calls and requires the phase bindings before clock initialization.
These are owner-local board tests, not a new framework or test target.

The five production/test/gate paths add 133/remove eight lines: production
adds twenty/removes six; the prevention gate adds nineteen/removes one; the
existing test adds ninety-four/removes one. No original phase body is edited.

## Receiving proof

Complete repository-only units pass 469/469 on x64 and 469/469 on x86,
sequentially (415.94 and 84.78 seconds). Both-width specialized gates, the
exact 37-edge dependency inventory and documentation/diff checks pass.
Eight optimized 0540 products and their matching boot probes build successfully;
no MyNES target is built. All eight EXEs have the expected PE architecture
and zero compiler `.debug` sections. SHA-256 identities:

- Model 40 x64: `7283B65E08E1C683D518F6B446928EE4CE47CAA7BA385B1A426016F71C22F3C2`.
- Model 40 x86: `9591B45705F263653B686613C6DCE694392FD1E2F8B33131C60DF387F149F142`.
- Default x64: `3F0AEDB940BF9707DCA377432C3C8DC90B1B31D2C09AE3767935E58E675170C5`.
- Default x86: `55BF8308A338196F9C86939129F44ABB39B4276E384BC8C4E181092803BF8A38`.
- XT x64: `B7C47E10831117C9C8A7B883FF8CE8D2B6FEF9E25C8BB47632793A6CF9E9956F`.
- XT x86: `81B9AB4CF7B4D440D2CDFCBE5BE6EE67AB558F1960780BBF8759613CCB432D3A`.
- AT x64: `594B0BAE6378183A7C2DBCB9A8576D1BED37CFCBC7583C56567A0B08CE5C8BC3`.
- AT x86: `B590C94DECDC914706EA6E1154D1F94989065C9D84303DD4F2C5E404D0C4B7CE`.

All eight one-shot external rows pass, exit zero: default x64/x86 reach
`dos-prompt`; XT, AT and Model 40 x64/x86 reach `installer-running`. Each
row has one qualifying run with the matching rebuilt probe and unchanged
deployed INI; media remain external overlays. These checkpoints do not replace
full T integration or establish indefinite absence of intermittent faults.
Receiving logs are the ignored `build/s68-unit-{x64,x86}.log`,
`s68-gates-{x64,x86}.log` and `s68-<profile>-<width>-{build,boot}.log` files.
Independent neutral compilation remains S69; physical Shared relocation is
S70. T540 and IBM-PC board extraction remain open.
