# T512 S6 P2: 80186 Direct-Jump Manual Correction

## Evidence

Intel *iAPX 86, 88, 186 and 188 User's Manual* (1985), order no. 210912-001,
Table 2-9, PDF page 182 / printed page 2-16, gives all three direct `JMP`
forms a scalar **14**-clock result:

- short (`EB`): 14;
- direct within segment (`E9`): 14;
- direct intersegment (`EA`): 14.

The owner-managed source asset has SHA-256
`2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF`.
The page was visually read because its OCR is not the authority.

## Sole-owner correction

The old `13` literals for `JMP_DIRECT` and `JMP_FAR_DIRECT` were a shared
ledger transcription error. Updating those two existing 80186 table entries
corrects all three representative recipes through the existing control-stack
selector: `EB`, `E9`, and `EA`. No handler, profile decision, alternate
retirement route, or new abstraction is added.

The previously committed P1 BOUND correction and this P2 correction show why
S6 is re-reading the manual rather than accepting a green inherited manifest.
