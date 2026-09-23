# T435 S7 - 80286 Decoder Reconciliation Contract

`core-machine-80286-decoder-inventory-runner` scans all 65,536 primary
opcode/next-byte candidates using `CORE_MACHINE_CPU_PROFILE_80286`. The
committed inventory records its 61,803 accepted candidates, 249 accepted
primary opcodes, each primary ModR/M acceptance mask and the separate masks
for escaped `0F 00`, `0F 01`, `0F 02`, `0F 03` and `0F 06` forms.

`tools/Verify-80286DecoderLedger.ps1` is the closure gate. It:

1. rejects a changed inventory denominator;
2. requires every S1 grouped row to carry a manual locator and requires the
   documented `XLAT` companion-manual resolution;
3. expands every manifest base template, assigns an actual opcode/ModR/M
   witness and checks that the current profile lexically accepts it;
4. uses escaped masks for every `0F` system witness, so an invalid R/M system
   form cannot re-enter the successful universe; and
5. expands all legal contexts and rejects duplicate, absent-route or
   absent-status canonical keys.

The exact required result is 286 base keys and 807 canonical keys. Base
statuses are 238 `missing-test`, 47 `missing-input`, one `unallocated`
(`I286-XLAT`), and zero `conforming` or `wrong-value`. These are current
implementation facts, not an implementation waiver.

Markers: `M5:T435:S7:I286-DECODER-LEDGER-ZERO-DIFFERENCE:PASS` and
`M5:T435:S7:I286-S2-CANONICAL-STATUS-RECONCILED:PASS`.
