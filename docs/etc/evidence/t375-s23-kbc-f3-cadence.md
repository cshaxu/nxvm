# T375 S23: Keyboard F3 Cadence

IBM PC/AT Technical Reference 6139362, Keyboard "Set Typematic Rate/Delay"
defines the F3 parameter: bits 6--5 select `(1 + value) * 250 ms` delay and
bits 4--0 select the repeat period `(8 + A) * 2^B * 0.00417 seconds`, where
`A` is bits 2--0 and `B` is bits 4--3.  The same manual specifies the keyboard
default as 500 ms and 10 cps, each plus or minus 20 percent.

The prior implementation exposed the F3 protocol byte but only stored it.  Its
default `20h` selected a 500-ms delay with the `00000b` 30-cps rate field, and
therefore contradicted the selected 10-cps nominal profile.  S23 corrects the
default to `2Ch`: delay field `01b` and rate field `01100b` (10 cps).

`kbc.c` retains one profile-provided nominal calibration and derives active
timings at the existing KBC owner whenever F3 changes or defaults are restored.
The default units are delay factor two and repeat factor 24.  Thus the Model-339
calibration 4,000,000/800,000 ticks produces its default values exactly;
valid F3 parameters scale both values by the IBM fields.  For example, `7Fh`
selects a fourfold delay and fivefold repeat period, producing
8,000,000/4,000,000 ticks for that profile.

The mechanism has no test-only ingress or timing API: guest F3 writes use the
normal keyboard command port and physical keyboard bytes continue through the
native input boundary.  Reset, F5, and F6 use the one defaults helper and
therefore restore `2Ch` and its derived cadence.

## Verification

`core-machine-kbc-controller-smoke` configures a local nominal calibration,
proves F3 `1Fh` and `2Ch` values, then retains default-command and existing
repeat/response checks.  `vm-model-339-clock-contract-smoke` proves Model-339
default timing, uses F3 `7Fh`, and retains its profile/clock contract.

```text
M5:T227:S3:KBC-CONTROLLER:OK
M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK
M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK
M5:T375:S22:MODEL339-TYPEMATIC:OK
M5:T375:S23:KBC-F3-CADENCE:OK
```

## Boundaries and sweep

The sweep covered default constant, F3 pending command and parameter, reset,
F5/F6 defaults, timing configuration, repeat scheduling, VM profile binding,
and both controller/profile proofs.  All production timing mutations now use
the one KBC decode helper.  IBM's plus-or-minus-20-percent physical keyboard
tolerance remains a range, not an exact oscillator or serial-wire timing
claim; command response remains the separately bounded transfer from S22.
