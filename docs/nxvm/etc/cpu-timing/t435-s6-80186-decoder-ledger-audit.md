# T435 S6 P3 - 80186 Decoder-to-Ledger Audit

## Result

The 80186 base-key register is now a reproducible 279-key decoder/manual/S1/S2
partition.  This is deliberately an audit result, not a timing-implementation
pass: the current source still has no `conforming` 80186 base key.

| checked relation | result |
| --- | --- |
| S2 base key -> S1 primary/supplemental manual rule | 279/279 |
| S1 grouped manual rows -> non-empty primary-manual locator | 23/23 |
| S2 base key -> accepted representative current-decoder recipe | 279/279 |
| current lexical primary opcode -> prefix, ESC, or successful family | 247/247: 6 / 8 / 233 |
| duplicate base key | 0 |
| unclassified successful primary opcode | 0 |
| base status vector | 0 conforming; 14 wrong-value; 43 unallocated; 222 missing-test |
| all generated S2 keys have a status, source rule and route | 603/603: 102 wrong-value; 81 unallocated; 144 missing-input; 276 missing-test |

## Reproduction and interpretation

First regenerate the inventory from the current Core decoder, then run both
manifest gates from the repository root:

```text
cmake -S . -B build/t435-s6-p3 -G "MinGW Makefiles"
cmake --build build/t435-s6-p3 --target core-machine-80186-decoder-inventory-runner -- -j1
build/t435-s6-p3/core-machine-80186-decoder-inventory-runner.exe
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186TimingManifest.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186DecoderLedger.ps1
```

The first runner invokes `core_machine_cpu_instruction_lexeme_scan`; the P3
verifier consumes its 247-primary/61,530-pair matrix.  Its representative
recipe is a membership witness for one concrete encoding of each S2 base key.
It is not a claim that all ModR/M aliases are distinct timing keys: the S1/S2
ledger intentionally models manual timing forms and legal context axes, not
every byte-layout duplicate.  Prefix contexts remain generated under the S2
context contract; `LOCK` is semantic-only in the lexical scanner and is not a
standalone decoder candidate.

The only allowed P3 markers are:

```text
M5:T435:S6:I186-MANUAL-DECODER-PARTITION:OK:247:233:8:6
M5:T435:S6:I186-S1-PRIMARY-LOCATORS:PASS:23
M5:T435:S6:I186-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:279
M5:T435:S6:I186-S2-STATUS-RECONCILED:PASS:0:14:43:222
M5:T435:S6:I186-S2-CANONICAL-STATUS-RECONCILED:PASS:603:102:81:144:276
```

The final line is intentionally non-zero in every deficit category.  S6 makes
those deficits finite and attributable; S6 does not close them.  T435 S6 may
close only as an evidence/audit continuation.  T435's later 80186
implementation batch may close only after every generated base/context key has
a real retirement result and becomes `conforming` under the existing tracker.

## Coordinator acceptance

After the executor P sequence, the one-session coordinator role independently
reviewed the actual changed S1 ledger, S2 manifest/tracker, P2 inventory,
P3 verifier and audit artifacts at `f425b07e`, `049dc516`, `f91b7a96`, and
`e4657202`.  It also re-ran the inventory generator, both manifest/verifier
gates, the focused 80186 decoder/profile smoke tests, documentation governance,
and `git diff --check`.

The review accepts the S6 audit boundary because it proves the 23 S1 manual
locators, 279 base recipe witnesses, 247-primary opcode partition, and all 603
generated key dispositions.  No source or CMake runtime path changed in this
acceptance review.  The 603 nonconforming runtime dispositions remain the
explicit next 80186 implementation batch; they are not an S6 audit failure or
an S6 runtime-conformance claim.

Marker: `M5:T435:S6:I186-AUDIT-ACCEPTED:OK`.
